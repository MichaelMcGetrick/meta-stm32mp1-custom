/* --------------------------------------------------------------------------------------------
 * TEST PROGRAM TO RUN DS3231(RTC) FROM USER SPACE
   Author: Michael McGetrick
 * --------------------------------------------------------------------------------------------
 * The program can be run in three modes:
 
   RTC_CLOCK: 
   The program will run to show current RTC time for a specific interval of time. 
   The user has the opportunity to reset the time on initial startup if required.
   
   SQUARE_WAVE:
   If configured for SQUARE_WAVE the SQW pin will output a square wave. The user can configure for:
   1 Hz
   1.024kHz
   4.096kHz
   8.192kHz
   
   ALARM_TRIGGER:
   The RTC is configured to trigger an interrupt on SQW pin after a certain time period. In this mode,
   the SQW falls from HIGH to LOW when the alarm condition is met.
   This mode can be realised in two ways:
   i) The alarm condition is detected by user the status register of the DS3231 device
   ii) GPIO pin PA8 on the STM32MP1 is wired to RTC SQW and configured as an interrupt pin to detect the
       alarm condition. This is performed by use of the Linux Kernel module  alarm_driver.ko.
       The driver will send a signal to the user app when a trigger event has been detected.
       To use this method, the kernel module must be inserted in the kernel using the command
       
       $> modprobe alarm_driver
       
       before instantiating the user application.
         
   
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
#include <linux/gpio.h>
#include <signal.h>
#include "rtc.h"
#include "i2c.h"

// Enable this #define to enable an alarm interrupt from the gpio driver
#define GPIO_CONFIG


// GPIO CONTROL ATTRIBUTES -------------------------------------------------------------------------------------------------
//IOCTL defines (for communication to kernel:
#define REG_CURRENT_TASK _IOW('a','a',int32_t)  //We use this to give a user application registration request to the kernel
#define SIGMJM  44    //Signal value to identity user app
char chrdev_name[20];
int fd, ret;
int alarm_flg;
//---------------------------------------------------------------------------------------------------------------------------
 
 
// Define the test type
char* test_type = "RTC_CLOCK"; //ALARM_TRIGGER"; //"RTC_CLOCK"; //"SQUARE_WAVE"

//Declare i2c slave address:
int i2c_cl_addr;

void ctrl_c_handler(int sig);


#ifdef GPIO_CONFIG
//Callback function for processing signal from kernel
void sig_event_handler(int n, siginfo_t *info,void *unused)
{
            
      int check;
      char buf;
      if(n == SIGMJM)  //Check we have the right user id:
      {
         check = info->si_int;
         //printf("Received signal from kernel: Value = %d\n",check);
         alarm_flg = 1;
               
      } 
      
}// sig_event_handler   
#endif



// MAIN PROGRAM STARTS HERE
int main(int argc, char* argv[])   //argc: argument count (inc program name)
{                                  //argv: the argument vector array (ind program name)

	
#ifdef GPIO_CONFIG	
   	   int32_t number;
	   struct sigaction act;
	   
	   // Initialise alarm flag
	   alarm_flg = 0;
	   
	   //Set up signal handler:
	   //sigemptyset (&act.sa_mask);
	   //act.sa_flags = (SA_SIGINFO | SA_RESTART);
	   act.sa_flags = SA_SIGINFO;
	   act.sa_sigaction = sig_event_handler;
	   if( sigaction(SIGMJM,&act,NULL) == -1)
	   {
	      printf("Problem setting up signal handler for SIGMJM = %d\n",SIGMJM);
	   }   
	   else
	   {
	      printf("Signal handler for SIGMJM = %d  set up OK!\n" ,SIGMJM);   
	   }   
      
	
	
	// Check the char device:
	strcpy(chrdev_name, "/dev/rtc_alarm");
	//Open alarm char device */
	fd = open(chrdev_name, 0);
	if (fd == -1) {
		ret = -errno;
		printf("Failed to open device\n");

		return ret;
	}
	else
	{
		printf("Opened char device OK!\n");
	}
	
	
	
	//Register this application with the kernel:
	if( ioctl(fd, REG_CURRENT_TASK,(int32_t*) &number) < 0)
	{
	      printf("User registration with kernel module has failed\n");
	      close(fd);
	      exit(1);
	}   
	printf("User registration with kernel module SUCCESSFUL!\n");
	
#endif	   
	
	
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


	// Ensure status interrupt alarm bit is cleared to ensure we can trigger alarms
	clear_alarm1Flg ();	
		

	
	char ans;
         	
      	if (strcmp(test_type,"ALARM_TRIGGER") == 0)
      	{
      			      	
	      	// Set Alarm1 configuration:
      	 	printf ("Setting Alarm1 Configuration ..\n");
      		set_alarm1 ();
      		printf ("Setting RTC ref. time ..\n");
      		set_DateTime();
		
	      	
		int num_triggers = 5;
		for (int i=0;i< num_triggers;i++)
		{
			printf ("STARTING NEW ALARM TEST %d\n",i);
			if (i > 0)
			{				
				usleep (1000000); // Leave a bit of time to see changes on scope
				reset_alarm1 ();
				usleep (100);
			}
			
			int cnt = 1;
			while (1)
			{	
				//printf ("Waiting for flag ...\n");
				//char flg = rd_status_register ();
				usleep (1000000);
				
				
#ifdef GPIO_CONFIG				
				printf ("Cnt: %d\n",cnt);
				if (alarm_flg)
				{
					printf ("Alarm flag has been triggered!\n");
					alarm_flg = 0;
					break;	
				}
#else				
				
				char flg = rd_status_register ();
				printf ("Cnt: %d  Status register: 0x%X\n",cnt,flg);
				if ((flg & 0x01) > 0)
				{
					printf ("Status register: 0x%X\n",flg);
					printf ("Interrupt has been triggered!\n");
					break;
					
				}
				
#endif				
				
				cnt = cnt + 1;
			
			}
		}
			
		// Clear the alarm1 status bit and set INT/SQW to INACTIVE (HIGH)
		char flg = rd_status_register ();
		usleep(100);
		flg = flg & 0xFE;
		set_status_register (STATUS_REG,flg);
		usleep (100);	
			
		//Close RTC device:
		close_device();
		
#ifdef GPIO_CONFIG		
		// Close char device:
		close(fd);
#endif		
		
	}// ALARM_TRIGGER_TEST	
	
	if (strcmp(test_type,"RTC_CLOCK") == 0)
      	{
      		
      		// Set Alarm1 configuration:
	      		      	
	      	printf("Do you want to set the date? (y/n): ");
	      	scanf("%c", &ans);
	      	if (ans == 'y')
	      	{
	      		set_DateTime();
			usleep(500);
	      	
	      	}
	   
				
		
		// Get the Date and Time:
		int numReadings = 4;
		int delay = 5;
		for (int i = 0; i < numReadings; i++)
		{
			if (i > 0)	
				sleep (delay);
			//get_DateTime();
			get_Alarm1Time();
			usleep(500);
			read_temp();
		}
			
		//Close device:
		close_device();
      	
	
	 
	}//RTC_CLOCK TEST
	

	if (strcmp(test_type,"SQUARE_WAVE") == 0)
      	{
      	
		      	
	      	// Set Square Wave Frequency:
      	 	printf ("Enabling Square wave ...\n");
      		enable_SQW (SQUARE_WAVE_4KHz);
      		usleep (500);
      		
      		int num_secs = 100;
      		//while (1)
      		for (int i=0; i < num_secs; i++)
      		{
      			usleep (1000000);
      			printf ("Secs: %d\n", i);
      		}
			
		// Disable the square wave:
		//Set the INTCNT bit:
		char reg = rd_control_register ();
		usleep (100);
		reg = reg | (1 << 2);
		
		printf ("Disabling SQW  0x%0X\n",reg);
		// Write to control register:
		set_control_register (CONTROL_REG,reg);
		usleep (100); 
		
			
		//Close device:
		close_device();
		
		
	}// SQUARE_WAVE	
	

	
}


//NB: This handler is working for the CTL-C keyboard signal
//	  This will exit the while loop sple (us):  1157
//   to exit the program gracefully
//	  (We need to close the connection to the serial terminal)
void ctrl_c_handler(int sig)
{
   if(sig == SIGINT) //Check it is the right user ID //NB: Not working at the moment
   {
         printf("\nWe have received the CTRL-C signal - aborting sample looping!\n");
        
        
        if (strcmp(test_type,"SQUARE_WAVE") == 0)
      	{
		//Disable the Square wave:
		
		//Set the INTCNT bit:
		char reg = rd_control_register ();
		usleep (100);
		reg = reg | (1 << 2);
		
		printf ("Edited control register  0x%0X\n",reg);
		// Write to control register:
		set_control_register (CONTROL_REG,reg);
		usleep (100); 
	
		
		// Close device
		 close_device();
	}
         
         //EXIT_PROGRAM = 1; //This will stop program
         exit(0);
   }
   
         
}//ctrl_c_handler(int sig)



