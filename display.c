#include "sys.h"
#include "display.h"
#include "pinout.h"
#include "rtc.h"
#include "bmxx80.h"
#include "si7021.h"
#include "key.h"
#include "settings.h"
#include "alarm.h"
#include "fonts.h"
#include "weekpicture.h"
#include "menupicture.h"
#include "holidays.h"

uint8_t data disp[DISPLAYSIZE];
uint8_t render_buffer_size = 0;
int16_t scroll_index = -1;
int16_t scroll_index_string = -1;
uint8_t xdata render_buffer[RENDSERBUFFERSIZE];
uint8_t *pdisp;
uint8_t code *fptr;
uint8_t displayBright;
uint8_t dispMode = MODE_MAIN;
uint8_t code hourbright[12] = { 0x00, 0x00, 0x12, 0x34, 0x55, 0x55, 0x55, 0x55, 0x55, 0x54, 0x32, 0x10 };

uint8_t menuNumber = MODE_EDIT_TIME;
uint16_t screenTime = 0;
uint16_t screenTimeString = 0;
uint8_t widgetNumber = 0;
uint8_t stringNumber = 0;
bit reversed;
bit refstart;
uint16_t refcount;
uint16_t refcountString;
uint16_t dotcount;
int8_t timerSet = 0;
uint16_t timerSecStart;
uint16_t secSum;
uint16_t secMin;
int8_t eepromStringShow;

void displayInit(void)
{
	P0M1 = 0x00;
	P0M0 = 0x3F;
	P1M1 = 0x00;
	P1M0 = 0xFF;
	P2M1 = 0x00;
	P2M0 = 0xFF;
	P3M1 = 0x00;
	P3M0 = 0x7F;
	displayBright = eep.bright;
	refstart = 0;
	reversed = key_mer;
	updateFont();
	pdisp = &render_buffer[0];

	return;
}

void displayClear(void)
{
	P3 &= 0x80;
	P1 |= 0xff;
	P2 |= 0xff;
	P0 |= 0x3F;

	return;
}

void displayRefresh(void)
{
	static uint8_t columnrefnum = 0;
	static uint8_t columnrefwidth = 0;
	uint8_t k;

	if(columnrefwidth < 5 ) {
		if ( columnrefwidth == displayBright )
			displayClear();
		columnrefwidth++;
		return;
	}
	else {
		columnrefwidth = 0;
		displayClear();
		if(!reversed) {
			k = 21-columnrefnum;
		}
		else {
			k = columnrefnum;
		}
		switch(k) {
			case 0:		col1=0;	 break;
			case 1:		col2=0;	 break;
			case 2:		col3=0;	 break;	
			case 3:		col4=0;	 break;
			case 4:		col5=0;	 break;
			case 5:		col6=0;	 break;	
			case 6:		col7=0;	 break;
			case 7:		col8=0;	 break;
			case 8:		col9=0;	 break;
			case 9:		col10=0; break;
			case 10:	col11=0; break;
			case 11:	col12=0; break;
			case 12:	col13=0; break;
			case 13:	col14=0; break;	
			case 14:	col15=0; break;
			case 15:	col16=0; break;
			case 16:	col17=0; break;	
			case 17:	col18=0; break;
			case 18:	col19=0; break;
			case 19:	col20=0; break;
			case 20:	col21=0; break;
			case 21:	col22=0; break;
			default:break;
		}

		if(!reversed) {	
			P3 = disp[21-k];
		}
		else {
			P3 = ((disp[k]>>6)&0x01)|((disp[k]>>4)&0x02)|((disp[k]>>2)&0x04)| (disp[k]&0x08)|((disp[k]<<2)&0x10)|((disp[k]<<4)&0x20)|((disp[k]<<6)&0x40);
		}
		columnrefnum++;
		if( columnrefnum > 21 ) {
			columnrefnum = 0;
			reversed = key_mer;
			if(refstart==0) {
				refstart = 1;
			}
		}
	}

	return;
}

void resetDispLoop(void)
{
	dispMode = MODE_MAIN;
	screenTime = 0;
	widgetNumber = 0;

	return;
}

void checkAlarm(void)
{
	static bit firstCheck = 1;

	rtcReadTime();

	// Once check if it's a new second
	if (rtc.sec == 0) {
		if (firstCheck) {
			firstCheck = 0;
			// Check alarm
			if (alarm.on && rtc.hour == alarm.hour && rtc.min == alarm.min) {
				if (*((int8_t*)&alarm.mon + rtc.wday - 1))
					alarmTimer = 60 * (uint16_t)eep.alarmTimeout;
			}
			else {
				// Check new hour
				if (rtc.hour > alarm.hour && rtc.min == 0 && eep.hourSignal)
					startBeeper(BEEP_LONG);
			}
		}
	}
	else {
		firstCheck = 1;
	}

	return;
}

void updateFont(void)
{
	switch(eep.fontMode) {
		default:
		case 0: {fptr = &num_font1[0]; break; }
		case 1: {fptr = &num_font2[0]; break; }
		case 2: {fptr = &num_font3[0]; break; }
		case 3: {fptr = &num_font4[0]; break; }
		case 4: {fptr = &num_font5[0]; break; }
	}

	return;
}

void showDS3231(void)
{
	uint8_t i, code *sptr = &pic_DS3231[0];

	for(i=0; i<DISPLAYSIZE; i++, sptr++, pdisp++) {
		*pdisp = *sptr;
	}

	return;
}

void showDotMinSec()
{
	uint8_t i, dot;

	if (dotcount < 30) dot = 0; // dont show semicolon
	else if (dotcount < 90) dot = 3; // show semicolon
	else dot = 0; // dont show semicolon

	for( i=0; i<4; i++, pdisp++ ) *pdisp = dot_font[4*dot+i];
}

void showDot(void)
{
	uint8_t i, dot;

	switch(eep.dotMode)
	{
		default:
		case 0:
		{
			if (dotcount < 30) { dot = 0; } // dont show semicolon
			else if (dotcount < 90) { dot = 3; } // show semicolon
			else { dot = 0; } // dont show semicolon
			break;
		}
		case 1:
		{
			if (dotcount < 30) dot = 2; // show two points together
			else if (dotcount < 90) dot = 3; // show semicolon
			else dot = 2; // show two points together
			break;
		}
			
		case 2:
		{
			if (dotcount < 10) { dot = 0; }
			else if (dotcount < 26) { dot = 1; }
			else if (dotcount < 44) { dot = 2; }
			else if (dotcount < 78) { dot = 3; }
			else if (dotcount < 96) { dot = 2; }
			else if (dotcount < 112) { dot = 1; }
			else { dot = 0; }
			break;
		}
		case 3:
		{
			if (dotcount < 12) { dot = 0; }
			else if (dotcount < 36) { dot = 3; }
			else if (dotcount < 60) { dot = 4; }
			else if (dotcount < 84) { dot = 5; }
			else if (dotcount < 108) { dot = 6; }
			else { dot = 0; }
			break;
		}
		case 4:
		{
			if (dotcount < 14) { dot = 0; }
			else if (dotcount < 44) { dot = 7; }
			else if (dotcount < 74) { dot = 3; }
			else if (dotcount < 104) { dot = 8; }
			else { dot = 0; }
			break;
		}
		case 5:
		{
			if (dotcount < 14) { dot = 0; }
			else if (dotcount < 44) { dot = 1; }
			else if (dotcount < 74) { dot = 2; }
			else if (dotcount < 104) { dot = 1; }
			else { dot = 0; }
			break;
		}
	}
	
	for(i=0; i<4; i++, pdisp++) { *pdisp = dot_font[4*dot+i]; }
}

void showNumber(uint16_t num, uint8_t clean, uint8_t dig )
{
	uint8_t i, code *sptr;

	for(i=0; i<4; i++, pdisp++)
	{
		if( !clean && ( !dig ||((num/10) > 0 ) ) )
		{
			sptr = fptr + (4*(num/10)+i);
			*pdisp = *sptr;
		}
		else *pdisp = 0x00;
	}
	
	*pdisp = 0x00;
	pdisp++;
	
	for(i=0; i<4; i++, pdisp++)
	{
		if( !clean )
		{
			sptr = fptr + (4*(num%10)+i);
			*pdisp = *sptr;
		}
		else *pdisp = 0x00;
	}
}

void showTime(void)
{
	if( rtc.hour > 24 || rtc.min > 60 )
	{
		showDS3231();
		return;
	}
	showNumber(rtc.hour, 0, 0);
	showDot();
	showNumber(rtc.min, 0, 0);
}

void showYear()
{
	pdisp = &disp[0];
	
	*pdisp = 0x00; // 1 space
	pdisp++;
	*pdisp = 0x00; // 1 space
	pdisp++;
	showNumber(20, 0, 0); // First two numbers of year (20..)
	*pdisp = 0x00; // 1 space
	pdisp++;
	showNumber(rtc.year, 0, 0); // Last two numbers of year (..18)
	*pdisp = 0x00; // 1 space
}

void showDate(void)
{
	uint8_t i;

	if( rtc.month > 12 || rtc.date > 32 )
	{
		showDS3231();
		return;
	}
	
	showNumber(rtc.date, 0, 0);
	
	for(i=0; i<4; i++, pdisp++) *pdisp = dot_font[4+i];
	
	showNumber(rtc.month, 0, 0);
}

void showDayWeek(void)
{
	uint8_t i, code *sptr;

	switch(rtc.wday)
	{
		case 1: sptr = &pic_mon[0];break;
		case 2: sptr = &pic_tue[0];break;
		case 3: sptr = &pic_wed[0];break;
		case 4: sptr = &pic_thu[0];break;
		case 5: sptr = &pic_fri[0];break;
		case 6: sptr = &pic_sat[0];break;
		case 7: sptr = &pic_sun[0];break;
		default: sptr = &pic_DS3231[0];break;
	}
	
	for(i=0; i<DISPLAYSIZE; i++, sptr++, pdisp++) *pdisp = *sptr;

	return;
}

void showMinSec()
{
	showNumber(rtc.min, 0, 0);
	showDotMinSec();
	showNumber(rtc.sec, 0, 0);
}

void showTemperature(void)
{
	static uint8_t buf[5];
	uint8_t code *sptr;
	uint8_t i;
	int16_t temp = 0;

	if( /*eep.tempsource == TS_SI &&*/ si7021SensorExists() )
	{
		temp = si7021GetTemp()*10;
	}
	else if ( /*eep.tempsource == TS_BMP &&*/ bmxx80HaveSensor() )
	{
		temp = bmxx80GetTemp()*10;
	}
	else
	{
		temp = rtc.temp*100;
	}

	temp += eep.tempcoef*100;
	
	for (i = 0; i < 5; i++) buf[i] = 0;

	i=4;
	
	while (temp > 0 || i > 0)
	{
		buf[i] = temp % 10;
		i--;
		temp /= 10;
	}
	
	if( buf[0] == 0 )
	{
		for(i=0; i<4; i++, pdisp++)
		{
			sptr = fptr + (4*buf[1]+i);
			*pdisp = *sptr;
		}
		
		*pdisp = 0x00;
		pdisp++;
		
		for(i=0; i<4; i++, pdisp++)
		{
			sptr = fptr + (4*buf[2]+i);
			*pdisp = *sptr;
		}
		
		*pdisp = 0x00;
		pdisp++;
		*pdisp = 0x03;
		pdisp++;
		*pdisp = 0x03;
		pdisp++;

		*pdisp = 0x00;
		pdisp++;
		
		for(i=0; i<4; i++, pdisp++)
		{
			sptr = fptr + (4*buf[3]+i);
			*pdisp = *sptr;
		}
	}
	else
	{
		*pdisp = 0x00;
		pdisp++;
		
		for(i=0; i<4; i++, pdisp++)
		{
			sptr = fptr + (4*buf[0]+i);
			*pdisp = *sptr;
		}
		
		*pdisp = 0x00;
		pdisp++;
		
		for(i=0; i<4; i++, pdisp++)
		{
			sptr = fptr + (4*buf[1]+i);
			*pdisp = *sptr;
		}
		
		*pdisp = 0x00;
		pdisp++;
		
		for(i=0; i<4; i++, pdisp++)
		{
			sptr = fptr + (4*buf[2]+i);
			*pdisp = *sptr;
		}
		
		*pdisp = 0x00;
		pdisp++;
	}

// C	for grad C
	*pdisp = 0x00;
	pdisp++;
	*pdisp = 0x3E;
	pdisp++;
	*pdisp = 0x41;
	pdisp++;
	*pdisp = 0x41;
	pdisp++;
	*pdisp = 0x41;
	pdisp++;
	*pdisp = 0x22;
	pdisp++;
	
	return;
}

void showPressure(void)
{
	int8_t i;
	int16_t pres = 0;
	float floatpres = 0.0;
	
	floatpres = (float)bmxx80GetPressure();
	floatpres = floatpres/10.0 + 0.51;
	pres = floatpres;
	
	*pdisp = 0x00;
	pdisp++;
	*pdisp = 0x00;
	pdisp++;

	for(i=0; i<5; i++, pdisp++) *pdisp = temperature_font[5*(pres/100)+i];

	*pdisp = 0x00;
	pdisp++;
	
	for(i=0; i<5; i++, pdisp++) *pdisp = temperature_font[5*((pres/10)%10)+i];
	
	*pdisp = 0x00;
	pdisp++;
	
	for(i=0; i<5; i++, pdisp++) *pdisp = temperature_font[5*(pres%10)+i];

	*pdisp = 0x00;
	pdisp++;
	*pdisp = 0x00;
	pdisp++;
	*pdisp = 0x00;
	pdisp++;
	
	return;
}

void showHumidity(void)
{
	int8_t i;
	uint16_t humi = 0;
	float floathumi = 0.0;
	
	if(si7021SensorExists()) humi = si7021GetHumidity();
	else if(bmxx80HaveSensor()==BME280_CHIP_ID) humi = bme280GetHumidity();
	
	floathumi = (float)humi;
	floathumi = floathumi/100.0 +0.51;
	humi = floathumi;

	*pdisp = 0x00;
	pdisp++;
	*pdisp = 0x00;
	pdisp++;
	
	if (humi > 9)
	{
		for(i=0; i<5; i++, pdisp++) { *pdisp = temperature_font[5*(humi/10)+i]; }
	}
	else
	{
		for(i=0; i<5; i++, pdisp++) { *pdisp = 0x00; }
	}
	
	*pdisp = 0x00;
	pdisp++;

	for(i=0; i<5; i++, pdisp++) { *pdisp = temperature_font[5*(humi%10)+i]; }

// %	
	*pdisp = 0x00;
	pdisp++;
	*pdisp = 0x00;
	pdisp++;
	*pdisp = 0x62;
	pdisp++;
	*pdisp = 0x64;
	pdisp++;
	*pdisp = 0x08;
	pdisp++;
	*pdisp = 0x13;
	pdisp++;
	*pdisp = 0x23;
	pdisp++;
	*pdisp = 0x00;
	pdisp++;
	*pdisp = 0x00;
	pdisp++;

	return;
}

void autoBright(void)
{
	if( rtc.hour < 24 && eep.bright == 6 ) {
		displayBright = (rtc.hour&0x01)?(hourbright[rtc.hour>>1] & 0x0F):((hourbright[rtc.hour>>1]>>4 )& 0x0F);
	}

	return;
}

void showString()
{
	pdisp = &disp[0];
	updateFont();
	autoBright();

	switch(stringNumber)
	{
		case WI_STRING_TIME: { showTime(); break; }
		case WI_STRING: { showRenderBufferString(); break; }
		default: { showTime(); break;}
	}
}

void showMainScreen(void)
{
	pdisp = &disp[0];
	updateFont();
	autoBright();

	switch(widgetNumber)
	{
		case WI_TIME: { showTime(); break;}
		case WI_YEAR: { showYear(); break;}
		case WI_DATE: { showDate(); break;}
		case WI_WEEK: { showDayWeek(); break;}
		case WI_MINSEC: {showMinSec(); break;}
		case WI_TEMP: { showTemperature(); break;}
		case WI_PRES: { showPressure(); break;}
		case WI_HUMI: { showHumidity(); break;}
		case WI_HOLY: { showRenderBuffer(); break;}
		default: { showTime(); break;}
	}

	return;
}

void checkParam(int8_t *param, uint8_t diff, int8_t paramMin, int8_t paramMax)
{
	*param += diff;

	if (*param > paramMax)
		*param = paramMin;
	if (*param < paramMin)
		*param = paramMax;

	return;
}

void changeMenu(int8_t diff)
{
	checkParam(&menuNumber, diff, MODE_EDIT_TIME, MODE_EXIT);

	return;
}

void showMenu()
{
	uint8_t i, code *sptr;

	switch(menuNumber)
	{
		case MODE_EDIT_TIME: sptr = &pic_Time[0]; break;
		case MODE_EDIT_DATE: sptr = &pic_Date[0]; break;
		case MODE_EDIT_ALARM: sptr = &pic_Alarm[0]; break;
		case MODE_EDIT_HOURSIGNAL: sptr = &pic_HourSignal[0]; break;
		case MODE_EDIT_FONT: sptr = &pic_Font[0]; break;
		case MODE_EDIT_DISP: sptr = &pic_Disp[0]; break;
		case MODE_EDIT_DOT: sptr = &pic_Dot[0]; break;
		case MODE_EDIT_BRIGHT: sptr = &pic_Bright[0]; break;
		case MODE_EDIT_TIME_COEF: sptr = &pic_TimeCoef[0]; break;
		case MODE_EDIT_TEMP_COEF: sptr = &pic_TempCoef[0]; break;
		case MODE_EDIT_STRING_SHOW: sptr = &pic_StrShow[0]; break;
		case MODE_TIMER_SET: sptr = &pic_Timer[0]; break;
		case MODE_RESET: sptr = &pic_Reset[0]; break;
		case MODE_EXIT: sptr = &pic_Exit[0]; break;
		default: break;
	}
	
	for(i=0; i<DISPLAYSIZE; i++, sptr++)
	{
		disp[i] = *sptr;
	}
}

void showTimeEdit(void)
{
	uint8_t i;
	bit flash;

	pdisp = &disp[0];
	updateFont();

	if (refcount < 15) { flash = 0; }
	else if (refcount < 45) { flash = 1; }
	else { flash = 0; }

	if(rtc.etm == RTC_SEC) {
		for(i=0; i<9; i++, pdisp++) {
			*pdisp = 0x00;
		}
		for(i=0; i<4; i++, pdisp++) {
			*pdisp = dot_font[4*3+i];
		}
		showNumber(rtc.sec, !((rtc.etm != RTC_SEC)||(flash && (rtc.etm == RTC_SEC))), 0);
	}
	else {
		showNumber(rtc.hour, !((rtc.etm != RTC_HOUR)||(flash && (rtc.etm == RTC_HOUR))), 0);
		for(i=0; i<4; i++, pdisp++) {
			*pdisp = dot_font[4*3+i];
		}
		showNumber(rtc.min, !((rtc.etm != RTC_MIN)||(flash && (rtc.etm == RTC_MIN))), 0);
	}

	return;
}

void showDateEdit(void)
{
	uint8_t i;
	bit flash;

	pdisp = &disp[0];
	updateFont();

	if (refcount < 15) { flash = 0; }
	else if (refcount < 45) { flash = 1; }
	else { flash = 0; }

	if(rtc.etm == RTC_YEAR) {
		*pdisp = 0x00;
		pdisp++;
		*pdisp = 0x00;
		pdisp++;
		showNumber(20, 0, 0);
		*pdisp = 0x00;
		pdisp++;
		showNumber(rtc.year, !((rtc.etm != RTC_YEAR)||(flash && (rtc.etm == RTC_YEAR))), 0);
		*pdisp = 0x00;
		pdisp++;
	}
	else {
		showNumber(rtc.date, !((rtc.etm != RTC_DATE)||(flash && (rtc.etm == RTC_DATE))), 1);
		for(i=0; i<4; i++, pdisp++) {
			*pdisp = dot_font[4+i];
		}
		showNumber(rtc.month, !((rtc.etm != RTC_MONTH)||(flash && (rtc.etm == RTC_MONTH))), 0);
	}

	return;
}

void showAlarmEdit(void)
{
	uint8_t i, j, code *sptr;
	bit flash;

	pdisp = &disp[0];
	updateFont();

	if( alarm.etm == ALARM_ON) {
		if(alarm.on) {
			sptr = &pic_On[0];
		}
		else {
			sptr = &pic_Off[0];
		}
		for(i=0; i<DISPLAYSIZE; i++, sptr++) {
			disp[i] = *sptr;
		}
	}
	else if((alarm.etm == ALARM_HOUR)||(alarm.etm == ALARM_MIN)) {
		if (refcount < 15) { flash = 0; }
		else if (refcount < 45) { flash = 1; }
		else { flash = 0; }

		showNumber(alarm.hour, !((alarm.etm != ALARM_HOUR)||(flash && (alarm.etm == ALARM_HOUR))), 0);
		for(i=0; i<4; i++, pdisp++) {
			*pdisp = dot_font[4*3+i];
		}
		showNumber(alarm.min, !((alarm.etm != ALARM_MIN)||(flash && (alarm.etm == ALARM_MIN))), 0);
	}
	else {
		if (refcount < 27) { flash = 0; }
		else if (refcount < 33) { flash = 1; }
		else { flash = 0; }

		switch(alarm.etm) {
			case ALARM_MON: {sptr = &pic_alarm_mon[0]; break; }
			case ALARM_TUE: {sptr = &pic_alarm_tue[0]; break; }
			case ALARM_WED: {sptr = &pic_alarm_wed[0]; break; }
			case ALARM_THU: {sptr = &pic_alarm_thu[0]; break; }
			case ALARM_FRI: {sptr = &pic_alarm_fri[0]; break; }
			case ALARM_SAT: {sptr = &pic_alarm_sat[0]; break; }
			case ALARM_SUN: {sptr = &pic_alarm_sun[0]; break; }
			default: break;
		}

		for(i=0; i<DISPLAYSIZE; i++, sptr++) {
			j = 0;
			switch(i) {
				case 1:
				case 2: {
					if(((alarm.etm != ALARM_MON)&&alarm.mon)||((alarm.etm == ALARM_MON)&&((alarm.mon && !flash)||(!alarm.mon && flash)))) j = 1;
					break;
				}
				case 4:
				case 5: {
					if(((alarm.etm != ALARM_TUE)&&alarm.tue)||((alarm.etm == ALARM_TUE)&&((alarm.tue && !flash)||(!alarm.tue && flash)))) j = 1;
					break;
				}
				case 7:
				case 8: {
					if(((alarm.etm != ALARM_WED)&&alarm.wed)||((alarm.etm == ALARM_WED)&&((alarm.wed && !flash)||(!alarm.wed && flash)))) j = 1;
					break;
				}
				case 10:
				case 11: {
					if(((alarm.etm != ALARM_THU)&&alarm.thu)||((alarm.etm == ALARM_THU)&&((alarm.thu && !flash)||(!alarm.thu && flash)))) j = 1;
					break;
				}
				case 13:
				case 14: {
					if(((alarm.etm != ALARM_FRI)&&alarm.fri)||((alarm.etm == ALARM_FRI)&&((alarm.fri && !flash)||(!alarm.fri && flash)))) j = 1;
					break;
				}
				case 16:
				case 17: {
					if(((alarm.etm != ALARM_SAT)&&alarm.sat)||((alarm.etm == ALARM_SAT)&&((alarm.sat && !flash)||(!alarm.sat && flash)))) j = 1;
					break;
				}
				case 19:
				case 20: {
					if(((alarm.etm != ALARM_SUN)&&alarm.sun)||((alarm.etm == ALARM_SUN)&&((alarm.sun && !flash)||(!alarm.sun && flash)))) j = 1;
					break;
				}
			}
			disp[i] = *sptr | j;
		}
	}

	return;
}

void showEditType(uint8_t type)
{
	uint8_t i;

	pdisp = &disp[0];

	for(i=0; i<16; i++, pdisp++) *pdisp = pic_Type[i];
	
	*pdisp = 0x00;
	pdisp++;

	for(i=0; i<5; i++, pdisp++) *pdisp = temperature_font[5*type+i];
}

void changeFont(int8_t diff)
{
	checkParam(&eep.fontMode, diff, eepMin.fontMode/*0*/, eepMax.fontMode/*4*/);
}

void showFontEdit()
{
	uint8_t i;
	bit flash;

	pdisp = &disp[0];
	updateFont();

	if (refcount < 20) { flash = 0; }
	else if (refcount < 40) { flash = 1; }
	else { flash = 0; }
	
	showNumber(rtc.hour, flash, 0);
	
	for( i=0; i<4; i++, pdisp++ ) *pdisp = 0x00; // 4 spaces
	
	showNumber(rtc.min, flash, 0);
}

void changeDisp(int8_t diff)
{
	checkParam(&eep.dispMode, diff, eepMin.dispMode/*1*/, eepMax.dispMode/*5*/);
}

void changeDot(int8_t diff)
{
	checkParam(&eep.dotMode, diff, eepMin.dotMode/*0*/, eepMax.dotMode/*5*/);
}

void showDotEdit()
{
	uint8_t i;
	
	pdisp = &disp[0];
	
	for( i=0; i<10; i++, pdisp++ ) *pdisp = 0x00; // 10 spaces
	
	showDot();
	
	for( i=0; i<10; i++, pdisp++ ) *pdisp = 0x00; // 10 spaces
}

void changeBright(int8_t diff)
{
	checkParam(&eep.bright, diff, eepMin.bright/*0*/, eepMax.bright/*6*/);
	displayBright = eep.bright;
	autoBright();
}

void changeHourSignal(int8_t diff)
{
	checkParam(&eep.hourSignal, diff, eepMin.hourSignal/*0*/, eepMax.hourSignal/*1*/);
}

void showHourSignalEdit()
{
	uint8_t i, code *sptr;

	if(eep.hourSignal) sptr = &pic_On[0];
	else {
		sptr = &pic_Off[0];
	}
	for(i=0; i<DISPLAYSIZE; i++, sptr++) disp[i] = *sptr;
}

void changeTimeCoef(int8_t diff)
{
	checkParam(&eep.timecoef, diff, eepMin.timecoef/*-128*/, eepMax.timecoef/*127*/);
}

void showCoef(int8_t timeTempCoef)
{
	static uint8_t buf[3];
	uint8_t code *sptr;
	int8_t i;
	uint8_t coef;
	bit sign;

	pdisp = &disp[0];
	sign = (timeTempCoef >= 0)? 0: 1;

	if(sign)
	{
		coef = -timeTempCoef;
	}
	else
	{
		coef = timeTempCoef;
	}
	
	for (i = 0; i < 3; i++) buf[i] = 0;

	i=2;
	
	while (coef > 0 || i > 0)
	{
		buf[i] = coef % 10;
		i--;
		coef /= 10;
	}

	coef = 3;
	if( !sign )
		coef +=5;
	
	if( buf[0] == 0 )
	{
		coef += 5;
		
		if( buf[1] == 0 ) coef += 5;
	}

	for(i=0; i<coef; i++, pdisp++) *pdisp = 0x00;
	
	if( sign )
	{
		for(i=0; i<4; i++, pdisp++) *pdisp = 0x08;

		*pdisp = 0x00;
		pdisp++;
	}

	if(buf[0] > 0 ) 
	{
		for(i=0; i<4; i++, pdisp++)
		{
			sptr = fptr + (4*buf[0]+i);
			*pdisp = *sptr;
		}

		*pdisp = 0x00;
		pdisp++;
	}

	if((buf[0] > 0 )||(buf[1] > 0 ))
	{
		for(i=0; i<4; i++, pdisp++)
		{
			sptr = fptr + (4*buf[1]+i);
			*pdisp = *sptr;
		}

		*pdisp = 0x00;
		pdisp++;
	}

	for(i=0; i<4; i++, pdisp++)
	{
		sptr = fptr + (4*buf[2]+i);
		*pdisp = *sptr;
	}
}

void showTimeCoefEdit()
{
	showCoef(eep.timecoef);
}

void changeTempCoef(int8_t diff)
{
	checkParam(&eep.tempcoef, diff, eepMin.tempcoef/*-9*/, eepMax.tempcoef/*9*/);
}

void showTempCoefEdit()
{
	showCoef(eep.tempcoef);
}

void changeTimerSet(uint8_t diff)
{
	checkParam(&timerSet, diff, 0, 99);	
}

void showTimerStart()
{
	if ( (rtc.sec - timerSecStart >= 1) && (timerSet >= 1) )
	{
		timerSecStart = rtc.sec;
		secSum++;
		secMin--;
	}

	showTimer(timerSet-1, secMin);

	if ( secSum == 60 )
	{
		secSum = 0;
		secMin = 59;
		
		if ( timerSet >= 1 )
		{
			timerSet--;
		}
	}

	if ( timerSet == 0 )
	{	
		showTimer(0, 0);
		alarmTimer = 60 * (uint16_t)eep.alarmTimeout;
		dispMode = MODE_TIMER_END;
	}
}

void showTimer(uint8_t min, uint8_t sec)
{
	uint8_t i;
	pdisp = &disp[0];
	showNumber(min, 0, 0);

	for( i=0; i<4; i++, pdisp++ ) *pdisp = dot_font[12+i]; // show semicolon

	showNumber(sec, 0, 0);
}

void changeStringShow(int8_t diff)
{
	checkParam(&eep.stringShow, diff, eepMin.stringShow/*0*/, eepMax.stringShow/*1*/);
}

void showStringShowEdit()
{
	uint8_t i, code *sptr;

	if(eep.stringShow) sptr = &pic_On[0];
	else sptr = &pic_Off[0];
	
	for(i=0; i<DISPLAYSIZE; i++, sptr++) disp[i] = *sptr;
}

void wiString(uint16_t wiSec)
{
	if( screenTimeString > wiSec )
	{
		stringNumber++;
		screenTimeString = 0;
		
		if( stringNumber > 2 ) stringNumber = WI_STRING_TIME;
		
		if ( stringNumber == WI_STRING_TIME ) stringNumber = WI_STRING;
		
		if(stringNumber == WI_STRING)
		{
			if( holiday && (dispMode == MODE_MAIN) )
			{
				dispMode = MODE_STRING;
				scroll_index_string = 0;
			}
			else
			{
				stringNumber = WI_STRING_TIME;
				scroll_index_string = -1;
			}
		}
	}
}

void wiNext(uint16_t wiSec)
{
	if( screenTime > wiSec )
	{
		widgetNumber++;
		screenTime = 0;
		
		if( widgetNumber > WIDGET_NUMBER ) widgetNumber = WI_TIME;
		
		if(widgetNumber == WI_PRES && !bmxx80HaveSensor()) widgetNumber = WI_HUMI;
		
		if(widgetNumber == WI_HUMI && 
			( !(bmxx80HaveSensor()==BME280_CHIP_ID||si7021SensorExists()) )) widgetNumber = WI_HOLY;
		
		if(widgetNumber == WI_HOLY)
		{
			if( holiday && (dispMode == MODE_MAIN) ) scroll_index = 0;
			else
			{
				widgetNumber = WI_TIME;
				scroll_index = -1;
			}
		}
	}
}

void showRenderBuffer(void)
{
	uint8_t i;
	int16_t ind = scroll_index - DISPLAYSIZE;

	if( scroll_index > (render_buffer_size + DISPLAYSIZE ))
	{
		scroll_index = -1;
		widgetNumber = 0; screenTime = 0;
	}

	for(i=0; i<DISPLAYSIZE; i++)
	{
		if(( ind + i >= 0 )&&(ind + i < render_buffer_size )) disp[i] = render_buffer[(uint8_t)(ind + i)];
		else disp[i] = 0x00;
	}
}

void showRenderBufferString()
{
	uint8_t i;
	int16_t ind = scroll_index_string - DISPLAYSIZE;

	if( scroll_index_string > (render_buffer_size + DISPLAYSIZE ))
	{
		scroll_index_string = -1;
		screenTimeString = 0; stringNumber = 0; dispMode = MODE_MAIN;
	}

	for(i=0; i<DISPLAYSIZE; i++)
	{
		if(( ind + i >= 0 )&&(ind + i < render_buffer_size )) disp[i] = render_buffer[(uint8_t)(ind + i)];
		else disp[i] = 0x00;
	}
}

void writeRenderBuffer(uint8_t value)
{
	if ( render_buffer_size < RENDSERBUFFERSIZE) render_buffer[render_buffer_size++] = value;
}

void renderHoliday(uint8_t length, char *str)
{
	uint8_t i, j, t, c;
	render_buffer_size = 0;

	for(i=0; i < (length - 1); i++, str++)
	{
		c = *str;

		if( c >= 0xA0 ) c -= 0x40;
		else if( c >= 0x20 ) c -= 0x20;
		else c = 0x1F;
		
		for(j=0; j<5; j++)
		{
			t = font_cp1251_07[5*c+j];
			
			if( t != VOID ) writeRenderBuffer(t);
		}
		
		writeRenderBuffer(0x00);
	}
}

void makeReset()
{
/*  default values:	
		pageBlock = 0xFF
		hourSignal = 0
		dispMode = 5
		dotMode = 0
		fontMode = 0
		alarmTimeout = 1
		bright = 2
		on = 0
		hour = 7
		min = 30
		mon = 1 
		tue = 1
		wed = 1
		thu = 1
		fri = 1
		sat = 0
		sun = 0
		tempcoef = 0
		timecoef = 0
		tempsource = 0
		stringShow = 1
*/

	defReset();
	saveEdit();
	displayBright = eep.bright;
	autoBright();
	resetDispLoop();
	menuNumber = MODE_EDIT_TIME;
}

/*
static char *mkNumberString(int16_t value, uint8_t width, uint8_t lead)
{
	static char strbuf[8];

	uint8_t sign = lead;
	int8_t pos;

	if (value < 0) {
		sign = '-';
		value = -value;
	}

	// Clear buffer and go to it's tail
	for (pos = 0; pos < width; pos++)
		strbuf[pos] = lead;
	strbuf[pos--] = '\0';

	// Fill buffer from right to left
	while (value > 0 || pos > width - 2) {
		strbuf[pos] = value % 10 + 0x30;
		pos--;
		value /= 10;
	}

	if (pos >= 0)
		strbuf[pos] = sign;

	return strbuf;
}
*/

#ifdef _DEBUG_

void showTestRender(void)
{
	/*
	 ? = \xFD
	*/
	//char test[] = "TEST RENDER!!! Proverka???\x63";
	//char test[] = "? ????? ?????!!!";
	//char test[] = "?????????????\xFD??";
	char code test[] = "???????? ??????????, ?? ????? ??????.";
	uint8_t i,j,t;
	uint8_t c;

	scroll_index = 0;
	for(i=0; i < strlen(test); i++) {
		c = test[i];
		if( c >= 0xA0 ) {
			c -= 0x40;
		}
		else if( c >= 0x20 ) {
			c -= 0x20;
		}
		else {
			c = 0x1F;
		}
		for(j=0; j<5; j++) {
			t = font_cp1251_07[5*c+j];
			if( t != VOID ) {
				writeRenderBuffer(t);
			}
		}
		writeRenderBuffer(0x00);
	}

	return;
}

#endif