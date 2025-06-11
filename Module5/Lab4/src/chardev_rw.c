#include <linux/cdev.h> 
#include <linux/delay.h> 
#include <linux/device.h> 
#include <linux/fs.h> 
#include <linux/init.h> 
#include <linux/irq.h> 
#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/poll.h> 

// Прототипы
static int device_open(struct inode *, struct file *); 
static int device_release(struct inode *, struct file *); 
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *); 
static ssize_t device_write(struct file *, const char __user *, size_t, loff_t *); 

#define SUCCESS 0               // Успех
#define DEVICE_NAME "chardev"   // Имя устройства
#define BUF_LEN 80              // Размер буфера

static int major; // Номер драйвера
// Состояния использования устройства
enum { 
    CDEV_NOT_USED = 0, 
    CDEV_EXCLUSIVE_OPEN = 1, 
}; 

static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED); 
static char initial_msg[BUF_LEN];   // Сообщение
static char user_buf[BUF_LEN] = "\0";
static struct class *cls;   // Класс для создаваемого устройства
// Набор операций
static struct file_operations chardev_fops = { 
    .read = device_read, 
    .write = device_write, 
    .open = device_open, 
    .release = device_release, 
}; 

static int __init chardev_init(void) { 
    // Регистрация драйвера chardev
    major = register_chrdev(0, DEVICE_NAME, &chardev_fops); 

    // Проверка результата регистрации
    if (major < 0) { 
        pr_alert("Registering char device failed with %d\n", major); 
        return major; 
    }

    pr_info("I was assigned major number %d.\n", major); 

    // Создание класса chardev (/sys/class/chardev)
    cls = class_create(DEVICE_NAME); 
    // Создание устройства, обслуживаемое драйвером (/dev/chardev)
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME); 
    pr_info("Device created on /dev/%s\n", DEVICE_NAME); 
    return SUCCESS; 
} 

static void __exit chardev_exit(void){ 
    device_destroy(cls, MKDEV(major, 0)); 
    class_destroy(cls); 

    unregister_chrdev(major, DEVICE_NAME); 
} 

static int device_open(struct inode *inode, struct file *file){ 
    static int counter = 0; 

    // Атомарная операция «сравнить и заменить»
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) 
        return -EBUSY; 

    sprintf(initial_msg, "I already told you %d times Hello world!\n", counter++); 
    // Увеличение счётчика использования модуля
    try_module_get(THIS_MODULE); 
    return SUCCESS; 
} 

static int device_release(struct inode *inode, struct file *file){ 
    atomic_set(&already_open, CDEV_NOT_USED); 
    module_put(THIS_MODULE); 
    return SUCCESS; 
} 

static ssize_t device_read(struct file *filp,
                           char __user *buffer,
                           size_t length,
                           loff_t *offset) {
    int bytes_read = 0;         // Счетчик прочитанных байтов

    // Заполнение строки с данными
    const char *msg_ptr;
    if (user_buf[0] != '\0') {
        msg_ptr = user_buf;
    } else {
        msg_ptr = initial_msg;
    }

    // Проверка конца строки при смещении (проверна на нулевой символ)
    if (!*(msg_ptr + *offset)) {
        *offset = 0; // Возвращение смещения к началу 
        return 0; 
    } 
    msg_ptr += *offset; 

    // Копирование данных в буфер
    while (length && *msg_ptr) { 
        // Копирование байта из пространства ядра в пространство пользователя         
        put_user(*(msg_ptr++), buffer++); 
        length--; 
        bytes_read++; 
    } 
    
    // Обновление смещения
    *offset += bytes_read;
    
    return bytes_read; 
} 

static ssize_t device_write(struct file *filp, const char __user *buff, 
                            size_t len, loff_t *off){ 
    ssize_t to_copy;
    ssize_t not_copied;
    ssize_t written;

    // Проверка переполнения буфера
    if (len > BUF_LEN - 1)
        to_copy = BUF_LEN - 1;
    else
        to_copy = len;

    // Копирование из юзера в ядро
    not_copied = copy_from_user(user_buf, buff, to_copy); // Число байт, которые не удалось скопировать
    written = to_copy - not_copied;
    user_buf[written] = '\0';

    *off = 0;   // Установка смещения в начало

    return written;
} 

module_init(chardev_init); 
module_exit(chardev_exit); 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Egor Zemlyanoi");
MODULE_DESCRIPTION("Пример простого символьного устройства");
