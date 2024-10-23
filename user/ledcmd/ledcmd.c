#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <linux/ledman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define LED_ENT(x)		{ #x , LEDMAN_##x },

static struct {
	const char *name;
	int   led;
} led_name[] = {
	LED_ENT(ALL)
	LED_ENT(POWER)
	LED_ENT(HEARTBEAT)
	LED_ENT(COM1_RX)
	LED_ENT(COM1_TX)
	LED_ENT(COM2_RX)
	LED_ENT(COM2_TX)
	LED_ENT(LAN1_RX)
	LED_ENT(LAN1_TX)
	LED_ENT(LAN2_RX)
	LED_ENT(LAN2_TX)
	LED_ENT(USB1_RX)
	LED_ENT(USB1_TX)
	LED_ENT(USB2_RX)
	LED_ENT(USB2_TX)
	LED_ENT(NVRAM_1)
	LED_ENT(NVRAM_2)
	LED_ENT(VPN)
	LED_ENT(LAN1_DHCP)
	LED_ENT(LAN2_DHCP)
	LED_ENT(COM1_DCD)
	LED_ENT(COM2_DCD)
	LED_ENT(ONLINE)
	LED_ENT(LAN1_LINK)
	LED_ENT(LAN2_LINK)
	LED_ENT(VPN_RX)
	LED_ENT(VPN_TX)
	LED_ENT(RESET)
	LED_ENT(STATIC)
	LED_ENT(LAN3_RX)
	LED_ENT(LAN3_TX)
	LED_ENT(LAN3_LINK)
	LED_ENT(LAN3_DHCP)
	LED_ENT(FAILOVER)
	LED_ENT(HIGHAVAIL)
	LED_ENT(LAN4_LINK)
	LED_ENT(RSS1)
	LED_ENT(RSS2)
	LED_ENT(RSS3)
	LED_ENT(RSS4)
	LED_ENT(RSS5)
	LED_ENT(ETH)
	LED_ENT(USB)
	LED_ENT(COM)
	LED_ENT(LAN4_RX)
	LED_ENT(LAN4_TX)
	LED_ENT(VM)
	LED_ENT(SIM1)
	LED_ENT(SIM2)
	LED_ENT(SIM1_RED)
	LED_ENT(SIM1_GREEN)
	LED_ENT(SIM1_BLUE)
	LED_ENT(SIM2_RED)
	LED_ENT(SIM2_GREEN)
	LED_ENT(SIM2_BLUE)
	LED_ENT(SIM_FAIL)
	LED_ENT(XBEE1)
	LED_ENT(XBEE2)
	LED_ENT(XBEE3)
#ifdef LEDMAN_NVRAM_ALL
	LED_ENT(NVRAM_ALL)
#endif
	{ NULL, -1 }
};

static void print_led_names(void);

void
usage(int rc)
{
	printf("usage: ledcmd [-h?] [-a] ((-s|-o|-O|-f|-r|-n|-N) <led#|name>) ...\n\n"
		"\t-h?\thelp  \t- what you see below\n"
		"\t-s\tset    \t- turn LED on briefly\n"
		"\t-o\ton     \t- turn LED on\n"
		"\t-O\toff    \t- turn LED off\n"
		"\t-f\tflash  \t- make LED flash\n"
		"\t-r\treset  \t- return LED to default behaviour\n"
		"\t-n\talton  \t- use alternate LED mode\n"
		"\t-N\taltoff \t- stop alternate LED mode\n"
		"\t-a\taltbit \t- alt LED mode applies to following commands\n"
		"\t-A\t~altbit\t- alt LED mode does not apply to following commands\n"
		"\t-l\tlist   \t- list LEDs\n"
		"\t-L\tstates \t- list LED states\n"
		"\n");
	print_led_names();
	exit(rc);
}

static void print_led_names(void)
{
	int i;
	for (i = 0; led_name[i].name; i++) {
		printf("%s%s", led_name[i].name, i % 8 == 7 ? "\n" : " ");
	}
	printf("\n\n");
}

static void print_led_states(void)
{
	printf("OFF ON FLASH RESET\n\n");
}

static void
set_led(int cmd, char *led)
{
	int i, _led = -1;

	if (*led < '0' || *led > '9') {
		for (i = 0; led_name[i].name; i++)
			if (strcasecmp(led, led_name[i].name) == 0) {
				_led = led_name[i].led;
				break;
			}
	} else
		_led = atoi(led);

	if (_led < 0 || _led >= LEDMAN_MAX) {
		printf("ERROR: Invalid LED %s (0-%d)\n", led, LEDMAN_MAX);
		usage(1);
	}

	ledman_cmd(cmd, _led);
}

int
main(int argc, char *argv[])
{
	int alt, c;

	alt = 0;

	while ((c = getopt(argc, argv, "?haAlLqs:o:O:f:r:n:N:")) != -1) {
		switch (c) {
		case 's': set_led(LEDMAN_CMD_SET    | alt, optarg); break;
		case 'o': set_led(LEDMAN_CMD_ON     | alt, optarg); break;
		case 'O': set_led(LEDMAN_CMD_OFF    | alt, optarg); break;
		case 'f': set_led(LEDMAN_CMD_FLASH  | alt, optarg); break;
		case 'r': set_led(LEDMAN_CMD_RESET  | alt, optarg); break;
		case 'n': set_led(LEDMAN_CMD_ALT_ON | alt, optarg); break;
		case 'N': set_led(LEDMAN_CMD_ALT_OFF| alt, optarg); break;

		case 'a': alt |= LEDMAN_CMD_ALTBIT;  break;
		case 'A': alt &= ~LEDMAN_CMD_ALTBIT; break;

		case 'l': print_led_names(); return 0;
		case 'L': print_led_states(); return 0;

		case '?':
		case 'h':
			usage(0);
			break;
		case 'q':
			printf("\"query led\" not supported for this device\n");
		default:
			usage(1);
			break;
		}
	}

	if (argc < 3 || optind != argc)
		usage(1);
	return 0;
}
