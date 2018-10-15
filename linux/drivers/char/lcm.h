/*  lcm.h - the header file with the ioctl definitions.
 *
 *  The declarations here have to be in a header file, because
 *  they need to be known both to the kernel module
 *  (in lcm_module.c) and any processes sending ioctls to the
 *  lcm driver.
 */

#ifndef LCM_H
#define LCM_H

#include <linux/ioctl.h>


/* The name of the device
 */
#define LCM_DEVICE_NAME     "lcm"

/*
 * IOCTL Codes
 */
#define LCM_MAGIC_NUM       0x95
#define KPAD_MAGIC_NUM       0x96

/*write from application,
    #define _IOW(type,nr,size)      _IOC(_IOC_WRITE,(type),(nr),sizeof(size))
*/
#define IOCTL_CLEAR_DISPLAY _IOW(LCM_MAGIC_NUM, 1, int)
#define IOCTL_KEYPAD_ACTION _IOW(KPAD_MAGIC_NUM, 1, int)

#endif  /* end LCM_H */

