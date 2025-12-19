#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timekeeping.h>
#include <linux/time64.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kuznetsov Roman");
MODULE_DESCRIPTION("Unixlab3.2");
MODULE_VERSION("1.1");
MODULE_INFO(build_date, "2025-12-18");

#define URANUS_PERIOD_SEC 2650083840ULL
#define ECLIPTIC_CORRECTION_SEC ((URANUS_PERIOD_SEC * 9) / 360)

static int __init uranus_init(void) {
    struct timespec64 ts;
    struct tm tm_now, tm_future;
    uint64_t total_seconds;

    ktime_get_real_ts64(&ts);
    time64_to_tm(ts.tv_sec, 0, &tm_now);

    total_seconds = ts.tv_sec + URANUS_PERIOD_SEC + ECLIPTIC_CORRECTION_SEC;
    time64_to_tm(total_seconds, 0, &tm_future);

    printk(KERN_INFO "Welcome to the Tomsk State University\n");
    printk(KERN_INFO "TSU Status: Current year is %ld\n", (long)tm_now.tm_year + 1900);
    printk(KERN_INFO "TSU Info: 9-degree correction applied.\n");
    printk(KERN_INFO "TSU Result: Uranus returns in %ld year\n", (long)tm_future.tm_year + 1900);

    return 0;
}

static void __exit uranus_exit(void) {
    printk(KERN_INFO "Tomsk State University forever!\n");
}


module_init(uranus_init);
module_exit(uranus_exit);