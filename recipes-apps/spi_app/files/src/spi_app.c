/*
 ======================================================================================
 Name        : main.c
 Author      : Michael McGetrick
 Version     :
 Description : Program test application to check spi bas and mcp3008 by sampling data at a given frequency
              and displaying ascii data on stadard output.
               
              NB: This program must be run as root to access GPIO and SPI functionality 


 	 	 	   
 =====================================================================================
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

#include "mcp3008.h"


#include <sys/mman.h>  //temp
#include <sys/types.h>



//DEFINES ---------------------------------------
//#define SAMPLING_RATE     4000.0f//1000.0f  
#define NUM_ADC_CHANS 		3
#define SAMPLE_RATE	   100.0f //1000.0f
#define DATA_LEN	   100.0f //1000.0f

//ATTRIBUTES: -----------------------------------------
int EXIT_PROGRAM = 	0;


//MCP3008 Parameters
int chanIndex = 7; // Index of ADC channel
int readMode = 1 ; //Single:1; Differential: 0
int chanVal[NUM_ADC_CHANS];
 
uint64_t sample_delay;
float data[2000];

struct timespec req,rem;

//File attributes
FILE *fp;
char *filename = "spi_data.txt";
bool saveToFile = false;



//FUNCTION DECLARATIONS -------------------------
void ctrl_c_handler(int sig);
void sys_delay();

void run_test();


//-----------------------------------------------
int main(void) {

   //Register handler for capturing CTL-C signal:
	signal(SIGINT,ctrl_c_handler);
            
	
	    
	//Initialise SPI for MCP3008:
	printf("Initialising SPI for MCP3008 peripheral.....\n");
	

	

	int res = MCP3008_Init(SAMPLE_RATE);
	if(res < 0)
	{
		printf("Problem initialising device - exiting program!\n");
		exit(0);	
	}	


	
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



	run_test();
	
	// Close device
	//MCP3008_Close(void);

	
	printf("Exiting program!\n");
	exit(0);      
	


}//main


void sys_delay()
{
	
	//Delay execution of program for desired time:
	int res = nanosleep(&req,&rem);
	if(res < 0)
	{
		printf("nanosleep() ERROR!\n");
		printf("Error number %d: %s\n",errno,strerror(errno));
		if(errno == EINVAL)
		{
			printf("Nanosecond parameter out of bounds (< 0 or > 999999999)\n");
		}
		if(errno == EINTR)
		{
			printf("Sleep interrupted by a signal handler\n");
			
		}	
		
	}	
	
	
}//sys_delay	


void run_test()  
{
	
	//Open file for data:
	 if (saveToFile)
	 {
		 if( (fp=fopen(filename,"wt")) == NULL)
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

		while (EXIT_PROGRAM == 0)
		{
			//Get sample
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
   			    
   			    //Close the devuice:
   			    MCP3008_Close();
   			    exit (0);	
			}
	 		


		}

	}
	else if ( strcmp(DRIVER_NAME, "mcp3008") == 0)

	{
		printf ("Running custom driver for mcp3008:\n!");
		while (EXIT_PROGRAM == 0)
		{
			//Get sample
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
   			    
   			    //Close the devuice:
   			    MCP3008_Close();
   			    exit (0);	
			}
	 		


		}


	}
				
	
}//run_test




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


    



