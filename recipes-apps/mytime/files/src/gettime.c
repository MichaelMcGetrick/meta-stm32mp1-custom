/* --------------------------------------------------------------------------------------------
 * TEST PROGRAM TO RUN DS3231 FROM USER SPACE
 * 
 * 
 * 
 *  -------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#define NUM_RTC_REGS 19    //Number of registers provided by DS3231 - RTC 
char ds3231[NUM_RTC_REGS] = {0};
#define EEPROM_SIZE 4000 //1024    //Define the number of bytes in the EERPOM 
char eeprom[EEPROM_SIZE] = {0};


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
// TEMPERATURE register addresses
#define TEMP_UPPER 0x11
#define TEMP_LOWER 0x12


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

struct DateTime dt;
 
int file;
int adapter_num = 1; //Number of an existing h/w adapter (adapter is h/w relating to a specific i2c bus)
					//This is best to be determined dynamically (can chnage from boot to boot)	

char filename[20];

//Declare i2c slave address:
int i2c_cl_addr;



void init_device(int addr);
int32_t read_data(char addr);
void write_data(char addr, char data);
void close_device(void);

void get_DateTime(void);
void set_DateTime(void);
void set_year(char y);
int32_t get_year(void);
void set_month(char m);
int32_t get_month(void);
void set_hour(char h);
int32_t get_hour(void);
void set_minute(char m);
int32_t get_minute(void);
void set_second(char s);
int32_t get_second(void);
void set_date(char d);
int32_t get_date(void);
void set_day(char d);
int32_t get_day(void);
void read_temp (void);





int main(int argc, char* argv[])   //argc: argument count (inc program name)
{                                  //argv: the argument vector array (ind program name)
	
	
	
	
	if (strcmp (I2C_DEV,"RTC") == 0)
	{
		i2c_cl_addr = RTC_SLAVE_ADDR;
	}
	if (strcmp (I2C_DEV,"EEPROM") == 0)
	{
		i2c_cl_addr = EEPROM_SLAVE_ADDR;
	}		
	
	printf("I2C Slave address: %x\n",i2c_cl_addr);
	
	
	//Initialise I2c adaptor 
	init_device(i2c_cl_addr);	
	
		
	char ans;
   
      	printf("Do you want to set the date? (y/n): ");
      	scanf("%c", &ans);
      	if (ans == 'y')
      	{
      		set_DateTime();
		usleep(500);
      	
      	}
   
	// Get the Date and Time:
	int numReadings = 10;
	int delay = 5;
	for (int i = 0; i < numReadings; i++)
	{
		if (i > 0)	
			sleep (delay);
		get_DateTime();
		usleep(500);
		read_temp();
	}
		
	//Close device:
	close_device();
		
	
}

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
	if( ioctl(file,I2C_SLAVE,i2c_cl_addr) < 0 )
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

void set_year(char y)
{
	write_data(RTC_YEAR,y);
}	
int32_t get_year()
{
	int32_t year = read_data(RTC_YEAR);
	return year;
}


void set_month(char m)
{
	write_data(RTC_MONTH,m);
}	
int32_t get_month()
{
	int32_t month = read_data(RTC_MONTH);
	return month;
}


void set_day(char d)
{
	write_data(RTC_DAY,d);
}	
int32_t get_day()
{
	int32_t day = read_data(RTC_DAY);
	return day;
}


void set_date(char d)
{
	write_data(RTC_DATE,d);
}	
int32_t get_date()
{
	int32_t date = read_data(RTC_DATE);
	return date;
}



void set_hour(char h)
{
	write_data(RTC_HOUR,h);
}	
int32_t get_hour()
{
	int32_t hour = read_data(RTC_HOUR);
	return hour;
}

void set_minute(char m)
{
	write_data(RTC_MINUTE,m);
}	
int32_t get_minute()
{
	int32_t min = read_data(RTC_MINUTE);
	return min;
}

void set_second(char s)
{
	write_data(RTC_SECOND,s);
}	
int32_t get_second()
{
	int32_t sec = read_data(RTC_SECOND);
	return sec;
}


void get_DateTime()
{
	char digit1, digit2;
	char mask = 0xf;
	
	printf("\nRTC Date and Time:\n");
		

	//Get Day
	switch (get_day())
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
	char date = get_date();
	//Get first digit:
	digit1 = date >> 4;
	//Get second digit:
	digit2 = date & mask;
	printf(", %d%d ",digit1, digit2);
	
	//Get Month
	//Get first digit:
	char month = get_month();
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
    char year = get_year();
	//Get first digit:
	digit1 = year >> 4;
	//Get second digit:
	digit2 = year & mask;
	
	printf(", 20%d%d |",digit1,digit2);
	
	//Get Hour:
	char hour = get_hour();
	//Get first digit:
	digit1 = hour >> 4;
	//Get second digit:
	digit2 = hour & mask;
	//printf(" -- %d%d: ",digit1,digit2);
	printf(" %d%d:",digit1,digit2);
	
	//Get minutes:
	int8_t min = get_minute();
	//Get first digit:
	digit1 = min >> 4;
	//Get second digit:
	digit2 = min & mask;
	printf("%d%d:",(int8_t) digit1, (int8_t) digit2);
	
	//Get seconds:
	char sec = get_second();
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
	char d1 = 0x02;  //First digit
	char d2 = 0x04;  //Second digit
	dt.Year = (d1 << 4) | d2;
			
	//Set Day
	//05 ("thur")
	dt.Day = 0x05;  //Synchronise with Sunday
	
	//Set Date:
	//18
	d1 = 0x01;  //First digit
	d2 = 0x08;  //Second digit
	dt.Date = (d1 << 4) | d2;
	
	
	//Set month:
	//10
	d1 = 0x01;  //Lower bits
	d2 = 0x00;  //Upper bits
	dt.Month = (d2 << 4) | d1;
		
	//Set hour (24 hr mode)
	//19 
	d1 = 0x01;  //First digit
	d2 = 0x09;  //Second digit
	dt.Hour = (d1 << 4) | d2;
	
	
	//Set minute 
	//05
	d1 = 0x00;  //First digit
	d2 = 0x05;  //Second digit
	dt.Minute = (d1 << 4) | d2;
	//dt.Minute = 0x19;
	
	//Set second 
	//0
	d1 = 0x00;  //First digit
	d2 = 0x00;  //Second digit
	dt.Second = (d1 << 4) | d2;
	
	
		
	
	//Implement changes:
	set_year(dt.Year);
	usleep(100);
	set_day(dt.Day);
	usleep(100);
	set_date(dt.Date);
	usleep(100);
	set_month(dt.Month);
	usleep(100);
        set_hour(dt.Hour);
        usleep(100);
        set_minute(dt.Minute);
        usleep(100);
       set_second(dt.Second);
    
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


void close_device()
{
	close(file);
	printf("\nClosed I2C adaptor OK\n");
	
}

