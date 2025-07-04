#include <linux/module.h> 
#include <linux/printk.h> 
#include <linux/kobject.h> 
#include <linux/sysfs.h> 
#include <linux/init.h> 
#include <linux/fs.h> 
#include <linux/string.h>
#include <linux/configfs.h>
#include <linux/tty.h>          
#include <linux/kd.h>        
#include <linux/vt.h>
#include <linux/console_struct.h>  
#include <linux/vt_kern.h>
#include <linux/timer.h>

struct timer_list my_timer; // Таймер (отложенное выполнение функций)
struct tty_driver *my_driver; // Указатель на драйвер TTY
static int _kbledstatus = 0; // Текущее состояние светодиодов
static int test = 7; // Маска светодиодов для мигания
#define BLINK_DELAY   HZ/5 // Задержка мигания (1/5 секунды)
#define ALL_LEDS_ON   0x07 // Включение всех светодиодов
#define RESTORE_LEDS  0xFF // Восстановления стандартного режима (выводит действуюшие состояния)

// БаZоVая структура для организации объектов в ядре
static struct kobject *example_kobject;

// Чтение из пространства ядра
static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", test);
}

// Запись в пространство ядра
static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    if (sscanf(buf, "%du", &test) != count) {
        return -EINVAL;
    } else {
        return count;
    }
}

// Структура атрибута (файла) для нашего kobject
static struct kobj_attribute foo_attribute =__ATTR(test, 0660, foo_show, foo_store);

static void my_timer_func(struct timer_list *ptr) {
    // Остатки староой реализации, где требовался указатель
    int *pstatus = &_kbledstatus;

    // Проверяем статус светодиодов:
    // Если он равен заданному, то восстанавливаем индикаторы
    // Если он не равен заданному, то устанавливаем заданное значение 
    if (*pstatus == test)
        *pstatus = RESTORE_LEDS;
    else
        *pstatus = test;

    // Указатель на функцию ioctl конкретного драйвера
    // Передаем устройство + команду + заданное состояние светодиодов
    (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, *pstatus);

    // Текущее время + задержка мигания
    my_timer.expires = jiffies + BLINK_DELAY;

    // Снова вызываем через время
    add_timer(&my_timer);
}

static int kbleds_init(void) {
    int i;

    printk(KERN_INFO "kbleds: loading\n");
    printk(KERN_INFO "kbleds: fgconsole is %x\n", fg_console);
    
    // Перебор виртуальный консолей
    for (i = 0; i < MAX_NR_CONSOLES; i++) {
        // Если консоль не инициализирована, то выход
        if (!vc_cons[i].d)
            break;
        printk(KERN_INFO "poet_atkm: console[%i/%i] #%i, tty %lx\n", i,
            MAX_NR_CONSOLES, vc_cons[i].d->vc_num,
            (unsigned long)vc_cons[i].d->port.tty);
    }
    printk(KERN_INFO "kbleds: finished scanning consoles\n");

    // Драйвер активной консоли
    my_driver = vc_cons[fg_console].d->port.tty->driver;
    printk(KERN_INFO "kbleds: tty driver name: %s\n", my_driver->name);

    // Инициализация таймера и первый запуск нашей функции
    timer_setup(&my_timer, my_timer_func, 0);
    my_timer.expires = jiffies + BLINK_DELAY;
    add_timer(&my_timer);

    return 0;
}

static void kbleds_cleanup(void) {
    printk(KERN_INFO "kbleds: unloading...\n");
    // Удаление таймера
    del_timer(&my_timer);
    // Восстановление светодиодов
    (my_driver->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_LEDS);
}

// Загрузка модуля
static int __init sys_init (void) {
    int error = 0;

    pr_debug("Module initialized successfully \n");

    // Создание обьекта ядра
    example_kobject = kobject_create_and_add("systest", kernel_kobj); 
    if(!example_kobject) 
        return -ENOMEM;

    // Создание атрибута (файла) для нашего kobject
    error = sysfs_create_file(example_kobject, &foo_attribute.attr);
    if (error) {
        pr_debug("failed to create the foo file in /sys/kernel/systest \n");
        return error;
    }

    // Инициализация мигания
    kbleds_init();

    return error;
}

// Выгрузка модуля
static void __exit sys_exit (void) {
    // Завершение мигания
    kbleds_cleanup();
    pr_debug ("Module un initialized successfully \n");
    kobject_put(example_kobject);
}

module_init(sys_init);
module_exit(sys_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Egor Zemlyanoi");
MODULE_DESCRIPTION("Example module illustrating the use of Keyboard LEDs.");