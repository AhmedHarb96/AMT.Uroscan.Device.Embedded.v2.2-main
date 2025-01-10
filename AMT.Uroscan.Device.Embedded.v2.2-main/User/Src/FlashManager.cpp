/*
 * FlashManager.cpp
 *
 *  Created on: Aug 8, 2024
 *      Author: OrioN
 */

#include "../Inc/GeneralHeaders.h"

FlashManager::FlashManager() {
	// TODO Auto-generated constructor stub
	serialN = rand() % 1000;

}

FlashManager::~FlashManager() {
	// TODO Auto-generated destructor stub
}
void FlashManager::BackupData(void){
	ReadData(SystemConfig.Backup,FlashMemAddress,SystemConfig.BackupLen);
	SystemConfig.isBackup=true;
}
FlashStatus FlashManager::HardReset(bool useUnlock){

	if(!useUnlock){
		FLASH_EraseInitTypeDef	 FlashErase;
		uint32_t status = 0;
		FlashErase.Sector = FLASH_SECTOR_5;
		FlashErase.Banks=FLASH_BANK_1;
		FlashErase.NbSectors = 1;
		FlashErase.TypeErase    = FLASH_TYPEERASE_SECTORS;
		FlashErase.VoltageRange = VOLTAGE_RANGE_3;
		HAL_FLASHEx_Erase(&FlashErase, &status);
		return FlashStatus::Success;
	}
	if(useUnlock&&HAL_FLASH_Unlock()==HAL_OK){

		FLASH_EraseInitTypeDef	 FlashErase;
		uint32_t status = 0;
		FlashErase.Sector = FLASH_SECTOR_5;
		FlashErase.Banks=FLASH_BANK_1;
		FlashErase.NbSectors = 1;
		FlashErase.TypeErase    = FLASH_TYPEERASE_SECTORS;
		FlashErase.VoltageRange = VOLTAGE_RANGE_3;
		HAL_FLASHEx_Erase(&FlashErase, &status);
		HAL_FLASH_Lock();
		return FlashStatus::Success;
	}
	return FlashStatus::Error;

}
FlashStatus FlashManager::Update(void){
	if(!SystemConfig.isBackup){
		return FlashStatus::NotBackup;
	}
	if(HAL_FLASH_Unlock()==HAL_OK){
		HardReset(false);
		for (int i = 0; i < SystemConfig.BackupLen; ++i) {
			uint32_t address=FlashMemAddress+i;
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address,SystemConfig.Backup[i] );
		}
		HAL_FLASH_Lock();
		return FlashStatus::Success;
	}
	return FlashStatus::Error;
}

FlashStatus FlashManager::WriteData(uint8_t appendAddress,int len, uint8_t *data){
	if(!SystemConfig.isBackup){
		return FlashStatus::NotBackup;
	}
	for (int i = 0; i < len; ++i) {
		uint32_t address=appendAddress+i;
		SystemConfig.Backup[address]=data[i];
	}
	return FlashStatus::Success;
}
void FlashManager::Setup(){
	size_t size=1;
    uint8_t data[size]={0};
	WriteData(SetupAppendAddress,size, data);
}
bool FlashManager::IsSetup(){
	BackupData();
	size_t size=1;
	uint8_t data[size]={0xFF};
	ReadData(data,SetupAppendAddress,size, false);
	uint8_t value;
    memcpy(&value, data, size);
    return (value==0x00);
}
void FlashManager::WriteSendPerSecond(uint8_t count){
	size_t size=1;
    uint8_t data[size]={count};
	WriteData(SendPerSecondAppendAddress,size, data);
}
uint8_t FlashManager::ReadSendPerSecond(){
	size_t size=1;
	uint8_t data[size]={0XFF};
	ReadData(data,SendPerSecondAppendAddress,size, false);
	uint8_t value;
    memcpy(&value, data, size);
    return value;
}
void FlashManager::WriteFirstEmgPerSecond(uint8_t count){
	size_t size=1;
    uint8_t data[size]={count};
	WriteData(FirstEmgPerSecondAppendAddress,size, data);
}
uint8_t FlashManager::ReadFirstEmgPerSecond(){
	size_t size=1;
	uint8_t data[size]={0xFF};
	ReadData(data,FirstEmgPerSecondAppendAddress,size, false);
	uint8_t value;
    memcpy(&value, data, size);
    return value;
}
void FlashManager::WriteSecondEmgPerSecond(uint8_t count){
	size_t size=1;
    uint8_t data[size]={count};
	WriteData(SecondEmgPerSecondAppendAddress,size, data);
}
uint8_t FlashManager::ReadSecondEmgPerSecond(){
	size_t size=1;
	uint8_t data[size]={0xFF};
	ReadData(data,SecondEmgPerSecondAppendAddress,size, false);
	uint8_t value;
    memcpy(&value, data, size);
    return value;
}
void FlashManager::WriteVolumePerSecond(uint8_t count){
	size_t size=1;
    uint8_t data[size]={count};
	WriteData(VolumePerSecondAppendAddress,size, data);
}
uint8_t FlashManager::ReadVolumePerSecond(){
	size_t size=1;
	uint8_t data[size]={0xFF};
	ReadData(data,VolumePerSecondAppendAddress,size, false);
	uint8_t value;
    memcpy(&value, data, size);
    return value;
}
void FlashManager::WriteFlowPerSecond(uint8_t count){
	size_t size=1;
    uint8_t data[size]={count};
	WriteData(FlowPerSecondAppendAddress,size, data);
}
uint8_t FlashManager::ReadFlowPerSecond(){
	size_t size=1;
	uint8_t data[size]={0xFF};
	ReadData(data,FlowPerSecondAppendAddress,size, false);
	uint8_t value;
    memcpy(&value, data, size);
    return value;
}
void FlashManager::WritePumpMaxRunTime(uint8_t second){
	size_t size=1;
    uint8_t data[size]={second};
	WriteData(PumpMaxRunTimeAppendAddress,size, data);
}
uint8_t FlashManager::ReadPumpMaxRunTime(){
	size_t size=1;
	uint8_t data[size]={0xFF};
	ReadData(data,PumpMaxRunTimeAppendAddress,size, false);
	uint8_t value;
    memcpy(&value, data, size);
    return value;
}
void FlashManager::WriteValveMaxRunTime(uint8_t second){
	size_t size=1;
    uint8_t data[size]={second};
	WriteData(ValveMaxRunTimeAppendAddress,size, data);
}
uint8_t FlashManager::ReadValveMaxRunTime(){
	size_t size=1;
	uint8_t data[size]={0xFF};
	ReadData(data,ValveMaxRunTimeAppendAddress,size, false);
	uint8_t value;
    memcpy(&value, data, size);
    return value;
}
void FlashManager::WriteFlowRate(double_t rate){
	size_t size=8;
    uint8_t data[size];
    memcpy(data, &rate, size);
	WriteData(FlowRateAppendAddress,size, data);
}
double_t FlashManager::ReadFlowRate(){
	size_t size=8;
	uint8_t data[size]={0};
	ReadData(data,FlowRateAppendAddress,size, false);
    double_t value;
    memcpy(&value, data, size);
    return value;
}

void FlashManager::WriteVolumeRate(double_t rate){
	size_t size=8;
    uint8_t data[size];
    memcpy(data, &rate, size);
	WriteData(VolumeRateAppendAddress, size, data);
}
double_t FlashManager::ReadVolumeRate(){
	size_t size=8;
	uint8_t data[size]={0};
	ReadData(data,VolumeRateAppendAddress, size, false);
    double_t value;
    memcpy(&value, data, size);
    return value;
}

void FlashManager::WriteVolumeAverage(uint32_t avg){
	size_t size=4;
    uint8_t data[size];
    memcpy(data, &avg, size);
	WriteData(VolumeAverageAppendAddress, size, data);
}
uint32_t FlashManager::ReadVolumeAverage(){
	size_t size=4;
	uint8_t data[size]={0};
	ReadData(data,VolumeAverageAppendAddress,size, false);
	uint32_t value;
    memcpy(&value, data, size);
    return value;
}
void FlashManager::WriteFlowAverage(uint32_t avg){
	size_t size=4;
    uint8_t data[size];
    memcpy(data, &avg, size);
	WriteData(FlowAverageAppendAddress, size, data);
}
uint32_t FlashManager::ReadFlowAverage(){
	size_t size=4;
	uint8_t data[size]={0};
	ReadData(data,FlowAverageAppendAddress,size, false);
	uint32_t value;
    memcpy(&value, data, size);
    return value;
}
void FlashManager::WriteFirstEmgData(uint16_t maxTrim, uint16_t zeroPointMaxTrim, uint16_t zeroPointMinTrim){
	size_t size=2;
    uint8_t maxTrimData[size];
    uint8_t zeroPointMaxTrimData[size];
    uint8_t zeroPointMinTrimData[size];
    memcpy(maxTrimData, &maxTrim, size);
    memcpy(zeroPointMaxTrimData, &zeroPointMaxTrim, size);
    memcpy(zeroPointMinTrimData, &zeroPointMinTrim, size);
	WriteData(FirstEmgAppendAddress, size, maxTrimData);
	WriteData(FirstEmgAppendAddress+2, size, zeroPointMaxTrimData);
	WriteData(FirstEmgAppendAddress+4, size, zeroPointMinTrimData);
}
uint16_t FlashManager::ReadFirstEmgMaxTrim(){
	size_t size=2;
	uint8_t data[size]={0};
	ReadData(data,FirstEmgAppendAddress,size, false);
	uint32_t value;
    memcpy(&value, data, size);
    return value;
}
uint16_t FlashManager::ReadFirstEmgZeroPointMaxTrim(){
	size_t size=2;
	uint8_t data[size]={0};
	ReadData(data,FirstEmgAppendAddress+2,size, false);
	uint32_t value;
    memcpy(&value, data, size);
    return value;
}
uint16_t FlashManager::ReadFirstEmgZeroPointMinTrim(){
	size_t size=2;
	uint8_t data[size]={0};
	ReadData(data,FirstEmgAppendAddress+4,size, false);
	uint32_t value;
    memcpy(&value, data, size);
    return value;
}

void FlashManager::WriteSecondEmgData(uint16_t maxTrim, uint16_t zeroPointMaxTrim, uint16_t zeroPointMinTrim){
	size_t size=2;
    uint8_t maxTrimData[size];
    uint8_t zeroPointMaxTrimData[size];
    uint8_t zeroPointMinTrimData[size];
    memcpy(maxTrimData, &maxTrim, size);
    memcpy(zeroPointMaxTrimData, &zeroPointMaxTrim, size);
    memcpy(zeroPointMinTrimData, &zeroPointMinTrim, size);
	WriteData(SecondEmgAppendAddress, size, maxTrimData);
	WriteData(SecondEmgAppendAddress+2, size, zeroPointMaxTrimData);
	WriteData(SecondEmgAppendAddress+4, size, zeroPointMinTrimData);
}
uint16_t FlashManager::ReadSecondEmgMaxTrim(){
	size_t size=2;
	uint8_t data[size]={0};
	ReadData(data,SecondEmgAppendAddress,size, false);
	uint32_t value;
    memcpy(&value, data, size);
    return value;
}
uint16_t FlashManager::ReadSecondEmgZeroPointMaxTrim(){
	size_t size=2;
	uint8_t data[size]={0};
	ReadData(data,SecondEmgAppendAddress+2,size, false);
	uint32_t value;
    memcpy(&value, data, size);
    return value;
}
uint16_t FlashManager::ReadSecondEmgZeroPointMinTrim(){
	size_t size=2;
	uint8_t data[size]={0};
	ReadData(data,SecondEmgAppendAddress+4,size, false);
	uint32_t value;
    memcpy(&value, data, size);
    return value;
}
