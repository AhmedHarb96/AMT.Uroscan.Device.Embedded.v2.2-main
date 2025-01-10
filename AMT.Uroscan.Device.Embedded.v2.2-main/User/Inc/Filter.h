/*
 * Filter.h
 *
 *  Created on: Aug 19, 2024
 *      Author: OrioN
 */

#ifndef INC_FILTER_H_
#define INC_FILTER_H_


typedef struct {
	float32_t Alpha; //0.8f;
	float32_t Input[2];
	float32_t Output[2];
	float32_t FilteredValue;

} HighPassFilterStruct;


typedef struct {
	float32_t AlphaLowPass; //= 0.2;
	float32_t PreviousFilteredValue;

} LowPassFilterStruct;


class Filter {
public:
	Filter();
	virtual ~Filter();
	void HighPassFilterInit(HighPassFilterStruct *highPassFilterStruct, float32_t alpha, float32_t cutOffFreq,float32_t samplingFreq);
	float32_t HighPassFilter(HighPassFilterStruct *highPassFilterStruct, float32_t inputValue);
	void LowPassFilterInit(LowPassFilterStruct *lowPassFilterStruct, float32_t cutOffFrequency, float32_t sampleFrequency);
	float32_t LowPassFilter(LowPassFilterStruct *lowPassFilterStruct, float32_t currentValue, float32_t &previousFilteredValue);
	float32_t MoovingAverage(float32_t *dataArray, float32_t *sumData, uint32_t index, uint32_t len, float32_t nextValue);
};

#endif /* INC_FILTER_H_ */
