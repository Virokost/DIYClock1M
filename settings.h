#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "sys.h"
#include "alarm.h"

typedef struct {
	uint8_t pageBlock;
	uint8_t hourSignal;
	uint8_t dispMode;
	uint8_t dotMode;
	uint8_t fontMode;
	uint8_t alarmTimeout;
	uint8_t bright;

	//Alarm_type alarm;
	int8_t on;
	int8_t hour;
	int8_t min;
	int8_t mon;
	int8_t tue;
	int8_t wed;
	int8_t thu;
	int8_t fri;
	int8_t sat;
	int8_t sun;
	
	int8_t tempcoef;
	int8_t timecoef;
	int8_t tempsource;
	int8_t stringShow;
} EEP_Param;

extern EEP_Param eep;
extern code EEP_Param eepMin;
extern code EEP_Param eepMax;

void settingsInit(void);
void settingsSave(void);

#endif /* _SETTINGS_H_ */