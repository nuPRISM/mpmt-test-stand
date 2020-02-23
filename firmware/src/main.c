#include <zephyr.h>
#include <kernel.h>
#include <sys/printk.h>
#include <string.h>

// Stack per thread
#define STACK_SIZE 1024

// Priority for each thread
#define PRIORITY 7

void uart_out(void)
{
    while (1)
    {
        printk("Hello World\n");
        k_sleep(1000);
    }
}

K_THREAD_DEFINE(uart_out_id, STACK_SIZE, uart_out, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);