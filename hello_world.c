#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

int value, arr_value[4];
char* name;
int cb_value = 0;

module_param(value, int, S_IRUSR|S_IWUSR);
module_param(name, charp, S_IRUSR|S_IWUSR);
module_param_array(arr_value, int, NULL, S_IRUSR|S_IWUSR);

/*-------------Module_param_cb()-------------------*/
int notify_param(const char*val, const struct kernel_param *kp)
{
	int res = param_set_int(val,kp); // use helper for write variable
	if(res ==0){
		pr_info("Call back function called...\n");
		pr_info("New value of cb_value = %d\n",cb_value);
		return 0;
	}
	return -1;
}
const struct kernel_param_ops my_param_ops =
{
	.set = &notify_param,	//use our setter
	.get = &param_get_int, // and standard getter 
};

module_param_cb(cb_value,&my_param_ops, &cb_value, S_IRUGO|S_IWUSR);

//creating the dev with our custom major and minor number
//dev_t dev = MKDEV(235,0); for static
dev_t dev =0; // for dynamic

//device file
static struct class *dev_class;


/* module init fuction */
static int __init hello_world_init(void)
{
	/*for static 
	register_chrdev_region(dev,1,"etx_Dev");
	*/
	// Allocating Major number with dynamic way
	if((alloc_chrdev_region(&dev,0,1,"etx_Dev")) < 0){
		pr_err("Cannot allocate major number for device\n");
		return -1;	
	}
	pr_info("Major = %d Minor = %d \n ",MAJOR(dev),MINOR(dev));
	
	//creating struct class
	dev_class = class_create("etx_class");
	if(IS_ERR(dev_class)){
		pr_err("Cannot create the struct class for device\n");
		goto r_class;
	}
	//Creating device
	if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_device"))){
		pr_err("Cannot create the Device\n");
		goto r_device;
	}
	
	pr_info("Welcome to Hello world\n");
	pr_info("This is the Simple Module\n");
	pr_info("Kernel Module Insert successfully\n");
	for(int i=0; i<(sizeof(arr_value) / sizeof(int));i++){
		pr_info("Arr_value[%d] = %d\n",i,arr_value[i]);
	}
	return 0;

r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev,1);
	return -1;
}

/*module Exit function */
static void __exit hello_world_exit(void)
{
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	unregister_chrdev_region(dev,1);
	pr_info("Kernel Module Removed Successfully....\n");
}


module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SEHWAN <hyn0216388@gmail.com>");
MODULE_DESCRIPTION("A simpe hello world driver");
MODULE_VERSION("2:1.0");
