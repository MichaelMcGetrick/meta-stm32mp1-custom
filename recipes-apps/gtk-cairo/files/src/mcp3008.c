/*   MODULE: mcp33008.c ------------------------------------------------- 
 *   Source file for building a shared library to be used by JNI for
 *   communicationg with the LKM (device driver) for the MCP3008 8-channel
 *   analog to digital converter.   
 ----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>               //Communication facility between kernel and user space
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "mcp3008.h"



//Define approrpiate logging function protoptye
//logf will be the alias for both Android App and normal Linux builds
#ifdef ANDROID_APP
	#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "** NATIVE C++ **: ", __VA_ARGS__))
#else
    #define LOGI(...) printf(__VA_ARGS__)
#endif 



struct adcSettings settings;


#define BUFFER_LENGTH 256               ///< The buffer length for messages
static char rec_buf[BUFFER_LENGTH] = {0};     ///< The receive buffer from the LKM

int INPUT_CHAN = 7;
int RD_MODE = 1;
int SPI_CTRL_CFG;

uint64_t delay_us;
unsigned int delay_ms;


int res;
  
//File pointer for file commands  
static int fd;

unsigned char tx_data[NUM_WORDS],rx_data[NUM_WORDS];

struct spi_ioc_transfer tr = {
        .len = NUM_WORDS,
	.bits_per_word = BITS_PER_WORD,
        //.delay_usecs = 10,
        .cs_change = 0, // Keep CS activated
        .speed_hz = SPI_CLK_FREQ,      //0: Use default board specification

};



//Flag for logging:
bool logging = true;
  
  
#ifdef ANDROID_APP
void mcp3008_test(void)
{
	int num = 5;
	char *str = "Hello from mcp3008!";
	
	LOGI("Integer number: %d, String %s\n", num, str);


}//mcp3008_test

char* jni_test(void)
{
	//Test to check we can interface with Java space
	return "This is an updated jni test on 5 April from keypad!\n";
	 	

}//jni_test
#endif


//Open the device driver
int MCP3008_Init(float sample_rate)
{

   char path[20];
   sprintf(path,"%s%s", "/dev/",DRIVER_NAME);




   //Open driver:
   fd = open(path, O_RDWR);             // Open the device with read/write access
   if (fd < 0)
   {
		printf ("Failied tp open the device driver ...\n");
		return -errno;
   }

   printf("Opened the %s device driver OK!\n",DRIVER_NAME);


  	//Set sample delay period:
  	delay_us = (1.0/(float)sample_rate)*1000000;


	// Get current driver settings:
	if ( strcmp(DRIVER_NAME, "spidev0.0") == 0)
	{
  		getSpiDevConfig();
  	}


   return 0;


}//MCP3008_Init



uint64_t getSamplingRate(void)
{
   return delay_us;

}//getSamplingRate


void setParams(int rm, int ch)
{

	settings.readMode = rm;
	settings.chanIndex = ch;

	char wbuf[2];
   wbuf[0] = settings.readMode;
   wbuf[1] = settings.chanIndex;
	write(fd,wbuf,sizeof(wbuf));


}//setParams


float readADC()
{

	float volts;
	uint8_t wbuf[2];
   wbuf[0] = 0x00;
   wbuf[1] = 0x00;

   uint16_t val;


	read(fd,wbuf,sizeof(wbuf));

   val = getVal(wbuf[0],wbuf[1]);


   //Convert to floating point:
   volts = ((float)val*VREF)/MAX_ANALOG_VAL;


   return volts;



}//readADC



uint16_t readADC1()
{

	uint8_t wbuf[2];
	wbuf[0] = 0x00;
	wbuf[1] = 0x00;

   uint16_t val;


	read(fd,wbuf,sizeof(wbuf));

   val = getVal(wbuf[0],wbuf[1]);


   return val;



} // readADC1



uint16_t getVal(uint8_t msb,uint8_t lsb)
{


   uint16_t val_msb = msb;
   uint16_t val_lsb = lsb;
	val_msb = val_msb << 8;


   return  val_msb | val_lsb;

}//getVal



int32_t readByte(char addr)
{
   //Read value back from kernel driver
   //Define address to read from:
   rec_buf[0] = addr;
   res = read(fd, rec_buf,BUFFER_LENGTH);
   if (res < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }
   printf("The received message is: %s\n", rec_buf);

   return res;
}



int32_t readData(char* elem)
{
   //Read value back from kernel driver
   //Define address to read from:
   strcpy(rec_buf,elem);
   //res = read(fd, receive,BUFFER_LENGTH);
   res = read(fd, rec_buf,strlen(rec_buf));
   if (res < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }

   return res;
}


int32_t writeByte(char elem, char data)
{
  //Read value back from kernel driver
   //Define address to read from:
   char buf[3];
   buf[0] = elem;
   buf[1] = data;
   res = write(fd, buf,strlen(buf));
   if (res < 0){
      perror("Failed to write to the device.");
      return errno;
   }
   return res;
}


void getSpiDevConfig(void)
{

	uint8_t mode;

	if (ioctl(fd, SPI_IOC_RD_MODE, &mode) < 0)
	{
		perror("SPI rd_mode");
		return;
	}
	printf ("SPI Mode: %d\n",mode);


	uint8_t bits;
	if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0)
	{
		perror("SPI bits_per_word");
		return;
	}
	printf ("SPI Bits per word: %d\n",bits);


	int freq;
	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &freq) < 0)
	{
	perror("SPI max_speed_hz");
	return;
	}
	printf ("SPI Max Freq: %d\n",freq);


}



void setSpiDevTxWords(void)
{

   SPI_CTRL_CFG = (RD_MODE << 7) | (INPUT_CHAN << 4);
   tx_data[0] = 0x01;              //Least significant bit the start bit
   tx_data[1] = SPI_CTRL_CFG;      //Define inupt mode and input pin address
   tx_data[2] = 0x01;


}


float readSpiDev(void)
{


	int status;
	float volts;
	 uint16_t val;

	// Initialise rx buffer:
	 rx_data[0] = 0;
	 rx_data[1] = 0;
	 rx_data[2] = 0;
	 // Set the tx and rx api  transfer buffers:
	tr.tx_buf = (unsigned long) tx_data;
	tr.rx_buf = (unsigned long) rx_data;



	status = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (status < 0)
	{
			perror("SPI_IOC_MESSAGE");
			printf ("Error with reading from PSI bus - abort!\n");
			exit (0);
	}


	uint8_t mask = 0x03;
	rx_data[1] &= mask;

	val = getVal(rx_data[1],rx_data[2]);


	//Convert to floating point:
	volts = ((float)val*VREF)/MAX_ANALOG_VAL;


	return volts;


}


//Close the device driver
void MCP3008_Close(void)
{
    printf("Closing the %s device driver\n",DRIVER_NAME);
    close(fd);

}//close_dev



//---------------------------------------------------------------------
//DEFINE METHODS FOR JAVA CALLBACK HERE (If Android App realisation):
#ifdef ANDROID_APP
void init_callbacks(char *f_name, char *f_sig)
{
	//strcpy(callback_name,f_name);
	callback_name = f_name;
	callback_sig = f_sig;
	LOGI("Set callback strings OK!");
	LOGI("Callback name: %s",callback_name);
	LOGI("Callback signature: %s",callback_sig);
}

void sendToJava(char *c)
{

	//LOGI("JNIEnv pointer 0x%p",glo_env);
	//LOGI("JVM pointer 0x%p",JVM);
	//LOGI("Global class 0x%p",&glo_clazz);
	//LOGI("Global object 0x%p",&glo_obj);


    jstring js = (*glo_env)->NewStringUTF(glo_env,c);
    jclass clazz = (*glo_env)->GetObjectClass(glo_env,glo_obj);
	jmethodID mid_cb = (*glo_env)->GetMethodID(glo_env,clazz,callback_name, callback_sig);
    (*glo_env)->CallVoidMethod(glo_env, glo_obj,mid_cb,js); //OK with this


	LOGI("Data sent to Java!");


}

#endif


