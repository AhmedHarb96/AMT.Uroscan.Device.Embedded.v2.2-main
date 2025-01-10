/*
 * Emg.h
 *
 *  Created on: Aug 1, 2024
 *      Author: OrioN
 */

#ifndef INC_EMG_H_
#define INC_EMG_H_




#define NUMBER_OG_EMG_CHANNEL					2
#define WIDTH									2
#define BIT_16 									0xFFFF
#define VREF    								2.37F
#define BITS    								16
#define GAIN    								1
#define SINC_ORDER    							3
#define CHAR_BIT								8


typedef struct {

	float FirstChannel;
	float SecondChannel;
	unsigned int FirstChannelCounter;
	unsigned int SecondChannelCounter;
	float VoltPerCount;

} EmgModeParamType;


class Emg{
public:
	Emg();
	virtual ~Emg();
	void FirstEmgSetup(void);
	void SecondEmgSetup(void);
	void FirstEmgRead(bool useBuffer);
	void SecondEmgRead(bool useBuffer);
	void FirstEmgWriteToSPI(uint16_t emgSPIAddress, uint16_t emgValue);
	void SecondEmgWriteToSPI(uint16_t emgSPIAddress, uint16_t emgValue);
	void FirstEmgReadFromSPI(uint8_t emgValue);
	void SecondEmgReadFromSPI(uint8_t emgValue);
	uint32_t FirstEmgMovingAverageFilter(uint32_t newSample);
	uint32_t SecondEmgMovingAverageFilter(uint32_t newSample);
	uint32_t ReadFirstEmgValueArray[3];
	uint32_t ReadSecondEmgValueArray[3];
	uint8_t  FirstEmgReadSPIValue[1];
	uint8_t  SecondEmgReadSPIValue[1];
	uint32_t FirstEmgValue;
	uint32_t SecondEmgValue;
	uint32_t FirstEmgBuffer[1024];
	uint32_t SecondEmgBuffer[1024];
	int FirstEmgBufferLen;
	int SecondEmgBufferLen;
private:
	const float alpha = 0.5;
	uint32_t LastFirstEmg = 0;
	uint32_t LastSecondEmg = 0;

};

#endif /* INC_EMG_H_ */
