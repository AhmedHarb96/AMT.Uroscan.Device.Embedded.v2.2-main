/*
 * FlashManager.h
 *
 *  Created on: Aug 8, 2024
 *      Author: OrioN
 */

#ifndef INC_FLASHMANAGER_H_
#define INC_FLASHMANAGER_H_

enum FlashStatus {
	Success,
	Error,
	NotBackup
};
class FlashManager {
public:
	int serialN;
	FlashManager();
	virtual ~FlashManager();
	void BackupData(void);
	FlashStatus HardReset(bool useUnlock);
	void Setup(void);
	bool IsSetup(void);
	void WriteSendPerSecond(uint8_t count);
	uint8_t ReadSendPerSecond(void);
	void WriteFirstEmgPerSecond(uint8_t count);
	uint8_t ReadFirstEmgPerSecond(void);
	void WriteSecondEmgPerSecond(uint8_t count);
	uint8_t ReadSecondEmgPerSecond(void);
	void WriteVolumePerSecond(uint8_t count);
	uint8_t ReadVolumePerSecond(void);
	void WriteFlowPerSecond(uint8_t count);
	uint8_t ReadFlowPerSecond(void);
	void WritePumpMaxRunTime(uint8_t second);
	uint8_t ReadPumpMaxRunTime(void);
	void WriteValveMaxRunTime(uint8_t second);
	uint8_t ReadValveMaxRunTime(void);
	void WriteFlowRate(double_t rate);
	double_t ReadFlowRate(void);
	void WriteVolumeRate(double_t rate);
	double_t ReadVolumeRate(void);
	void WriteVolumeAverage(uint32_t avg);
	uint32_t ReadVolumeAverage(void);
	void WriteFlowAverage(uint32_t avg);
	uint32_t ReadFlowAverage(void);
	void WriteFirstEmgData(uint16_t maxTrim, uint16_t zeroPointMaxTrim, uint16_t zeroPointMinTrim);
	uint16_t ReadFirstEmgMaxTrim(void);
	uint16_t ReadFirstEmgZeroPointMinTrim(void);
	uint16_t ReadFirstEmgZeroPointMaxTrim(void);
	void WriteSecondEmgData(uint16_t maxTrim, uint16_t zeroPointMaxTrim, uint16_t zeroPointMinTrim);
	uint16_t ReadSecondEmgMaxTrim(void);
	uint16_t ReadSecondEmgZeroPointMinTrim(void);
	uint16_t ReadSecondEmgZeroPointMaxTrim(void);
	FlashStatus Update(void);
	FlashStatus WriteData(uint8_t appendAddress,int len, uint8_t *data);
private:
	void ReadData(uint8_t *retVal, uint8_t appendAddress,int len, bool isReverse=false){
		if(HAL_FLASH_Unlock()==HAL_OK){
			for (int i = 0; i < len; ++i) {
				uint32_t address=FlashMemAddress+appendAddress+i;
				if(isReverse){
					retVal[len-i-1]=*(__IO uint8_t*) address;
				}
				else{
					retVal[i]=*(__IO uint8_t*) address;
				}
			}
			HAL_FLASH_Lock();
		}
	}
	uint32_t FlashMemAddress=0x08020000;  //0x08020000;
	uint8_t  FlowRateAppendAddress=0;
	uint8_t  VolumeRateAppendAddress=8;
	uint8_t  FlowAverageAppendAddress=16;
	uint8_t  VolumeAverageAppendAddress=20;
	uint8_t  FirstEmgAppendAddress=24;
	uint8_t  SecondEmgAppendAddress=30;
	uint8_t  SetupAppendAddress=36;
	uint8_t  SendPerSecondAppendAddress=37;
	uint8_t  FirstEmgPerSecondAppendAddress=38;
	uint8_t  SecondEmgPerSecondAppendAddress=39;
	uint8_t  VolumePerSecondAppendAddress=40;
	uint8_t  FlowPerSecondAppendAddress=41;
	uint8_t  PumpMaxRunTimeAppendAddress=42;
	uint8_t  ValveMaxRunTimeAppendAddress=43;

};

#endif /* INC_FLASHMANAGER_H_ */
