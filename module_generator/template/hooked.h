#include <linux/ftrace.h>
#include <linux/kallsyms.h>
#include <linux/delay.h>
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/kprobes.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <asm/signal.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/fcntl.h>
#include <linux/types.h>
#include <linux/dirent.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/fs.h>


MODULE_DESCRIPTION("CASPER_MODULE_NAME Version 0.2 - Module to turn a private file in hidden. The second function is to protect a confidential file to prevent reading, writing and removal.");
MODULE_AUTHOR("CoolerVoid <coolerlair@gmail.com>");
MODULE_LICENSE("Dual BSD/GPL");

static unsigned int major; 
static struct class *fake_class;
static struct cdev fake_cdev;
static char message[128];

static struct list_head *module_previous;
static short module_hidden = 0;
static short fs_hidden = 1;
static short fs_protect = 1;

void module_hide(void);

static inline void tidy(void)
{
	kfree(THIS_MODULE->sect_attrs);
	THIS_MODULE->sect_attrs = NULL;
}

int fake_open(struct inode * inode, struct file * filp);
int fake_release(struct inode * inode, struct file * filp);
ssize_t fake_read (struct file *filp, char __user * buf, size_t count, loff_t * offset);
ssize_t fake_write(struct file * filp, const char __user * buf, size_t count,loff_t * offset);


static struct file_operations fake_fops = {
    open:       fake_open,
    release:    fake_release,
    read:       fake_read,
    write:      fake_write,
};



_Bool check_fs_blocklist(char *input);
_Bool check_fs_hidelist(char *input);

static unsigned int target_fd = 0;
static unsigned int target_pid = 0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,7,0)
static unsigned long lookup_name(const char *name)
{
	struct kprobe kp = {
		.symbol_name = name
	};
	unsigned long retval;

	if (register_kprobe(&kp) < 0) return 0;
	retval = (unsigned long) kp.addr;
	unregister_kprobe(&kp);
	return retval;
}
#else
static unsigned long lookup_name(const char *name)
{
	return kallsyms_lookup_name(name);
}
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
#define FTRACE_OPS_FL_RECURSION FTRACE_OPS_FL_RECURSION_SAFE
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
#define ftrace_regs pt_regs

static __always_inline struct pt_regs *ftrace_get_regs(struct ftrace_regs *fregs)
{
	return fregs;
}
#endif


#define USE_FENTRY_OFFSET 0


struct ftrace_hook {
	const char *name;
	void *function;
	void *original;

	unsigned long address;
	struct ftrace_ops ops;
};

static int fh_resolve_hook_address(struct ftrace_hook *hook)
{
	hook->address = lookup_name(hook->name);

	if (!hook->address) {
		pr_debug("unresolved symbol: %s\n", hook->name);
		return -ENOENT;
	}

#if USE_FENTRY_OFFSET
	*((unsigned long*) hook->original) = hook->address + MCOUNT_INSN_SIZE;
#else
	*((unsigned long*) hook->original) = hook->address;
#endif

	return 0;
}

static void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
		struct ftrace_ops *ops, struct ftrace_regs *fregs)
{
	struct pt_regs *regs = ftrace_get_regs(fregs);
	struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

#if USE_FENTRY_OFFSET
	regs->ip = (unsigned long)hook->function;
#else
	if (!within_module(parent_ip, THIS_MODULE))
		regs->ip = (unsigned long)hook->function;
#endif
}

int fh_install_hook(struct ftrace_hook *hook);
void fh_remove_hook(struct ftrace_hook *hook);
int fh_install_hooks(struct ftrace_hook *hooks, size_t count);
void fh_remove_hooks(struct ftrace_hook *hooks, size_t count);

#ifndef CONFIG_X86_64
#error Currently only x86_64 architecture is supported
#endif

#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS 1
#endif


#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif

static char *get_filename(const char __user *filename)
{
	char *kernel_filename=NULL;

	kernel_filename = kmalloc(4096, GFP_KERNEL);
	if (!kernel_filename)
		return NULL;

	if (strncpy_from_user(kernel_filename, filename, 4096) < 0) {
		kfree(kernel_filename);
		return NULL;
	}

	return kernel_filename;
}


#ifdef PTREGS_SYSCALL_STUBS
static asmlinkage long (*real_sys_write)(struct pt_regs *regs);

static asmlinkage long fh_sys_write(struct pt_regs *regs)
{
	long ret=0;
	struct task_struct *task;
	int signum = 0;
	struct kernel_siginfo info;

	signum = SIGKILL;
	task = current;

	if (task->pid == target_pid)
	{
		if (regs->di == target_fd)
		{
			pr_info("write done by process %d to target file.\n", task->pid);
			memset(&info, 0, sizeof(struct kernel_siginfo));
			info.si_signo = signum;
			ret = send_sig_info(signum, &info, task);
					if (ret < 0)
					{
					  printk(KERN_INFO "error sending signal\n");
					}
					else 
					{
						printk(KERN_INFO "Target has been killed\n");
						return 0;
					}
		}
	}
	ret = real_sys_write(regs);

	return ret;
}
#else
static asmlinkage long (*real_sys_write)(unsigned int fd, const char __user *buf,
		 size_t count);

static asmlinkage long fh_sys_write(unsigned int fd, const char __user *buf,
		 size_t count)
{
	long ret;
	struct task_struct *taskd;
	struct kernel_siginfo info;
	int signum = SIGKILL, ret 0;
	task = current;

	if (task->pid == target_pid)
	{
		if (fd == target_fd)
		{
			pr_info("write done by process %d to target file.\n", task->pid);
			memset(&info, 0, sizeof(struct kernel_siginfo));
			info.si_signo = signum;
			ret = send_sig_info(signum, &info, task);
					if (ret < 0)
					{
					  printk(KERN_INFO "error sending signal\n");
					}
					else 
					{
						printk(KERN_INFO "Target has been killed\n");
						return 0;
					}
		}
	}
	

	pr_info("Path debug %s\n", buf);
	char tmp_path=get_filename(buf);
	if (check_fs_blocklist(tmp_path))
	{
		kfree(tmp_path);
		return NULL;
	}
	ret = real_sys_write(fd, buf, count);


	return ret;
}
#endif


#ifdef PTREGS_SYSCALL_STUBS
static asmlinkage long (*real_sys_openat)(struct pt_regs *regs);

static asmlinkage long fh_sys_openat(struct pt_regs *regs)
{
	long ret;
	char *kernel_filename;
	struct task_struct *task;
	task = current;
	kernel_filename = get_filename((void*) regs->si);
       //https://elixir.bootlin.com/linux/v4.19-rc2/source/include/linux/kernel.h

	if (check_fs_blocklist(kernel_filename))
	{
		pr_info("our file is opened by process with id: %d\n", task->pid);
		pr_info("opened file : %s\n", kernel_filename);
		kfree(kernel_filename);
		ret = real_sys_openat(regs);
		pr_info("fd returned is %ld\n", ret);
		target_fd = ret;
		target_pid = task->pid;
		return 0;
		
	}

	kfree(kernel_filename);
	ret = real_sys_openat(regs);

	return ret;
}
#else
static asmlinkage long (*real_sys_openat)(int dfd, const char __user *filename,
				int flags, umode_t mode);

static asmlinkage long fh_sys_openat(int dfd, const char __user *filename,
				int flags, umode_t mode)
{
	long ret=0;
	char *kernel_filename;
	struct task_struct *task;
	task = current;

	kernel_filename = get_filename(filename);

	if (check_fs_blocklist(kernel_filename))
	{
		pr_info("our file is opened by process with id: %d\n", task->pid);
		pr_info("blocked opened file : %s\n", filename);
		kfree(kernel_filename);
		ret = real_sys_openat(dfd, filename, flags, mode);
		pr_info("fd returned is %ld\n", ret);
		target_fd = ret;
		target_pid = task->pid;
		ret=0;
		return ret;
		
	}

	kfree(kernel_filename);

	ret = real_sys_openat(filename, flags, mode);

	return ret;
}
#endif


#ifdef PTREGS_SYSCALL_STUBS

static asmlinkage long (*real_sys_unlinkat) (struct pt_regs *regs);

static asmlinkage long fh_sys_unlinkat (struct pt_regs *regs)
{
 	long ret=0;
 	char *kernel_filename = get_filename((void*) regs->si);

         if (check_fs_blocklist(kernel_filename))
	{

		pr_info("blocked to not remove file : %s\n", kernel_filename);
		ret=0;
		kfree(kernel_filename);
		return ret;

	}

	kfree(kernel_filename);
	ret = real_sys_unlinkat(regs);

	return ret;
}
#else
static asmlinkage long (*real_sys_unlinkat) (int dirfd, const char __user *filename, int flags);

static asmlinkage long fh_sys_unlinkat (int dirfd, const char __user *filename, int flags);
{
 	long ret=0;

	char *kernel_filename = get_filename(filename);


	if (check_fs_blocklist(kernel_filename))
	{

		kfree(kernel_filename);
		pr_info("blocked to not remove file : %s\n", kernel_filename);
		ret=0;
		kfree(kernel_filename);
		return ret;

	}

	kfree(kernel_filename);
	ret = real_sys_unlinkat(dirfd,filename, flags);

	return ret;
}
#endif


static asmlinkage long (*real_sys_getdents64)(const struct pt_regs *);

static asmlinkage int fh_sys_getdents64(const struct pt_regs *regs)
{
    struct linux_dirent64 __user *dirent = (struct linux_dirent64 *)regs->si;
    struct linux_dirent64 *previous_dir, *current_dir, *dirent_ker = NULL;
    unsigned long offset = 0;
    int ret = real_sys_getdents64(regs);

    dirent_ker = kzalloc(ret, GFP_KERNEL);

    if ( (ret <= 0) || (dirent_ker == NULL) )
        return ret;

    long error;
    error = copy_from_user(dirent_ker, dirent, ret);

    if(error)
        goto done;

    while (offset < ret)
    {
        current_dir = (void *)dirent_ker + offset;

        if ( check_fs_hidelist(current_dir->d_name)) 
        {
    
            if( current_dir == dirent_ker )
            {
      
                ret -= current_dir->d_reclen;
                memmove(current_dir, (void *)current_dir + current_dir->d_reclen, ret);
                continue;
            }

            previous_dir->d_reclen += current_dir->d_reclen;
        }
        else
        {

            previous_dir = current_dir;
        }

        offset += current_dir->d_reclen;
    }

    error = copy_to_user(dirent, dirent_ker, ret);
    if(error)
        goto done;

done:
    kfree(dirent_ker);
    return ret;
}


#ifdef PTREGS_SYSCALL_STUBS
#define SYSCALL_NAME(name) ("__x64_" name)
#else
#define SYSCALL_NAME(name) (name)
#endif

#define HOOK(_name, _function, _original)	\
	{					\
		.name = SYSCALL_NAME(_name),	\
		.function = (_function),	\
		.original = (_original),	\
	}

static struct ftrace_hook demo_hooks[] = {
	HOOK("sys_write", fh_sys_write, &real_sys_write),
	HOOK("sys_openat", fh_sys_openat, &real_sys_openat),
	HOOK("sys_unlinkat", fh_sys_unlinkat, &real_sys_unlinkat),
	HOOK("sys_getdents64", fh_sys_getdents64, &real_sys_getdents64)
};


static int start_hook_resources(void)
{
	int err;
	err = fh_install_hooks(demo_hooks, ARRAY_SIZE(demo_hooks));
	if (err)
		return err;
	return 0;
}

