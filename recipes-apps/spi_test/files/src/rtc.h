/*   MODULE HEADER FILE: rtc.h --------------------------------------- 
 * 
 *   Header file for setting defines for RTC registers, times and alarms
 *   
 ----------------------------------------------------------------------*/ 
#ifndef RTC_H
#define RTC_H


#define NUM_RTC_REGS 19    //Number of registers provided by DS3231 - RTC 
//char ds3231[NUM_RTC_REGS] = {0};
#define EEPROM_SIZE 4000 //1024    //Define the number of bytes in the EERPOM 
//char eeprom[EEPROM_SIZE] = {0};

//Define I2c device to use
#define I2C_DEV "RTC"  //RTC; EEPROM
#define RTC_SLAVE_ADDR 0x68
#define EEPROM_SLAVE_ADDR 0x57
#define CNTRL_WRITE (EEPROM_SLAVE_ADDR << 1)
#define CNTRL_READ (EEPROM_SLAVE_ADDR << 1) + 0x01


//RTC register addresses:
#define RTC_YEAR 0x06
#define RTC_MONTH 0x05
#define RTC_DATE 0x04
#define RTC_DAY 0x03
#define RTC_HOUR 0x02
#define RTC_MINUTE 0x01
#define RTC_SECOND 0x00
//Alarm1 register addresses:
#define ALARM_1_DAY 0x0A
#define ALARM_1_HOUR 0x09
#define ALARM_1_MINUTE 0x08
#define ALARM_1_SECOND 0x07

//Special Purpose Registers
#define CONTROL_REG 0x0E
#define STATUS_REG 0x0F 


// TEMPERATURE register addresses
#define TEMP_UPPER 0x11
#define TEMP_LOWER 0x12


// DEFINES FOR SETTING THE RTC CLOCK
//---------------------------------------------------------------------------------
#define YEAR_FIRST_DIGIT	0x02
#define YEAR_SECOND_DIGIT	0x04
#define DAY_FIRST_DIGIT		0x06
#define DATE_FIRST_DIGIT	0x01
#define DATE_SECOND_DIGIT	0x08
#define MONTH_LOWER_DIGIT	0x00
#define MONTH_UPPER_DIGIT	0x01
#define HOUR_FIRST_DIGIT	0x01
#define HOUR_SECOND_DIGIT	0x04	
#define MINUTE_FIRST_DIGIT	0x00
#define MINUTE_SECOND_DIGIT	0x05	
#define SECOND_FIRST_DIGIT	0x00
#define SECOND_SECOND_DIGIT	0x00
// DEFINES FOR SETTING ALARM1
//---------------------------------------------------------------------------------
#define ALARM_1_DAY_FIRST_DIGIT		0x06
#define ALARM_1_HOUR_FIRST_DIGIT	0x01
#define ALARM_1_HOUR_SECOND_DIGIT	0x04	
#define ALARM_1_MINUTE_FIRST_DIGIT	0x00
#define ALARM_1_MINUTE_SECOND_DIGIT	0x05	
#define ALARM_1_SECOND_FIRST_DIGIT	0x03
#define ALARM_1_SECOND_SECOND_DIGIT	0x00	
//---------------------------------------------------------------------------------	
// DEFINES FOR TIME / ALARM MATCHING
//#define ALARM_1_ONCE_PER_SECOND
#define ALARM_1_WHEN_SECONDS_MATCH
//---------------------------------------------------------------------------------

// DEFINES FOR SQUARE WAVE OUTPUT
#define SQUARE_WAVE_1Hz			0x00 // 1 Hz
#define SQUARE_WAVE_1KHz		0x01 // 1.024 kHz
#define SQUARE_WAVE_4KHz		0x02 // 4.096 kHz
#define SQUARE_WAVE_8KHz		0x03 // 8.192 kHz


//Define RTC Setting Structure:
struct DateTime {
	char Year;
	char Month;
	char Day;
	char Date;
	char Hour;
	char Minute;
	char Second;
};	

//struct DateTime dt;



#endif
