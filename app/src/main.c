#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

struct sensor_msg {
    int temperature;
    int sample_id;
};


/* Listener callback */
static void sensor_listener_cb(const struct zbus_channel *chan)
{
    const struct sensor_msg *msg = zbus_chan_const_msg(chan);

    LOG_INF("DISPLAY: sample=%d temp=%d",
            msg->sample_id,
            msg->temperature);
}


/* Define observers */
ZBUS_LISTENER_DEFINE(sensor_listener, sensor_listener_cb);

ZBUS_SUBSCRIBER_DEFINE(sensor_subscriber, 4);


/* Define channel */
ZBUS_CHAN_DEFINE(sensor_chan,
                 struct sensor_msg,
                 NULL,
                 NULL,
                 ZBUS_OBSERVERS(sensor_listener, sensor_subscriber),
                 ZBUS_MSG_INIT(0));


/* Subscriber thread */
static void logging_thread(void *p1, void *p2, void *p3)
{
    const struct zbus_channel *chan;
    struct sensor_msg msg;

    while (1) {
        zbus_sub_wait(&sensor_subscriber, &chan, K_FOREVER);

        zbus_chan_read(chan, &msg, K_FOREVER);

        LOG_INF("LOGGER: sample=%d temp=%d",
                msg.sample_id,
                msg.temperature);

        k_sleep(K_MSEC(300));
    }
}


K_THREAD_DEFINE(logging_thread_id,
                1024,
                logging_thread,
                NULL, NULL, NULL,
                5,
                0,
                0);


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

        LOG_INF("Published sample %d, temperature %d",
                msg.sample_id,
                msg.temperature);

        k_sleep(K_MSEC(100));
    }

    return 0;
}