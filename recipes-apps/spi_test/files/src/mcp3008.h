/*   MODULE HEADER FILE: mcp3008.h --------------------------------------- 
 * 
 *   Header file for building a shared library to be used by JNI for
 *   communicationg with the LKM (device driver) for the MCP3008 8-chanel
 *   Analog to digital converter.
 ----------------------------------------------------------------------*/ 
#ifndef MCP3008_H
#define MCP3008_H

#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>



#define LEN 10
#define DRIVER_NAME	"spidev0.0" 	 //spidev0.0; mcp3008
#define VREF              3.3    //Reference voltage for analog measurements    
#define MAX_ANALOG_VAL    1024   //Max. integer value of 10 bit analog signal 
#define NUM_WORDS	  	  3     // Number of words for write transaction to mcp3008
#define BITS_PER_WORD	  8
#define SPI_CLK_FREQ	  1000000


extern unsigned int delay_ms;      //Delay in milliseconds
extern uint64_t delay_us;          //Delay in microseconds


// Define transfer data when using spidev:
extern int INPUT_CHAN;
extern int RD_MODE;
extern int SPI_CTRL_CFG;



struct adcSettings {
	int readMode;   
	int chanIndex;   
};



extern struct adcSettings settings;


uint64_t getSamplingRate(void);
int MCP3008_Init(float sample_rate);
void setParams(int rm, int ch);  //Set ADC params
float readADC();
uint16_t readADC1();
uint16_t getVal(uint8_t msb,uint8_t lsb);


//File operations
void MCP3008_Close(void);
int32_t readByte(char addr);
int32_t readData(char* elem);
int32_t writeByte(char elem, char data);

//SPIDEV FUNCTIONS:
void setSpiDevTxWords(void);
void getSpiDevConfig(void);
float readSpiDev(void);

#endif
