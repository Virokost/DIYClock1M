#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include<intrins.h>
#include "sys.h"

#define PARAM_UP	1
#define PARAM_DOWN	-1
#define DISPLAYSIZE 22
#define RENDSERBUFFERSIZE 254
#define hbd(a,b) a | (b << 4)
#define WIDGET_NUMBER 9
#define WIDGET_SHOW_TIME 60  // 60 - time in seconds after which would be shown YEAR
#define STRING_SHOW_TIME 3600  // the string would be shown every 3600 seconds

enum
{
	MODE_MAIN = 0,
	MODE_MENU,
	MODE_EDIT_TIME,
	MODE_EDIT_DATE,
	MODE_EDIT_ALARM,
	MODE_EDIT_HOURSIGNAL,
	MODE_EDIT_DISP,
	MODE_EDIT_BRIGHT,
	MODE_EDIT_TIME_COEF,
	MODE_EDIT_TEMP_COEF,
	MODE_TIMER_SET,
	MODE_EDIT_FONT,
	MODE_EDIT_DOT,
	MODE_EDIT_STRING_SHOW,
	MODE_RESET,
	MODE_EXIT,
	MODE_WIDGET,
	MODE_TIMER_START,
	MODE_TIMER_END,
	MODE_STRING,
	MODE_TEST,

	MODE_END
};

enum
{
	WI_TIME = 0,
	WI_YEAR,
	WI_DATE,
	WI_WEEK,
	WI_MINSEC,
	WI_TEMP,
	WI_PRES,
	WI_HUMI,
	WI_HOLY
};

enum
{
	WI_STRING_TIME = 0,
	WI_STRING
};

enum
{
	TS_DS = 0,
	TS_SI,
	TS_BMP
};

extern uint8_t dispMode;
extern uint8_t data disp[DISPLAYSIZE];
extern uint8_t xdata render_buffer[RENDSERBUFFERSIZE];
extern uint8_t displayBright;
extern uint8_t render_buffer_size;
extern int16_t scroll_index;
extern int16_t scroll_index_string;
extern uint8_t menuNumber;
extern uint16_t screenTime;
extern uint16_t screenTimeString;
extern uint8_t widgetNumber;
extern uint8_t stringNumber;
extern bit refstart;
extern uint16_t refcount;
extern uint16_t refcountString;
extern uint16_t dotcount;
extern int8_t timerSet;
extern uint16_t timerSecStart;
extern uint16_t secSum;
extern uint16_t secMin;
extern int8_t eepromStringShow;
extern bit reversed;

void displayInit(void);
void displayClear(void);
void displayRefresh(void);
void resetDispLoop(void);
void checkAlarm(void);
void updateFont(void);
void showMainScreen(void);
void showTime(void);
void changeBright(int8_t diff);
void changeMenu(uint8_t diff);
void showMenu(void);
void showTimeEdit(void);
void showDateEdit(void);
void showAlarmEdit(void);
void showEditType(uint8_t type);
void changeFont(int8_t diff);
void changeDisp(int8_t diff);
void changeDot(int8_t diff);
void changeHourSignal(int8_t diff);
void showHourSignalEdit(void);
void changeTimeCoef(int8_t diff);
void showTimeCoefEdit(void);
void changeTempCoef(int8_t diff);
void showTempCoefEdit(void);
void changeTimerSet(uint8_t diff);
void showTimerStart();
void showTimer(uint8_t min, uint8_t sec);
void showRenderBuffer(void);
void showRenderBufferString();
void renderHoliday(uint8_t length, char *str);
void wiNext(uint16_t wiSec);
void wiString(uint16_t wiSec);
void showString();
void changeStringShow(int8_t diff);
void showStringShowEdit();
void showCoef(int8_t timeTempCoef);
void showFontEdit();
void showDotEdit();
void makeReset();
void autoBright();
void saveEdit();

#endif /* _DISPLAY_H_ */