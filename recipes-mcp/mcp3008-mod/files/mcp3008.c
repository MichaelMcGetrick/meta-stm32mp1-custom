/*uttersense.c  -------------------------------------------------------
 * Kernel module driver for the MCP3008 Analog-to-Digital Converter
 * This driver does not use the Board File method, but relies on providing
 * a list of compatible devices that should be availble on the system
 * Device Tree.
 *  
 * This device is accessed by SPI communitation
 * 
 * NOTES ON DRIVER IMPLEMENTATION
 * 
 * i) Set up a driver structure (that will drive the target device)
 *    We need to define:
 *     - driver name (must match the name given in device board info
 *     - probe callback function
 *     - remove callback function
 * 
 * NB: This is known as a 'protocol' driver. It works via the 'controller'
 *     driver that is set up for SPI. The controller driver interacts directly
 *     with the target hardware. The 'protocol' driver sends messages via the
 *     controller driver in order to interface with the target hardware. 
 *      
 *
 * NB: Designating driver name:
 *     In order to avoid possible conflicts of operation, choose a name that does not
 *     resemble possible devices already registered in spi sub-system 
 *     
 *   
 * i) Set up a compatible table of devices using of_device_id structure
 *       The compatible property should match entries in the Device Tree
 *       If matched, it will send back a spi_device object to probe().
 *       This is the object that will send required data/messages between
 *       controller (master) and slave.
 *       
 * 
 * ii) Set up a probe callback function
 *      This will handle a callback from spi sub system if driver and device names match 
 *         
 * iii) Initialise the driver in LKM  _init
 *      Register the SPI driver dor trhe slave device
 * 
 * 
  --------------------------------------------------------------------*/ 
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/mm.h>
#include <linux/ioctl.h>               //Communication facility between kernel and user space
#include <linux/uaccess.h>          // Required for the copy to user function
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/signal.h>
//#include <linux/sched/signal.h>      //Use instead of <linux/signal.h> - header file location chenaged in more recent kernels
#include <linux/fs.h>



#define  DEVICE_NAME "mcp3008"    	///< The device will appear at /dev/ using this value
#define  CLASS_NAME  "cl_mcp3008"   ///< The device class -- this is a character device driver

//DEFINES FOR MCP3008 DEVICE:
#define BITS_PER_WORD			8
#define NUM_WORDS				3  //Require 3 bytes for complete data transmission
#define VREF              3.3    //Reference voltage for analog measurements    
#define MAX_ANALOG_VAL    1024   //Max. integer value of 10 bit analog signal 
#define SPI_CLK_FREQ		  683594 //683594 //1367188//1000000	//CPU: 1.4GHz: 1024 divider -> 1.37MHz; 2048 divider -> 0.68MHz

#define MES_LEN			  2     //Length of data set to return to user-space	

static DEFINE_MUTEX(spi0_mutex);	    ///Declare mutex			

int INPUT_CHAN = 7;
int RD_MODE = 1;
int SPI_CTRL_CFG;

char filename[20];
int fd;


//Define rx,tx buffers for SPI transfers:
char tx_data[NUM_WORDS],rx_data[NUM_WORDS];


//Function prototypes required for SPI Driver
static int mcp3008_probe(struct spi_device *spi);


static const struct of_device_id mcp3008_dt_ids[] = {
	
	{
		//.compatible = "mcp3008",
		.compatible = "mjmtek,mcp3008",
		
	},	
	
	{},
	
};	
MODULE_DEVICE_TABLE(of,mcp3008_dt_ids);



//SPI Driver for the MCP3008
static struct spi_driver mcp3008_driver = {
	.driver = { 
		         .name = "adc-mcp3008",   //Ensure this is not the name of an existing driver
		         //.name = "uttersense", 
		         .owner = THIS_MODULE,
		         .of_match_table = mcp3008_dt_ids, 
		         
	},
	.probe = mcp3008_probe, 
	//.remove = mcp320x_remove, 
	//.id_table = mcp3008_dt_ids,

};


static int    majorNumber;                  ///< Stores the device number -- determined automatically
static struct class*  spiClass  = NULL; ///< The device-driver class struct pointer
static struct device* spiDevice = NULL; ///< The device-driver device struct pointer

static char   message[MES_LEN + 1] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;              ///< Used to remember the size of the string stored


static struct spi_device *adcDevice;

static char rec_buffer[256] = {0};



//Define message structure for data transfer:
static struct spi_transfer mcp3008_transfer[] = {
	
	{
   	.tx_buf = tx_data,
  	   .rx_buf = rx_data,
  		.len = NUM_WORDS,
  		.bits_per_word = BITS_PER_WORD,
       //u16 delay_usecs;
       .speed_hz = SPI_CLK_FREQ,      //0: Use default board specification
       //struct list_head transfer_list;
   },
   
   {},
	
};

//Decode function for RTC register values:
void decode(char* elem, char byte);
//Cleanup functions for buffers:
void cleanUp(void);
//Setup standard char driver
int setUp(void);
//Get ADC value:
uint16_t readRaw(void);
uint16_t getVal(uint8_t msb,uint8_t lsb);
//Set transmit buffer words:
void setTransferWords(void);


 

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);



static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};



int setUp(void)
{
	   //Register char device to facilitate user program interaction with i2c:
      // Try to dynamically allocate a major number for the device -- more difficult but worth it
      majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
      if (majorNumber<0){
         printk(KERN_ALERT "%s  failed to register a major number\n",DEVICE_NAME);
         return majorNumber;
      }
      printk(KERN_INFO "%s registered correctly with major number %d\n", DEVICE_NAME,majorNumber);

      
      // Register the device class
      spiClass = class_create(THIS_MODULE, CLASS_NAME);
      if (IS_ERR(spiClass)){                // Check for error and clean up if there is
         unregister_chrdev(majorNumber, DEVICE_NAME);
         printk(KERN_ALERT "Failed to register device class %s\n", CLASS_NAME);
         return PTR_ERR(spiClass);          // Correct way to return an error on a pointer
      }
      printk(KERN_INFO "%s device class registered correctly\n",CLASS_NAME);

      
      // Register the device driver
      spiDevice = device_create(spiClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
      if (IS_ERR(spiDevice)){               // Clean up if there is an error
         class_destroy(spiClass);           // Repeated code but the alternative is goto statements
         unregister_chrdev(majorNumber, DEVICE_NAME);
         printk(KERN_ALERT "Failed to create the device %s\n",DEVICE_NAME);
         return PTR_ERR(spiDevice);
      }
      printk(KERN_INFO "%s device created correctly\n",DEVICE_NAME); // Made it! device was initialized
     
      
   
   printk(KERN_INFO "Initialised LKM OK!\n");   

	return 0;   
	
}//setup_char_driver




//SPI Callbacks

//static int uttersense_probe(struct spi_device *spi)
static int  mcp3008_probe(struct spi_device *spi)
{
      int res = 0;
            
      //If we have a device match, the probe function returns with a valid spi_device structure
      //The we can do our required configuration
            
      printk(KERN_INFO "Have called back to probe function");
            
      
		//SPI setup with MCP3008 in mode 0 and bits per word as 10 */
		//CHECK ALL OF THESE!
		//Check here the passed values before resetting:
		
		printk(KERN_INFO "Chip Select: %d", spi->chip_select);
		printk(KERN_INFO "Max. frequency: %d", spi->max_speed_hz);
		printk(KERN_INFO "Bits per word: %d", spi->bits_per_word);
		printk(KERN_INFO "mode: %d", spi->mode);
		//printk(KERN_INFO "SPI_MODE_0: %d", spi->SPI_MODE_0);
		//spi->mode = SPI_MODE_0;  //Check with Struct spi_device and MCP3008
		spi->bits_per_word = 8; //Check
		spi->max_speed_hz	= SPI_CLK_FREQ, 
	   spi->chip_select	= 0,
		
		//This will update the device with values described above (will override default state)		
		res = spi_setup(spi);
				
		
      if(res < 0){
				printk(KERN_INFO "Problem updating the device settings!");
		}
		else {
			   printk(KERN_INFO "Updated the device settings OK!");
			   //Assign global device structure:
				adcDevice = spi;
		}	
			
					
		//Set up the char driver:
		setUp();	
		
		//Set up protocol driver transfer control bytes:
		setTransferWords();
		
		readRaw();   //For testing		
				
      return 0;
}    

uint16_t readRaw()
{
	int res = 0;
	uint16_t val;
	
	//Do ADC reading
	//Transmit bytes to spi device - this will normally be called by read() function:
	res = spi_sync_transfer(adcDevice,mcp3008_transfer,ARRAY_SIZE(mcp3008_transfer));
	if(res < 0){
			printk(KERN_INFO "There was a failure with the SPI data transfer!");
			return EINVAL;
	}
				
	//Get the ADC value:
	rx_data[1] &= 0x03;    //Define most significant byte
	val = getVal(rx_data[1],rx_data[2]);
	//printk(KERN_INFO "ADC value: %d", val);
	
	message[0] = rx_data[1];
   message[1] = rx_data[2];

   

	return val;
	
}//readRaw

	
uint16_t getVal(uint8_t msb,uint8_t lsb)
{
   
   uint16_t val_msb = msb;
   uint16_t val_lsb = lsb;
	val_msb = val_msb << 8;
	
   return  val_msb | val_lsb;
   
}//getVal

void setTransferWords()
{
	
	SPI_CTRL_CFG = (RD_MODE << 7) | (INPUT_CHAN << 4);
	tx_data[0] = 0x01;              //Least significant bit the start bit 
   tx_data[1] = SPI_CTRL_CFG;      //Define inupt mode and input pin address
   tx_data[2] = 0x01;              //Don't care bits
		
	//Refresh Read buffer:
	rx_data[0] = 0x00;
	rx_data[1] = 0x00;
	rx_data[2] = 0x00;		
	
}	



//FILE OPERAIONS HERE: -------------------------------------------------
static int dev_open(struct inode *inodep, struct file *filep){
   
   if(!mutex_trylock(&spi0_mutex)){  // Acquire the mutex (returns 0 on fail)
	   printk(KERN_ALERT "EBBChar: Device in use by another process");
	   return -EBUSY;
   }
   printk(KERN_INFO "Device %s has been opened with mutex \n",DEVICE_NAME);  
    
   return 0;
}



static int dev_release(struct inode *inodep, struct file *filep){
   
   mutex_unlock(&spi0_mutex);
   printk(KERN_INFO "Device %s has been closed \n",DEVICE_NAME);
   return 0; 
   
}
 


static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   
   //Set adc read configurations:
   int error_count = 0;

   
   //NB: Copy user space data to kernel space
   error_count = copy_from_user(rec_buffer,buffer,sizeof(buffer));
   RD_MODE = rec_buffer[0];
   INPUT_CHAN = rec_buffer[1];
   
   printk(KERN_INFO "LKM: readMode: %d\n",RD_MODE);
   printk(KERN_INFO "LKM: chanIndex: %d\n",INPUT_CHAN);
      
   //Reset the transfer control bytes:  
   setTransferWords();  
   
   return sizeof(buffer);
      
    
}


static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   
   
   int error_count = 0;
   uint16_t val;
      
   //NB: Need this function to copy from user space to kernel space!!
   //    If not, we are likely to crash (as in Android implementation)
   error_count = copy_from_user(rec_buffer,buffer,sizeof(buffer));
   //printk(KERN_INFO "Received:  %s\n",rec_buf);
	
	//Read ADC value
	val = readRaw();
	
	// printk(KERN_INFO "val: %x\n", val);
	//size_of_message = strlen(message);
 	 size_of_message = MES_LEN;  
      
   //NB: Need this function to copy from kernel space to user space!!
   error_count = copy_to_user(buffer, message, size_of_message);
     
   /*
   if (error_count==0){            // if true then have success
      printk(KERN_INFO "Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   } 
   */
   return 0;
   
}
//END: FILE OPERATIONS -------------------------------------------------------------

void cleanUp(void)
{
   //Clean up the receive/message arrays:
   strcpy(rec_buffer,"");
   strcpy(message,"");
   
   
}//cleanUp   



static int __init SPIModule_init(void)
{
   int res = 0;
   
   printk(KERN_INFO "started init() ");
   
	 //Register device driver: 
	 res = spi_register_driver(&mcp3008_driver);
	 if(res < 0){
		 printk(KERN_INFO "Failed to register mcp3008 driver\n");
	 }
	 else {
		printk(KERN_INFO "Registered mcp3008 driver OK using input channel 7!\n");
		
	 }		 
 
   mutex_init(&spi0_mutex); //Initialize mutex
   return res;
}



static void __exit SPIModule_exit(void)
{
   
   mutex_destroy(&spi0_mutex);  //Destroy  dynamically-allocated mutex
   
    //Unregister SPI driver:
	spi_unregister_driver(&mcp3008_driver);
	printk(KERN_INFO "Unregistered mcp3008 SPI driver.\n");
 
    	
         
   //Unregister the char device driver
   device_destroy(spiClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(spiClass);                          // unregister the device class
   class_destroy(spiClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   
   printk(KERN_INFO "%s: Goodbye from the LKM!\n",DEVICE_NAME);    
      
       
}


module_init(SPIModule_init);
module_exit(SPIModule_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael McGetrick");
MODULE_DESCRIPTION("SPI driver for mcp3008 on STM32Mp157F");
MODULE_VERSION("0.1");

