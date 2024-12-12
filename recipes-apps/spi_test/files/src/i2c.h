/*   MODULE HEADER FILE: i2c.h --------------------------------------- 
 * 
 *   Header file for functions for I2C connectivity
 *   
 ----------------------------------------------------------------------*/ 
#ifndef I2C_H
#define I2C_H

#include <stdlib.h>

void init_device(int addr);
int32_t read_data(char addr);
void write_data(char addr, char data);
void close_device(void);

void get_DateTime(void);
void set_DateTime(void);
void set_year(char regAddr,char y);
int32_t get_year(char regAddr);
void set_month(char regAddr,char m);
int32_t get_month(char regAddr);
void set_hour(char regAddr,char h);
int32_t get_hour(char regAddr);
void set_minute(char regAddr,char m);
int32_t get_minute(char regAddr);
void set_second(char regAddr,char s);
int32_t get_second(char regAddr);
void set_date(char regAddr,char d);
int32_t get_date(char regAddr);
void set_day(char regAddr,char d);
int32_t get_day(char regAddr);
void read_temp (void);
void set_alarm1 (void);
void set_alarm1_time (void);
void reset_alarm1 (void);
void clear_alarm1Flg (void);
void get_Alarm1Time (void);
void enable_alarm1_int (void);
char rd_control_register (void);
void set_control_register (char regAddr,char cr);
char rd_status_register (void);
void set_status_register (char regAddr,char sr);
void enable_SQW (char freq);



#endif
