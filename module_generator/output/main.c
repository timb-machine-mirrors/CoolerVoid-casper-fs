#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include "hooked.h"

MODULE_DESCRIPTION("Casper-fs custom module - - Custom LKM to protect secret resources on file system.");
MODULE_AUTHOR("CoolerVoid <coolerlair@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.2");


static int fh_init(void)
{
    struct device *fake_device;
    int error = 0,err = 0;
    dev_t devt = 0;

	err=start_hook_resources();
		if(err)
			pr_info("Problem in hook functions");

	module_hide();
	tidy();

   /* Get a range of minor numbers (starting with 0) to work with */
    error = alloc_chrdev_region(&devt, 0, 1, "usb15");

    if (error < 0) 
	{
        	pr_err("Can't get major number\n");
        	return error;
    }

    major = MAJOR(devt);

    /* Create device class, visible in /sys/class */
    fake_class = class_create(THIS_MODULE, "usb15");

    	if (IS_ERR(fake_class)) {
        	unregister_chrdev_region(MKDEV(major, 0), 1);
        	return PTR_ERR(fake_class);
    	}

    /* Initialize the char device and tie a file_operations to it */
    cdev_init(&fake_cdev, &fake_fops);
    fake_cdev.owner = THIS_MODULE;
    /* Now make the device live for the users to access */
    cdev_add(&fake_cdev, devt, 1);

    fake_device = device_create(fake_class,
                                NULL,   /* no parent device */
                                devt,    /* associated dev_t */
                                NULL,   /* no additional data */
                                "usb15");  /* device name */

    if (IS_ERR(fake_device)) 
	{
        	class_destroy(fake_class);
        	unregister_chrdev_region(devt, 1);
        	return -1;
    }


	return 0;
}
module_init(fh_init);



static void fh_exit(void)
{
	fh_remove_hooks(demo_hooks, ARRAY_SIZE(demo_hooks));
	unregister_chrdev_region(MKDEV(major, 0), 1);
	device_destroy(fake_class, MKDEV(major, 0));
	cdev_del(&fake_cdev);
	class_destroy(fake_class);
}
module_exit(fh_exit);

