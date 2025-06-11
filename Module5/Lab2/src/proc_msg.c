#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/types.h>

#define PROC_NAME "hello_proc"
#define BUF_SIZE  128
#define PROC_MODE 0644

static char *msg_buf;
static ssize_t msg_size;

static ssize_t hello_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos) {
    ssize_t avail; // Cколько байт ещё осталось отдать
    ssize_t ret;   // Cколько байт мы реально отдадим

    // Если пользователь дочитал до конца
    if (*ppos >= msg_size)
        return 0;
    
    // Сколько байт осталось в нашем буфере
    avail = msg_size - *ppos;

    // Не даём пользователю читать больше, чем он просит и чем есть
    if (count > avail)
        count = avail;

    // Читаем из пространства ядра в пространство пользователя
    if (copy_to_user(ubuf, msg_buf + *ppos, count))
        return -EFAULT; // Ошибка доступа

    // Обновляем смещение: пусть дальше продолжит оттуда
    *ppos += count;
    ret = count;

    return ret;
}

// Write-обработчик: принимает строку от пользователя
static ssize_t hello_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos) {
    // Предотвращение переполнения буфера
    ssize_t write_len = min(count, (size_t)(BUF_SIZE - 1));

    // Копироание из пользоватеского пространства в пространство ядра
    if (copy_from_user(msg_buf, ubuf, write_len))
        return -EFAULT; // Ошибка доступа    
    
    msg_buf[write_len] = '\0';
    msg_size = write_len;
    return write_len;
}

// Структура с указанием функции при чтении и записи
static const struct proc_ops hello_proc_ops = {
    .proc_read  = hello_read,
    .proc_write = hello_write,
};

// Структура с указателем на созданный фвйл
static struct proc_dir_entry *my_proc_entry;

// Загрузка модуля ядра
static int __init hello_init(void) {
    msg_buf = kmalloc(BUF_SIZE, GFP_KERNEL); // 
    // Проверка выделения памяти
    if (!msg_buf)
        return -ENOMEM;
    // Сообщение по умолчанию
    msg_size = snprintf(msg_buf, BUF_SIZE, "Hello, world!\n");

    // Создание файла
    my_proc_entry = proc_create(PROC_NAME, PROC_MODE, NULL, &hello_proc_ops);
    if (!my_proc_entry) {
        kfree(msg_buf);
        pr_err("hello_proc: не удалось создать /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    pr_info("hello_proc: модуль загружен, доступен /proc/%s\n", PROC_NAME);

    return 0;
}

// Выгрузка модуля ядра
static void __exit hello_exit(void) {
    proc_remove(my_proc_entry);
    kfree(msg_buf);
    pr_info("hello_proc: модуль выгружен\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Egor Zemlyanoi");
MODULE_DESCRIPTION("/proc-модуль, хранящий сообщения пользователя");