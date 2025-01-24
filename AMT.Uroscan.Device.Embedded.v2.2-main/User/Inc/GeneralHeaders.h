/*
 * GeneralHeaders.h
 *
 *  Created on: Aug 1, 2024
 *      Author: OrioN
 */

#ifndef INC_GENERALHEADERS_H_
#define INC_GENERALHEADERS_H_

#define ARM_MATH_CM4

#include "cmsis_os.h"
#include "stm32f4xx.h"
#include "arm_math.h"
#include "Errors.h"
#include "Filter.h"
#include "FlashManager.h"
#include "Emg.h"
#include "LoadCell.h"
#include "Communication.h"
#include "SystemAdapter.h"
#include "string.h"
#include "stdio.h"



extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;

extern TIM_HandleTypeDef htim11;
extern UART_HandleTypeDef huart1;

////////////////////  EMG MAKRO ////////////////////
#define EMG_SPI_CHANNEL							    &hspi1
#define EMG2_SPI_CHANNEL							&hspi2

#define EMG_PHASE_ADDRESS							0x0E
#define EMG_GAIN_ADDRESS							0x10
#define EMG_STATUS_COM_ADDRESS						0x12
#define EMG_CONFIG_1_ADDRESS						0x14
#define EMG_CONFIG_2_ADDRESS						0x16

#define EMG_PHASE_VALUE								0x00
#define EMG_GAIN_VALUE								0x00    //0x08
#define EMG_STATUS_COM_VALUE						0x03    //0x03  //0x60	//0xB3//0xA3   //0x03
#define EMG_CONFIG_1_VALUE							0x10    //0x10	//50 //0x3C  //0xC0
#define EMG_CONFIG_2_VALUE							0x0C    //0x0C

//////////////////// LOAD CELL ////////////////////
#define LOAD_CELL_SPI_CHANNEL					&hspi3

#define PHASE_ADDRESS							0x0E
#define GAIN_ADDRESS							0x10
#define STATUS_COM_ADDRESS						0x12
#define CONFIG_1_ADDRESS						0x14
#define CONFIG_2_ADDRESS						0x16

#define PHASE_VALUE								0x00
#define GAIN_VALUE								0x00
#define STATUS_COM_VALUE						0xA3    //0x03  //0x60	//0xB3//0xA3   //0x03
#define CONFIG_1_VALUE							0xF0	//0x30//0x10 //0xC0 //0x3C
#define CONFIG_2_VALUE							0x0C    //0x0C

#define DATA_CH0_0								0x01
#define DATA_CH0_1								0x03
#define DATA_CH0_2								0x05
#define DATA_CH1_0								0x07
#define DATA_CH1_1								0x09
#define DATA_CH1_2								0x0B

#define LOADCELL_SAMPLE_NUMBER					40//40
#define LOAD_CELL_VOLUME_CALIBRE_VAL			10.28
#define LOAD_CELL_VOLUME_CALIBRE_OFFSET			1700

#define LOAD_CELL_FLOW_CALIBRE_VAL				15
#define LOAD_CELL_FLOW_CALIBRE_OFFSET			110



//////////////////// LOAD CELL SPI ENUMS  ////////////////////
#define LOAD_CELL_ALL							1
#define LOAD_CELL_VOLUME						2
#define LOAD_CELL_FLOW							3


//*************** Type ******************//
#define X_SYSTEM						1
#define X_COMMAND						2
#define X_CONFIGURATION					3
//*************** Function **************//
#define X_STATUS						1     // SYSTEM FNs
#define X_RESTART						2
#define X_HARD_RESET					3
#define X_MEASUREMENT_AVERAGE			4
#define X_CALIBRATION_VOLUME			5
#define X_CALIBRATION_FLOW				6

#define X_FIRST_EMG						1     // COMMAND FNs
#define X_SECOND_EMG					2
#define X_LOAD_CELL						3
#define X_PUMP							4
#define X_VALVE							5
#define X_DATA_STREAM					6


#define X_SET_SEND_PER_SECOND			0x01
#define X_SET_FIRST_EMG_PER_SECOND		0x02
#define X_SET_SECOND_EMG_PER_SECOND		0x03
#define X_SET_VOLUME_PER_SECOND			0x04
#define X_SET_FLOW_PER_SECOND			0x05
#define X_SET_PUMP_MAX_RUN_TIME			0x06
#define X_SET_VALVE_MAX_RUN_TIME		0x07
#define X_SET_CALIBRATION_FIRST_EMG		0x08
#define X_SET_CALIBRATION_SECOND_EMG	0x09
#define X_SUBMIT						0x10






/////////////////// Uroscan dynamic Params/////////

enum RequestType{
	R_Unknown=0x00,
	R_System=0x01,
	R_Command=0x02,
	R_Configuration=0x03
};
enum SystemRequestType{
	SYSR_Unknown=0x00,
	SYSR_Status=0x01,
	SYSR_Restart=0x02,
	SYSR_FactoryReset=0x03,
	SYSR_MeasurementAverage=0x04,
	SYSR_CalibrationVolume=0x05,
	SYSR_CalibrationFlow=0x06
};
enum CommandRequestType{
	CMDR_Unknown=0x00,
	CMDR_FirstEmg=0x01,
	CMDR_SecondEmg=0x02,
	CMDR_LoadcellMeasure=0x03,
	CMDR_Pump=0x04,
	CMDR_Valve=0x05,
	CMDR_Clean=0x06,
	CMDR_Safe=0x07,
	CMDR_DataStream=0x08,
	CMDR_ReadVolume=0x09,
	CMDR_ReadFlow=0x0A,
	CMDR_ReadFirstEmg=0x0B,
	CMDR_ReadSecondEmg=0x0C,
	CMDR_StopTest=0xFD,
	CMDR_StartTest=0xFF
};
enum ConfigurationRequestType{
	CFGR_Unknown=0x00,
	CFGR_ReadConfiguration=0x01,
	CFGR_SetSendPerSecond=0x02,
	CFGR_SetFirstEmgPerSecond=0x03,
	CFGR_SetSecondEmgPerSecond=0x04,
	CFGR_SetVolumePerSecond=0x05,
	CFGR_SetFlowPerSecond=0x06,
	CFGR_SetPumpMaxRunTime=0x07,
	CFGR_SetValveMaxRunTime=0x08,
	CFGR_SetCalibrationFirstEmg=0x09,
	CFGR_SetCalibrationSecondEmg=0x0A,
	CFGR_SaveFlash=0x0B
};

enum SystemModes{
	EmptyMode=0x00,
	TestMode=0x01,
	CalibrationMode=0x02,
	CleanMode=0x03,
	SafeMode=0x04,
	ManuelMode=0x05,
	FlashMode=0x06


};
enum SuccessDataType{
	SD_Configuration=0x01,
	SD_Status=0x02,
	SD_CleanMode=0x03,
	SD_Volume=0x04,
	SD_Flow=0x05,
	SD_MeasurementAverage=0x06,
	SD_SafeMode=0x07,
	SD_VolumeCalibration=0x08,
	SD_FlowCalibration=0x09,
	SD_FirstEmg=0x0A,
	SD_SecondEmg=0x0B,
	SD_ProcessCommand=0xFD,
	SD_RepeatAgain=0xFE,
	SD_Start=0xFF,

};
typedef struct {
	osThreadId_t SendUARTThreadId;
	osThreadId_t ReadUARTThreadId;
	osThreadId_t FirstEmgThreadId;
	osThreadId_t SecondEmgThreadId;
	osThreadAttr_t FirstEmgThreadAttr;
	osThreadAttr_t SecondEmgThreadAttr;
	osThreadId_t VolumeThreadId;
	osThreadId_t FlowThreadId;
	osThreadId_t ReadVolumeThreadId;
	osThreadId_t ReadFlowThreadId;
	osThreadId_t ReadFirstEmgThreadId;
	osThreadId_t ReadSecondEmgThreadId;
	osThreadId_t PumpMaxRunThreadId;
	osThreadId_t ValveMaxRunThreadId;
	osThreadId_t LoadcellAverageThreadId;
	osThreadId_t CalibrationVolumeThreadId;
	osThreadId_t CalibrationFlowThreadId;
	osThreadId_t CleanThreadId;
	osThreadId_t SafeModeThreadId;
	osThreadAttr_t VolumeThreadAttr;
	osThreadAttr_t FlowThreadAttr;
	osThreadAttr_t PumpMaxRunThreadAttr;
	osThreadAttr_t ValveMaxRunThreadAttr;
	osThreadAttr_t LoadcellAverageThreadAttr;
	osThreadAttr_t CalibrationVolumeThreadAttr;
	osThreadAttr_t CalibrationFlowThreadAttr;
	osThreadAttr_t CleanThreadAttr;
	osThreadAttr_t SafeModeThreadAttr;
	osSemaphoreId_t FirstEmgSemaphoreHandle;
	osSemaphoreId_t SecondEmgSemaphoreHandle;
	osSemaphoreId_t FlowSemaphoreHandle;
	osSemaphoreId_t VolumeSemaphoreHandle;
	osSemaphoreId_t CommunicationSemaphoreHandle;
	osSemaphoreId_t LoadcellSPISemaphoreHandle;
} ThreadStorageStruct;

extern ThreadStorageStruct ThreadStorage;

typedef struct {
	uint8_t SendPerSecond=20;
	uint8_t FirstEmgPerSecond=40;
	uint8_t SecondEmgPerSecond=40;
	uint8_t VolumePerSecond=10;
	uint8_t FlowPerSecond=40;
	uint8_t PumpMaxRunSecond=30;
	uint8_t ValveMaxRunSecond=30;
	uint16_t FirstEmgMaxTrim=7500;
	uint16_t FirstEmgZeroMaxTrim=12500;
	uint16_t FirstEmgZeroMinTrim=11000;
	uint16_t SecondEmgMaxTrim=7500;
	uint16_t SecondEmgZeroMaxTrim=12500;
	uint16_t SecondEmgZeroMinTrim=11000;
	uint32_t FlowAverage=0;
	uint32_t VolumeAverage=0;
	float32_t VolumeRate=1;
	float32_t FlowRate=1;
	float32_t CalibrationWeight=1;
	float32_t CalibrationFlow=1;
	uint16_t CleanTime=10;
	uint8_t StartHandleSeconds=0;
	bool IsStartTest=false;
	uint32_t StartTestTime=0;
	bool hasRunProcess=false;
	uint8_t  Backup[44];
	int	 BackupLen=44;
	bool isBackup=false;
	int PocketIndex=0;
	SystemModes systemMode=SystemModes::EmptyMode;

	uint8_t AverageSampleCount=100;
	uint8_t VolumeCalibrationSampleCount=100;
	uint8_t FlowCalibrationSampleCount=100;
	uint8_t MinimumFlowSensiblity=0;
	uint8_t WaitAfterProcessSeconds=0;
	bool IsInternalClean=false;
} SystemConfigStruct;

extern SystemConfigStruct SystemConfig;

typedef struct{
	uint32_t Channel1;
	uint32_t Channel2;
	uint32_t Channel3;
	uint32_t Channel4;
	uint32_t Channel5;
	uint32_t Channel6;
} LoadcellSPIStruct;

extern LoadcellSPIStruct LoadcellSPI;
typedef struct{
	int VolumeBufferLen;
	int FlowBufferLen;
	int FirstEmgBufferLen;
	int SecondEmgBufferLen;
	float32_t ReadedVolume;
	float32_t ReadedFlow;
	float32_t LastVolume;
	float32_t LastFlow;
	float32_t Volume;
	float32_t Flow;
	uint16_t FirstEmg;
	uint16_t SecondEmg;
	uint16_t FirstEmgNewVal;
	uint16_t SecondEmgNewVal;
	uint16_t FirstEmgNewValAfter;
	uint16_t SecondEmgNewValAfter;
	uint16_t NonFilterFirstEmg;
	uint16_t NonFilterSecondEmg;
	double_t VolumeRate;
	double_t FlowRate;
	float32_t RateVolumeValue;
	float32_t RateFlowValue;
	float32_t MovingAverageVolumeValue;
	uint8_t AveragePercent;
	uint8_t VolumeCalibrationPercent;
	uint8_t FlowCalibrationPercent;
	float32_t DiffAverageVolumeValue;
	float32_t MovingAverageFlowValue;
	float32_t MaxFlow;
	float32_t MinFlow;
	float32_t DiffAverageFlowValue;
	float32_t ConsecutiveVolumeValue;
	float32_t SensitiveVolumeValue;
	uint8_t CleanType;
	float32_t DiffVolumeValue;

	LoadcellSPIStruct LoadcellSPI;
	HAL_StatusTypeDef FirstEmgWriteStatus;
	HAL_StatusTypeDef SecondEmgWriteStatus;
	HAL_StatusTypeDef FirstEmgReadStatus;
	HAL_StatusTypeDef SecondEmgReadStatus;
} DebuggerStruct;
extern DebuggerStruct Debugger;
typedef struct{
	bool FirstEmg=false;
	bool SecondEmg=false;
	bool Volume=false;
	bool Flow=false;
	bool Pump=false;
	bool Valve=false;
	bool SafeMode=false;

} StatusStruct;
extern StatusStruct Statuses;

enum ProcessStatuses{
	PS_Empty,
	PS_Processing,
	PS_End
};

typedef struct {
	uint8_t ProcessType;
	uint8_t SubType;
	ProcessStatuses Status=ProcessStatuses::PS_Empty;
} ProcessTrackerStruct;
extern ProcessTrackerStruct ProcessTracker;




#endif /* INC_GENERALHEADERS_H_ */
