/*
 * A standalone Ralink WPS Led/Push Button control utility.
 *
 * Compile:
 * mipsel-linux-gcc wps_pbc_led.c -o wps_pbc_led.c
 *
 * Usage:
 * # wpsled [LED_gpio] &
 * # wpsled [LED_gpio PBC_gpio] &
 * # wpsled [LED_gpio PBC_gpio wlan] &
 * LED_gpio: the Led GPIO number(default 13)
 * PBC_gpio: the PBC GPIO number(default 0)
 * wlan: the wireless interface(default 'ra0')
 *
 */
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <linux/if.h>
#include <linux/wireless.h>
#include <ctype.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

//#include "ralink_gpio.h"

#define WPS_AP_LED_GPIO		13
#define WPS_AP_PBC_GPIO		0

#define WPS_LED_RESET           1
#define WPS_LED_PROGRESS        2
#define WPS_LED_ERROR           3
#define WPS_LED_SESSION_OVERLAP 4
#define WPS_LED_SUCCESS         5

#define LedReset()                  {ledWps(led_gpio, WPS_LED_RESET);}
#define LedInProgress()             {ledWps(led_gpio, WPS_LED_PROGRESS);}
#define LedError()                  {ledWps(led_gpio, WPS_LED_ERROR);}
#define LedSessionOverlapDetected() {ledWps(led_gpio, WPS_LED_SESSION_OVERLAP);}
#define LedSuccess()                {ledWps(led_gpio, WPS_LED_SUCCESS);}

/* WPS ioctl */
#define RT_PRIV_IOCTL				(SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_WSC_PROFILE	(SIOCIWFIRSTPRIV + 0x12)
#define RT_OID_SYNC_RT61			0x0D010750
#define RT_OID_WSC_QUERY_STATUS		((RT_OID_SYNC_RT61 + 0x01) & 0xffff)

typedef struct  __attribute__ ((packed)) _WSC_CONFIGURED_VALUE {
	unsigned short	WscConfigured; // 1 un-configured; 2 configured
	unsigned char	WscSsid[32 + 1];
	unsigned short	WscAuthMode; // mandatory, 0x01: open, 0x02: wpa-psk, 0x04: shared, 0x08:wpa, 0x10: wpa2, 0x
	unsigned short	WscEncrypType;  // 0x01: none, 0x02: wep, 0x04: tkip, 0x08: aes
	unsigned char	DefaultKeyIdx;
	unsigned char	WscWPAKey[64 + 1];
} WSC_CONFIGURED_VALUE;


/* GPIO ioctl */
typedef struct {
    unsigned int irq;       //request irq pin number
    pid_t pid;          //process id to notify
} ralink_gpio_reg_info;


typedef struct {
    int gpio;           //gpio number (0 ~ 23)
    unsigned int on;        //interval of led on
    unsigned int off;       //interval of led off
    unsigned int blinks;        //number of blinking cycles
    unsigned int rests;     //number of break cycles
    unsigned int times;     //blinking times
} ralink_gpio_led_info;

#define RALINK_GPIO_LED_SET		0x41
/*
 * bit is the unit of length
 */
#define RALINK_GPIO_NUMBER		24
#define RALINK_GPIO_DATA_MASK		0x00FFFFFF
#define RALINK_GPIO_DATA_LEN		24
#define RALINK_GPIO_DIR_IN		0
#define RALINK_GPIO_DIR_OUT		1
#define RALINK_GPIO_DIR_ALLIN		0
#define RALINK_GPIO_DIR_ALLOUT		0x00FFFFFF

#define RALINK_GPIO_LED_LOW_ACT		1
#define RALINK_GPIO_LED_INFINITY	4000

#define RALINK_GPIO_SET_DIR     0x01
#define RALINK_GPIO_SET_DIR_IN      0x11
#define RALINK_GPIO_SET_DIR_OUT     0x12
#define RALINK_GPIO_READ        0x02
#define RALINK_GPIO_WRITE       0x03
#define RALINK_GPIO_SET         0x21
#define RALINK_GPIO_CLEAR       0x31
#define RALINK_GPIO_READ_BIT        0x04
#define RALINK_GPIO_WRITE_BIT       0x05
#define RALINK_GPIO_READ_BYTE       0x06
#define RALINK_GPIO_WRITE_BYTE      0x07
#define RALINK_GPIO_READ_INT        0x02 //same as read
#define RALINK_GPIO_WRITE_INT       0x03 //same as write
#define RALINK_GPIO_SET_INT     0x21 //same as set
#define RALINK_GPIO_CLEAR_INT       0x31 //same as clear
#define RALINK_GPIO_ENABLE_INTP     0x08
#define RALINK_GPIO_DISABLE_INTP    0x09
#define RALINK_GPIO_REG_IRQ     0x0A
#define RALINK_GPIO_LED_SET     0x41



int CurrentState = 0;	/* default state is "not used" */
int CurrentConf = 1;	/* default is un-configured */

int led_gpio = WPS_AP_LED_GPIO;
int pbc_gpio = WPS_AP_PBC_GPIO;
char *wlan_if = "ra0";

int getWscProfile(char *interface, WSC_CONFIGURED_VALUE *data, int len)
{
    int socket_id;
    struct iwreq wrq;

    socket_id = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy((char *)data, "get_wsc_profile");
    strcpy(wrq.ifr_name, interface);
    wrq.u.data.length = len;
    wrq.u.data.pointer = (caddr_t) data;
    wrq.u.data.flags = 0;
    if( ioctl(socket_id, RTPRIV_IOCTL_WSC_PROFILE, &wrq) == -1){
		perror("ioctl error");
		close(socket_id);
		return -1;
	}
    close(socket_id);
    return 0;
}


int getWscStatus(char *interface)
{
	int socket_id;
	struct iwreq wrq;
	int data = 0;
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	strcpy(wrq.ifr_name, interface);
	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = (caddr_t) &data;
	wrq.u.data.flags = RT_OID_WSC_QUERY_STATUS;
	if( ioctl(socket_id, RT_PRIV_IOCTL, &wrq) == -1){
		perror("ioctl error");
		close(socket_id);
		return -1;
	}
	close(socket_id);
	return data;
}


/*
 * these definitions are from rt2860v2 driver include/wsc.h 
 */
/*
char *getWscStatusStr(int status)
{
	switch(status){
	case 0:
		return "Not used";
	case 1:
		return "Idle";
	case 2:
		return "WSC Fail(Ignore this if Intel/Marvell registrar used)";
	case 3:
		return "Start WSC Process";
	case 4:
		return "Received EAPOL-Start";
	case 5:
		return "Sending EAP-Req(ID)";
	case 6:
		return "Receive EAP-Rsp(ID)";
	case 7:
		return "Receive EAP-Req with wrong WSC SMI Vendor Id";
	case 8:
		return "Receive EAPReq with wrong WSC Vendor Type";
	case 9:
		return "Sending EAP-Req(WSC_START)";
	case 10:
		return "Send M1";
	case 11:
		return "Received M1";
	case 12:
		return "Send M2";
	case 13:
		return "Received M2";
	case 14:
		return "Received M2D";
	case 15:
		return "Send M3";
	case 16:
		return "Received M3";
	case 17:
		return "Send M4";
	case 18:
		return "Received M4";
	case 19:
		return "Send M5";
	case 20:
		return "Received M5";
	case 21:
		return "Send M6";
	case 22:
		return "Received M6";
	case 23:
		return "Send M7";
	case 24:
		return "Received M7";
	case 25:
		return "Send M8";
	case 26:
		return "Received M8";
	case 27:
		return "Processing EAP Response (ACK)";
	case 28:
		return "Processing EAP Request (Done)";
	case 29:
		return "Processing EAP Response (Done)";
	case 30:
		return "Sending EAP-Fail";
	case 31:
		return "WSC_ERROR_HASH_FAIL";
	case 32:
		return "WSC_ERROR_HMAC_FAIL";
	case 33:
		return "WSC_ERROR_DEV_PWD_AUTH_FAIL";
	case 34:
		return "Configured";
	case 35:
		return "SCAN AP";
	case 36:
		return "EAPOL START SENT";
	case 37:
		return "WSC_EAP_RSP_DONE_SENT";
	case 38:
		return "WAIT PINCODE";
	case 39:
		return "WSC_START_ASSOC";
	case 0x101:
		return "PBC:TOO MANY AP";
	case 0x102:
		return "PBC:NO AP";
	case 0x103:
		return "EAP_FAIL_RECEIVED";
	case 0x104:
		return "EAP_NONCE_MISMATCH";
	case 0x105:
		return "EAP_INVALID_DATA";
	case 0x106:
		return "PASSWORD_MISMATCH";
	case 0x107:
		return "EAP_REQ_WRONG_SMI";
	case 0x108:
		return "EAP_REQ_WRONG_VENDOR_TYPE";
	case 0x109:
		return "PBC_SESSION_OVERLAP";
	default:
		return "Unknown";
	}
}
*/


/*
 * The setitimer() is Linux-specified.
 */
int setTimer(int microsec, void ((*sigroutine)(int)))
{
	struct itimerval value, ovalue;

	signal(SIGALRM, sigroutine);
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = microsec;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = microsec;
	return setitimer(ITIMER_REAL, &value, &ovalue);
}

void stopTimer(void)
{
	struct itimerval value, ovalue;

	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 0;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &value, &ovalue);
}

/*
 * configure LED blinking with proper frequency (privatly use only)
 *   on: number of ticks that LED is on
 *   off: number of ticks that LED is off
 *   blinks: number of on/off cycles that LED blinks
 *   rests: number of on/off cycles that LED resting
 *   times: stop blinking after <times> times of blinking
 * where 1 tick == 100 ms
 */
static int gpioLedSet(int gpio, unsigned int on, unsigned int off,
		unsigned int blinks, unsigned int rests, unsigned int times)
{
	int fd;
	ralink_gpio_led_info led;

	//parameters range check
	if (gpio < 0 || gpio >= RALINK_GPIO_NUMBER ||
			on > RALINK_GPIO_LED_INFINITY ||
			off > RALINK_GPIO_LED_INFINITY ||
			blinks > RALINK_GPIO_LED_INFINITY ||
			rests > RALINK_GPIO_LED_INFINITY ||
			times > RALINK_GPIO_LED_INFINITY) {
		return -1;
	}
	led.gpio = gpio;
	led.on = on;
	led.off = off;
	led.blinks = blinks;
	led.rests = rests;
	led.times = times;

	fd = open("/dev/gpio", O_RDONLY);
	if (fd < 0) {
		perror("/dev/gpio");
		return -1;
	}
	if (ioctl(fd, RALINK_GPIO_LED_SET, &led) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

static int ledAlways(int gpio, int on)
{
	if (on)
		return gpioLedSet(gpio, RALINK_GPIO_LED_INFINITY, 0, 1, 1, RALINK_GPIO_LED_INFINITY);
	else
		return gpioLedSet(gpio, 0, RALINK_GPIO_LED_INFINITY, 1, 1, RALINK_GPIO_LED_INFINITY);
}

static int ledWps(int gpio, int mode)
{
	switch (mode) {
		case WPS_LED_RESET:
			//printf("********* LED Reset\n");
			return gpioLedSet(gpio, 0, RALINK_GPIO_LED_INFINITY, 1, 1, RALINK_GPIO_LED_INFINITY);
			break;
		case WPS_LED_PROGRESS:
			//printf("********* LED In Progress\n");
			return gpioLedSet(gpio, 2, 1, RALINK_GPIO_LED_INFINITY, 1, RALINK_GPIO_LED_INFINITY);
			break;
		case WPS_LED_ERROR:
			//printf("********* LED Error\n");
			return gpioLedSet(gpio, 1, 1, RALINK_GPIO_LED_INFINITY, 1, RALINK_GPIO_LED_INFINITY);
			break;
		case WPS_LED_SESSION_OVERLAP:
			//printf("********* LED Overlap\n");
			return gpioLedSet(gpio, 1, 1, 10, 5, RALINK_GPIO_LED_INFINITY);
			break;
		case WPS_LED_SUCCESS:
			//printf("********* LED Success\n");
			gpioLedSet(gpio, 3000, 1, 1, 1, 1);
			break;
	}
	return 0;
}

inline int isProgressGroup(int i)
{
	if(i == 3 || (i>=10 && i<=26))
		return 1;
	return 0;
}

inline int isIdleNotUsedGroup(int i)
{
	if(i == 0 || i == 1)
		return 1;
	return 0;
}


static void timerHandler(int signo)
{
	int rc, WscStatus = 0;
    WSC_CONFIGURED_VALUE WscProfile;

	rc = getWscProfile(wlan_if, &WscProfile, sizeof(WSC_CONFIGURED_VALUE));
	if(rc == -1)
		return;

	WscStatus = getWscStatus(wlan_if);
	if(WscStatus == -1)
		return;

	//printf("WscStatus == %d, %s\n", WscStatus, getWscStatusStr(WscStatus));

	/* Deal with Configure state first*/
	if(CurrentConf == 2 && WscProfile.WscConfigured == 1)
		LedReset();						// user press OOB(reset to default)
	CurrentConf = WscProfile.WscConfigured;

	/* then Deal with WSC Status */
	if(WscStatus != 0 && WscStatus != 1 && WscStatus != 2 && WscStatus != 3 && WscStatus != 34 && WscStatus != 0x109 && !isProgressGroup(WscStatus) )
		return;

	if(WscStatus == CurrentState)
		return;

	if(isIdleNotUsedGroup(WscStatus) && isIdleNotUsedGroup(CurrentState) )
		return;

	if(isProgressGroup(WscStatus) && isProgressGroup(CurrentState))
		return;

	switch(WscStatus){
		case 0: /* Not used*/
		case 1: /* Idle */
			if(CurrentState == 34){
				/* 
				 * Do nothing, let LED light until time's up.
				 */
				;
			}else
				LedReset();
			break;
		
		case 2: /* WSC Fail */
			LedError();
			break;

		case 34: /* Configured */
			LedSuccess();
			break;

		case 0x109: /* Overlap detection */
			LedSessionOverlapDetected();
			break;

		default:
			if(isProgressGroup(WscStatus)){  		/*Start WSC Process" */
				LedInProgress();
				break;
			}
	}
	CurrentState = WscStatus;
}

static int initGpio(void)
{
    int fd;
    ralink_gpio_reg_info info;

    info.pid = getpid();
    info.irq = pbc_gpio;

    fd = open("/dev/gpio", O_RDONLY);
    if (fd < 0) {
        perror("/dev/gpio");
        return -1;
    }
    //set gpio direction to input
    if (ioctl(fd, RALINK_GPIO_SET_DIR_IN, (1<<info.irq)) < 0)
        goto ioctl_err;
    //enable gpio interrupt
    if (ioctl(fd, RALINK_GPIO_ENABLE_INTP) < 0)
        goto ioctl_err;

    //register my information
    if (ioctl(fd, RALINK_GPIO_REG_IRQ, &info) < 0)
        goto ioctl_err;
    close(fd);

	return 0;

ioctl_err:
    perror("ioctl");
    close(fd);
    return -1;
}

static void nvramIrqHandler(int signum)
{
	//printf("iwpriv ra0 set WscMode=2\n");
	system("iwpriv ra0 set WscMode=2");
	//printf("iwpriv ra0 set WscGetConf=1\n");
	system("iwpriv ra0 set WscGetConf=1");
}

int main(int argc, char *argv[])
{
	if(argc >= 2)
		led_gpio = atoi(argv[1]);
	if(!led_gpio)
		led_gpio = WPS_AP_LED_GPIO;

	if(argc >= 3)
		pbc_gpio = atoi(argv[2]);
	if(!pbc_gpio)
		pbc_gpio = WPS_AP_PBC_GPIO;

	if(argc >= 4)
		wlan_if = argv[3];

	initGpio();

	LedReset();

	//signal(SIGXFSZ, WPSSingleTriggerHandler);
	signal(SIGUSR1, nvramIrqHandler);
	setTimer(100000, timerHandler);
	while(1)
		sleep(1);
	return 0;
}

