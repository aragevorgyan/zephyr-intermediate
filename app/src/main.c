#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(demo, LOG_LEVEL_DBG);

#define STACK_SIZE 1024

#define PRIO 5
#define ITERATIONS 1000000

int num = 0;

K_MUTEX_DEFINE(my_mutex);


void inc_thread_fn(void *p1, void *p2, void *p3)
{
    for (size_t i = 0; i < ITERATIONS; i++)
    {
        k_mutex_lock(&my_mutex, K_FOREVER);
        num++;
        k_mutex_unlock(&my_mutex);
    }
    LOG_INF("num: %d\n", num);
}


K_THREAD_DEFINE(inc_thread_a, STACK_SIZE, inc_thread_fn,
                NULL, NULL, NULL, PRIO, 0, 0);
K_THREAD_DEFINE(inc_thread_b, STACK_SIZE, inc_thread_fn,
                NULL, NULL, NULL, PRIO, 0, 0);


int main(void)
{
    return 0;
}

