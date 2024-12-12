/*
 ====================================================================================================================
 Name        : main.c
 Author      : Michael McGetrick
 Version     :
 Description : Program test application to check spi bus and mcp3008 by sampling data at a given frequency
              and displaying ascii data on stadard output.
              
              There are three types of test:
              SAMPLE_VALUE_TEST:	Check that the values of samples are correct
              SAMPLE_RATE_TEST:		Get the maximum sample rate achievable by fast looping (no time delays)
              SAMPLING_INTEGRITY:	Check that the number of observed samples is the same as the expected number or
              				samples. Due to the linux OS not being real time, we need to assess how many samples
              				may be dropped.	
              				
              SAMPLING_DELAY_ONLY	This is a subtest of SAMPLING_INTEGRITY. Only the Linux delay function is tested
              				(ADC sampling omitted).
              				To run this option, define both
              				#define SAMPLING_INTEGRITY
              				#define SAMPLING_DELAY_ONLY						
               
              NB: This program must be run as root to access GPIO and SPI functionality 
              
              
              USING RTC MODULE DS3231 AS AN ALARM TO SEND INTERRUPT SIGNAL WHEN SAMPLING TIME REACHED
              ---------------------------------------------------------------------------------------
              We will use this method to try and alleviate excessive use of Linux functions for processing
              time difference in order to stop the sampling loop. We use STM32MP1 GPIO pin PA8 as an interrupt pin
              which receives the trigger from the RTC SQW/INT pin. A node is provided in the Device Tree and a kernel
              driver configured to this pin is run as a Loadable Kernel Module (LKM). 
              
              This functionality is enabled by defining GPIO_CONFIG
              i.e. include
              #define GPIO_CONFIG
              
              The alarm trigger time interval is defined in rtc.h by setting appropriate values for
              #define ALARM_1_SECOND_FIRST_DIGIT	0x03
	      #define ALARM_1_SECOND_SECOND_DIGIT	0x00	

	      The above example sets the alarm trigger interval to 30sec. (See datasheet for details) 		
              
              Before running the application, the kernel module must be loaded with the command
              
              modprobe alarm_driver
              
              
              
              NB: Curently, GPIO_CONFIG functionality is only available for SAMPLING_INTEGRITY tests.   


 	 	 	   
 ===================================================================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <signal.h>

#include "mcp3008.h"


#include <sys/mman.h>  //temp
#include <sys/types.h>
#include "rtc.h"
#include "i2c.h"




//DEFINES ---------------------------------------
//#define SAMPLING_RATE     4000.0f//1000.0f  
//#define NUM_ADC_CHANS 		3
#define SAMPLE_RATE	   1.0f //1000.0f //1000.0f
#define DATA_LEN	   100000.0f //400000.0f //100.0f //1000.0f

//Test type defines:
#define SAMPLE_RATE_TEST    		//Get sample rate for given length of data samples (no system delays between samples)
//#define SAMPLE_VALUE_TEST   		// Get sample values for given length of data samples
//#define SAMPLING_INTEGRITY		// Test to whether expected number of samples have been gathered in specified time 
//#define SAMPLING_DELAY_ONLY		// Test only the delay function (mcp3008 sampling omitted)
				

// DEFINE TO INCLUDE USE OF RTC AND GPIO PA8 as input from RTC interrupt
// Enable this #define to enable an alarm interrupt from the gpio driver
//#define GPIO_CONFIG
// GPIO CONTROL ATTRIBUTES -------------------------------------------------------------------------------------------------
//IOCTL defines (for communication to kernel:
#define REG_CURRENT_TASK _IOW('a','a',int32_t)  //We use this to give a user application registration request to the kernel
#define SIGMJM  44    //Signal value to identity user app
char chrdev_name[20];
int fd, ret;
int alarm_flg;
//---------------------------------------------------------------------------------------------------------------------------
 




//ATTRIBUTES: -----------------------------------------
int EXIT_PROGRAM = 	0;


//MCP3008 Parameters
int chanIndex = 7; // Index of ADC channel
int readMode = 1 ; //Single:1; Differential: 0
//int chanVal[NUM_ADC_CHANS];
 
uint64_t sample_delay;
float data[2000];

struct timespec req,rem;

//File attributes
FILE *fp;
char *f_name = "delay_only.txt";
bool saveToFile = true;


// Sample Rate test attributes:
struct timeval tv_start,tv_end;
long unsigned time_diff = 0;
// Sampling integrity attributes
long unsigned sample_cnt_req, sample_cnt_obs;
long unsigned sampling_time = 30; //Sampling time in seconds
//float sample_rates[] = {1.0f,2.0f,5.0f,10.0f,25.0f,50.0f,100.0f,250.0f,500.0f,1000.0f,2000.0f,2500.0f,3000.0f};
float sample_rates[] = {1024.0f};

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

//Declare i2c slave address:
int i2c_cl_addr;

#endif



//FUNCTION DECLARATIONS -------------------------
void ctrl_c_handler(int sig);
void sys_delay();

void run_test();


//-----------------------------------------------
int main(void) {

   //Register handler for capturing CTL-C signal:
   signal(SIGINT,ctrl_c_handler);
            
	
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
		

	
	
	
#endif	   
	
	
	
#ifndef SAMPLING_INTEGRITY	
	    
	//Initialise SPI for MCP3008:
	printf("Initialising SPI for MCP3008 peripheral.....\n");
	
	

	int res = MCP3008_Init(SAMPLE_RATE);
	if(res < 0)
	{
		printf("Problem initialising device - exiting program!\n");
		exit(0);	
	}	
#endif
	


#ifdef SAMPLE_VALUE_TEST	
	//set parameters (if not default)
	//setParams(readMode,chanIndex);	
	printf("Sample rate: %f\n",SAMPLE_RATE);
	
					 
	sample_delay = getSamplingRate();
	printf("Delay in micro-secs: %lld\n",sample_delay);
		
	
	if(sample_delay < 1000000)
	{
		req.tv_sec = 0;
	   req.tv_nsec = sample_delay*1000;
	}
	else
	{
		req.tv_sec = sample_delay/1000000;
	   req.tv_nsec = 0;
		
	}	

#endif

#ifdef SAMPLING_INTEGRITY
	printf("PERFORMING SAMPLING INTEGRITY TEST .... \n");
#endif
#ifdef SAMPLING_DELAY_ONLY
	printf("(ADC sampling excluded) \n");
#endif



#ifdef SAMPLE_RATE_TEST
	printf("PERFORMING MAX. SAMPLE RATE TEST .... \n");
	

#endif



	run_test();
	
	// Close device
	//MCP3008_Close();

	
	printf("Exiting program .\n");
	exit(0);      
	


}//main


void sys_delay()
{
	//Notes:
	// If the interval specified in struct timespec req is not an exact
	// multiple of the underlying clock, then the interval will be rounded
	// up to the next multiple.
	//TBC for more accuracy with fast sampling
	
	
	//Delay execution of program for desired time:
	int res = nanosleep(&req,&rem);
	if(res < 0)
	{
#ifndef GPIO_CONFIG	
		printf("nanosleep() ERROR!\n");
		printf("Error number %d: %s\n",errno,strerror(errno));
		if(errno == EINVAL)
		{
			printf("Nanosecond parameter out of bounds (< 0 or > 999999999)\n");
		}
		if(errno == EINTR)
		{
			printf("Sleep interrupted by a signal handler\n");
			//USe this to schedule the remainin time
			//If we have defined &rem, this will contain the remaining time
			//needed to complete request by calling nanosleep(&rem,NULL);
		}
#endif			
		
	}	
	
	
}//sys_delay	


void run_test()  
{
	
	float sample_val = 0.0;
	
	//Open file for data:
	 if (saveToFile)
	 {
		 if( (fp=fopen(f_name,"wt")) == NULL)
		 {
			printf("Error opening file -aborting program!\n");
			exit(0);	 
		 }
    }
 
	
	float data_val;
	if ( strcmp(DRIVER_NAME, "spidev0.0") == 0)
	{

		//Define transfer words required for using SPIDEV
		setSpiDevTxWords();
		printf("SPI_CTRL_CFG: 0x%x\n",SPI_CTRL_CFG);

		while (EXIT_PROGRAM == 0)
		{
			//Get sample
#ifdef SAMPLE_RATE_TEST			
			gettimeofday(&tv_start,NULL);
			//for(int i=0; i< 1;i++)
			for(int i=0; i< DATA_LEN;i++)
			{
				sys_delay();
				sample_val = readSpiDev();
											
			}
			gettimeofday(&tv_end,NULL);	
		 	time_diff = (tv_end.tv_sec*(uint64_t)1000000 + (uint64_t)tv_end.tv_usec)
					  - (tv_start.tv_sec*(uint64_t)1000000 + (uint64_t)tv_start.tv_usec);
			//Close the devuice:
   			MCP3008_Close();
   			EXIT_PROGRAM = 1;		  
					  
					  
#endif

#ifdef SAMPLING_INTEGRITY			
			
#ifdef GPIO_CONFIG			
			// Set Alarm1 configuration:
      	 		printf ("Setting Alarm1 Configuration ..\n");
      			set_alarm1 ();
      			printf ("Setting RTC ref. time ..\n");
      			set_DateTime();
#endif			
			
			int array_len = sizeof(sample_rates) / sizeof(sample_rates[0]);		
			printf ("Sample array length: %d\n",array_len);
					
			// Print header information to file:
			fprintf (fp,"INTEGRITY TEST: \n");
			fprintf (fp,"%s%s%s%s\n","Rate,", "Cnt-Exp,", "Cnt-Obs,", "Integrity(%)");
			
			//for (int i=0;i < 2; i++)
			for (int i=0;i < array_len; i++)
			{
				
				printf ("Starting Test - SAMPLE RATE %.2f  %s\n",sample_rates[i],"(Hz)");
#ifndef GPIO_CONFIG				
				// Wait for a while before commencing with next sample rate:
				int test_delay = 5;
				printf("Waiting for %d secs ....  \n",test_delay);
								
				req.tv_sec = test_delay;
				req.tv_nsec = 0;
				sys_delay ();
#endif				
																
				
				int res = MCP3008_Init( sample_rates[i]);
				if(res < 0)
				{
					printf("Problem initialising device - exiting program!\n");
					exit(0);	
				}
				
				
								 
				sample_delay = getSamplingRate();
				printf("Delay in micro-secs: %lld\n",sample_delay);
					
				
				if(sample_delay < 1000000)
				{
					req.tv_sec = 0;
				   req.tv_nsec = sample_delay*1000;
				}
				else
				{
					req.tv_sec = sample_delay/1000000;
				   req.tv_nsec = 0;
					
				}
				
				
				printf ("Time assigned for integrity test: %d (sec) ... \n", sampling_time);
				//Initialise observed sample count:
				sample_cnt_obs = 0;
				// Expected sample count:
				sample_cnt_req = (sampling_time * 1000000) / sample_delay;
										
#ifdef GPIO_CONFIG			
				if (i > 0)
				{				
					reset_alarm1 ();
					usleep (100);
				}
#else						
				//Perform sampling integrity test for this sample rate
				gettimeofday(&tv_start,NULL);
#endif				
				int cnt = 0;
				while (1)
				{
#ifdef SAMPLING_DELAY_ONLY				
					sys_delay();
#else
					sys_delay();
					sample_val = readSpiDev();
#endif
					sample_cnt_obs ++;
#ifdef GPIO_CONFIG
					//Check value of PA8:
					//int val = gpio_val ();
					//printf ("Cnt: %d \n",sample_cnt_obs);
					if (alarm_flg)
					{
						printf ("Alarm flag has been triggered!\n");
						alarm_flg = 0;
						break;	
					}
					
				
#else					
					gettimeofday(&tv_end,NULL);
					time_diff = (tv_end.tv_sec*(uint64_t)1000000 + (uint64_t)tv_end.tv_usec)
						  - (tv_start.tv_sec*(uint64_t)1000000 + (uint64_t)tv_start.tv_usec);
					if (time_diff >= sampling_time *1000000)
					{
						break;
					}
#endif						  
					
				}
								
								
				//Close the device:
	   			MCP3008_Close();
	   			  			
	   			printf("TEST RESULTS --------------------------------------------\n");	   						
	   			printf("Sampling time (s):  %lu\n",sampling_time);
				printf("Expected sample count:  %lu\n",sample_cnt_req);
				printf("Actual sample count:  %lu\n",sample_cnt_obs);
				int sample_integrity = (sample_cnt_obs * 100) / sample_cnt_req;
				printf ("Sample rate integrity: %d\%\n",sample_integrity);
				printf("----------------------------------------------------------\n");
				if(saveToFile)
				{ 
				 fprintf (fp,"%.2f%c%lu%c%lu%c%d\n",sample_rates[i],',',sample_cnt_req,',',sample_cnt_obs,',',sample_integrity);
				}
						   			
	   			
   			}
   			if (saveToFile)
   			{
   			   fclose(fp);
   			}
   			EXIT_PROGRAM = 1;   
   				  
#endif



#ifdef SAMPLE_VALUE_TEST
			for(int i=0; i< DATA_LEN;i++)
			{
				//Delay for set time:
				sys_delay();
				data [i] = readSpiDev();
				printf ("Data val: %f\n",data [i]);

			}
		

					  
			if (saveToFile)
			{
			    //Save data to file and exit program
			    for (int i = 0; i < DATA_LEN; i++)
			    {
			    	fprintf(fp, "%f \n", data[i]);
   				
   			    }
   			    fclose(fp);
   			}    
   			    
   			    //Close the devuice:
   			    MCP3008_Close();
   			    exit (0);	
#endif   		    
				 		


		}//while (EXIT_PROGRAM == 0)

	}//if (strcmp(DRIVER_NAME, "mcp3008") == 0 )
	else if ( strcmp(DRIVER_NAME, "mcp3008") == 0)

	{
		printf ("Running custom driver for mcp3008!\n");
		while (EXIT_PROGRAM == 0)
		{
			//Get sample
#ifdef SAMPLE_RATE_TEST			
			gettimeofday(&tv_start,NULL);
			for(int i=0; i< DATA_LEN;i++)
			{
				sample_val = readADC();
				
			}
			gettimeofday(&tv_end,NULL);	
		 	time_diff = (tv_end.tv_sec*(uint64_t)1000000 + (uint64_t)tv_end.tv_usec)
					  - (tv_start.tv_sec*(uint64_t)1000000 + (uint64_t)tv_start.tv_usec);
			//Close the devuice:
   			MCP3008_Close();
   			EXIT_PROGRAM = 1;		  
#endif

#ifdef SAMPLING_INTEGRITY			
			
#ifdef GPIO_CONFIG			
			// Set Alarm1 configuration:
      	 		printf ("Setting Alarm1 Configuration ..\n");
      			set_alarm1 ();
      			printf ("Setting RTC ref. time ..\n");
      			set_DateTime();
#endif			

			
			int array_len = sizeof(sample_rates) / sizeof(sample_rates[0]);		
			printf ("Sample array length: %d\n",array_len);
					
			// Print header information to file:
			fprintf (fp,"INTEGRITY TEST: \n");
			fprintf (fp,"%s%s%s%s\n","Rate,", "Cnt-Exp,", "Cnt-Obs,", "Integrity(%)");
			
			//for (int i=0;i < 2; i++)
			for (int i=0;i < array_len; i++)
			{
								
	
				printf ("Starting Test - SAMPLE RATE %.2f  %s\n",sample_rates[i],"(Hz)");
#ifndef GPIO_CONFIG				
				// Wait for a while before commencing with next sample rate:
				int test_delay = 5;
				printf("Waiting for %d secs ....  \n",test_delay);
								
				req.tv_sec = test_delay;
				req.tv_nsec = 0;
				sys_delay ();
#endif				
																	
				
				int res = MCP3008_Init( sample_rates[i]);
				if(res < 0)
				{
					printf("Problem initialising device - exiting program!\n");
					exit(0);	
				}
				
				
								 
				sample_delay = getSamplingRate();
				printf("Delay in micro-secs: %lld\n",sample_delay);
					
				
				if(sample_delay < 1000000)
				{
					req.tv_sec = 0;
				   req.tv_nsec = sample_delay*1000;
				}
				else
				{
					req.tv_sec = sample_delay/1000000;
				   req.tv_nsec = 0;
					
				}
				
				
				printf ("Time assigned for integrity test: %d (sec) ... \n", sampling_time);
				//Initialise observed sample count:
				sample_cnt_obs = 0;
				// Expected sample count:
				sample_cnt_req = (sampling_time * 1000000) / sample_delay;

#ifdef GPIO_CONFIG			
				if (i > 0)
				{				
					reset_alarm1 ();
					usleep (100);
				}
#else						
				//Perform sampling integrity test for this sample rate
				gettimeofday(&tv_start,NULL);
#endif				

				while (1)
				{
#ifdef SAMPLING_DELAY_ONLY				
					sys_delay();
#else
					sys_delay();
					sample_val = readSpiDev();
#endif
					sample_cnt_obs ++;
				
#ifdef GPIO_CONFIG
					//Check value of PA8:
					//int val = gpio_val ();
					//printf ("Cnt: %d gpio val: %d\n",sample_cnt_obs,val);
					if (alarm_flg)
					{
						printf ("Alarm flag has been triggered!\n");
						alarm_flg = 0;
						break;	
					}
				
#else					
					gettimeofday(&tv_end,NULL);
					time_diff = (tv_end.tv_sec*(uint64_t)1000000 + (uint64_t)tv_end.tv_usec)
						  - (tv_start.tv_sec*(uint64_t)1000000 + (uint64_t)tv_start.tv_usec);
					if (time_diff >= sampling_time *1000000)
					{
						break;
					}
#endif						  
					
				}
								
								
				//Close the device:
	   			MCP3008_Close();
	   			  			
	   			printf("TEST RESULTS --------------------------------------------\n");	   						
	   			printf("Sampling time (s):  %lu\n",sampling_time);
				printf("Expected sample count:  %lu\n",sample_cnt_req);
				printf("Actual sample count:  %lu\n",sample_cnt_obs);
				int sample_integrity = (sample_cnt_obs * 100) / sample_cnt_req;
				printf ("Sample rate integrity: %d\%\n",sample_integrity);
				printf("----------------------------------------------------------\n");
				if(saveToFile)
				{ 
				 fprintf (fp,"%.2f%c%lu%c%lu%c%d\n",sample_rates[i],',',sample_cnt_req,',',sample_cnt_obs,',',sample_integrity);
				}
						   			
	   			
   			}
   			if (saveToFile)
   			{
   			   fclose(fp);
   			}
   			EXIT_PROGRAM = 1;   
   				  
#endif

#ifdef SAMPLE_VALUE_TEST
			for(int i=0; i< DATA_LEN;i++)
			{
				//Delay for set time:
				sys_delay();
				data [i] = readADC();
				printf ("Data val: %f\n",data [i]);

			}

					  
			if (saveToFile)
			{
			    //Save data to file and exit program
			    for (int i = 0; i < DATA_LEN; i++)
			    {
			    	fprintf(fp, "%f \n", data[i]);
   				
   			    }
   			    fclose(fp);
   			    
   			    	
			}
			//Close the devuice:
   			MCP3008_Close();
   			exit (0);
	 		
#endif

		}//while (EXIT_PROGRAM == 0)


	}//if (strcmp(DRIVER_NAME, "mcp3008") == 0 )
	
#ifdef SAMPLE_RATE_TEST	
	 printf("Time difference for loop (us):  %lu\n",time_diff);
	 printf("Data Length:: %lu\n",(unsigned long)DATA_LEN);
	 printf("Time difference per sample (us):  %lu\n",time_diff / (unsigned long)DATA_LEN);
	 	 
	 float eff_rate = 1.0/( ((float)time_diff/(float)DATA_LEN))*1000000;
	 printf("Maximum measured sampling rate (S/sec):  %.2f\n",eff_rate) ;       
#endif		

#ifdef GPIO_CONFIG
	// Close I2C device (RTC)
	close_device ();
	// Close char device:
	close(fd);
	
#endif	
		
	
}//run_test



//NB: This handler is working for the CTL-C keyboard signal
//	  This will exit the while loop sple (us):  1157
//   to exit the program gracefully
//	  (We need to close the connection to the serial terminal)
void ctrl_c_handler(int sig)
{
   if(sig == SIGINT) //Check it is the right user ID //NB: Not working at the moment
   {
         printf("\nWe have received the CTRL-C signal - aborting sample looping!\n");
        // Close device
         MCP3008_Close();

         
         //EXIT_PROGRAM = 1; //This will stop program
         exit(0);
   }
   
         
}//ctrl_c_handler(int sig)


    



