/*
 * LoadCell.h
 *
 *  Created on: Aug 9, 2024
 *      Author: OrioN
 */

#ifndef INC_ERRORS_H_
#define INC_ERRORS_H_
enum OperationCodes{
	ReadData,
	SendData,

};
enum Errors{
	Undefined,
	Unknown,
	HasRunProcess,
	BeforeStartedProcess,
	HasNotCalibration,
	HasNotAverage,
	ValueShouldNotBeZero,
	MinValueCanNotBeGreaterThanMaxValue,
	UndefinedProcessType,
	UndefinedSystemType,
	UndefinedCommandType,
	UndefinedConfigurationType,
};

#endif /* INC_ERRORS_H_ */
