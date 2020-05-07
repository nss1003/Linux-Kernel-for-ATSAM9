#include <linux/init.h>
#include <linux/module.h>

static int hello_init(void){

	printk(KERN_ALERT "Test: Hello Ngoufack first kernel module\n";)
	return 0;
}

static void hello_exit(void){
	printk(KERN_ALERT "TOTO: Good bye nigga");
}


module_init(hello_init);
module_exit(hello_exit);
