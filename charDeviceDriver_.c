#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include "linkedList.h"

// Defile the name and the class of the device 
#define  DEVICE_NAME "opsysmem"
#define  CLASS_NAME  "opsys"


// Include some information about the module, mainly for fun :D 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Georgios Vasilakis");
MODULE_DESCRIPTION("Simple character device");
MODULE_VERSION("0.1");

// We will use a dynamically allocated major number for our device, 
// define our variables here 
static int    majorNumber;

// Define the functions that will handle the file operations 
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static long    dev_ioctl(struct file *, unsigned int , unsigned long );


// That's the 4K limit per message 
static unsigned long snglMsgLimit = 4096;
// That's the default 2MB limit for all messages 
static unsigned long allMsgLimit = 2097152;
static unsigned long curDataSize= 0;

static struct linkedList *myQueue;
//Register our functions with the responsible  file operation as 
//defined at linux/fs.h
static struct file_operations fops =
{
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
	.unlocked_ioctl = dev_ioctl,
};

// Define two mutexes 
DEFINE_MUTEX  (limitsLock);
DEFINE_MUTEX  (listLock);

// Initialization fucntion, this function will run when we insert the module to the kernel.
// If this module was native to the kernel this function will run only one time. 
static int __init opsysDriver_init(void){
	printk(KERN_INFO "Opssys Driver: I am finally alive! I thought I was forever dead! \n");

	// Lets find an available major number, note that the DEVICE_NAME is not necessary the same as the /dev/ entry name
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if ( majorNumber < 0 ) {
		printk(KERN_ALERT "Opssys Driver: Really??? That was my life? Like 2 ns?! At least find me a proper major number the next time...");
		return majorNumber;

	}

	//We print the mknod we need to run to create the /dev/ node, without that command there is no /dev/ node
	//If you need to skip running the mknod command manually you can use device_create 
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, majorNumber);
	myQueue = initLinkedList();
	return 0;
}

// This function will run every time we remove the module  from the kernel. If the module is 
// a built-in one this function will never run.
static void __exit opsysDriver_exit(void){
	struct nodeDataStruct *myData = NULL;
	myData = removeNode(myQueue);
	while(myData != NULL){
		kfree(myData->charData);
		kfree(myData);
		myData = removeNode(myQueue);
	}

	kfree(myQueue);

	unregister_chrdev(majorNumber, DEVICE_NAME);             
	printk(KERN_INFO "Opssys Driver: Two gunshots were heard and nobody saw the opsys driver again, not it's memory, not its instructions, nothing... RIP opsys driver, RIP!\n");
}

// This function is called whenever the /dev/ node is opened 
static int     dev_open(struct inode *myInode, struct file *myFile){
	return 0;
};

// This function is called whenever the /dev/ node is released 
static int     dev_release(struct inode *myInode, struct file *myFile){
	return 0;
};

// This function is called whenver we read from the /dev/ node, for exammple when we do "cat /dev/ourDevice"
static ssize_t dev_read(struct file *myFile, char *buffer, size_t buffSiz, loff_t *loff){
	struct nodeDataStruct *myData = NULL;
	unsigned long strLength; 
		
	mutex_lock(&listLock);
	myData = removeNode(myQueue);
	mutex_unlock(&listLock);

	// If the driver doesn't hold any messages we will return -EAGAIN.
	if ( myData == NULL ) {
		return -EAGAIN;
	}


	mutex_lock(&limitsLock);
	curDataSize -= myData->charSize;
	mutex_unlock(&limitsLock);

	strLength = myData->charSize;

	if(copy_to_user(buffer, myData->charData, myData->charSize) != 0 ){
		kfree(myData->charData);
		kfree(myData);
		return -EFAULT;
	}

	kfree(myData->charData);
	kfree(myData);
	return strLength;
};

// This function is called whenver we want to write to the /dev/ node, for exammple when we do "cat 'test' > /dev/ourDevice"
static ssize_t dev_write(struct file *myFile, const char *buff, size_t buffSiz, loff_t *loff){
	struct nodeDataStruct *myData = NULL;

	printk(KERN_INFO "Opssys Driver: My buffSize is : %zu \n", buffSiz); 

	if ( buffSiz >  snglMsgLimit ) {
		return -EINVAL;
	}

	// We have to lock before the message size check because the curDataSize and 
	// the allMsgLimit are global variables that can change 
	mutex_lock(&limitsLock);
	if ( (curDataSize + buffSiz) > allMsgLimit){
		mutex_unlock(&limitsLock);
		return -EAGAIN;
	}

	curDataSize += buffSiz ;
	mutex_unlock(&limitsLock);

	// Quick tip, kmallon as malloc can sleep so NEVER do kmalloc holding a mutex
	myData = kmalloc( sizeof(struct nodeDataStruct), GFP_KERNEL);
	if ( myData == NULL ){
		mutex_lock(&limitsLock);
		curDataSize -= buffSiz ;
		mutex_unlock(&limitsLock);
		return -EFAULT;
	}

	myData->charData = kmalloc( sizeof(char) * buffSiz, GFP_KERNEL);
	if ( myData->charData == NULL ){
		kfree(myData);
		mutex_lock(&limitsLock);
		curDataSize -= buffSiz ;
		mutex_unlock(&limitsLock);
		return -EFAULT;
	}

	myData->charSize = buffSiz;
	if ( copy_from_user(myData->charData,buff, buffSiz) != 0 ) { 
		mutex_lock(&limitsLock);
		curDataSize -= buffSiz ;
		mutex_unlock(&limitsLock);
		kfree(myData->charData);
		kfree(myData);
		return -EFAULT;
	}

	mutex_lock(&listLock);
	myQueue = insertNode(myQueue,myData);
	mutex_unlock(&listLock);

	return buffSiz;
};

// We can call the ioctl syscall to change the parameters of the device from the user space. Here we can change the global limit of all messages.
static long dev_ioctl(struct file *myFile, unsigned int ioctl_num, unsigned long ioctl_param){
	if ( ioctl_num == 0 ) {
		mutex_lock(&limitsLock);
		// If the new limit is biger thta the current data length
		if ( ioctl_param > curDataSize ){
			allMsgLimit = ioctl_param;
			mutex_unlock(&limitsLock);
			return 0;
		}
		mutex_unlock(&limitsLock);
	}

	return -EINVAL;
}

module_init(opsysDriver_init);
module_exit(opsysDriver_exit);
