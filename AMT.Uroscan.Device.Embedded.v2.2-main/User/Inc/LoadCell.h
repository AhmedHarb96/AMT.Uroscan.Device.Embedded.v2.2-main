/*
 * LoadCell.h
 *
 *  Created on: Aug 1, 2024
 *      Author: OrioN
 */

#ifndef INC_LOADCELL_H_
#define INC_LOADCELL_H_



typedef struct {
	float32_t VolumePhysicalValue;
	float32_t VolumeOffsetValue;
	float32_t VolumeGain;

	float32_t FlowPhysicalValue;
	float32_t FlowOffsetValue;
	float32_t FlowGain;

} LoadCellCalibrationStruct;


typedef struct {

	float32_t SumFirstVolumeMAFValue;
	float32_t FirstVolumeMAFArray[20];
	uint16_t FirstVolumeMAFLength;
	uint16_t Len=20;
	float PreviousHPFilteredValue;

} VolumeFilterStruct;



typedef struct {

	float32_t SumFirstFlowMAFValue;
	float32_t FirstFlowMAFArray[40];
	uint16_t Len=40;
	uint8_t FirstFlowMAFLength;

} FlowFilterStruct;


typedef struct {

	float32_t ConsecutiveCount = 0;
	float32_t LastValue = 0;
	bool CheckConsecutiveVolumeFlag = true;

} ConsecutiveLoadcellStruct;

extern ConsecutiveLoadcellStruct ConsecutiveLoadcell;

typedef struct {

	uint16_t VolumeToSaveData = 14587;
	uint16_t VolumeReadData = 0;

} VolumeEEPROMStruct;

extern VolumeEEPROMStruct VolumeEEPROM;

class LoadCell {
public:
	LoadCell();
	virtual ~LoadCell();
	void Setup(void);
	void ReadVolumeAndFlow(void);
	void ReadVolume(bool useBuffer);
	void ReadFlow(bool useBuffer);
	void ClearParams(void);
	void WriteToSPI(uint16_t SPIAddress, uint16_t value);
	void ReadFromSPI(uint8_t value, int type);
	float32_t CalculateRealVolumeData(float32_t volume);
	float32_t CalculateRealFlowData(float32_t flow);
	float32_t VolumeMovingAverageFilter(float32_t volume);
	float32_t FlowMovingAverageFilter(float32_t flow);
	float32_t ExponantialSmoothingFilter(float32_t alpha,float32_t value,float32_t lastValue);
	void ClearSamples(void);
	float32_t VolumeBuffer[1024];
	int VolumeBufferLen;
	float32_t FlowBuffer[1024];
	int FlowBufferLen;
	volatile float32_t FlowValue;
	volatile float32_t VolumeValue;
	volatile float32_t MaxVolumeValue;
	volatile float32_t ReadedLastFlowValue;
	volatile float32_t LastReadFlowValue;
	volatile float32_t LastReadVolumeValue;
	volatile float32_t LastFlowValue;
	volatile float32_t LastVolumeValue;
	volatile bool IsFirstHandle;
	uint32_t LastHandleProcessTime;
private:
	uint8_t VolumeAndFlowReadSPIValue;
	uint8_t VolumeReadSPIValue;
	uint8_t FlowReadSPIValue;
};

#endif /* INC_LOADCELL_H_ */
