#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Egor Zemlyanoi");
MODULE_DESCRIPTION("A Simple Hello World module");

static int __init hello_init(void) {
    printk(KERN_INFO "Hello world from EgZeml!\n");
    return 0;    // Non-zero return means that the module couldn't be loaded.
}

static void __exit hello_cleanup(void) {
    printk(KERN_INFO "Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);
