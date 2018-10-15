#ifndef __MANAGEMENT_H__
#define __MANAGEMENT_H__

#define PROC_IF_STATISTIC	"/proc/net/dev"
#define PROC_MEM_STATISTIC	"/proc/meminfo"


#define TXBYTE		0
#define TXPACKET	1
#define RXBYTE		2
#define RXPACKET	3

//#define WIFI_STATUS "iwpriv ra0 stat"

#define SYSTEM_COMMAND_LOG "/var/system_command.log"

void formDefineManagement(void);
void management_init(void);
char *setNthValueLong(int index, char *old_values, char *new_value);
#endif
