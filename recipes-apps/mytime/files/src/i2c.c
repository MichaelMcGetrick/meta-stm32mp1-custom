/*   MODULE SOURCE FILE: i2c.c --------------------------------------- 
 * 
 *   Function definitions for I2C connectivity
 *   Author: Michael McGetrick
 ----------------------------------------------------------------------*/
#include "i2c.h"
#include "rtc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

char eeprom[EEPROM_SIZE] = {0};
char ds3231[NUM_RTC_REGS] = {0};
struct DateTime dt;

char filename[20];
int file;
int adapter_num = 1; //Number of an existing h/w adapter (adapter is h/w relating to a specific i2c bus)
					//This is best to be determined dynamically (can chnage from boot to boot)	


// FUNCTION DEFINTIONS --------------------------------------------------
void init_device(int addr)
{
	//printf("Opening i2c device adapter...\n");
	//Open the device:
	snprintf(filename,19,"/dev/i2c-%d", adapter_num);
	file = open(filename, O_RDWR);
	if(file < 0)
	{
		//Error condition
		printf("Failed to open the device driver...");
		printf("The error number: %d",errno);
		printf("Error description: %s",strerror(errno));
		
		//Terminate program				
		exit(1);
	}	
	//printf("Opened I2C adaptor OK!\n");
	
	//Set the address of required I2C client on bus:
	if( ioctl(file,I2C_SLAVE,addr) < 0 )
	{
		//Error condition
		printf("Failed to find the i2c client...");
		printf("The error number: %d",errno);
		printf("Error description: %s",strerror(errno));
		//Terminate program				
		exit(1);
	}	
	
	
}


int32_t read_data(char addr)
{
		
	//__u8 reg = 0x11; //MSB for temperature
	//__u8 reg = 0x06;  //Year 
	//__s32 res;
	
	
	if (strcmp (I2C_DEV,"RTC") == 0)
	{
		
				
			
	//RANDOM READ METHOD
	//Write the value of the address first
	int len = 1;
	if(write(file,&addr,len) != len )
	{
		printf("Failed to write address byte to i2c client...");
		printf("The error number: %d",errno);
		printf("Error description: %s",strerror(errno));
		//Terminate program
		close(file);
		exit(1);
	}		
	//printf("Have written address to i2c client OK!");
	
	//Read the byte value at that point:
	char buf[10];
	if( read(file,buf,len) != len )
	{
		//Error condition
		printf("Failed to read byte from i2c client...");
		printf("The error number: %d",errno);
		printf("Error description: %s",strerror(errno));
		//Terminate program
		close(file);
		exit(1);	
	}
	//printf("Read data from  i2c client using read() - Ok\n");
	return buf[0];
	

	
	/* ---------------------------------------------------------
	 * THIS CODE WIll READ DATA SEQUENTIALLY FROM THE RTC
	 * IF AN ADDRESS IS NOT GIEN IT WILL READ FROM THE LAST
	 * READ/WRITE POSITION. THIUS WIll BE AT 00xh IF WE HAVE STARTED
	 * A NEW BUS SESSION
	 *
	   
	int len = NUM_RTC_REGS;  
	int32_t res;
	if( read(file,ds3231,len) != len )
	{
		//Error condition
		printf("Failed to read from i2c client...");
		printf("The error number: %d",errno);
		printf("Error description: %s",strerror(errno));
		//Terminate program
		close(file);
		exit(1);	
	}
	printf("Read data from  i2c client using read() - Ok\n");
	printf("Data read: \n");
	for(int i=0;i<len;i++)
	{
			printf("%x  0x%x\n ", i,ds3231[i]);
	}
	 	
	return res;
	*/
	

		
   }
   if (strcmp (I2C_DEV,"EEPROM") == 0)
   {
		printf("Reading from EEPROM......\n");
		
		//Send a sixteen bit address command
		//This is performed as a write operation for the EEPROM
		
		//Send a read command to read data at this point
		//This is performed a s a read operationb# for the EEPROM
			
			char data[10];

			//Define address (0x77):
			char addr[3];
			addr[0] = 0xf3;
			addr[1] = 0xf3;
			 
			 
			int len = 2;  
			int i;
			for(i =0;i<len;i++)
			{
				if( write(file,addr,1) != 1 )
				{
					//Error condition
					printf("Failed to write address  i2c client...");
					printf("The error number: %d",errno);
					printf("Error description: %s",strerror(errno));
					//Terminate program
					close(file);
					exit(1);	
				}
				printf("Write required address to i2c client using wwrtie() - Ok\n");
				printf("Data read: \n");
				usleep(100);
		    }
			
			//Read from this location:
			if( read(file,data,1) != 1 )
			{
				//Error condition
				printf("Failed to read from  i2c client...");
				printf("The error number: %d",errno);
				printf("Error description: %s",strerror(errno));
				//Terminate program
				close(file);
				exit(1);
			}	
			printf("Data read from i2c client: 0x%x\n",data[0]);
			
		
			/* ---------------------------------------------------------
			 * THIS CODE WIll READ DATA SEQUENTIALLY FROM THE RTC
			 * IF AN ADDRESS IS NOT GIEN IT WILL READ FROM THE LAST
			 * READ/WRITE POSITION. THIUS WIll BE AT 00xh IF WE HAVE STARTED
			 * A NEW BUS SESSION
			 *
			   
			int len = EEPROM_SIZE;  
			if( read(file,eeprom,len) != len )
			{
				//Error condition
				printf("Failed to read from i2c client...");
				printf("The error number: %d",errno);
				printf("Error description: %s",strerror(errno));
				//Terminate program
				close(file);
				exit(1);	
			}
			printf("Read data from  i2c client using read() - Ok\n");
			printf("Data read: \n");
			for(int i=0;i<len;i++)
			{
					printf("%d  %x  0x%x\n ", i, i,eeprom[i]);
			}
			 	
			return res;
			*/ 
					
					
		
   }	
   	
	
}

void write_data(char addr, char data)
{
		
	if (strcmp (I2C_DEV,"RTC") == 0)
	{
		
		int len = 2;
		char buf[3];
		buf[0] = addr; //Define address of register:
		buf[1] = data;  //Define data byte
		
		if(write(file,buf,len) != len )
		{
			printf("Failed to write address byte to i2c client...");
			printf("The error number: %d",errno);
			printf("Error description: %s",strerror(errno));
			//Terminate program
			close(file);
			exit(1);
		}
		
		
		
	}
	if (strcmp (I2C_DEV,"EEPROM") == 0)
	{
		
		
		
		/*
		char buf[10];
		//Define control byte:
		int RW_FLG = 0; //1: read; 0: write
		int control = 0x57;
		buf[0] =  (char)((control << 1) + RW_FLG);  
		//printf("Control value after convert to char: 0x%x\n",buf[0]);
		*/
		//int16_t address;
		//address = htons(3807);   //Change byte order
		
		
		
		/*
		int len = 4;
		if(write(file,buf,len) != len )
		{
			printf("Failed to write to i2c client...");
			printf("The error number: %d",errno);
			printf("Error description: %s",strerror(errno));
			//Terminate program
			close(file);
			exit(1);
		}		
		printf("Have written to i2c client OK!");
		*/
		char addr[3], data[10];
		//cntrl[0] = CNTRL_WRITE;
		
		addr[0] = 0x00;
		addr[1] = 0x00;
		
		data[0] = 0x05;
		
					
		//Write address:
		if(write(file,&addr,2) != 2 )
		{
			printf("Failed to write to i2c client...");
			printf("The error number: %d",errno);
			printf("Error description: %s",strerror(errno));
			//Terminate program
			close(file);
			exit(1);
		}	
		printf("Have written Address (MSB and LSB)!");
		usleep(500);
		//Write data byte:
		if(write(file,&data,1) != 1 )
		{
			printf("Failed to write to i2c client...");
			printf("The error number: %d",errno);
			printf("Error description: %s",strerror(errno));
			//Terminate program
			close(file);
			exit(1);
		}	
		printf("Have written data byte!");
		
				
					
	}	
	
	
}//write_data	


void set_year(char regAddr,char y)
{
	write_data(regAddr,y);
}	
int32_t get_year(char regAddr)
{
	int32_t year = read_data(regAddr);
	return year;
}


void set_month(char regAddr,char m)
{
	write_data(regAddr,m);
}	
int32_t get_month(char regAddr)
{
	int32_t month = read_data(regAddr);
	return month;
}


void set_day(char regAddr,char d)
{
	write_data(regAddr,d);
}	
int32_t get_day(char regAddr)
{
	int32_t day = read_data(regAddr);
	return day;
}


void set_date(char regAddr,char d)
{
	write_data(regAddr,d);
}	
int32_t get_date(char regAddr)
{
	int32_t date = read_data(regAddr);
	return date;
}



void set_hour(char regAddr,char h)
{
	write_data(regAddr,h);
}	
int32_t get_hour(char regAddr)
{
	int32_t hour = read_data(regAddr);
	return hour;
}

void set_minute(char regAddr,char m)
{
	write_data(regAddr,m);
}	
int32_t get_minute(char regAddr)
{
	int32_t min = read_data(regAddr);
	return min;
}

void set_second(char regAddr,char s)
{
	write_data(regAddr,s);
}	

int32_t get_second(char regAddr)
{
	int32_t sec = read_data(regAddr);
	return sec;
}


void read_temp (void)
{

	int32_t t_upper = read_data(TEMP_UPPER);
	
	int t_lower = read_data(TEMP_LOWER);
	t_lower = t_lower >> 6;
	
		
	float frac = t_lower / 4.0;
	float temp = t_upper + frac;
	//printf ("temp: %.2f \n",temp);
	
	
	printf ("The Temperature is: %.2f C\n",temp);

}// read_temp


void get_DateTime()
{
	char digit1, digit2;
	char mask = 0xf;
	
	printf("\nRTC Date and Time:\n");
		

	//Get Day
	switch (get_day(RTC_DAY))
	{
		case 1 :
	      printf("%s","Sunday");	
		break;
		case 2 :
	      printf("%s","Monday");	
		break;
		case 3 :
	      printf("%s ","Tuesday");	
		break;
		case 4 :
	      printf("%s","Wednesday");	
		break;
		case 5 :
	      printf("%s","Thursday");	
		break;
		case 6 :
	      printf("%s","Friday");	
		break;
		case 7 :
	      printf("%s","Saturday");	
		break;
	}	
	
	//Get date:
	char date = get_date(RTC_DATE);
	//Get first digit:
	digit1 = date >> 4;
	//Get second digit:
	digit2 = date & mask;
	printf(", %d%d ",digit1, digit2);
	
	//Get Month
	//Get first digit:
	char month = get_month(RTC_MONTH);
	digit1 = month & mask;
	digit2 = 0;
	//Check for bit 5:
	if ( (month & (1 << 4)) > 0)
	{
	   digit2 = 10;	
	}
	month = digit1 + digit2;
	switch (month)
	{
		case 0x1 :
	      printf("%s","January");	
		break;
		case 0x2 :
	      printf("%s","February");	
		break;
		case 0x3 :
	      printf("%s","March");	
		break;
		case 0x4 :
	      printf("%s","April");	
		break;
		case 0x5 :
	      printf("%s ","May");	
		break;
		case 0x6 :
	      printf("%s","June");	
		break;
		case 0x7 :
	      printf("%s","July");	
		break;
		case 0x8 :
	      printf("%s","August");	
		break;
		case 0x9 :
	      printf("%s","September");	
		break;
		case 0x0a :
	      printf("%s","October");	
		break;
		case 0x0b :
	      printf("%s","November");	
		break;
		case 0xc :
	      printf("%s","December");	
		break;
	}
			
    //Get Year:
    char year = get_year(RTC_YEAR);
	//Get first digit:
	digit1 = year >> 4;
	//Get second digit:
	digit2 = year & mask;
	
	printf(", 20%d%d |",digit1,digit2);
	
	//Get Hour:
	char hour = get_hour(RTC_HOUR);
	//Get first digit:
	digit1 = hour >> 4;
	//Get second digit:
	digit2 = hour & mask;
	//printf(" -- %d%d: ",digit1,digit2);
	printf(" %d%d:",digit1,digit2);
	
	//Get minutes:
	int8_t min = get_minute(RTC_MINUTE);
	//Get first digit:
	digit1 = min >> 4;
	//Get second digit:
	digit2 = min & mask;
	printf("%d%d:",(int8_t) digit1, (int8_t) digit2);
	
	//Get seconds:
	char sec = get_second(RTC_SECOND);
	//Get first digit:
	digit1 = sec >> 4;
	//Get second digit:
	digit2 = sec & mask;
	printf("%d%d\n",digit1,digit2);
	
	        
}	



void set_DateTime()
{
	//Set Year:
	//24
	char d1 = YEAR_FIRST_DIGIT;  //First digit
	char d2 = YEAR_SECOND_DIGIT; //Second digit
	dt.Year = (d1 << 4) | d2;
			
	//Set Day
	//05 ("thur")
	dt.Day = DAY_FIRST_DIGIT;  //Synchronise with Sunday
	
	//Set Date:
	//18
	d1 = DATE_FIRST_DIGIT;  //First digit
	d2 = DATE_SECOND_DIGIT;  //Second digit
	dt.Date = (d1 << 4) | d2;
	
	
	//Set month:
	//10
	d1 = MONTH_LOWER_DIGIT;  //Lower bits
	d2 = MONTH_UPPER_DIGIT;  //Upper bits
	dt.Month = (d2 << 4) | d1;
		
	//Set hour (24 hr mode)
	//19 
	d1 = HOUR_FIRST_DIGIT;  //First digit
	d2 = HOUR_SECOND_DIGIT;  //Second digit
	dt.Hour = (d1 << 4) | d2;
	
	
	//Set minute 
	//05
	d1 = MINUTE_FIRST_DIGIT;  //First digit
	d2 = MINUTE_SECOND_DIGIT;  //Second digit
	dt.Minute = (d1 << 4) | d2;
	//dt.Minute = 0x19;
	
	//Set second 
	//0
	d1 = SECOND_FIRST_DIGIT;  //First digit
	d2 = SECOND_SECOND_DIGIT;  //Second digit
	dt.Second = (d1 << 4) | d2;
	
		
	
	//Implement changes:
	set_year(RTC_YEAR,dt.Year);
	usleep(100);
	set_day(RTC_DAY,dt.Day);
	usleep(100);
	set_date(RTC_DATE,dt.Date);
	usleep(100);
	set_month(RTC_MONTH,dt.Month);
	usleep(100);
        set_hour(RTC_HOUR,dt.Hour);
        usleep(100);
        set_minute(RTC_MINUTE,dt.Minute);
        usleep(100);
       set_second(RTC_SECOND,dt.Second);
    
}





void set_alarm1 (void)
{
	// Set the interupt control bit
	
	// Set alarm1 time
	set_alarm1_time ();
	
	// Clear the alarm1 status bit (Ia flagged to 1 on startup)
	char flg = rd_status_register ();
	usleep(100);
	flg = flg & 0xFE;
	set_status_register (STATUS_REG,flg);
	usleep (100);
	
	// Enable Alarm1 
	enable_alarm1_int ();

}//set_alarm1



void set_alarm1_time (void)
{
	// Set the alarm1 time register
	char d1,d2;
			
	//Set Day
	//05 ("thur")
	dt.Day = ALARM_1_DAY_FIRST_DIGIT;  //Synchronise with Sunday
	// Need to set Bit6 to flag that we are defining DAY (not Date)
	dt.Day = dt.Day | (1 << 6);
					
	//Set hour (24 hr mode)
	//19 
	d1 = ALARM_1_HOUR_FIRST_DIGIT;  //First digit
	d2 = ALARM_1_HOUR_SECOND_DIGIT;  //Second digit
	dt.Hour = (d1 << 4) | d2;
	
	
	//Set minute 
	//05
	d1 = ALARM_1_MINUTE_FIRST_DIGIT;  //First digit
	d2 = ALARM_1_MINUTE_SECOND_DIGIT;  //Second digit
	dt.Minute = (d1 << 4) | d2;
	//dt.Minute = 0x19;
	
	//Set second 
	d1 = ALARM_1_SECOND_FIRST_DIGIT;  //First digit
	d2 = ALARM_1_SECOND_SECOND_DIGIT;  //Second digit
	dt.Second = (d1 << 4) | d2;
	
		
	
	// Set appropriate bit masks for Alarm1
	// NB: Default value all mask bits zero - alarm triggered when all time registers match 
#ifdef 	ALARM_1_WHEN_SECONDS_MATCH
	dt.Day = dt.Day | (1 << 7);
	dt.Hour = dt.Hour | (1 << 7);
	dt.Minute = dt.Minute | (1 << 7);
#endif
#ifdef 	ALARM_1_ONCE_PER_SECOND
	dt.Day = dt.Day | (1 << 7);
	dt.Hour = dt.Hour | (1 << 7);
	dt.Minute = dt.Minute | (1 << 7);
	dt.Second = dt.Second | (1 << 7);
#endif


	
	//Implement changes:
	set_day(ALARM_1_DAY,dt.Day);   //Set day rather than date
	usleep(100);
	set_hour(ALARM_1_HOUR,dt.Hour);
        usleep(100);
        set_minute(ALARM_1_MINUTE,dt.Minute);
        usleep(100);
        set_second(ALARM_1_SECOND,dt.Second);
	usleep(100);
	
	
	// Check the set values:
	printf ("Alarm1 Day register:  0x%X\n",get_day(ALARM_1_DAY));
	usleep(100);
	printf ("Alarm1 Hour register:  0x%X\n",get_hour(ALARM_1_HOUR));
	usleep(100);
	printf ("Alarm1 Minute register:  0x%X\n",get_minute(ALARM_1_MINUTE));
	usleep(100);
	printf ("Alarm1 Second register:  0x%X\n",get_second(ALARM_1_SECOND));
	usleep(100);
	
	
	
	
}

void get_Alarm1Time()
{
	
	char digit1, digit2;
	char mask = 0xf;
	int32_t mask1 = 0x3f; //Mask to screen out alarm flag bits 6-7	
	 
	printf("\nRTC Alarm1 Settings:\n");
		

	//Get Day
	int32_t day = (get_day(ALARM_1_DAY)) & mask1; 
	switch (day)
	{
		case 1 :
	      printf("%s","Sunday");	
		break;
		case 2 :
	      printf("%s","Monday");	
		break;
		case 3 :
	      printf("%s ","Tuesday");	
		break;
		case 4 :
	      printf("%s","Wednesday");	
		break;
		case 5 :
	      printf("%s","Thursday");	
		break;
		case 6 :
	      printf("%s","Friday");	
		break;
		case 7 :
	      printf("%s","Saturday");	
		break;
	}	
	
	//Get Hour:
	int32_t hour = (get_day(ALARM_1_HOUR)) & mask1; 
	//Get first digit:
	digit1 = hour >> 4;
	//Get second digit:
	digit2 = hour & mask;
	//printf(" -- %d%d: ",digit1,digit2);
	printf(" %d%d:",digit1,digit2);
	
	//Get minutes:
	int32_t min = (get_day(ALARM_1_MINUTE)) & mask1; 
	//Get first digit:
	digit1 = min >> 4;
	//Get second digit:
	digit2 = min & mask;
	printf("%d%d:",(int8_t) digit1, (int8_t) digit2);
	
	//Get seconds:
	int32_t sec = (get_day(ALARM_1_SECOND)) & mask1; 
	//Get first digit:
	digit1 = sec >> 4;
	//Get second digit:
	digit2 = sec & mask;
	printf("%d%d\n",digit1,digit2);
	
	        
}//get_Alarm1Time	



void reset_alarm1 (void)
{
	
	
	// Reset RTC to reference time
	set_DateTime();
	
	
	// Clear the alarm1 status bit
	char flg = rd_status_register ();
	usleep(100);
	flg = flg & 0xFE;
	set_status_register (STATUS_REG,flg);
	usleep (100);
			

}

void clear_alarm1Flg (void)
{
	// Clear the alarm1 status bit
	char flg = rd_status_register ();
	usleep(100);
	flg = flg & 0xFE;
	set_status_register (STATUS_REG,flg);
	usleep (100);
	
}// clear_alarm1Flg



void enable_alarm1_int (void)
{
   //NB - To enable the Alarm1 interrupt, we only need to enable Bit0 - A1IE
   //	  By default, INTCN (Interrupt Control bit) is set. 
   
   
   int32_t reg;
   
   //Checking default value of control register:
   reg = rd_control_register ();
   printf ("Control register: %d 0x%x\n",reg,reg);
   
   //Set the A1IE bit:
   reg = reg | (1 << 0);
    // Wait a while before resetting:
   usleep(100);
   set_control_register (CONTROL_REG,reg);	
   usleep(100);
   
   reg = rd_control_register ();
   printf ("Control register after setting A1IE bit (should be 0x1D): 0x%X\n",reg);
      
   	

}//enable_alarm1_int


char rd_control_register (void)
{
	int32_t reg = read_data(CONTROL_REG);
	return (char) reg;

}//rd_control_register


void set_control_register (char regAddr,char cr)
{
	write_data(regAddr,cr);

}//set_control_register


char rd_status_register (void)
{
	int32_t reg = read_data(STATUS_REG);
	return (char) reg;


}//rd_status_register

void set_status_register (char regAddr,char sr)
{
	write_data(regAddr,sr);
	

}


void enable_SQW (char freq)
{
	char reg = rd_control_register ();
	usleep (100);
	
	// Check value of control register:
	printf ("Control reg: 0x%0X\n", reg); 
	
	//Clear the INTCNT bit and frequency bits:
	reg = reg & ~(1 << 2);
	
	
	
	// Set INTCN bit and frequency bits:
	if (freq == SQUARE_WAVE_1Hz)
	{
		printf ("Setting for 1Hz ...\n");
		reg = reg & ~(1 << 3);
		reg = reg & ~(1 << 4);
		
	}
	else if (freq == SQUARE_WAVE_1KHz)
	{
		printf ("Setting for 1kHz ...\n");
		reg = reg & ~(1 << 4);
		reg = reg | (1 << 3);
		
	}
	else if (freq == SQUARE_WAVE_4KHz)
	{
		reg = reg | (1 << 4);
		reg = reg & ~(1 << 3);
		
	}
	else if (freq == SQUARE_WAVE_8KHz)
	{
		reg = reg | (1 << 4);
		reg = reg |(1 << 3);
		
	}
	 
	printf ("Control reg after edit: 0x%0X\n", reg);
	
	
	
	// Write to control register:
	set_control_register (CONTROL_REG,reg);
	usleep (100); 
	

}//set_SQW_Freq


void close_device()
{
	close(file);
	printf("\nClosed I2C adaptor OK\n");
	
}



