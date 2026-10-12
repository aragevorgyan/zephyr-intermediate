#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/task_wdt/task_wdt.h>
#include <zephyr/zbus/zbus.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#define QUEUE_DEPTH        8
#define PRODUCER_PERIOD_MS 100
#define CONSUMER_STUCK_MS  2500
#define WDT_TIMEOUT_MS     1000

struct sensor_msg {
    int temperature;
    int sample_id;
};

static int consumer_wdt_chan;

static void consumer_wdt_callback(int channel_id, void *user_data)
{
    ARG_UNUSED(user_data);

    LOG_ERR("task watchdog fired on channel %d: consumer is stuck", channel_id);
}

static void sensor_listener_cb(const struct zbus_channel *chan)
{
    const struct sensor_msg *msg = zbus_chan_const_msg(chan);

    LOG_INF("DISPLAY: sample=%d temp=%d", msg->sample_id, msg->temperature);
}

ZBUS_LISTENER_DEFINE(sensor_listener, sensor_listener_cb);
ZBUS_SUBSCRIBER_DEFINE(sensor_subscriber, QUEUE_DEPTH);

ZBUS_CHAN_DEFINE(sensor_chan,
         struct sensor_msg,
         NULL,
         NULL,
         ZBUS_OBSERVERS(sensor_listener, sensor_subscriber),
         ZBUS_MSG_INIT(0));

static void logging_thread(void *p1, void *p2, void *p3)
{
    const struct zbus_channel *chan;
    struct sensor_msg msg;
    int ret;

    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    ret = task_wdt_init(NULL);
    if (ret < 0) {
        LOG_ERR("task_wdt_init failed: %d", ret);
        return;
    }

    consumer_wdt_chan = task_wdt_add(WDT_TIMEOUT_MS, consumer_wdt_callback, NULL);
    if (consumer_wdt_chan < 0) {
        LOG_ERR("task_wdt_add failed: %d", consumer_wdt_chan);
        return;
    }

    LOG_WRN("consumer stuck for %d ms", CONSUMER_STUCK_MS);
    k_sleep(K_MSEC(CONSUMER_STUCK_MS));

    while (1) {
        zbus_sub_wait(&sensor_subscriber, &chan, K_FOREVER);
        zbus_chan_read(chan, &msg, K_FOREVER);

        LOG_INF("LOGGER: sample=%d temp=%d", msg.sample_id, msg.temperature);

        task_wdt_feed(consumer_wdt_chan);
        k_sleep(K_MSEC(300));
    }
}

K_THREAD_DEFINE(logging_thread_id, 1024, logging_thread, NULL, NULL, NULL, 5, 0, 0);

static void health_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        uint32_t used = k_msgq_num_used_get(sensor_subscriber.queue);

        if (used > (QUEUE_DEPTH * 3 / 4)) {
            LOG_WRN("Queue at %u/%u", used, QUEUE_DEPTH);
        }

        k_msleep(200);
    }
}

K_THREAD_DEFINE(health_thread_id, 1024, health_thread, NULL, NULL, NULL, 6, 0, 0);

int main(void)
{
    struct sensor_msg msg = {
        .temperature = 20,
        .sample_id = 0,
    };

    while (1) {
        msg.sample_id++;
        msg.temperature++;

        zbus_chan_pub(&sensor_chan, &msg, K_FOREVER);

        LOG_INF("Published sample %d, temperature %d", msg.sample_id, msg.temperature);

        k_sleep(K_MSEC(PRODUCER_PERIOD_MS));
    }

    return 0;
}
