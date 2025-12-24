#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/timekeeping.h>
#include <linux/time64.h>
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kuznetsov Roman");
MODULE_DESCRIPTION("Unixlab3.2");
MODULE_VERSION("1.2");

#define FILENAME "tsulab"
#define URANUS_PERIOD_SEC 2650083840ULL
#define ECLIPTIC_CORRECTION_SEC ((URANUS_PERIOD_SEC * 9) / 360)

static struct proc_dir_entry *our_proc_file = NULL;

static void calculate_result(char *buffer, size_t max_len)
{
    struct timespec64 ts;
    struct tm tm_now, tm_future;
    uint64_t total_seconds;

    ktime_get_real_ts64(&ts);
    time64_to_tm(ts.tv_sec, 0, &tm_now);

    total_seconds = ts.tv_sec + URANUS_PERIOD_SEC + ECLIPTIC_CORRECTION_SEC;
    time64_to_tm(total_seconds, 0, &tm_future);

    snprintf(buffer, max_len,
             "Current year is %ld\nResult: Uranus returns in %ld year\n",
             (long)tm_now.tm_year + 1900,
             (long)tm_future.tm_year + 1900);
}

static ssize_t reading(struct file *file_pointer, char __user *buffer, size_t buffer_length, loff_t *offset)
{
    char s[256];
    int len;

    if (*offset > 0)
        return 0;

    calculate_result(s, sizeof(s));
    len = strlen(s);

    if (len > buffer_length)
        len = buffer_length;

    if (copy_to_user(buffer, s, len))
        return -EFAULT;

    *offset += len;

    return len;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_file_fops = {
    .proc_read = reading,
};
#else
static const struct file_operations proc_file_fops = {
    .read = reading,
};
#endif

static int __init uranus_init(void)
{
    pr_info("Welcome to the Tomsk State University\n");

    our_proc_file = proc_create(FILENAME, 0644, NULL, &proc_file_fops);

    if (our_proc_file == NULL)
    {
        pr_alert("Error: Could not initialize /proc/%s\n", FILENAME);
        return -ENOMEM;
    }

    return 0;
}

static void __exit uranus_exit(void)
{
    proc_remove(our_proc_file);
    pr_info("Tomsk State University forever!\n");
}

module_init(uranus_init);
module_exit(uranus_exit);