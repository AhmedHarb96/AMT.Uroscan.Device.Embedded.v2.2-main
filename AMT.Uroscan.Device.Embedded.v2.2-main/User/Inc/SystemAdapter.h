/*
 * SystemAdapter.h
 *
 *  Created on: Aug 2, 2024
 *      Author: OrioN
 */

#ifndef INC_SYSTEMADAPTER_H_
#define INC_SYSTEMADAPTER_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "main.h"


extern volatile uint32_t StartTimerTicks;

void SetupOS(void);
void StartOS(void);
void HardReset(void);
void FlashInitialize(void);
void WriteUart(uint8_t *data,int len);
void WriteUartSafe(uint8_t *data,int len);
void SuccessResult(void);
void SuccessDataResult(uint8_t percent,uint8_t dataType,uint8_t *data,int len);
void ErrorDataResult(uint8_t percent, uint8_t opCode,uint16_t errorCode);
void ErrorResult(uint8_t opCode,uint16_t errorCode);
void StartReadUARTTask(void *argument);
void StartSendUARTTask(void *argument);
void StartFirstEmgTask(void *argument);
void StartSecondEmgTask(void *argument);
void StartVolumeTask(void *argument);
void StartFlowTask(void *argument);
void StartReadVolumeTask(void *argument);
void StartReadFlowTask(void *argument);
void StartReadFirstEmgTask(void *argument);
void StartReadSecondEmgTask(void *argument);
void StartAutoClosePumpTask(void *argument);
void StartAutoCloseValveTask(void *argument);
void StartLoadcellAverageTask(void *argument);
void StartCalibrationVolumeTask(void *argument);
void StartCalibrationFlowTask(void *argument);
void StartCleanTask(void *argument);
void StartSafeModeTask(void *argument);
void ClearLoadcellParams();
uint8_t HasCalibration(void);
uint8_t HasLoadcell(void);
uint8_t HasFirstEmg(void);
uint8_t HasSecondEmg(void);

#ifdef __cplusplus
}
#endif
#endif /* INC_SYSTEMADAPTER_H_ */
