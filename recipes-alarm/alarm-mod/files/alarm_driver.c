#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/kernel.h>
#include <linux/ioctl.h>  
#include <linux/fs.h>      
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/sched/signal.h>      //Use instead of <linux/signal.h> - header file location chenaged in more recent kernels



/* NOTES ---------------------------------------------------------------------------------------------------

1. We are using the new descriptor / consumer gpio library (sysfs version is now deprecated).
2. The new library requires we map gpios to a 'descriptive' source such as a device Tree. Then, we can reference
   a descriptor handle in the S/W API.
3. The Device Tree node for the gpio 'device' follows certain rules: We define <function>-gpios to refer to the
   actual gpio pin. The <function> is then referred to in devm_gpiod_get() methos (see below).
4. The gpio entries in the device tree for the gpio 'device' (in our case alarm-device) define the hardware requirements.
   configuration of pins for DIRECTION and VALUES are performed with s/w (as below).    
5. Our gpio pin for the RTC alarm is defined as: 
	alarmpin-gpios = <&gpioa 8 (GPIO_ACTIVE_HIGH | GPIO_PULL_UP)>;
   NB: Whatever flags we use, the default signal value is zero. This will only be changed when an interrupt
       signal appears on the pin.
6. alarm-pin is configured to an interrupt pin triggered when the pin goes LOW (trigger event for RTC
   alarm).        		       



----------------------------------------------------------------------------------------------------------*/

#define  DEVICE_NAME "rtc_alarm"
#define  CLASS_NAME  "rtc_alarm_class"


struct gpio_desc *led_pin, *alarm_pin;
static unsigned int irqNum;

//For signalling to user application
#define REG_CURRENT_TASK _IOW('a','a',int32_t)
#define SIGMJM 44       //Define a signal that identifies the sender 
static struct task_struct *task = NULL;
static int signum = 0;



static int    majorNumber;                  ///< Stores the device number -- determined automatically
static struct class*  alarmClass  = NULL; ///< The device-driver class struct pointer
static struct device* alarmDevice = NULL; ///< The device-driver device struct pointer


//Function prototypes:
static irq_handler_t  alarm_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);
static long alarm_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);

static void sendSignal(void);
static int setUp(void);

static struct file_operations fops =
{
   .unlocked_ioctl = alarm_ioctl,   //Define the file pointer for the ioctl command
   .open = dev_open,
   .release = dev_release,
};



static int gpio_init_probe(struct platform_device *pdev)
{
   
   printk("KERNEL: RTC Alarm module initialised ....\n");

     
   
   // SET UP GPIO FOR RTC ALARM
   /* "ledpin" label is matching the device tree declaration. OUT_LOW is the value at init */
   led_pin = devm_gpiod_get(&pdev->dev, "ledpin", GPIOD_OUT_LOW);
	
   printk("KERNEL: Set up led_pin OK\n");	
   
   /* "ledpin" label is matching the device tree declaration. Pin is defined as an INPUT pin */
   alarm_pin = devm_gpiod_get(&pdev->dev, "alarmpin", GPIOD_IN);
   //NB: Declaring GPIO_IN flag is optional at this stage. However, if omitted must be defined be s/w 
   //    function gpiod_direction_input().
   
   
   // Checking the return for devm_gpiod_get
   // According to Linux documentation (https://www.kernel.org/doc/html/v5.0/driver-api/gpio/consumer.html): when referring
   // to gpiod_direction_input():
   //"The return value is zero for success, else a negative errno. It should be checked, since the get/set calls don’t return
   // errors and since misconfiguration is possible." 
   
   
   
   //Set direction for "alarmpin":
   int ret =  gpiod_direction_input(alarm_pin);
   if (ret >= 0)
   {
   	printk("KERNEL: Set alarm_pin DIRECTION OK\n");
   }
   else
   {
   	printk("KERNEL: Problem setting alarm_pin DIRECTION OK\n");
   }
      
      
   //gpiod_set_debounce(alarm_pin, 200);      // Debounce the button with a delay of 200ms    
   					      // Useful if we are changing signal with wiring	    
   
   // Request IRQ number for the pin:
   irqNum = gpiod_to_irq(alarm_pin);
   if (irqNum >= 0)
   {
   	printk(KERN_INFO "KERNEL: Set alarm_pin IRQ number OK : %d\n", irqNum);
   }
   else
   {
   	printk(KERN_INFO "KERNEL: Problem setting alarm_pin IRQ number: %d\n", irqNum);
   }	
   
   
   //Set up interrupt for "alarmpin"
   ret = devm_request_irq(&pdev->dev,				// Device GPIO Descriptor
   		        irqNum,				// Interrupt number assigned
   			(irq_handler_t) alarm_handler, 	// Pointer to the interrupt handler
   			IRQF_TRIGGER_FALLING,		// Interrupt when when RTC alarm triggered)
   			"alarm_device",			// Name of the device generating the interrupt
   			NULL);
   if (ret == 0)
   {
   	printk(KERN_INFO "KERNEL: Interrupt handler has been set up OK :\n");
   }
   else
   {
   	printk(KERN_INFO "KERNEL: Problem setting up interrupt handler : \n");
   }
   
 
    // Set up char device
    setUp();	     
      
      
   return(0);
}


static int dev_open(struct inode *inodep, struct file *filep)
{
   
   printk(KERN_INFO "KERNEL: Device has been opened \n");
   
   return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
   
   struct task_struct *ref_task = get_current();
   //Delete the task:
   if(ref_task == task)
   {
    task = NULL;   
   }    
   
   
   printk(KERN_INFO "KERNEL: Device successfully closed\n");
   return 0;
}




static irq_handler_t alarm_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
     
   //printk("KERNEL IRQ: GPIO RTC Alarm triggered!!!\n");
      
   sendSignal();
   
   return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}



//Allow user to register itself with the kernel so that the kernel can call back when required
//(essentially, registering a callback function) 
static long alarm_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
   
  printk(KERN_INFO "KERNEL: In alarm_ioctl - attempting to register user app...\n");
  if(cmd == REG_CURRENT_TASK)
  {
  	printk(KERN_INFO "Processing register request...\n");
        task = get_current();
        signum = SIGMJM;
  }
   
   return 0;
}


static void sendSignal()  //Function to send signal back to user space:
{
   
   //struct siginfo info;
   struct kernel_siginfo info;
   //memset(&info, 0,sizeof(struct siginfo));
   memset(&info, 0,sizeof(struct kernel_siginfo));
   info.si_signo = SIGMJM;
   info.si_code = SI_QUEUE;
   info.si_int = 1;
   
   if(task != NULL)
   {
      if(send_sig_info(SIGMJM,&info, task) < 0)
      {
          printk(KERN_INFO "Unable to send signal\n");
          
      } 
      else
      {
         //printk(KERN_INFO "KERNEL: Sent signal successfully to user-space\n");
      }    
   } 
   
   
}//sendSignal   


static int setUp(void)
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
      alarmClass = class_create(THIS_MODULE, CLASS_NAME);
      if (IS_ERR(alarmClass)){                // Check for error and clean up if there is
         unregister_chrdev(majorNumber, DEVICE_NAME);
         printk(KERN_ALERT "Failed to register device class %s\n", CLASS_NAME);
         return PTR_ERR(alarmClass);          // Correct way to return an error on a pointer
      }
      printk(KERN_INFO "%s device class registered correctly\n",CLASS_NAME);

      
      // Register the device driver
      alarmDevice = device_create(alarmClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
      if (IS_ERR(alarmDevice)){               // Clean up if there is an error
         class_destroy(alarmClass);           // Repeated code but the alternative is goto statements
         unregister_chrdev(majorNumber, DEVICE_NAME);
         printk(KERN_ALERT "Failed to create the device %s\n",DEVICE_NAME);
         return PTR_ERR(alarmDevice);
      }
      printk(KERN_INFO "%s device created correctly\n",DEVICE_NAME); // Made it! device was initialized
     
      
   
   printk(KERN_INFO "Initialised LKM OK!\n");   

	return 0;   
	
}//Setup





static int gpio_exit_remove(struct platform_device *pdev)
{
   printk("KERNEL: RTC Alarm example exit\n");
   //Unregister the char device driver
   device_destroy(alarmClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(alarmClass);                          // unregister the device class
   class_destroy(alarmClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
      
   
   printk(KERN_INFO "%s: Goodbye from the LKM!\n",DEVICE_NAME);    
    
   
   
   
   return(0);
}

/* This structure does the matching with the device tree */
/* if it does not match the compatible field of DT, nothing happens */
static struct of_device_id alarm_match[] = {
    {.compatible = "mjm,rtcalarm"},
    {/* end node */}
};

static struct platform_driver alarm_driver = {
    .probe = gpio_init_probe,
    .remove = gpio_exit_remove,
    .driver = {
        .name = "alarm_driver",
                .owner = THIS_MODULE,
                .of_match_table = alarm_match,
    }
};

   
module_platform_driver(alarm_driver); //NB: This does init /exit module https://ruach.github.io/posts/register-platform-device-driver/

MODULE_AUTHOR("Michael MGetrick");
MODULE_DESCRIPTION("Gpio example");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:alarm_driver");


