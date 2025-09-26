/*
 * SystemAdapter.cpp
 *
 *  Created on: Aug 2, 2024
 *      Author: OrioN
 */

#include "../Inc/GeneralHeaders.h"

volatile uint32_t sendCounter=0;
volatile uint32_t firstEmgCounter=0;
volatile uint32_t secondEmgCounter=0;
volatile uint32_t flowCounter=0;
volatile uint32_t volumeCounter=0;
uint8_t CommandUart[8]={0};
Emg EmgInstance;
LoadCell LoadCellInstance;
Communication CommunicationInstance;
FlashManager FlashManagerInstance;
ThreadStorageStruct ThreadStorage;
StatusStruct Statuses;
SystemConfigStruct SystemConfig;
DebuggerStruct Debugger;
uint16_t totalLen=7;
uint8_t allData[4096];


const osThreadAttr_t normalPriority = {
  .name = "normalPriority",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
const osThreadAttr_t highPriority = {
  .name = "highPriority",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for firstEmgSemaphoreHandle */
const osSemaphoreAttr_t firstEmgSemaphore_attributes = {
  .name = "FirstEmgSemaphoreHandle"
};
/* Definitions for secondEmgSemaphoreHandle */
const osSemaphoreAttr_t secondEmgSemaphore_attributes = {
  .name = "SecondEmgSemaphoreHandle"
};
/* Definitions for flowSemaphoreHandle */
const osSemaphoreAttr_t flowSemaphore_attributes = {
  .name = "FlowSemaphoreHandle"
};
/* Definitions for volumeSemaphoreHandle */
const osSemaphoreAttr_t volumeSemaphore_attributes = {
  .name = "VolumeSemaphoreHandle"
};
/* Definitions for CommunicationSemaphoreHandle */
const osSemaphoreAttr_t communicationSemaphore_attributes = {
  .name = "CommunicationSemaphoreHandle"
};

void SetupOS(void){
	EmgInstance.FirstEmgSetup();
	EmgInstance.SecondEmgSetup();
	LoadCellInstance.Setup();
	FlashInitialize();
	CommunicationInstance.SetFlashManager(FlashManagerInstance);
	osKernelInitialize();
}

void UpdatePriority(void){
	HAL_TIM_Base_Start_IT(&htim11);
	ThreadStorage.FirstEmgThreadAttr=normalPriority;
	ThreadStorage.SecondEmgThreadAttr=normalPriority;
	ThreadStorage.FlowThreadAttr=normalPriority;
	ThreadStorage.VolumeThreadAttr=normalPriority;
	ThreadStorage.PumpMaxRunThreadAttr=normalPriority;
	ThreadStorage.ValveMaxRunThreadAttr=normalPriority;
	ThreadStorage.CalibrationFlowThreadAttr=normalPriority;
	ThreadStorage.CalibrationVolumeThreadAttr=normalPriority;
	ThreadStorage.CleanThreadAttr=normalPriority;
	ThreadStorage.SafeModeThreadAttr=normalPriority;
}

void StartOS(void){
	UpdatePriority();
	/*
	int i=200;
	while(i>=0){
		LoadCellInstance.ReadVolumeAndFlow();
		i--;
	}
	SystemConfig.VolumeAverage=LoadCellInstance.VolumeValue;
	SystemConfig.FlowAverage=LoadCellInstance.FlowValue;
	while(1){
		EmgInstance.FirstEmgRead();
		EmgInstance.SecondEmgRead();
		LoadCellInstance.ReadVolume(false);
		LoadCellInstance.ReadFlow(false);
	}

	uint16_t cnt=0;
	uint8_t uartData[2];
	while(true){
		cnt++;
		uartData[0]=(cnt & 0xFF00)>>8;
		uartData[1]=(cnt & 0x00FF);
		WriteUart(uartData, 2);
	}
	*/
	HAL_Delay( 1000 );
   	uint8_t data[4];
   	data[0]=HasCalibration();//HasCalibration
   	data[1]=HasLoadcell(true);//HasLoadcell
   	data[2]=HasFirstEmg(true);//HasFirstEmg
   	data[3]=HasSecondEmg(true);//HasSecondEmg
	SuccessDataResult(0, SuccessDataType::SD_Start, data, 4);
	SendFeedback(0,0,0);

	ThreadStorage.ReadUARTThreadId=osThreadNew(StartReadUARTTask, NULL, &highPriority);
	osKernelStart();
    HAL_NVIC_SystemReset();
}

void HardReset(void){
	FlashManagerInstance.HardReset(true);
    HAL_NVIC_SystemReset();
}
void FlashInitialize(void){
	if(!FlashManagerInstance.IsSetup()){
		FlashManagerInstance.WriteSendPerSecond(SystemConfig.SendPerSecond);
		FlashManagerInstance.WriteFirstEmgPerSecond(SystemConfig.FirstEmgPerSecond);
		FlashManagerInstance.WriteSecondEmgPerSecond(SystemConfig.SecondEmgPerSecond);
		FlashManagerInstance.WriteVolumePerSecond(SystemConfig.VolumePerSecond);
		FlashManagerInstance.WriteFlowPerSecond(SystemConfig.FlowPerSecond);
		FlashManagerInstance.WritePumpMaxRunTime(SystemConfig.PumpMaxRunSecond);
		FlashManagerInstance.WriteValveMaxRunTime(SystemConfig.ValveMaxRunSecond);
		FlashManagerInstance.WriteFlowRate(SystemConfig.FlowRate);
		FlashManagerInstance.WriteVolumeRate(SystemConfig.VolumeRate);
		FlashManagerInstance.WriteFlowAverage(SystemConfig.FlowAverage);
		FlashManagerInstance.WriteVolumeAverage(SystemConfig.VolumeAverage);
		FlashManagerInstance.WriteFirstEmgData(
				SystemConfig.FirstEmgMaxTrim,
				SystemConfig.FirstEmgZeroMaxTrim,
				SystemConfig.FirstEmgZeroMinTrim);
		FlashManagerInstance.WriteSecondEmgData(
				SystemConfig.SecondEmgMaxTrim,
				SystemConfig.SecondEmgZeroMaxTrim,
				SystemConfig.SecondEmgZeroMinTrim);
		FlashManagerInstance.Setup();
		FlashManagerInstance.Update();
	}
	//FlashManagerInstance.WriteFirstEmgData(0,0,0);
	//FlashManagerInstance.WriteSecondEmgData(0,0,0);
	//FlashManagerInstance.Update();
	SystemConfig.SendPerSecond=FlashManagerInstance.ReadSendPerSecond();
	SystemConfig.FirstEmgPerSecond=FlashManagerInstance.ReadFirstEmgPerSecond();
	SystemConfig.SecondEmgPerSecond=FlashManagerInstance.ReadSecondEmgPerSecond();
	SystemConfig.VolumePerSecond=FlashManagerInstance.ReadVolumePerSecond();
	SystemConfig.FlowPerSecond=FlashManagerInstance.ReadFlowPerSecond();
	SystemConfig.PumpMaxRunSecond=FlashManagerInstance.ReadPumpMaxRunTime();
	SystemConfig.ValveMaxRunSecond=FlashManagerInstance.ReadValveMaxRunTime();
	SystemConfig.FlowRate=FlashManagerInstance.ReadFlowRate();
	SystemConfig.VolumeRate=FlashManagerInstance.ReadVolumeRate();
	SystemConfig.FlowAverage=FlashManagerInstance.ReadFlowAverage();
	SystemConfig.VolumeAverage=FlashManagerInstance.ReadVolumeAverage();
	SystemConfig.FirstEmgMaxTrim=FlashManagerInstance.ReadFirstEmgMaxTrim();
	SystemConfig.FirstEmgZeroMaxTrim=FlashManagerInstance.ReadFirstEmgZeroPointMaxTrim();
	SystemConfig.FirstEmgZeroMinTrim=FlashManagerInstance.ReadFirstEmgZeroPointMinTrim();
	SystemConfig.SecondEmgMaxTrim=FlashManagerInstance.ReadSecondEmgMaxTrim();
	SystemConfig.SecondEmgZeroMaxTrim=FlashManagerInstance.ReadSecondEmgZeroPointMaxTrim();
	SystemConfig.SecondEmgZeroMinTrim=FlashManagerInstance.ReadSecondEmgZeroPointMinTrim();
}

void SuccessResult(void){
	uint8_t data[1]={0x01};
	WriteUart(data, 1);
}

void SendFeedback(uint8_t processType,uint8_t subType, uint8_t status){
	uint8_t uartData[6]={0};
	uint16_t xLen=3;
	uartData[0]=0x04;
	uartData[1]=(xLen & 0xFF00)>>8;
	uartData[2]=(xLen & 0x00FF);
	uartData[3]=processType;
	uartData[4]=subType;
	uartData[5]=status;
	WriteUart(uartData, 6);
}
void SuccessDataResult(uint8_t percent,uint8_t dataType,uint8_t *data,int len){
	uint8_t uartData[len+6]={0};
	uint16_t xLen=len+3;
	uartData[0]=0x02;
	uartData[1]=(xLen & 0xFF00)>>8;
	uartData[2]=(xLen & 0x00FF);
	uartData[3]=1;
	uartData[4]=percent;
	uartData[5]=dataType;
	for (int i = 6; i < len+6; ++i) {
		uartData[i]=data[i-6];
	}
	WriteUart(uartData, len+6);
}
void ErrorDataResult(uint8_t percent, uint8_t opCode,uint16_t errorCode){
	uint8_t uartData[8]={0};
	uint16_t xLen=5;
	uartData[0]=0x02;
	uartData[1]=(xLen & 0xFF00)>>8;
	uartData[2]=(xLen & 0x00FF);
	uartData[3]=0;
	uartData[4]=percent;
	uartData[5]=opCode;
	uartData[6]=(errorCode>>8) & 0xFF;
	uartData[7]=errorCode & 0xFF;
	WriteUart(uartData, 8);
}
void ErrorResult(uint8_t opCode,uint16_t errorCode){
	uint8_t data[4];
	data[0]=0x00;
	data[1]=opCode;
	data[2]=(errorCode>>8) & 0xFF;
	data[3]=errorCode & 0xFF;
	WriteUart(data, 4);
}
void StartReadUARTTask(void *argument){

	const TickType_t xDelay = 50 / portTICK_PERIOD_MS;
	//osStatus_t communicationSemaphoreVal;
	ThreadStorage.CommunicationSemaphoreHandle = osSemaphoreNew(1, 1, &communicationSemaphore_attributes);
	ThreadStorage.SendUARTThreadId=osThreadNew(StartSendUARTTask, NULL, &normalPriority);
	//bool retryStatus=false;
    uint8_t data[8];
	for(;;){
		//uint16_t newPos = __HAL_DMA_GET_COUNTER(huart1.hdmarx);
		//if(newPos>0){
			HAL_StatusTypeDef status=HAL_UART_Receive_DMA(&huart1, data, 8);
			if (status == HAL_OK&&(data[0]==1||data[0]==2||data[0]==3)) {
				SendFeedback(data[0], data[1], ProcessStatuses::PS_Processing);
				CommunicationInstance.ProcessCommand(data);
	        }
			else if(status != HAL_TIMEOUT && status != HAL_BUSY && status != HAL_OK){
				SuccessDataResult(100, SuccessDataType::SD_RepeatAgain, data, 8);
	        }
		//}
		vTaskDelay(xDelay);
			/*if (retryStatus||HAL_UART_Receive(&huart1, CommandUart, 8,HAL_MAX_DELAY) == HAL_OK)   //100
			{
				retryStatus=false;
				if(CommandUart[0]!=0){
					communicationSemaphoreVal = osSemaphoreAcquire(ThreadStorage.CommunicationSemaphoreHandle, 1000);
					if(communicationSemaphoreVal==osOK){
						uint8_t data[8];
						for (int i = 0; i < 8; ++i) {
							data[i]=CommandUart[i];
							CommandUart[i]=0;
						}
						CommunicationInstance.ProcessCommand(data);
						osSemaphoreRelease(ThreadStorage.CommunicationSemaphoreHandle);
						//osThreadTerminate(ThreadStorage.SendUARTThreadId);
						//ThreadStorage.SendUARTThreadId=NULL;
					}else{
						retryStatus=true;
					}

				}
				else{
					uint16_t* len=&huart1.RxXferSize;
					uint8_t xData[sizeof(len)];
					HAL_UART_Receive(&huart1, xData, sizeof(len),HAL_MAX_DELAY);
					SuccessDataResult(100, SuccessDataType::SD_RepeatAgain, xData, sizeof(len));
				}
			vTaskDelay(xDelay);
		}*/
	}
}
void WriteUart(uint8_t *data,int len){
	  uint8_t count=0;
	  HAL_StatusTypeDef status=HAL_UART_Transmit(&huart1, data,len ,HAL_MAX_DELAY);
	  if(status!=HAL_OK && count<5)
	  {
		  status=HAL_UART_Transmit(&huart1, data,len ,HAL_MAX_DELAY);
		  count++;
	  }
	  /*if(status==HAL_OK){
		  uint8_t nl = 0x0A;
		  HAL_UART_Transmit(&huart1, &nl, 1, HAL_MAX_DELAY);
	  }*/
}
void WriteUartSafe(uint8_t *data,int len){
	osStatus_t communicationSemaphoreVal;
	communicationSemaphoreVal = osSemaphoreAcquire(ThreadStorage.CommunicationSemaphoreHandle, 1000);
	if(communicationSemaphoreVal==osOK){
		  uint8_t count=0;
		  HAL_StatusTypeDef status=HAL_UART_Transmit(&huart1, data,len ,(1000/SystemConfig.SendPerSecond));
		  if(status!=HAL_OK && count<5)
		  {
			  status=HAL_UART_Transmit(&huart1, data,len ,(1000/SystemConfig.SendPerSecond));
			  count++;
		  }
		osSemaphoreRelease(ThreadStorage.CommunicationSemaphoreHandle);
	}
}
/* USER CODE BEGIN Header_StartSendUARTTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartSendUARTTask */
void StartSendUARTTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  // uint8_t data[] = "SendTask\r\n";
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xFrequency = (1000/SystemConfig.SendPerSecond);
	osStatus_t firstEmgSemaphoreVal;
	osStatus_t secondEmgSemaphoreVal;
	osStatus_t volumeSemaphoreVal;
	osStatus_t flowSemaphoreVal;
	osStatus_t communicationSemaphoreVal;
  for(;;)
  {
	  uint16_t totalLen=12;
	  uint8_t allData[4096];
	  firstEmgSemaphoreVal = osSemaphoreAcquire(ThreadStorage.FirstEmgSemaphoreHandle,xFrequency/10);
	  if(firstEmgSemaphoreVal==osOK){
		  int xLen=EmgInstance.FirstEmgBufferLen;
		  if(xLen>0){
			  allData[totalLen]=1;
			  totalLen++;
			  allData[totalLen]=(xLen & 0xFF00)>>8;
			  totalLen++;
			  allData[totalLen]=xLen & 0xFF;
			  totalLen++;
			   for (int i=0;i < xLen; ++i) {
				   allData[totalLen]=EmgInstance.FirstEmgBuffer[i];
				   totalLen++;
				   EmgInstance.FirstEmgBuffer[i]=0;
			   }
			   EmgInstance.FirstEmgBufferLen=0;
		  }
		  osSemaphoreRelease(ThreadStorage.FirstEmgSemaphoreHandle);
	  }
	  secondEmgSemaphoreVal = osSemaphoreAcquire(ThreadStorage.SecondEmgSemaphoreHandle, xFrequency/10);
	  if(secondEmgSemaphoreVal==osOK){
		  int xLen=EmgInstance.SecondEmgBufferLen;
		  if(xLen>0){
			  allData[totalLen]=2;
			  totalLen++;
			  allData[totalLen]=(xLen & 0xFF00)>>8;
			  totalLen++;
			  allData[totalLen]=xLen & 0xFF;
			  totalLen++;
			   for (int i=0;i < xLen; ++i) {
				   allData[totalLen]=EmgInstance.SecondEmgBuffer[i];
				   totalLen++;
				   EmgInstance.SecondEmgBuffer[i]=0;
			   }
			   EmgInstance.SecondEmgBufferLen=0;
		  }
		  osSemaphoreRelease(ThreadStorage.SecondEmgSemaphoreHandle);
	  }
	  volumeSemaphoreVal = osSemaphoreAcquire(ThreadStorage.VolumeSemaphoreHandle, xFrequency/10);
	  if(volumeSemaphoreVal==osOK){
		  int xLen=LoadCellInstance.VolumeBufferLen;
		  if(xLen>0){
			  allData[totalLen]=3;
			  totalLen++;
			  allData[totalLen]=(xLen & 0xFF00)>>8;
			  totalLen++;
			  allData[totalLen]=xLen & 0xFF;
			  totalLen++;
			   for (int i=0;i < xLen; ++i) {
				   allData[totalLen]=LoadCellInstance.VolumeBuffer[i];
				   totalLen++;
				   LoadCellInstance.VolumeBuffer[i]=0;
			   }
			   LoadCellInstance.VolumeBufferLen=0;
		  }
		  osSemaphoreRelease(ThreadStorage.VolumeSemaphoreHandle);
	  }
	  flowSemaphoreVal = osSemaphoreAcquire(ThreadStorage.FlowSemaphoreHandle, xFrequency/10);
	  if(flowSemaphoreVal==osOK){
		  int xLen=LoadCellInstance.FlowBufferLen;
		  if(xLen>0){
			  allData[totalLen]=4;
			  totalLen++;
			  allData[totalLen]=(xLen & 0xFF00)>>8;
			  totalLen++;
			  allData[totalLen]=xLen & 0xFF;
			  totalLen++;
			   for (int i=0;i < xLen; ++i) {
				   allData[totalLen]=LoadCellInstance.FlowBuffer[i];
				   totalLen++;
				   LoadCellInstance.FlowBuffer[i]=0;
			   }
			   LoadCellInstance.FlowBufferLen=0;
		  }
		  osSemaphoreRelease(ThreadStorage.FlowSemaphoreHandle);
	  }
	  if(totalLen>11){
		  uint16_t len=totalLen-3;
		  allData[0]=3;
		  allData[1]=(len & 0xFF00)>>8;
		  allData[2]=len & 0xFF;
		  allData[3]=(SystemConfig.PocketIndex & 0xFF000000) >> 24;
		  allData[4]=(SystemConfig.PocketIndex & 0x00FF0000) >> 16;
		  allData[5]=(SystemConfig.PocketIndex & 0x0000FF00) >> 8;
		  allData[6]=(SystemConfig.PocketIndex & 0x000000FF);
		  uint32_t time=(StartTimerTicks-SystemConfig.StartTestTime)/10;
		  allData[7]=(time & 0xFF000000) >> 24;
		  allData[8]=(time & 0x00FF0000) >> 16;
		  allData[9]=(time & 0x0000FF00) >> 8;
		  allData[10]=(time & 0x000000FF);
		  uint8_t status=0;
		  if(HasCalibration()){
			  status+=1;
		  }
		  if(HasLoadcell(false)){
			  status+=2;
		  }
		  if(HasFirstEmg(false)){
			  status+=4;
		  }
		  if(HasSecondEmg(false)){
			  status+=8;
		  }
		  allData[11]=status;
		  if(SystemConfig.IsStartTest){
			  if(SystemConfig.StartHandleSeconds>0&&SystemConfig.StartHandleSeconds*1000<time&&LoadCellInstance.IsFirstHandle==false){
				  //SystemConfig.IsInternalClean=true;
				  //CommunicationInstance.StopTest(5000);
				  CommunicationInstance.cancelTest();
				continue;
			  }
			  if(SystemConfig.WaitAfterProcessSeconds>0&&SystemConfig.WaitAfterProcessSeconds*10000<StartTimerTicks-LoadCellInstance.LastHandleProcessTime&&LoadCellInstance.IsFirstHandle==true){
				  SystemConfig.IsInternalClean=true;
				  CommunicationInstance.StopTest(5000);
				  //CommunicationInstance.cancelTest();
				continue;
			  }
			  communicationSemaphoreVal = osSemaphoreAcquire(ThreadStorage.CommunicationSemaphoreHandle, 1000);
			  if(communicationSemaphoreVal==osOK){
				  WriteUart(allData,totalLen);
				  osSemaphoreRelease(ThreadStorage.CommunicationSemaphoreHandle);
			  }
		  }
		SystemConfig.PocketIndex++;
	  }
	  vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
  /* USER CODE END 5 */
}


/* USER CODE BEGIN Header_StartFirstEmgTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartFirstEmgTask */
void StartFirstEmgTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	const TickType_t xDelay = (1000/SystemConfig.FirstEmgPerSecond) / portTICK_PERIOD_MS;
	ThreadStorage.FirstEmgSemaphoreHandle = osSemaphoreNew(1, 1, &firstEmgSemaphore_attributes);
	osStatus_t emgSemaphoreVal;
	for(;;)
	{
		emgSemaphoreVal = osSemaphoreAcquire(ThreadStorage.FirstEmgSemaphoreHandle, 1000);
		if(emgSemaphoreVal==osOK){
			firstEmgCounter++;
			EmgInstance.FirstEmgRead(true);
			osSemaphoreRelease(ThreadStorage.FirstEmgSemaphoreHandle);
			vTaskDelay( xDelay );
		}
	}
  /* USER CODE END 5 */
}

void StartReadFirstEmgTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	const TickType_t xDelay = (1000/SystemConfig.FirstEmgPerSecond) / portTICK_PERIOD_MS;
	ThreadStorage.FirstEmgSemaphoreHandle = osSemaphoreNew(1, 1, &firstEmgSemaphore_attributes);
	osStatus_t emgSemaphoreVal;
	uint8_t data[8];
	SystemConfig.StartTestTime=StartTimerTicks;
	for(;;)
	{
		emgSemaphoreVal = osSemaphoreAcquire(ThreadStorage.FirstEmgSemaphoreHandle, 1000);
		if(emgSemaphoreVal==osOK){
			EmgInstance.FirstEmgRead(false);
			float32_t emgValue=EmgInstance.FirstEmgValue;
			uint8_t *emgArray;
			emgArray = (uint8_t*)(&emgValue );
			uint32_t time=(StartTimerTicks-SystemConfig.StartTestTime)/10;
			data[0]=(time & 0xFF000000) >> 24;
			data[1]=(time & 0x00FF0000) >> 16;
			data[2]=(time & 0x0000FF00) >> 8;
			data[3]=(time & 0x000000FF);
			data[4]=emgArray[0];
			data[5]=emgArray[1];
			data[6]=emgArray[2];
			data[7]=emgArray[3];
			//WriteUart(emgArray, 4);
			SuccessDataResult(0, SuccessDataType::SD_FirstEmg, data, 8);
			osSemaphoreRelease(ThreadStorage.FirstEmgSemaphoreHandle);
		}
		vTaskDelay( xDelay );
	}
  /* USER CODE END 5 */
}


/* USER CODE BEGIN Header_StartSecondEmgTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartSecondEmgTask */
void StartSecondEmgTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	const TickType_t xDelay = (1000/SystemConfig.SecondEmgPerSecond) / portTICK_PERIOD_MS;
	ThreadStorage.SecondEmgSemaphoreHandle = osSemaphoreNew(1, 1, &secondEmgSemaphore_attributes);
	osStatus_t emgSemaphoreVal;
	for(;;)
	{
		emgSemaphoreVal = osSemaphoreAcquire(ThreadStorage.SecondEmgSemaphoreHandle, 1000);
		if(emgSemaphoreVal==osOK){
			secondEmgCounter++;
			EmgInstance.SecondEmgRead(true);
			osSemaphoreRelease(ThreadStorage.SecondEmgSemaphoreHandle);
		}
		vTaskDelay( xDelay );
	}
  /* USER CODE END 5 */
}

void StartReadSecondEmgTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	const TickType_t xDelay = (1000/SystemConfig.SecondEmgPerSecond) / portTICK_PERIOD_MS;
	ThreadStorage.SecondEmgSemaphoreHandle = osSemaphoreNew(1, 1, &secondEmgSemaphore_attributes);
	osStatus_t emgSemaphoreVal;
	uint8_t data[8];
	SystemConfig.StartTestTime=StartTimerTicks;
	for(;;)
	{
		emgSemaphoreVal = osSemaphoreAcquire(ThreadStorage.SecondEmgSemaphoreHandle, 1000);
		if(emgSemaphoreVal==osOK){
			EmgInstance.SecondEmgRead(false);
			float32_t emgValue=EmgInstance.SecondEmgValue;
			uint8_t *emgArray;
			emgArray = (uint8_t*)(&emgValue );
			uint32_t time=(StartTimerTicks-SystemConfig.StartTestTime)/10;
			data[0]=(time & 0xFF000000) >> 24;
			data[1]=(time & 0x00FF0000) >> 16;
			data[2]=(time & 0x0000FF00) >> 8;
			data[3]=(time & 0x000000FF);
			data[4]=emgArray[0];
			data[5]=emgArray[1];
			data[6]=emgArray[2];
			data[7]=emgArray[3];
			SuccessDataResult(0, SuccessDataType::SD_SecondEmg, data, 8);
			osSemaphoreRelease(ThreadStorage.SecondEmgSemaphoreHandle);
		}
		vTaskDelay( xDelay );
	}
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartFlowTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartFlowTask */
void StartFlowTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	const TickType_t xDelay = (1000/SystemConfig.FlowPerSecond) / portTICK_PERIOD_MS;
	ThreadStorage.FlowSemaphoreHandle = osSemaphoreNew(1, 1, &flowSemaphore_attributes);
	osStatus_t flowSemaphoreVal;
	for(;;)
	{
		flowSemaphoreVal = osSemaphoreAcquire(ThreadStorage.FlowSemaphoreHandle, 1000);
		if(flowSemaphoreVal==osOK){
			flowCounter++;
			LoadCellInstance.ReadFlow(true);
			osSemaphoreRelease(ThreadStorage.FlowSemaphoreHandle);
		}
		vTaskDelay( xDelay );
	}
  /* USER CODE END 5 */
}
void StartReadFlowTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	LoadCellInstance.ClearSamples();
	const TickType_t xDelay = (1000/SystemConfig.FlowPerSecond) / portTICK_PERIOD_MS;
	ThreadStorage.FlowSemaphoreHandle = osSemaphoreNew(1, 1, &flowSemaphore_attributes);
	osStatus_t flowSemaphoreVal;
	float32_t avg=SystemConfig.FlowAverage;
	uint8_t *avgArray;
	avgArray = (uint8_t*)(&avg );
	uint8_t data[12];
	data[0]=avgArray[0];
	data[1]=avgArray[1];
	data[2]=avgArray[2];
	data[3]=avgArray[3];
	for(;;)
	{
		flowSemaphoreVal = osSemaphoreAcquire(ThreadStorage.FlowSemaphoreHandle, 1000);
		if(flowSemaphoreVal==osOK){
			LoadCellInstance.ReadFlow(false);
			float32_t rate=SystemConfig.FlowRate;
			uint8_t *rateArray;
			rateArray = (uint8_t*)(&rate );
			float32_t flow=Debugger.LastFlow;//LoadCellInstance.LastReadFlowValue; //250926
			uint8_t *flowArray;
			flowArray = (uint8_t*)(&flow );
			data[4]=rateArray[0];
			data[5]=rateArray[1];
			data[6]=rateArray[2];
			data[7]=rateArray[3];
			data[8]=flowArray[0];
			data[9]=flowArray[1];
			data[10]=flowArray[2];
			data[11]=flowArray[3];
			SuccessDataResult(0, SuccessDataType::SD_Flow, data, 12);
			osSemaphoreRelease(ThreadStorage.FlowSemaphoreHandle);
		}
		vTaskDelay( xDelay );
	}
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartVolumeTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartVolumeTask */
void StartVolumeTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	LoadCellInstance.ClearSamples();
	const TickType_t xDelay = (1000/SystemConfig.VolumePerSecond) / portTICK_PERIOD_MS;
	ThreadStorage.VolumeSemaphoreHandle = osSemaphoreNew(1, 1, &volumeSemaphore_attributes);
	osStatus_t volumeSemaphoreVal;
	for(;;)
	{
		volumeSemaphoreVal = osSemaphoreAcquire(ThreadStorage.VolumeSemaphoreHandle, 1000);
		if(volumeSemaphoreVal==osOK){
			volumeCounter++;
			LoadCellInstance.ReadVolume(true);
			osSemaphoreRelease(ThreadStorage.VolumeSemaphoreHandle);
			vTaskDelay( xDelay );
		}
	}
  /* USER CODE END 5 */
}

void StartReadVolumeTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	LoadCellInstance.ClearSamples();
	const TickType_t xDelay = (1000/SystemConfig.VolumePerSecond) / portTICK_PERIOD_MS;
	ThreadStorage.VolumeSemaphoreHandle = osSemaphoreNew(1, 1, &volumeSemaphore_attributes);
	osStatus_t volumeSemaphoreVal;
	float32_t avg=SystemConfig.VolumeAverage;
	uint8_t *avgArray;
	avgArray = (uint8_t*)(&avg );
	uint8_t data[12];
	data[0]=avgArray[0];
	data[1]=avgArray[1];
	data[2]=avgArray[2];
	data[3]=avgArray[3];
	for(;;)
	{
		volumeSemaphoreVal = osSemaphoreAcquire(ThreadStorage.VolumeSemaphoreHandle, 1000);
		if(volumeSemaphoreVal==osOK){
			LoadCellInstance.ReadVolume(false);
			float32_t rate=SystemConfig.VolumeRate;
			uint8_t *rateArray;
			rateArray = (uint8_t*)(&rate );
			float32_t volume=Debugger.LastVolume;//LoadCellInstance.LastReadVolumeValue; //250926
			uint8_t *volumeArray;
			volumeArray = (uint8_t*)(&volume );
			data[4]=rateArray[0];
			data[5]=rateArray[1];
			data[6]=rateArray[2];
			data[7]=rateArray[3];
			data[8]=volumeArray[0];
			data[9]=volumeArray[1];
			data[10]=volumeArray[2];
			data[11]=volumeArray[3];
			SuccessDataResult(0, SuccessDataType::SD_Volume, data, 12);
			osSemaphoreRelease(ThreadStorage.VolumeSemaphoreHandle);
		}
		vTaskDelay( xDelay );
	}
  /* USER CODE END 5 */
}


/* USER CODE BEGIN Header_StartAutoClosePumpTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartAutoClosePumpTask */
void StartAutoClosePumpTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	const TickType_t xDelay = (1000*SystemConfig.PumpMaxRunSecond) / portTICK_PERIOD_MS;
	for(;;)
	{
		vTaskDelay( xDelay );
		CommunicationInstance.TogglePump(false);
	}
  /* USER CODE END 5 */
}
void StartAutoCloseValveTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
	const TickType_t xDelay = (1000*SystemConfig.ValveMaxRunSecond) / portTICK_PERIOD_MS;
	for(;;)
	{
		vTaskDelay( xDelay );
		CommunicationInstance.ToggleValve(false);
		osThreadTerminate(ThreadStorage.ValveMaxRunThreadId);
		ThreadStorage.ValveMaxRunThreadId=NULL;
	}
  /* USER CODE END 5 */
}
void StartLoadcellAverageTask(void *argument){

	const TickType_t xDelay = 100 / portTICK_PERIOD_MS;
	float32_t sumFlow = 0;
	float32_t sumVolume = 0;
	uint16_t instanceCount=0;
	uint8_t jumpCount=50;
	uint8_t percent=0;
	uint8_t data[8];
	LoadCellInstance.ClearSamples();
	for(;;)
	{
		LoadCellInstance.ReadVolumeAndFlow();
		if(jumpCount>0){
			jumpCount--;
			vTaskDelay( xDelay );
			continue;
		}
		sumFlow+=LoadCellInstance.FlowValue;
		sumVolume+=LoadCellInstance.VolumeValue;
		uint8_t *volumeArray;
		volumeArray = (uint8_t*)(&LoadCellInstance.VolumeValue );
		uint8_t *flowArray;
		flowArray = (uint8_t*)(&LoadCellInstance.FlowValue );
		data[0]=volumeArray[0];
		data[1]=volumeArray[1];
		data[2]=volumeArray[2];
		data[3]=volumeArray[3];
		data[4]=flowArray[0];
		data[5]=flowArray[1];
		data[6]=flowArray[2];
		data[7]=flowArray[3];
		instanceCount++;
		percent=((instanceCount*100)/SystemConfig.AverageSampleCount)-1;
		Debugger.AveragePercent=percent;
		SuccessDataResult(percent, SuccessDataType::SD_MeasurementAverage, data, 8);
		if(instanceCount>=SystemConfig.AverageSampleCount){
			SystemConfig.VolumeAverage=sumVolume/instanceCount;
			SystemConfig.FlowAverage=sumFlow/(instanceCount-1);
			FlashManagerInstance.WriteVolumeAverage(SystemConfig.VolumeAverage);
			FlashManagerInstance.WriteFlowAverage(SystemConfig.FlowAverage);
			SuccessDataResult(100, SuccessDataType::SD_MeasurementAverage, data, 8);
			SendFeedback(RequestType::R_System, SystemRequestType::SYSR_MeasurementAverage, ProcessStatuses::PS_End);
			SystemConfig.systemMode=SystemModes::EmptyMode;
			osThreadTerminate(ThreadStorage.LoadcellAverageThreadId);
			ThreadStorage.LoadcellAverageThreadId=NULL;
		}
		vTaskDelay( xDelay );
	}
}
void StartCalibrationVolumeTask(void *argument){

	const TickType_t xDelay = 100 / portTICK_PERIOD_MS;
	float32_t sumVolume = 0;
	uint16_t instanceCount=0;
	uint8_t jumpCount=50;
	uint8_t percent=0;
	for(;;)
	{
		LoadCellInstance.ReadVolumeAndFlow();
		if(jumpCount>0){
			jumpCount--;
			vTaskDelay( xDelay );
			continue;
		}
		float32_t diff =LoadCellInstance.VolumeValue-SystemConfig.VolumeAverage;
		if(diff<1){
			vTaskDelay( xDelay );
			continue;
		}
		sumVolume+=diff;
		float32_t rate=(sumVolume/instanceCount)/SystemConfig.CalibrationWeight;
		Debugger.VolumeRate=rate;
		uint8_t *rateArray;
		rateArray = (uint8_t*)(&rate );
		instanceCount++;
		percent=((instanceCount*100)/SystemConfig.VolumeCalibrationSampleCount)-1;
		Debugger.AveragePercent=percent;
		SuccessDataResult(percent, SuccessDataType::SD_VolumeCalibration, rateArray, 4);
		if(instanceCount>=SystemConfig.VolumeCalibrationSampleCount){
			SystemConfig.VolumeRate=((double_t)sumVolume/(double_t)instanceCount)/SystemConfig.CalibrationWeight;
			FlashManagerInstance.WriteVolumeRate(SystemConfig.VolumeRate);
			SuccessDataResult(100, SuccessDataType::SD_VolumeCalibration, rateArray, 4);
			SendFeedback(RequestType::R_System, SystemRequestType::SYSR_CalibrationVolume, ProcessStatuses::PS_End);
			SystemConfig.systemMode=SystemModes::EmptyMode;
			osThreadTerminate(ThreadStorage.CalibrationVolumeThreadId);
			ThreadStorage.CalibrationVolumeThreadId=NULL;
		}
		vTaskDelay( xDelay );
	}
}
void StartCalibrationFlowTask(void *argument){

	const TickType_t xDelay = 100 / portTICK_PERIOD_MS;
	float32_t maxFlow=1;
	uint16_t instanceCount=0;
	uint8_t jumpCount=50;
	uint8_t percent=0;
	for(;;)
	{
		LoadCellInstance.ReadVolumeAndFlow();
		if(jumpCount>0){
			jumpCount--;
			vTaskDelay( xDelay );
			continue;
		}
		if(SystemConfig.FlowAverage>LoadCellInstance.FlowValue){
			LoadCellInstance.FlowValue=SystemConfig.FlowAverage;
		}
		uint32_t flow=LoadCellInstance.FlowValue-SystemConfig.FlowAverage;
		if(flow>maxFlow){
			maxFlow=flow;
		}
		float32_t rate=maxFlow/SystemConfig.CalibrationFlow;
		Debugger.FlowRate=rate;
		instanceCount++;
		uint8_t *rateArray;
		rateArray = (uint8_t*)(&rate );
		percent=((instanceCount*100)/SystemConfig.FlowCalibrationSampleCount)-1;
		SuccessDataResult(percent, SuccessDataType::SD_FlowCalibration, rateArray, 4);
		if(instanceCount>SystemConfig.FlowCalibrationSampleCount){
			SystemConfig.FlowRate=(double_t)maxFlow/SystemConfig.CalibrationFlow;
			FlashManagerInstance.WriteFlowRate(SystemConfig.FlowRate);
			SystemConfig.systemMode=SystemModes::EmptyMode;
			SuccessDataResult(100, SuccessDataType::SD_FlowCalibration, rateArray, 4);
			SendFeedback(RequestType::R_System, SystemRequestType::SYSR_CalibrationFlow, ProcessStatuses::PS_End);
			osThreadTerminate(ThreadStorage.CalibrationFlowThreadId);
			ThreadStorage.CalibrationFlowThreadId=NULL;
		}
		vTaskDelay( xDelay );
	}
}
void StartCleanTask(void *argument){

	const TickType_t xDelay = 100 / portTICK_PERIOD_MS;
	uint16_t clearedCount=0;
	volatile uint8_t type=0;
	LoadCellInstance.ClearSamples();
	CommunicationInstance.TogglePump(true);
	if(ThreadStorage.PumpMaxRunThreadId!=NULL&&ThreadStorage.PumpMaxRunThreadId!=0x00){
		osThreadTerminate(ThreadStorage.PumpMaxRunThreadId);
		ThreadStorage.PumpMaxRunThreadId=NULL;
	}
	for(;;)
	{
		LoadCellInstance.ReadVolumeAndFlow();
		Debugger.ReadedVolume=LoadCellInstance.VolumeValue;
		if(LoadCellInstance.VolumeValue>LoadCellInstance.VolumeValue){
			Debugger.DiffVolumeValue=LoadCellInstance.VolumeValue-(SystemConfig.VolumeAverage+500);
		}else{
			Debugger.DiffVolumeValue=(SystemConfig.VolumeAverage+10000)-LoadCellInstance.VolumeValue;
		}
		Debugger.DiffVolumeValue=LoadCellInstance.VolumeValue-(SystemConfig.VolumeAverage+10000);
		Debugger.CleanType=type;
		if(type==0||type==2){
			if(SystemConfig.VolumeAverage+10000>LoadCellInstance.VolumeValue){
				clearedCount++;
			}
			if(clearedCount>5){
				if(type==0){
					CommunicationInstance.ToggleValve(true);
					if(SystemConfig.IsInternalClean){
						HAL_Delay(SystemConfig.CleanTime);
					}
					else{
						vTaskDelay( SystemConfig.CleanTime );
					}
				}else{
					CommunicationInstance.TogglePump(false);
				}
				type++;
				clearedCount=0;
			}
		}
		else if(type==1){
			CommunicationInstance.ToggleValve(false);
			CommunicationInstance.TogglePump(true);
			if(ThreadStorage.PumpMaxRunThreadId!=NULL&&ThreadStorage.PumpMaxRunThreadId!=0x00){
				osThreadTerminate(ThreadStorage.PumpMaxRunThreadId);
				ThreadStorage.PumpMaxRunThreadId=NULL;
			}
			type++;
		}
		else if(type==3){
			if(!SystemConfig.IsInternalClean){
				SendFeedback(RequestType::R_Command, CommandRequestType::CMDR_Clean, ProcessStatuses::PS_End);
			}
			SystemConfig.systemMode=SystemModes::EmptyMode;
			if(ThreadStorage.CleanThreadId!=0x00&&ThreadStorage.CleanThreadId!=NULL){
				osThreadTerminate(ThreadStorage.CleanThreadId);
				ThreadStorage.CleanThreadId=NULL;
			}
			break;
		}
		if(SystemConfig.IsInternalClean){
			HAL_Delay(xDelay);
		}
		else{
			vTaskDelay( xDelay );
		}
	}
}

void StartSafeModeTask(void *argument){

	const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
	const TickType_t xDelay2 = 100 / portTICK_PERIOD_MS;
	uint8_t type=0;
	uint8_t clearedCount=0;
	for(;;)
	{
		LoadCellInstance.ReadVolumeAndFlow();
		if(type==0){
			if(SystemConfig.VolumeAverage+5000<LoadCellInstance.VolumeValue){
				CommunicationInstance.TogglePump(true);
				osThreadTerminate(ThreadStorage.PumpMaxRunThreadId);
				ThreadStorage.PumpMaxRunThreadId=NULL;
				type++;
			}
			vTaskDelay( xDelay );
		}
		else if(type==1){
			if(SystemConfig.VolumeAverage>LoadCellInstance.VolumeValue|| SystemConfig.VolumeAverage+5000>=LoadCellInstance.VolumeValue){
				clearedCount++;
			}
			if(clearedCount>3){
				CommunicationInstance.TogglePump(false);
				type=0;
				clearedCount=0;
			}
			vTaskDelay( xDelay2 );
		}
	}
}
void FixVolume(void){
	ClearLoadcellParams();
	uint8_t i=0;
	float32_t maxReaded=0;
	while(i<10){
		HAL_Delay(10);
		LoadCellInstance.ReadVolume(false);
		if(LoadCellInstance.LastReadVolumeValue>maxReaded){
			maxReaded=LoadCellInstance.LastReadVolumeValue;
		}
		i++;
	}
	if(maxReaded>SystemConfig.VolumeAverage){
		SystemConfig.VolumeAverage=maxReaded;
	}
}
void ClearLoadcellParams(){
	LoadCellInstance.LastFlowValue=0;
	LoadCellInstance.LastVolumeValue=0;
	LoadCellInstance.ClearParams();
}
FlashManager GetFlashManager(){
	return FlashManagerInstance;
}
uint8_t HasCalibration(void){

	if(SystemConfig.FlowRate<2 || SystemConfig.VolumeRate<2){
		return 0;
	}
	return 1;
}
uint8_t HasLoadcell(bool read){

	if(read){
		LoadCellInstance.ReadVolumeAndFlow();
	}
	if(LoadCellInstance.VolumeValue==1310680){
		return 0;
	}
	return 1;
}
uint8_t HasFirstEmg(bool read){

	if(read){
		EmgInstance.FirstEmgRead(false);
	}
	if(EmgInstance.NonFilterFirstEmg==65535||EmgInstance.NonFilterFirstEmg==0){
		return 0;
	}
	return 1;
}
uint8_t HasSecondEmg(bool read){
	if(read){
		EmgInstance.SecondEmgRead(false);
	}
	if(EmgInstance.NonFilterSecondEmg==65535||EmgInstance.NonFilterSecondEmg==0){
		return 0;
	}
	return 1;
}
