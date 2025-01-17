/*
 * LoadCell.cpp
 *
 *  Created on: Aug 1, 2024
 *      Author: OrioN
 */

#include "../Inc/GeneralHeaders.h"

#define VOLUME_WINDOW_SIZE 11
#define FLOW_WINDOW_SIZE 5
#define LOAD_CELL_VOLUME_CALIBRE_VAL			10.28
#define LOAD_CELL_VOLUME_CALIBRE_OFFSET			1700
float32_t  VolumeSamples[VOLUME_WINDOW_SIZE] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
float32_t  FlowSamples[FLOW_WINDOW_SIZE] = {
		-1,-1,-1,-1,-1,
};
uint8_t FlowIndex=0;
uint8_t VolumeIndex=0;
volatile float32_t minFlow=MAXFLOAT;
volatile float32_t maxFlow=0;
volatile float32_t minVolume=MAXFLOAT;
volatile float32_t maxVolume=0;
float32_t volumeRefreshRange = 500;
float32_t flowRefreshRange = 50;
Filter FilterInstance;
VolumeFilterStruct VolumeFilter;
ConsecutiveLoadcellStruct ConsecutiveLoadcell;
FlowFilterStruct FlowFilter;




LoadCell::LoadCell() {
	// TODO Auto-generated constructor stub

}

LoadCell::~LoadCell() {
	// TODO Auto-generated destructor stub
}

void LoadCell::Setup(void){
	WriteToSPI(PHASE_ADDRESS, PHASE_VALUE);
	WriteToSPI(GAIN_ADDRESS, GAIN_VALUE);
	WriteToSPI(STATUS_COM_ADDRESS, STATUS_COM_VALUE);
	WriteToSPI(CONFIG_1_ADDRESS, CONFIG_1_VALUE);
	WriteToSPI(CONFIG_2_ADDRESS, CONFIG_2_VALUE);
}
void LoadCell::ReadVolumeAndFlow(void){
	VolumeValue=0;
	uint32_t readLoadCellValue[6];

	ReadFromSPI(DATA_CH0_0,LOAD_CELL_ALL);
	readLoadCellValue[0] = VolumeAndFlowReadSPIValue;
	Debugger.LoadcellSPI.Channel1=VolumeAndFlowReadSPIValue;
	ReadFromSPI(DATA_CH0_1,LOAD_CELL_ALL);
	readLoadCellValue[1] = VolumeAndFlowReadSPIValue;
	Debugger.LoadcellSPI.Channel2=VolumeAndFlowReadSPIValue;

	ReadFromSPI(DATA_CH0_2,LOAD_CELL_ALL);
	readLoadCellValue[2] = VolumeAndFlowReadSPIValue;
	Debugger.LoadcellSPI.Channel3=VolumeAndFlowReadSPIValue;

	ReadFromSPI(DATA_CH1_0,LOAD_CELL_ALL);
	readLoadCellValue[3] = VolumeAndFlowReadSPIValue;
	Debugger.LoadcellSPI.Channel4=VolumeAndFlowReadSPIValue;

	ReadFromSPI(DATA_CH1_1,LOAD_CELL_ALL);
	readLoadCellValue[4] = VolumeAndFlowReadSPIValue;
	Debugger.LoadcellSPI.Channel5=VolumeAndFlowReadSPIValue;

	ReadFromSPI(DATA_CH1_2,LOAD_CELL_ALL);
	readLoadCellValue[5] = VolumeAndFlowReadSPIValue;
	Debugger.LoadcellSPI.Channel6=VolumeAndFlowReadSPIValue;
	for (int i = 0; i < LOADCELL_SAMPLE_NUMBER; i++) {
		VolumeValue = VolumeValue
				+ readLoadCellValue[4] + (readLoadCellValue[3] << 8);
		FlowValue = readLoadCellValue[1]
				+ (readLoadCellValue[0] << 8);
		FlowValue=(65536-FlowValue);
	}
	Debugger.ReadedVolume=VolumeValue;
	float32_t readedVolume=ExponantialSmoothingFilter(0.5, VolumeValue, LastReadVolumeValue);
	LastReadVolumeValue=readedVolume;

	Debugger.ReadedFlow=FlowValue;
	float32_t readedFlow=ExponantialSmoothingFilter(0.5, FlowValue, LastReadFlowValue);
	LastReadFlowValue=readedFlow;

}
void LoadCell::ReadVolume(bool useBuffer){
	VolumeValue=0;
	uint32_t readLoadCellValue[3];

	ReadFromSPI(DATA_CH1_0,LOAD_CELL_VOLUME);
	readLoadCellValue[0] = VolumeReadSPIValue;

	ReadFromSPI(DATA_CH1_1,LOAD_CELL_VOLUME);
	readLoadCellValue[1] = VolumeReadSPIValue;

	ReadFromSPI(DATA_CH1_2,LOAD_CELL_VOLUME);
	readLoadCellValue[2] = VolumeReadSPIValue;

	for (int i = 0; i < LOADCELL_SAMPLE_NUMBER; i++) {
		VolumeValue = VolumeValue
				+ readLoadCellValue[1] + (readLoadCellValue[0] << 8);
	}
	float32_t newVal=CalculateRealVolumeData(VolumeValue);
	if(newVal<MaxVolumeValue){
		newVal=MaxVolumeValue;
	}
	MaxVolumeValue=newVal;
	Debugger.Volume=newVal;
	if(useBuffer==false) return;
	VolumeBufferLen%=1024;
	uint8_t *array;
	array = (uint8_t*)(&newVal);
	VolumeBuffer[VolumeBufferLen]=array[0];
	VolumeBuffer[VolumeBufferLen+1]=array[1];
	VolumeBuffer[VolumeBufferLen+2]=array[2];
	VolumeBuffer[VolumeBufferLen+3]=array[3];
	VolumeBufferLen+=4;
	Debugger.VolumeBufferLen=VolumeBufferLen;
}
float32_t LoadCell::CalculateRealVolumeData(float32_t volume){
	float32_t newVolume = 0;
	float32_t movingVolumeData = 0;
	Debugger.ReadedVolume=volume;
	float32_t readedVolume=ExponantialSmoothingFilter(0.5, volume, LastReadVolumeValue);
	LastReadVolumeValue=readedVolume;

	VolumeFilter.FirstVolumeMAFLength++;
	if (VolumeFilter.FirstVolumeMAFLength
			>= VolumeFilter.Len) {
		VolumeFilter.FirstVolumeMAFLength = 0;
	}
	float32_t diff=readedVolume-SystemConfig.VolumeAverage;
	if(diff<0){
		diff=0;
	}
	Debugger.DiffAverageVolumeValue=diff;

	movingVolumeData = FilterInstance.MoovingAverage(
			VolumeFilter.FirstVolumeMAFArray,
			&VolumeFilter.SumFirstVolumeMAFValue,
			VolumeFilter.FirstVolumeMAFLength,VolumeFilter.Len , diff);
	Debugger.MovingAverageVolumeValue=movingVolumeData;
	float32_t newValue=movingVolumeData/SystemConfig.VolumeRate;
	newVolume=ExponantialSmoothingFilter((newValue>10?0.5:0.85), newValue,LastVolumeValue);
	if(newValue>5){
		newVolume=ceil(newVolume);
	}
	else{
		newVolume=floor(newVolume);
	}
	if(newVolume<0){
		newVolume=0;
	}
	LastVolumeValue=newVolume;
	Debugger.LastVolume=LastVolumeValue;
	return newVolume;
}
void LoadCell::ReadFlow(bool useBuffer){
	uint32_t readLoadCellValue[6];

	ReadFromSPI(DATA_CH0_0,LOAD_CELL_FLOW);
	readLoadCellValue[0] = FlowReadSPIValue;

	ReadFromSPI(DATA_CH0_1,LOAD_CELL_FLOW);
	readLoadCellValue[1] = FlowReadSPIValue;

	ReadFromSPI(DATA_CH0_2,LOAD_CELL_FLOW);
	readLoadCellValue[2] = FlowReadSPIValue;

	ReadFromSPI(DATA_CH1_0,LOAD_CELL_FLOW);
	readLoadCellValue[3] = FlowReadSPIValue;

	ReadFromSPI(DATA_CH1_1,LOAD_CELL_FLOW);
	readLoadCellValue[4] = FlowReadSPIValue;

	ReadFromSPI(DATA_CH1_2,LOAD_CELL_FLOW);
	readLoadCellValue[5] = FlowReadSPIValue;
	FlowValue = readLoadCellValue[1]
			+ (readLoadCellValue[0] << 8);
	FlowValue=(65536-FlowValue);
	float32_t newVal=CalculateRealFlowData(FlowValue);
	if(newVal>=SystemConfig.MinimumFlowSensiblity){
		IsFirstHandle=true;
		LastHandleProcessTime=StartTimerTicks;
	}
	Debugger.Flow=newVal;
	if(useBuffer==false) return;
	FlowBufferLen%=1024;
	uint8_t *array;
	array = (uint8_t*)(&newVal);
	FlowBuffer[FlowBufferLen]=array[0];
	FlowBuffer[FlowBufferLen+1]=array[1];
	FlowBuffer[FlowBufferLen+2]=array[2];
	FlowBuffer[FlowBufferLen+3]=array[3];
	FlowBufferLen+=4;
	Debugger.FlowBufferLen=FlowBufferLen;
}
float32_t LoadCell::CalculateRealFlowData(float32_t flow){
	float32_t newFlow = 0;
	float32_t movingFlowData = 0;

	Debugger.ReadedFlow=flow;
	float32_t readedFlow=ExponantialSmoothingFilter(0.5, flow, LastReadFlowValue);
	LastReadFlowValue=readedFlow;
	FlowFilter.FirstFlowMAFLength++;
	float32_t diffFlow=readedFlow-SystemConfig.FlowAverage;
	if(diffFlow<0){
		diffFlow=0;
	}
	if (FlowFilter.FirstFlowMAFLength >= FlowFilter.Len) {
		FlowFilter.FirstFlowMAFLength = 0;
	}

	movingFlowData = FilterInstance.MoovingAverage(
			FlowFilter.FirstFlowMAFArray,
			&FlowFilter.SumFirstFlowMAFValue,
			FlowFilter.FirstFlowMAFLength,
			FlowFilter.Len,
			diffFlow);
	float32_t newValue=movingFlowData/SystemConfig.FlowRate;
	newFlow=ExponantialSmoothingFilter((newValue>4?0.5:0.85), newValue,LastFlowValue);
	if(newFlow<1.5){
		newFlow=0;
	}
	LastFlowValue=newFlow;
	Debugger.LastFlow=LastFlowValue;
	return newFlow;
}
void LoadCell::ReadFromSPI(uint8_t value, int type){
	HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(LOAD_CELL_SPI_CHANNEL, &value, 1, 100);  //, 100
	switch(type){
		case LOAD_CELL_ALL:
			HAL_SPI_Receive(LOAD_CELL_SPI_CHANNEL, &VolumeAndFlowReadSPIValue,1, 100);  //, 100
		break;
		case LOAD_CELL_VOLUME:
			HAL_SPI_Receive(LOAD_CELL_SPI_CHANNEL, &VolumeReadSPIValue,1, 100);  //, 100
		break;
		case LOAD_CELL_FLOW:
			HAL_SPI_Receive(LOAD_CELL_SPI_CHANNEL, &FlowReadSPIValue,1, 100);  //, 100
		break;
		default:
			break;
	}
	HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_SET);
}
void LoadCell::ClearParams(void){
	LastHandleProcessTime=UINT32_MAX;
	IsFirstHandle=false;
	VolumeValue=0;
	FlowValue=0;
	LastVolumeValue=0;
	LastFlowValue=0;
	LastReadVolumeValue=0;
	LastReadFlowValue=0;
	for (int i = 0; i < VOLUME_WINDOW_SIZE; ++i) {
		VolumeSamples[i]=0;
	}
	for (int i = 0; i < FLOW_WINDOW_SIZE; ++i) {
		FlowSamples[i]=0;
	}
	FlowIndex=0;
	VolumeIndex=0;
}

void LoadCell::WriteToSPI(uint16_t SPIAddress, uint16_t value){

	uint8_t data[2];
	data[0] = SPIAddress;
	data[1] = value;

	HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(LOAD_CELL_SPI_CHANNEL, data, 2, 100);
	HAL_GPIO_WritePin(SPI3_CS_GPIO_Port, SPI3_CS_Pin, GPIO_PIN_SET);
}
float32_t LoadCell::VolumeMovingAverageFilter(float32_t volume){
	VolumeSamples[VolumeIndex%VOLUME_WINDOW_SIZE]=volume;
	VolumeIndex++;
	VolumeIndex%=VOLUME_WINDOW_SIZE;
	float32_t sum=0;
	float32_t cnt=0;
	for (int i = 0; i < VOLUME_WINDOW_SIZE; ++i) {
		float32_t val=VolumeSamples[i];
		if(val>=0){
			cnt++;
			sum+=val;
		}
	}
    return sum / cnt;
}
float32_t LoadCell::FlowMovingAverageFilter(float32_t flow){
	FlowSamples[FlowIndex%FLOW_WINDOW_SIZE]=flow;
	FlowIndex++;
	FlowIndex%=FLOW_WINDOW_SIZE;
	float32_t sum=0;
	float32_t cnt=0;
	for (int i = 0; i < FLOW_WINDOW_SIZE; ++i) {
		float32_t val=FlowSamples[i];
		if(val>=0){
			cnt++;
			sum+=val;
		}
	}
    return (sum==0)?0:sum / cnt;
}

float32_t LoadCell::ExponantialSmoothingFilter(float32_t alpha,float32_t value,float32_t lastValue){
    float forecast = alpha * value + (1 - alpha) * lastValue;
    return forecast;
}
void LoadCell::ClearSamples(){
	for (int i = 0; i < FLOW_WINDOW_SIZE; ++i) {
		FlowSamples[i]=-1;
	}
	for (int i = 0; i < VOLUME_WINDOW_SIZE; ++i) {
		VolumeSamples[i]=-1;
	}
	VolumeIndex=0;
	FlowIndex=0;

}
