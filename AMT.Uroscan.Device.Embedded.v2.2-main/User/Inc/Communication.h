/*
 * Communication.h
 *
 *  Created on: Aug 6, 2024
 *      Author: OrioN
 */

#ifndef INC_COMMUNICATION_H_
#define INC_COMMUNICATION_H_

class Communication {
public:
	Communication();
	virtual ~Communication();
	void ProcessCommand(uint8_t *command);
	void System(uint8_t *command);
	void Command(uint8_t *command);
	void Configuration(uint8_t *command);
	void ToggleFirstEmg(bool isStart);
	void ToggleSecondEmg(bool isStart);
	void ToggleLoadCell(bool isStart);
	void TogglePump(bool isStart);
	void ConfigPump(uint16_t timeout);
	void ToggleValve(bool isStart);
	void ConfigValve(uint16_t timeout);
	void ToggleDataStream(bool isStart);
	void StartClean(uint16_t cleanTime);
	void StartTest(bool isStartFirstEmg, bool isStartSecondEmg, bool isStartLoadcell, uint16_t cleanTime, uint8_t startHandleSeconds);
	void ToggleReadVolume(bool isStart);
	void ToggleReadFlow(bool isStart);
	void ToggleReadFirstEmg(bool isStart);
	void ToggleReadSecondEmg(bool isStart);
	void StopTest(uint16_t cleanTime);
	void ToggleSafeMode(bool isStart);
	void LoadcellAverage();
	void LoadcellVolumeCalibration(uint16_t weight);
	void LoadcellFlowCalibration(uint16_t maxFlow);
	void SetFlashManager(FlashManager flashManager);
private:
	FlashManager FMI;
};

#endif /* SRC_COMMUNICATION_H_ */
