/*
 * Communication.cpp
 *
 *  Created on: Aug 6, 2024
 *      Author: OrioN
 */

#include "../Inc/GeneralHeaders.h"

Communication::Communication() {
	// TODO Auto-generated constructor stub
	FMI.BackupData();

}

Communication::~Communication() {
	// TODO Auto-generated destructor stub
}

void Communication::ProcessCommand(uint8_t *command){
	switch(command[0]){
	case RequestType::R_System:
		System(command);
		break;
	case RequestType::R_Command:
		Command(command);
		break;
	case RequestType::R_Configuration:
		Configuration(command);
		break;
	default:
		ErrorResult(OperationCodes::ReadData, Errors::UndefinedProcessType);
		break;
	}
}
bool pauseFlg = false;//***//
bool CleanDone = false;
void Communication::System(uint8_t *command){
	if(command[0]!=RequestType::R_System) return;
	uint16_t calibrationWeight = 500;
	uint16_t calibrationFlow = 12;
   	uint8_t data[11];
	switch (command[1])   										      //Check Function
	{
	   case SystemRequestType::SYSR_Status:
		   	data[0]=Statuses.FirstEmg;
		   	data[1]=Statuses.SecondEmg;
		   	data[2]=Statuses.Volume;
		   	data[3]=Statuses.Flow;
		   	data[4]=Statuses.Pump;
		   	data[5]=Statuses.Valve;
		   	data[6]=ThreadStorage.LoadcellAverageThreadId!=NULL&&ThreadStorage.FirstEmgThreadId!=0x00;
		   	data[7]=ThreadStorage.CalibrationVolumeThreadId!=NULL&&ThreadStorage.FirstEmgThreadId!=0x00;
		   	data[8]=ThreadStorage.CalibrationFlowThreadId!=NULL&&ThreadStorage.FirstEmgThreadId!=0x00;
		   	data[9]=ThreadStorage.CleanThreadId!=NULL&&ThreadStorage.FirstEmgThreadId!=0x00;
		   	data[10]=Statuses.SafeMode;
		   	SuccessDataResult(100,SuccessDataType::SD_Status,data,11);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			break;
	   case SystemRequestType::SYSR_Restart:
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
		    HAL_NVIC_SystemReset();
	        break;
	   case SystemRequestType::SYSR_FactoryReset:
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			HardReset();
		   break;
	   case SystemRequestType::SYSR_MeasurementAverage:
		   	 ClearLoadcellParams();
		   	 LoadcellAverage();
		   	 break;
	   case SystemRequestType::SYSR_CalibrationVolume:
		   	 ClearLoadcellParams();
			calibrationWeight=(command[2]<<8)+command[3];
		   	 LoadcellVolumeCalibration(calibrationWeight);
		   	 break;
	   case SystemRequestType::SYSR_CalibrationFlow:
		   	 ClearLoadcellParams();
			calibrationFlow=(command[2]<<8)+command[3];
		   	 LoadcellFlowCalibration(calibrationFlow);
		   	 break;
		default:
			ErrorResult(OperationCodes::ReadData, Errors::UndefinedSystemType);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			break;
	 }
}
bool isOpenFirstEmg;
bool isOpenSecondEmg;
bool isOpenLoadcell;
void Communication::Command(uint8_t *command){

	uint16_t cleanTime=0;
	if(command[0]!=RequestType::R_Command) return;
    isOpenFirstEmg = command[2] & 0x04;
    isOpenSecondEmg = command[2] & 0x02;
    isOpenLoadcell = command[2] & 0x01;
	switch (command[1])   												//Check Function
	{
		case CommandRequestType::CMDR_FirstEmg:
			if(SystemConfig.systemMode!=SystemModes::EmptyMode&&SystemConfig.systemMode!=SystemModes::TestMode
					&&SystemConfig.systemMode!=SystemModes::ManuelMode){
				ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			 ToggleFirstEmg(command[2]==1);  	     //Check SubFunction
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case CommandRequestType::CMDR_SecondEmg:
			if(SystemConfig.systemMode!=SystemModes::EmptyMode&&SystemConfig.systemMode!=SystemModes::TestMode
					&&SystemConfig.systemMode!=SystemModes::ManuelMode){
				ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			ToggleSecondEmg(command[2]==1);    	 //Check SubFunction
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case CommandRequestType::CMDR_LoadcellMeasure:
			if(SystemConfig.systemMode!=SystemModes::EmptyMode&&SystemConfig.systemMode!=SystemModes::TestMode
					&&SystemConfig.systemMode!=SystemModes::ManuelMode){
				ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			if(SystemConfig.VolumeAverage==0 || SystemConfig.FlowAverage==0){
				ErrorResult(OperationCodes::ReadData, Errors::HasNotAverage);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			if(SystemConfig.FlowRate<2 || SystemConfig.VolumeRate<2){
				ErrorResult(OperationCodes::ReadData, Errors::HasNotCalibration);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			 ToggleLoadCell(command[2]==1);
			 SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case CommandRequestType::CMDR_Pump:
			if(SystemConfig.systemMode!=SystemModes::EmptyMode&&SystemConfig.systemMode!=SystemModes::TestMode
					&&SystemConfig.systemMode!=SystemModes::ManuelMode){
				ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
				 SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			TogglePump(command[2]==1); 	 	 //Check SubFunction
			 SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case CommandRequestType::CMDR_Valve:
			if(SystemConfig.systemMode!=SystemModes::EmptyMode&&SystemConfig.systemMode!=SystemModes::TestMode
					&&SystemConfig.systemMode!=SystemModes::ManuelMode){
				ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
				 SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			ToggleValve(command[2]==1);   	 //Check SubFunction
			 SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case CommandRequestType::CMDR_Clean: //Check SubFunction
			if(SystemConfig.systemMode!=SystemModes::EmptyMode){
				ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
				 SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			 cleanTime=(command[2]<<8)+command[3];
			if(cleanTime<1){
				ErrorResult(OperationCodes::ReadData, Errors::ValueShouldNotBeZero);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			 StartClean(cleanTime*1000);
			 break;
		case CommandRequestType::CMDR_Safe: //Check SubFunction
				if(SystemConfig.systemMode!=SystemModes::EmptyMode && SystemConfig.systemMode!=SystemModes::SafeMode){
					ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
					SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
					return;
				}
				ToggleSafeMode(command[2]==1);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case CommandRequestType::CMDR_DataStream:
			if(SystemConfig.systemMode!=SystemModes::EmptyMode && SystemConfig.systemMode!= SystemModes::TestMode){
				ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			ToggleDataStream(command[2]==1);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case CommandRequestType::CMDR_ReadVolume:
			if(SystemConfig.systemMode!=SystemModes::EmptyMode&&SystemConfig.systemMode!=SystemModes::TestMode
					&&SystemConfig.systemMode!=SystemModes::ManuelMode){
				ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			if(SystemConfig.VolumeAverage==0 || SystemConfig.FlowAverage==0){
				ErrorResult(OperationCodes::ReadData, Errors::HasNotAverage);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			 ToggleReadVolume(command[2]==1);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case CommandRequestType::CMDR_ReadFlow:
			if(SystemConfig.systemMode!=SystemModes::EmptyMode&&SystemConfig.systemMode!=SystemModes::TestMode
					&&SystemConfig.systemMode!=SystemModes::ManuelMode){
				ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			if(SystemConfig.FlowRate<2 || SystemConfig.VolumeRate<2){
				ErrorResult(OperationCodes::ReadData, Errors::HasNotCalibration);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			 ToggleReadFlow(command[2]==1);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case CommandRequestType::CMDR_ReadFirstEmg:
			if(SystemConfig.systemMode!=SystemModes::EmptyMode&&SystemConfig.systemMode!=SystemModes::TestMode
					&&SystemConfig.systemMode!=SystemModes::ManuelMode){
				ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			 ToggleReadFirstEmg(command[2]==1);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case CommandRequestType::CMDR_ReadSecondEmg:
			if(SystemConfig.systemMode!=SystemModes::EmptyMode&&SystemConfig.systemMode!=SystemModes::TestMode
					&&SystemConfig.systemMode!=SystemModes::ManuelMode){
				ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			 ToggleReadSecondEmg(command[2]==1);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case CommandRequestType::CMDR_StopTest:
			 cleanTime=(command[2]<<8)+command[3];
			StopTest(cleanTime);
			 break;
		case CommandRequestType::CMDR_StartTest:        //############################# Start Test #################
			if(SystemConfig.systemMode!=SystemModes::EmptyMode){
				ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			if(SystemConfig.VolumeAverage==0 || SystemConfig.FlowAverage==0){
				ErrorResult(OperationCodes::ReadData, Errors::HasNotAverage);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			if(SystemConfig.FlowRate<2 || SystemConfig.VolumeRate<2){
				ErrorResult(OperationCodes::ReadData, Errors::HasNotCalibration);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			 cleanTime=(command[3]<<8)+command[4];
			 SystemConfig.MinimumFlowSensiblity=command[5];
			 SystemConfig.WaitAfterProcessSeconds=command[6];
			StartTest(isOpenFirstEmg, isOpenSecondEmg, isOpenLoadcell,cleanTime,command[7]);
			 break;

		case CommandRequestType::CMDR_PauseTest:
			pauseTest();
			break;
		case CommandRequestType::CMDR_ResumeTest:
			resumeTest();
			break;
		case CommandRequestType::CMDR_CancelTest:
			cancelTest();
			break;

		case CommandRequestType::CMDR_StartManPrep:
			cleanTime=(command[3]<<8)+command[4];
			ManualPrep(cleanTime);
			break;
		case CommandRequestType::CMDR_StartManTest:
			if(CleanDone){
				cleanTime=(command[3]<<8)+command[4];  //0;
				StartManTest(isOpenFirstEmg, isOpenSecondEmg, isOpenLoadcell,cleanTime,command[7]);
			}
			break;

		default:
			ErrorResult(OperationCodes::ReadData, Errors::UndefinedCommandType);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
	}
}

void Communication::Configuration(uint8_t *command){
	if(command[0]!=RequestType::R_Configuration) return;
	if(SystemConfig.systemMode!=SystemModes::EmptyMode){
		ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
		return;
	}
	uint16_t maxTrim = 0;
	uint16_t zeroPointMaxTrim = 0;
	uint16_t zeroPointMinTrim = 0;
	switch (command[1])   												//Check Function
	{
		case ConfigurationRequestType::CFGR_ReadConfiguration:
			 SuccessDataResult(100,SuccessDataType::SD_Configuration, SystemConfig.Backup,SystemConfig.BackupLen);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case ConfigurationRequestType::CFGR_SetSendPerSecond:
			FMI.WriteSendPerSecond(command[2]);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case ConfigurationRequestType::CFGR_SetFirstEmgPerSecond:
			FMI.WriteFirstEmgPerSecond(command[2]);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case ConfigurationRequestType::CFGR_SetSecondEmgPerSecond:
			FMI.WriteSecondEmgPerSecond(command[2]);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case ConfigurationRequestType::CFGR_SetVolumePerSecond:
			FMI.WriteVolumePerSecond(command[2]);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case ConfigurationRequestType::CFGR_SetFlowPerSecond:
			FMI.WriteFlowPerSecond(command[2]);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case ConfigurationRequestType::CFGR_SetPumpMaxRunTime:
			FMI.WritePumpMaxRunTime(command[2]);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case ConfigurationRequestType::CFGR_SetValveMaxRunTime:
			FMI.WriteValveMaxRunTime(command[2]);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		 case ConfigurationRequestType::CFGR_SetCalibrationFirstEmg:
				maxTrim=(command[2]<<8)+command[3];
				zeroPointMaxTrim=(command[4]<<8)+command[5];
				zeroPointMinTrim=(command[6]<<8)+command[7];
				if(maxTrim<1){
					ErrorResult(OperationCodes::ReadData, Errors::ValueShouldNotBeZero);
					SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
					return;
				}
				if(zeroPointMinTrim>=zeroPointMaxTrim){
					ErrorResult(OperationCodes::ReadData, Errors::MinValueCanNotBeGreaterThanMaxValue);
					SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
					return;
				}
				FMI.WriteFirstEmgData(maxTrim, zeroPointMaxTrim, zeroPointMinTrim);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case ConfigurationRequestType::CFGR_SetCalibrationSecondEmg:
			maxTrim=(command[2]<<8)+command[3];
			zeroPointMaxTrim=(command[4]<<8)+command[5];
			zeroPointMinTrim=(command[6]<<8)+command[7];
			if(maxTrim<1){
				ErrorResult(OperationCodes::ReadData, Errors::ValueShouldNotBeZero);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			if(zeroPointMinTrim>=zeroPointMaxTrim){
				ErrorResult(OperationCodes::ReadData, Errors::MinValueCanNotBeGreaterThanMaxValue);
				SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
				return;
			}
			FMI.WriteSecondEmgData(maxTrim, zeroPointMaxTrim, zeroPointMinTrim);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
		case ConfigurationRequestType::CFGR_SaveFlash:
			FMI.Update();
			FlashInitialize();
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			break;
		default:
			ErrorResult(OperationCodes::ReadData, Errors::UndefinedConfigurationType);
			SendFeedback(command[0], command[1], ProcessStatuses::PS_End);
			 break;
	}
}

void Communication::ToggleFirstEmg(bool isStart){
	Statuses.FirstEmg=isStart;
	if(isStart){
		if(ThreadStorage.FirstEmgThreadId!=NULL) return;
		ThreadStorage.FirstEmgThreadId = osThreadNew(StartFirstEmgTask, NULL, &ThreadStorage.FirstEmgThreadAttr);
		return;
	}
	osThreadTerminate(ThreadStorage.FirstEmgThreadId);
	ThreadStorage.FirstEmgThreadId=NULL;
}
void Communication::ToggleSecondEmg(bool isStart){
	Statuses.SecondEmg=isStart;
	if(isStart){
		if(ThreadStorage.SecondEmgThreadId!=NULL) return;
		ThreadStorage.SecondEmgThreadId = osThreadNew(StartSecondEmgTask, NULL, &ThreadStorage.SecondEmgThreadAttr);
		return;
	}
	osThreadTerminate(ThreadStorage.SecondEmgThreadId);
	ThreadStorage.SecondEmgThreadId=NULL;
}
void Communication::ToggleLoadCell(bool isStart){
	Statuses.Volume=isStart;
	Statuses.Flow=isStart;
	if(SystemConfig.systemMode==SystemModes::SafeMode){
		ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
		return;
	}
	if(isStart){
		if(ThreadStorage.FlowThreadId!=NULL) return;
		if(ThreadStorage.VolumeThreadId!=NULL) return;
		ClearLoadcellParams();
		ThreadStorage.VolumeThreadId = osThreadNew(StartVolumeTask, NULL, &ThreadStorage.VolumeThreadAttr);
		ThreadStorage.FlowThreadId = osThreadNew(StartFlowTask, NULL, &ThreadStorage.FlowThreadAttr);
		return;
	}
	osThreadTerminate(ThreadStorage.VolumeThreadId);
	osThreadTerminate(ThreadStorage.FlowThreadId);
	ThreadStorage.VolumeThreadId=NULL;
	ThreadStorage.FlowThreadId=NULL;
}
void Communication::ToggleReadVolume(bool isStart){
	if(SystemConfig.systemMode==SystemModes::SafeMode){
		ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
		return;
	}
	if(isStart){
		if(ThreadStorage.ReadVolumeThreadId!=NULL) return;
		ClearLoadcellParams();
		ThreadStorage.ReadVolumeThreadId = osThreadNew(StartReadVolumeTask, NULL, &ThreadStorage.VolumeThreadAttr);
		return;
	}
	osThreadTerminate(ThreadStorage.ReadVolumeThreadId);
	ThreadStorage.ReadVolumeThreadId=NULL;
	ClearLoadcellParams();
}

void Communication::ToggleReadFlow(bool isStart){
	if(SystemConfig.systemMode==SystemModes::SafeMode){
		ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
		return;
	}
	if(isStart){
		if(ThreadStorage.ReadFlowThreadId!=NULL) return;
		ClearLoadcellParams();
		ThreadStorage.ReadFlowThreadId = osThreadNew(StartReadFlowTask, NULL, &ThreadStorage.FlowThreadAttr);
		return;
	}
	osThreadTerminate(ThreadStorage.ReadFlowThreadId);
	ThreadStorage.ReadFlowThreadId=NULL;
	ClearLoadcellParams();
}

void Communication::ToggleReadFirstEmg(bool isStart){
	if(SystemConfig.systemMode==SystemModes::SafeMode){
		ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
		return;
	}
	if(isStart){
		if(ThreadStorage.ReadFirstEmgThreadId!=NULL) return;
		ThreadStorage.ReadFirstEmgThreadId = osThreadNew(StartReadFirstEmgTask, NULL, &ThreadStorage.FlowThreadAttr);
		return;
	}
	osThreadTerminate(ThreadStorage.ReadFirstEmgThreadId);
	ThreadStorage.ReadFirstEmgThreadId=NULL;
}

void Communication::ToggleReadSecondEmg(bool isStart){
	if(SystemConfig.systemMode==SystemModes::SafeMode){
		ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
		return;
	}
	if(isStart){
		if(ThreadStorage.ReadSecondEmgThreadId!=NULL) return;
		ThreadStorage.ReadSecondEmgThreadId = osThreadNew(StartReadSecondEmgTask, NULL, &ThreadStorage.FlowThreadAttr);
		return;
	}
	osThreadTerminate(ThreadStorage.ReadSecondEmgThreadId);
	ThreadStorage.ReadSecondEmgThreadId=NULL;
}

void Communication::TogglePump(bool isStart){
	Statuses.Pump=isStart;
	if(isStart){
		if(ThreadStorage.PumpMaxRunThreadId!=NULL) return;
		HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_SET);
		ThreadStorage.PumpMaxRunThreadId = osThreadNew(StartAutoClosePumpTask, NULL, &ThreadStorage.PumpMaxRunThreadAttr);
		return;
	}
	HAL_GPIO_WritePin(PUMP_GPIO_Port, PUMP_Pin, GPIO_PIN_RESET);
	osThreadTerminate(ThreadStorage.PumpMaxRunThreadId);
	ThreadStorage.PumpMaxRunThreadId=NULL;

}

void Communication::ToggleValve(bool isStart){
	Statuses.Valve=isStart;
	if(isStart){
		if(ThreadStorage.ValveMaxRunThreadId!=NULL) return;
		HAL_GPIO_WritePin(VALVE_GPIO_Port, VALVE_Pin, GPIO_PIN_SET);
		ThreadStorage.ValveMaxRunThreadId = osThreadNew(StartAutoCloseValveTask, NULL, &ThreadStorage.ValveMaxRunThreadAttr);
		return;
	}
	HAL_GPIO_WritePin(VALVE_GPIO_Port, VALVE_Pin, GPIO_PIN_RESET);
	osThreadTerminate(ThreadStorage.ValveMaxRunThreadId);
	ThreadStorage.ValveMaxRunThreadId=NULL;
}
void Communication::ToggleDataStream(bool isStart){
	if(SystemConfig.systemMode!=SystemModes::EmptyMode && SystemConfig.systemMode!=SystemModes::TestMode){
		ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
		SendFeedback(RequestType::R_Command,CommandRequestType::CMDR_DataStream, ProcessStatuses::PS_End);
		return;
	}
	SystemConfig.IsStartTest=isStart;
	if(isStart&&!pauseFlg){
		SystemConfig.StartTestTime=StartTimerTicks;
		SystemConfig.systemMode=SystemModes::TestMode;
		return;
	}
	SystemConfig.systemMode=SystemModes::EmptyMode;
}
void Communication::StartTest(bool isStartFirstEmg,bool isStartSecondEmg,bool isStartLoadcell,uint16_t cleanTime,uint8_t startHandleSeconds){

	SystemConfig.CleanTime=cleanTime;
	StartCleanTask(NULL);
	FixVolume();
	ToggleFirstEmg(isStartFirstEmg);
	ToggleSecondEmg(isStartSecondEmg);
	ToggleLoadCell(isStartLoadcell);
	SystemConfig.StartHandleSeconds=startHandleSeconds;
	ToggleDataStream(true);
	SystemConfig.PocketIndex=0;
	SystemConfig.systemMode=SystemModes::TestMode;
}
void Communication::StopTest(uint16_t cleanTime){
	SystemConfig.CleanTime=cleanTime;
	ToggleDataStream(false);
	SendFeedback(RequestType::R_Command, CommandRequestType::CMDR_StopTest, ProcessStatuses::PS_Processing);
	ToggleFirstEmg(false);
	ToggleSecondEmg(false);
	ToggleLoadCell(false);
	StartCleanTask(NULL);
	SystemConfig.PocketIndex=0;
	SystemConfig.systemMode=SystemModes::EmptyMode;
	SendFeedback(RequestType::R_Command, CommandRequestType::CMDR_StopTest, ProcessStatuses::PS_End);
    HAL_NVIC_SystemReset();
}

//**************************************** Resume/Pause FNs *******************************************//
void Communication::resumeTest(void){
	if(isOpenFirstEmg) ToggleFirstEmg(true);
	if(isOpenSecondEmg) ToggleSecondEmg(true);
	//ToggleLoadCell(true);  // //250721
	ToggleDataStream(true);
	SendFeedback(RequestType::R_Command, CommandRequestType::CMDR_ResumeTest, ProcessStatuses::PS_End);
}
void Communication::pauseTest(void){
	pauseFlg = true;

	if(isOpenFirstEmg) ToggleFirstEmg(false);
	if(isOpenSecondEmg) ToggleSecondEmg(false);
	//ToggleLoadCell(false); // //250721
	ToggleDataStream(false);
	SendFeedback(RequestType::R_Command, CommandRequestType::CMDR_PauseTest, ProcessStatuses::PS_End);
	vTaskDelay(100);
}
void Communication::cancelTest(void){
	pauseFlg = false;
	SystemConfig.CleanTime=0;//cleanTime;
	ToggleDataStream(false);
	SendFeedback(RequestType::R_Command, CommandRequestType::CMDR_CancelTest, ProcessStatuses::PS_Processing);
	ToggleFirstEmg(false);
	ToggleSecondEmg(false);
	ToggleLoadCell(false);
	//StartCleanTask(NULL);
	//StartPreManualTask(NULL);
	SystemConfig.PocketIndex=0;
	SystemConfig.systemMode=SystemModes::EmptyMode;
	SendFeedback(RequestType::R_Command, CommandRequestType::CMDR_CancelTest, ProcessStatuses::PS_End);
	vTaskDelay(100);
    HAL_NVIC_SystemReset();
}
//*********************************  Manual Mode Functions **********************************//
void Communication::ManualPrep(uint16_t cleanTime){
	SystemConfig.CleanTime=cleanTime;
	SendFeedback(RequestType::R_Command, CommandRequestType::CMDR_StartManPrep, ProcessStatuses::PS_Processing);
	StartCleanTask(NULL);
	CleanDone = true;
	SendFeedback(RequestType::R_Command, CommandRequestType::CMDR_StartManPrep, ProcessStatuses::PS_End);
}
void Communication::StartManTest(bool isStartFirstEmg,bool isStartSecondEmg,bool isStartLoadcell,uint16_t cleanTime,uint8_t startHandleSeconds){
	SystemConfig.CleanTime=cleanTime;
	CleanDone = false;
	//StartCleanTask(NULL);
	FixVolume();
	ToggleFirstEmg(isStartFirstEmg);
	ToggleSecondEmg(isStartSecondEmg);
	ToggleLoadCell(isStartLoadcell);
	SystemConfig.StartHandleSeconds=startHandleSeconds;
	ToggleDataStream(true);
	SystemConfig.PocketIndex=0;
	SystemConfig.systemMode=SystemModes::TestMode;
	SendFeedback(RequestType::R_Command, CommandRequestType::CMDR_StartManTest, ProcessStatuses::PS_End);
}
//******************************************************************************************//
void Communication::StartClean(uint16_t cleanTime){
	if(SystemConfig.systemMode!=SystemModes::EmptyMode){
		ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
		 SendFeedback(RequestType::R_Command, CommandRequestType::CMDR_Clean, ProcessStatuses::PS_End);
		return;
	}
	SystemConfig.systemMode=SystemModes::CleanMode;
	if(ThreadStorage.CleanThreadId==NULL){
		SystemConfig.CleanTime=cleanTime;
		ThreadStorage.CleanThreadId = osThreadNew(StartCleanTask, NULL, &ThreadStorage.CleanThreadAttr);
		return;
	}
}

void Communication::ToggleSafeMode(bool isStart){

	Statuses.SafeMode=isStart;
	if(SystemConfig.systemMode!=SystemModes::EmptyMode && SystemConfig.systemMode!=SystemModes::SafeMode){
		ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
		SendFeedback(RequestType::R_Command, CommandRequestType::CMDR_Safe, ProcessStatuses::PS_End);
		return;
	}
	if(isStart){
		if(ThreadStorage.SafeModeThreadId!=NULL)return;
		SystemConfig.systemMode=SystemModes::SafeMode;
		ThreadStorage.SafeModeThreadId = osThreadNew(StartSafeModeTask, NULL, &ThreadStorage.SafeModeThreadAttr);
		return;
	}
	SystemConfig.systemMode=SystemModes::EmptyMode;
	osThreadTerminate(ThreadStorage.SafeModeThreadId);
	TogglePump(false);
	ToggleValve(false);
	ThreadStorage.SafeModeThreadId=NULL;
}

void Communication::LoadcellAverage(){
	if(SystemConfig.systemMode!=SystemModes::EmptyMode){
		ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
		SendFeedback(RequestType::R_System, SystemRequestType::SYSR_MeasurementAverage, ProcessStatuses::PS_End);
		return;
	}
	SystemConfig.systemMode=SystemModes::CalibrationMode;
	ThreadStorage.LoadcellAverageThreadId = osThreadNew(StartLoadcellAverageTask, NULL, &ThreadStorage.LoadcellAverageThreadAttr);
}

void Communication::LoadcellVolumeCalibration(uint16_t weight){
	if(SystemConfig.systemMode!=SystemModes::EmptyMode){
		ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
		SendFeedback(RequestType::R_System, SystemRequestType::SYSR_CalibrationVolume, ProcessStatuses::PS_End);
		return;
	}
	SystemConfig.systemMode=SystemModes::CalibrationMode;
	SystemConfig.VolumeRate=1;
	SystemConfig.CalibrationWeight=weight;
	ThreadStorage.CalibrationVolumeThreadId = osThreadNew(StartCalibrationVolumeTask, NULL, &ThreadStorage.CalibrationVolumeThreadAttr);
}

void Communication::LoadcellFlowCalibration(uint16_t flow){
	if(SystemConfig.systemMode!=SystemModes::EmptyMode){
		ErrorResult(OperationCodes::ReadData, Errors::HasRunProcess);
		SendFeedback(RequestType::R_System, SystemRequestType::SYSR_CalibrationFlow, ProcessStatuses::PS_End);
		return;
	}
	SystemConfig.systemMode=SystemModes::CalibrationMode;
	SystemConfig.FlowRate=1;
	SystemConfig.CalibrationFlow=flow;
	ThreadStorage.CalibrationFlowThreadId = osThreadNew(StartCalibrationFlowTask, NULL, &ThreadStorage.CalibrationFlowThreadAttr);
}

void Communication::SetFlashManager(FlashManager flashManager){
	FMI=flashManager;
}

