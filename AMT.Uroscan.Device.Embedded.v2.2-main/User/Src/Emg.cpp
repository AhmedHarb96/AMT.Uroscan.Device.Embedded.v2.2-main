/*
 * Emg.cpp
 *
 *  Created on: Aug 1, 2024
 *      Author: OrioN
 */

#include "../Inc/GeneralHeaders.h"

uint32_t firstEmgDebug;
uint32_t secondEmgDebug;
uint32_t newVal;
int firstEmgDataLen=0;
int secondEmgDataLen=0;



Emg::Emg() {
	// TODO Auto-generated constructor stub

}

Emg::~Emg() {
	// TODO Auto-generated destructor stub
}
void Emg::FirstEmgRead(bool useBuffer){

	FirstEmgReadFromSPI(DATA_CH0_0);
	ReadFirstEmgValueArray[0] = FirstEmgReadSPIValue[0];

	FirstEmgReadFromSPI(DATA_CH0_1);
	ReadFirstEmgValueArray[1] = FirstEmgReadSPIValue[0];

	FirstEmgReadFromSPI(DATA_CH0_2);
	ReadFirstEmgValueArray[2] = FirstEmgReadSPIValue[0];

	FirstEmgValue = (ReadFirstEmgValueArray[1] + (ReadFirstEmgValueArray[0] << 8));
	NonFilterFirstEmg=FirstEmgValue;

	uint32_t filteredValue = FirstEmgMovingAverageFilter(FirstEmgValue);
	Debugger.FirstEmg=filteredValue;
	FirstEmgValue=filteredValue;
	if(useBuffer){
		FirstEmgBufferLen%=1024;
	    FirstEmgBuffer[FirstEmgBufferLen]=(filteredValue >> 8) & 0xFF;
	    FirstEmgBuffer[FirstEmgBufferLen+1]=(filteredValue) & 0xFF;
	    FirstEmgBufferLen+=2;
	    Debugger.FirstEmgBufferLen=FirstEmgBufferLen;
	}
}

void Emg::SecondEmgRead(bool useBuffer){
	SecondEmgReadFromSPI(DATA_CH0_0);
	ReadSecondEmgValueArray[0] = SecondEmgReadSPIValue[0];

	SecondEmgReadFromSPI(DATA_CH0_1);
	ReadSecondEmgValueArray[1] = SecondEmgReadSPIValue[0];

	SecondEmgReadFromSPI(DATA_CH0_2);
	ReadSecondEmgValueArray[2] = SecondEmgReadSPIValue[0];

	SecondEmgValue = (ReadSecondEmgValueArray[1] + (ReadSecondEmgValueArray[0] << 8));
	NonFilterSecondEmg=SecondEmgValue;
	uint32_t filteredValue = SecondEmgMovingAverageFilter(SecondEmgValue);
	Debugger.SecondEmg=filteredValue;
	SecondEmgValue=filteredValue;
	if(useBuffer){
		SecondEmgBufferLen%=1024;
		SecondEmgBuffer[SecondEmgBufferLen]=(filteredValue >> 8) & 0xFF;
	    SecondEmgBuffer[SecondEmgBufferLen+1]=(filteredValue) & 0xFF;
	    SecondEmgBufferLen+=2;
	    Debugger.SecondEmgBufferLen=SecondEmgBufferLen;
	}
}

void Emg::FirstEmgSetup(void){
	FirstEmgWriteToSPI(EMG_PHASE_ADDRESS, EMG_PHASE_VALUE);
	FirstEmgWriteToSPI(EMG_GAIN_ADDRESS, EMG_GAIN_VALUE);
	FirstEmgWriteToSPI(EMG_STATUS_COM_ADDRESS, EMG_STATUS_COM_VALUE);
	FirstEmgWriteToSPI(EMG_CONFIG_1_ADDRESS, EMG_CONFIG_1_VALUE);
	FirstEmgWriteToSPI(EMG_CONFIG_2_ADDRESS, EMG_CONFIG_2_VALUE);
}

void Emg::SecondEmgSetup(void){

	SecondEmgWriteToSPI(EMG_PHASE_ADDRESS, EMG_PHASE_VALUE);
	SecondEmgWriteToSPI(EMG_GAIN_ADDRESS, EMG_GAIN_VALUE);
	SecondEmgWriteToSPI(EMG_STATUS_COM_ADDRESS, EMG_STATUS_COM_VALUE);
	SecondEmgWriteToSPI(EMG_CONFIG_1_ADDRESS, EMG_CONFIG_1_VALUE);
	SecondEmgWriteToSPI(EMG_CONFIG_2_ADDRESS, EMG_CONFIG_2_VALUE);
}

void Emg::FirstEmgWriteToSPI(uint16_t emgSPIAddress, uint16_t emgValue){
	uint8_t EmgWriteArray[2];
	EmgWriteArray[0] = emgSPIAddress;
	EmgWriteArray[1] = emgValue;
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
	Debugger.FirstEmgWriteStatus=HAL_SPI_Transmit(EMG_SPI_CHANNEL, EmgWriteArray, 2, 100);
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
}

void Emg::SecondEmgWriteToSPI(uint16_t emgSPIAddress, uint16_t emgValue){
	uint8_t EmgWriteArray[2];
	EmgWriteArray[0] = emgSPIAddress;
	EmgWriteArray[1] = emgValue;
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
	Debugger.SecondEmgWriteStatus=HAL_SPI_Transmit(EMG2_SPI_CHANNEL, EmgWriteArray, 2, 100);
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
}

void Emg::FirstEmgReadFromSPI(uint8_t emgValue){
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);
	Debugger.FirstEmgWriteStatus=HAL_SPI_Transmit_DMA(EMG_SPI_CHANNEL, &emgValue, 1);
	Debugger.FirstEmgReadStatus=HAL_SPI_Receive_DMA(EMG_SPI_CHANNEL, FirstEmgReadSPIValue, 1);
	HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET);
}

void Emg::SecondEmgReadFromSPI(uint8_t emgValue){
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_RESET);
	Debugger.SecondEmgWriteStatus=HAL_SPI_Transmit_DMA(EMG2_SPI_CHANNEL, &emgValue, 1);
	Debugger.SecondEmgReadStatus=HAL_SPI_Receive_DMA(EMG2_SPI_CHANNEL, SecondEmgReadSPIValue, 1);
	HAL_GPIO_WritePin(SPI2_CS_GPIO_Port, SPI2_CS_Pin, GPIO_PIN_SET);
}

uint32_t Emg::FirstEmgMovingAverageFilter(uint32_t newSample){
	if(SystemConfig.FirstEmgZeroMaxTrim==0) return newSample;

	newVal=(newSample-65536)*-1;
	Debugger.FirstEmgNewVal=newVal;
	uint16_t avgTrim=(SystemConfig.FirstEmgZeroMaxTrim+SystemConfig.FirstEmgZeroMinTrim)/2;
	if(newVal<SystemConfig.FirstEmgZeroMaxTrim && newVal>SystemConfig.FirstEmgZeroMinTrim){
		newVal=avgTrim;
	}
	if(newVal<avgTrim){
		newVal=avgTrim+(avgTrim-newVal);
	}
	newVal-=avgTrim;
	Debugger.FirstEmgNewValAfter=newVal;
	if(newVal>SystemConfig.FirstEmgMaxTrim){
		return LastFirstEmg;
	}
	newVal=(LastFirstEmg*90+newVal*10)/100;  //newVal=(LastFirstEmg*990+newVal*10)/1000;
	LastFirstEmg=newVal;
	newVal=newVal-50; //250
	if(newVal>=66000) newVal=0;
	return newVal;
}
uint32_t Emg::SecondEmgMovingAverageFilter(uint32_t newSample){
	if(SystemConfig.SecondEmgZeroMaxTrim==0) return newSample;
	uint32_t newVal=(newSample-65536)*-1;
	Debugger.SecondEmgNewVal=newVal;
	uint16_t avgTrim=(SystemConfig.SecondEmgZeroMaxTrim+SystemConfig.SecondEmgZeroMinTrim)/2;
	if(newVal<SystemConfig.SecondEmgZeroMaxTrim && newVal>SystemConfig.SecondEmgZeroMinTrim){
		newVal=avgTrim;
	}
	if(newVal<avgTrim){
		newVal=avgTrim+(avgTrim-newVal);
	}
	newVal-=avgTrim;
	Debugger.SecondEmgNewValAfter=newVal;
	if(newVal>SystemConfig.SecondEmgMaxTrim){
		return LastSecondEmg;
	}
	newVal=(LastSecondEmg*90+newVal*10)/100;
	LastSecondEmg=newVal;
	newVal=newVal-50; //250
	if(newVal>=66000) newVal=0;
	return newVal;
}
