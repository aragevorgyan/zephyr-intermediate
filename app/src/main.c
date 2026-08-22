#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(demo, LOG_LEVEL_DBG);

#define STACK_SIZE 1024

#define PRIO_L 7
#define PRIO_M 5
#define PRIO_H 3
#define PRIO_COOP -1

void t_low_fn(void *p1, void *p2, void *p3)
{
    while (1) {
        LOG_INF("T_LOW running");
        k_msleep(300);
    }
}

void t_med_fn(void *p1, void *p2, void *p3)
{
    while (1) {
        LOG_INF("T_MED running");
        k_msleep(200);
    }
}

void t_high_fn(void *p1, void *p2, void *p3)
{
    while (1) {
        LOG_INF("T_HIGH running");
        k_msleep(100);
    }
}

void t_coop_fn(void *p1, void *p2, void *p3)
{
    for (size_t i = 0; i < 5; i++)
    {
        LOG_INF("T_COOP running");
    }
    k_yield();
}


K_THREAD_DEFINE(t_high, STACK_SIZE, t_high_fn,
                NULL, NULL, NULL, PRIO_H, 0, 0);
K_THREAD_DEFINE(t_med, STACK_SIZE, t_med_fn,
                NULL, NULL, NULL, PRIO_M, 0, 0);
K_THREAD_DEFINE(t_low, STACK_SIZE, t_low_fn,
                NULL, NULL, NULL, PRIO_L, 0, 0);
K_THREAD_DEFINE(t_coop, STACK_SIZE, t_coop_fn,
                NULL, NULL, NULL, PRIO_COOP, 0, 0);


int main(void)
{
    return 0;
}

