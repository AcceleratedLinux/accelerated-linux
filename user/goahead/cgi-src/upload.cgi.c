#include "upload.cgi.h"

#include "../../netflash/exit_codes.h"


#define REFRESH_TIMEOUT		"40000"		/* 40000 = 40 secs*/

static int netflash_ok = 0;
static void sig_netflash_ok(int sig)
{
	netflash_ok = 1;
}

/*
 * I'm too lazy to use popen() instead of system()....
 * ( note:  static buffer used)
 */
#define DEFAULT_LAN_IP "10.10.10.254"
char *getLanIP(void)
{
	static char buf[64];
	char *nl;
	FILE *fp;

	memset(buf, 0, sizeof(buf));
	if( (fp = popen("nvram_get 2860 lan_ipaddr", "r")) == NULL )
		goto error;

	if(!fgets(buf, sizeof(buf), fp)){
		pclose(fp);
		goto error;
	}

	if(!strlen(buf)){
		pclose(fp);
		goto error;
	}
	pclose(fp);

	if((nl = strchr(buf, '\n')))
		*nl = '\0';

	return buf;

error:
	fprintf(stderr, "warning, cant find lan ip\n");
	return DEFAULT_LAN_IP;
}


void javascriptUpdate(int success)
{
    printf("<script language=\"JavaScript\" type=\"text/javascript\">");
    if(success){
        printf("function update(){ window.location.href = \"/adm/wait_reboot2.asp\";}");
    } else {
        printf("function update(){ parent.menu.setUnderFirmwareUpload(0);}");
    }
    printf("</script>");
}

inline void webFoot(void)
{
    printf("</body></html>\n");
}


int main (int argc, char *argv[])
{
	struct sigaction sa;
	int pid = 0;
	int status;

	/* use sigaction so that waitpid() does not automatically restart */
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sa.sa_handler = sig_netflash_ok;
	sigaction(SIGUSR1, &sa, NULL);

	pid  = vfork();
	if (pid == 0) {
		close(1);
		close(2);
		open("/dev/null", O_RDWR);
		dup(1);
		execl("/bin/netflash", "netflash",
				"cgi://data,params,flash_region", NULL);
		exit(100);
	}
	do {
		waitpid(pid, &status, 0);
	} while (!netflash_ok && !WIFEXITED(status));

    printf(
"\
Server: %s\n\
Pragma: no-cache\n\
Content-type: text/html\n",
getenv("SERVER_SOFTWARE"));

    printf("\n\
<html>\n\
<head>\n\
<TITLE>Upload Firmware</TITLE>\n\
<link rel=stylesheet href=/style/normal_ws.css type=text/css>\n\
<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\n\
</head>\n\
<body onload=\"update()\"> <h1> Upload Firmware</h1>");

	if (netflash_ok) {
		printf("Image is good, programming flash now,  do not turn off<br>");
		printf("Done...rebooting");
		javascriptUpdate(1);
		webFoot();
		exit(0);
	} else {
		switch (WEXITSTATUS(status)) {
		case ALREADY_CURRENT:
			printf("Uploaded firmware is the same as current firmware version.");
			break;
		case VERSION_OLDER:
			printf("Uploaded firmware is older than current firmware version.");
			break;
		case NO_VERSION:
			printf("Uploaded firmware has no version present.");
			break;
		case BAD_LANGUAGE:
			printf("Language is incorrect.");
			break;
		case WRONG_PRODUCT:
			printf("Uploaded firmware is not for this product.");
			break;
		case WRONG_VENDOR:
			printf("Uploaded firmware is not for this vendor.");
			break;
		case BAD_CGI_FORMAT:
			printf("Upload failed, bad format.");
			break;
		case BAD_CGI_DATA:
			printf("Upload failed, bad data.");
			break;
		case BAD_CHECKSUM:
			printf("Provided version has a bad checksum.");
			break;
		case IMAGE_GOOD:
			printf("Image is good");
			break;
		default:
			printf("Unknown error (%d)", WEXITSTATUS(status));
			break;
		}
        javascriptUpdate(0);
		webFoot();
		exit(-1);
	}
}



