/*
 * Filter.cpp
 *
 *  Created on: Aug 19, 2024
 *      Author: OrioN
 */

#include "../Inc/GeneralHeaders.h"

Filter::Filter() {
	// TODO Auto-generated constructor stub

}

Filter::~Filter() {
	// TODO Auto-generated destructor stub
}

void Filter::HighPassFilterInit(HighPassFilterStruct *highPassFilterStruct, float32_t alpha, float32_t cutOffFreq,float32_t samplingFreq){

	alpha = (1.0f / (cutOffFreq * 2.0f * PI))
			/ ((1.0f / (cutOffFreq * 2.0f * PI)) + (1.0f / samplingFreq));

	highPassFilterStruct->Alpha =  0.8;

	highPassFilterStruct->Input[0] = 0.0f;
	highPassFilterStruct->Input[1] = 0.0f;
	highPassFilterStruct->Output[0] = 0.0f;
	highPassFilterStruct->Output[1] = 0.0f;
}
float32_t Filter::HighPassFilter(HighPassFilterStruct *highPassFilterStruct, float32_t inputValue) {

	highPassFilterStruct->Input[1] = highPassFilterStruct->Input[0];
	highPassFilterStruct->Output[1] = highPassFilterStruct->Output[0];

	highPassFilterStruct->Input[0] = inputValue;

	highPassFilterStruct->FilteredValue = highPassFilterStruct->Alpha
			* (highPassFilterStruct->Input[0] - highPassFilterStruct->Input[1])
			+ highPassFilterStruct->Output[1];

	highPassFilterStruct->Output[0] = highPassFilterStruct->FilteredValue;

	return highPassFilterStruct->FilteredValue;
}
void Filter::LowPassFilterInit(LowPassFilterStruct *lowPassFilterStruct, float32_t cutOffFrequency, float32_t sampleFrequency) {

	if (cutOffFrequency <= 0 || sampleFrequency <= 0) {

		return;
	}

	float tau = 1.0 / (2.0 * M_PI * cutOffFrequency);
	lowPassFilterStruct->AlphaLowPass = 1.0 / (1.0 + tau * sampleFrequency);
}
float32_t Filter::LowPassFilter(LowPassFilterStruct *lowPassFilterStruct, float32_t currentValue, float32_t &previousFilteredValue) {

	float32_t filteredValue = lowPassFilterStruct->AlphaLowPass * currentValue
			+ (1 - lowPassFilterStruct->AlphaLowPass) * previousFilteredValue;
	previousFilteredValue = filteredValue;

	return (filteredValue < 0) ? 0 : filteredValue;
}

float32_t Filter::MoovingAverage(float32_t *dataArray, float32_t *sumData, uint32_t index, uint32_t len, float32_t nextValue){
	if (index >= len) {
			return 0.0f;
		}
		*sumData = *sumData - dataArray[index] + nextValue;
		*sumData = (*sumData < 1) ? 0 : *sumData;
		dataArray[index] = nextValue;
		float32_t average = (len != 0) ? *sumData / len : 0.0f;

		return average;
}

