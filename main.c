#include "sys.h"
#include "pinout.h"
#include "timer.h"
#include "display.h"
#include "rtc.h"
#include "key.h"
#include "alarm.h"
#include "settings.h"
#include "holidays.h"
#include "delay.h"
#include "i2c.h"

#include "bmxx80.h"
#include "si7021.h"

void hwInit(void)
{
	settingsInit();
	displayInit();
	timerInit();
	rtcInit();
	alarmInit();
	bmxx80Init();
	si7021Init();
	rtc.etm = RTC_NOEDIT;
	
	return;
}

void cancelEdit(void)
{
	while(refstart == 0) {}
		
	displayClear();
	EA = 0;
	settingsInit();
	EA = 1;
	dispMode = MODE_MAIN;
	widgetNumber = 0;
	stringNumber = 0;

	return;
}

void saveEdit(void)
{
	while(refstart == 0) {}
		
	displayClear();
	EA = 0;
	settingsSave();
	EA = 1;
	dispMode = MODE_MAIN;

	return;
}

void main(void)
{
	uint8_t cmd;
	uint8_t direction = PARAM_UP;

	hwInit();
	startBeeper(BEEP_SHORT);
	sensTimer = TEMP_MEASURE_TIME;
	
	while(1)
	{ // begin main while loop
		while(refstart == 0) {}
			
		refstart = 0;

		if(((refcount % 10) == 0) &&(dispMode == MODE_MAIN||!(dispMode == MODE_EDIT_TIME||dispMode == MODE_EDIT_DATE)))
		{
			if (sensTimer == 0)
			{
				sensTimer = SENSOR_POLL_INTERVAL;

				if ( bmxx80HaveSensor() )
				{
					bmxx80Convert();
				}
			
				if (si7021SensorExists())
				{	
					si7021Convert();
				}
			}
			
			checkAlarm();
			checkHolidays();
		}

		cmd = getBtnCmd();
		
		if (cmd != BTN_STATE_0)
		{
			if (cmd < BTN_0_LONG)
				startBeeper(BEEP_SHORT);
			else
				startBeeper(BEEP_LONG);
		}

		/*
		BTN_0 - 'SET'
		BTN_1 -  '+'
		BTN_2 -  '-'
		*/

		switch (cmd)
		{ // begin switch cmd
			case BTN_0:
			{ // begin BTN_0 case
				if ( eep.dispMode == 5 && dispMode == MODE_MAIN )
				{
					dispMode = MODE_WIDGET;
					
					if ( widgetNumber == 0 ) widgetNumber = 1;
					else if ( widgetNumber > 0 && widgetNumber < WIDGET_NUMBER - 2 ) widgetNumber++;
				}
				
				switch (dispMode)
				{ // begin switch1 dispMode
					case MODE_MAIN: { break; }
					case MODE_MENU:
					{
						switch(menuNumber)
						{ // begin switch menuNumber
							case MODE_EDIT_TIME: {rtc.etm = RTC_HOUR; dispMode = menuNumber; break;}
							case MODE_EDIT_DATE: {rtc.etm = RTC_YEAR; dispMode = menuNumber; break;}
							case MODE_EDIT_ALARM: {alarm.etm = ALARM_ON; dispMode = menuNumber; break;}
							case MODE_EXIT: { dispMode = MODE_MAIN; widgetNumber = 0; break; }
							case MODE_TIMER_START: { showTimerStart(); break; }
							case MODE_EDIT_STRING_SHOW: {	eepromStringShow = eep.stringShow; }
							case MODE_EDIT_HOURSIGNAL:
							case MODE_EDIT_DISP:
							case MODE_EDIT_BRIGHT:
							case MODE_EDIT_TEMP_COEF:
							case MODE_EDIT_TIME_COEF:
							case MODE_EDIT_FONT:
							case MODE_EDIT_DOT:
							case MODE_TIMER_SET: { dispMode = menuNumber; break; }
							default: { break; }
						}  // end switch menuNumber
						break;
					}
					case MODE_EDIT_TIME:
					{
						if(rtc.etm == RTC_SEC)
						{
							rtcSaveTime();
							resetDispLoop();
						}
						else
						{
							rtcNextEditParam();
						}
						break;
					}
					case MODE_EDIT_DATE:
					{
						if(rtc.etm == RTC_DATE)
						{
							rtcSaveDate();
							resetDispLoop();
						}
						else
						{
							rtcNextEditParam();
						}
						break;
					}
					case MODE_EDIT_ALARM:
					{
						if((alarm.etm == ALARM_ON && !alarm.on)||(alarm.etm == ALARM_SUN))
						{
							alarmSave();
							saveEdit();
							resetDispLoop();
						}
						else
						{
							alarmNextEditParam();
						}
						break;
					}
					case MODE_TIMER_SET:
					{
						dispMode = MODE_TIMER_START;
						timerSecStart = rtc.sec;
						secSum = 0;
						secMin = 59;
						break;
					}
					case MODE_EDIT_STRING_SHOW:
					{
						if ( !(eepromStringShow && eep.stringShow) ) // if eeprom = 0 and eep = 0; or eeprom = 0
						{ // and eep = 1; or eeprom = 1 and eep = 0:
							stringNumber = 0;
							screenTimeString = 0;
							scroll_index_string = -1;
						}
						
						saveEdit();
						resetDispLoop();
						break;
					}
					case MODE_EDIT_TIME_COEF: { rtcSavePPM(); }
					case MODE_EDIT_HOURSIGNAL:
					case MODE_EDIT_FONT:
					case MODE_EDIT_DISP:
					case MODE_EDIT_DOT:
					case MODE_EDIT_BRIGHT:
					case MODE_EDIT_TEMP_COEF:
					case MODE_TIMER_END:
					case MODE_EXIT: { saveEdit(); resetDispLoop(); break; }
				}  // end switch1 dispMode
				break;
			}  // end BTN_0 case
			case BTN_1: { direction = PARAM_UP; }
			case BTN_2:
			{ // begin BTN_2 case
				if (cmd == BTN_2)
					direction = PARAM_DOWN;
				switch (dispMode)
				{ // begin switch2 dispMode
					case MODE_MAIN: { changeBright(direction); break; }
					case MODE_MENU: { changeMenu(direction); break; }
					case MODE_EDIT_TIME:
					case MODE_EDIT_DATE: { rtcChangeTime(direction); break; }
					case MODE_EDIT_ALARM: { alarmChange(direction); break; }
					case MODE_EDIT_HOURSIGNAL: { changeHourSignal(direction); break; }
					case MODE_EDIT_FONT: { changeFont(direction); break; }
					case MODE_EDIT_DISP: { changeDisp(direction); break; }
					case MODE_EDIT_DOT: { changeDot(direction); break; }
					case MODE_EDIT_BRIGHT: { changeBright(direction); break; }
					case MODE_EDIT_TIME_COEF: { changeTimeCoef(direction); break; }
					case MODE_EDIT_TEMP_COEF: { changeTempCoef(direction); break; }
					case MODE_EDIT_STRING_SHOW: { changeStringShow(direction); break; }
					case MODE_TIMER_SET: { changeTimerSet(direction); break; }
					case MODE_EXIT: {break;}
				}   // end switch2 dispMode
				break;
			} // end BTN_2 case
			case BTN_0_LONG:
			{ // begin BTN_0_LONG case
				switch (dispMode)
				{ // begin switch3 dispMode
					case MODE_MAIN: { dispMode = MODE_MENU; /*menuNumber = MODE_EDIT_TIME;*/ break; }
					case MODE_MENU: { dispMode = MODE_MAIN; break; }
					case MODE_EDIT_ALARM: { alarmInit(); }
					case MODE_EDIT_HOURSIGNAL:
					case MODE_EDIT_FONT:
					case MODE_EDIT_DISP:
					case MODE_EDIT_DOT:
					case MODE_EDIT_BRIGHT:
					case MODE_EDIT_TEMP_COEF:
					case MODE_EDIT_TIME_COEF:
					case MODE_EDIT_STRING_SHOW:
					case MODE_STRING:
					case MODE_EXIT: { cancelEdit(); }
					case MODE_EDIT_TIME:
					case MODE_EDIT_DATE: { resetDispLoop(); break; }
					case MODE_TIMER_SET:
					case MODE_TIMER_START: { timerSet = 0; cancelEdit(); resetDispLoop(); break; }
				} // end switch3 dispMode
				break;
			} // end BTN_0_LONG case
			case BTN_1_LONG: { break; }
			case BTN_2_LONG: { break; }
			case BTN_0_LONG | BTN_1_LONG: { break; }
			case BTN_0_LONG | BTN_2_LONG: { break; }
			case BTN_1_LONG | BTN_2_LONG: { break; }
			case BTN_0_LONG | BTN_1_LONG | BTN_2_LONG: { break; }
		} // end switch cmd

		switch(dispMode)
		{ // begin switch4 dispMode
			case MODE_MAIN: { showMainScreen(); break; }
			case MODE_STRING: { showString(); break; }
			case MODE_MENU: { showMenu(); break; }
			case MODE_EDIT_TIME: { showTimeEdit(); break; } 
			case MODE_EDIT_DATE: { showDateEdit(); break; }
			case MODE_EDIT_ALARM: { showAlarmEdit(); break; }
			case MODE_EDIT_HOURSIGNAL: { showHourSignalEdit(); break; }
			case MODE_EDIT_FONT: { showEditType(eep.fontMode); break; }
			case MODE_EDIT_DISP: { showEditType(eep.dispMode); break; }
			case MODE_EDIT_DOT: { showEditType(eep.dotMode); break; }
			case MODE_EDIT_BRIGHT: { showEditType(eep.bright); break; }
			case MODE_EDIT_TIME_COEF: { showTimeCoefEdit(); break; }
			case MODE_EDIT_TEMP_COEF: { showTempCoefEdit(); break; }
			case MODE_EDIT_STRING_SHOW: { showStringShowEdit(); break; }
			case MODE_TIMER_SET: { showTimer(timerSet, 0); break; }
			case MODE_TIMER_START:
			{
				if ( timerSet > 0 ) showTimerStart();
				else dispMode = MODE_MENU;
				break;
			}
			case MODE_TIMER_END: { showTimer(0, 0); break;}
			case MODE_EXIT:
			case MODE_WIDGET: { dispMode = MODE_MAIN; break; }
		} // end switch4 dispMode

		dotcount++;
		refcount++;
		refcountString++;
		
		if( dotcount > 119 ) dotcount = 0;

		if( holiday&&(widgetNumber == WI_HOLY) && (refcount % 5) == 0 )
		{
			if(scroll_index >=0) scroll_index++;
		}

		if( refcount > 59 )
		{
			refcount = 0;
			screenTime++;

			switch(widgetNumber)
			{
				case WI_TIME:
				{ 
					if(eep.dispMode == 5) screenTime = 0; // DISP TYP = 5 - only time would be shown
					wiNext(WIDGET_SHOW_TIME);
					break; 
				} 
				case WI_YEAR:   { wiNext(2); break; }
				case WI_DATE:   { wiNext(2); break; }
				case WI_WEEK:   { wiNext(2); break; }
				case WI_MINSEC: { wiNext(5); break; }
				case WI_TEMP:   { wiNext(2); break; }
				case WI_PRES:   { wiNext(2); break; }
				case WI_HUMI:   { wiNext(5); break; }
				case WI_HOLY:   { if(scroll_index < 0) wiNext(0); break; }
			}
		}

		if ( holiday && eep.stringShow && (eep.dispMode == 5) ) // show the holiday string when 
		{ // only time is shown, it is the day of holiday and stringShow - is On
			if( holiday && (stringNumber == WI_STRING) && (refcountString % 5) == 0 )
			{
				if(scroll_index_string >=0) scroll_index_string++;
			}

			if( refcountString > 59 )
			{
				refcountString = 0;
				screenTimeString++;

				switch(stringNumber)
				{
					case WI_STRING_TIME: { wiString(STRING_SHOW_TIME); break; } 
					case WI_STRING: { if(scroll_index_string < 0) wiString(0); break; }
				}
			}
		}
		
	} // end main while loop
	
	return;
}

