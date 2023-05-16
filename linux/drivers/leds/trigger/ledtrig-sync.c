#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/leds.h>
#include <linux/timer.h>

struct sync_led_data {
	struct led_trigger *onehz;
	struct led_trigger *halfhz;
	bool on;
	char skip;
} sync_data;

static struct timer_list check_timer;

static void timer_callback(struct timer_list *t)
{
	unsigned long j = jiffies;

	switch (sync_data.skip) {
	case 0:
		led_trigger_event(sync_data.onehz, LED_FULL);
		led_trigger_event(sync_data.halfhz, LED_FULL);
		break;
	case 1:
		led_trigger_event(sync_data.halfhz, LED_OFF);
		break;
	case 2:
		led_trigger_event(sync_data.onehz, LED_OFF);
		led_trigger_event(sync_data.halfhz, LED_FULL);
		break;
	case 3:
		led_trigger_event(sync_data.halfhz, LED_OFF);
		break;
	default:
		led_trigger_event(sync_data.onehz, LED_OFF);
		led_trigger_event(sync_data.halfhz, LED_OFF);
	}
	sync_data.skip++;
	if (sync_data.skip == 4)
		sync_data.skip = 0;
	mod_timer(&check_timer, jiffies + msecs_to_jiffies(500) - (jiffies - j));
}

static int __init sync_trig_init(void)
{
	sync_data.on = false;

	led_trigger_register_simple("halfhz", &sync_data.halfhz);
	led_trigger_register_simple("onehz", &sync_data.onehz);

	timer_setup(&check_timer, timer_callback, 0);
	mod_timer(&check_timer, jiffies + msecs_to_jiffies(500));

	return 0;
}

static void __exit sync_trig_exit(void)
{
	del_timer_sync(&check_timer);
	led_trigger_unregister_simple(sync_data.halfhz);
	led_trigger_unregister_simple(sync_data.onehz);
}

module_init(sync_trig_init);
module_exit(sync_trig_exit);

MODULE_DESCRIPTION("Half and One HZ timer led trigger");
MODULE_AUTHOR("Prevas A/S");
MODULE_LICENSE("GPL");
