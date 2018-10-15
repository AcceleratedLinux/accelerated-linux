#include <linux/version.h>	// need LINUX_VERSION_CODE 
#include <linux/kernel.h>       // kernel work
#include <linux/module.h>       // building a module
#include <linux/fs.h>           // character device definitions
#include <linux/init.h>         // needed for the macros
#include <linux/ioport.h>       // io port access routines
#include <linux/delay.h>        // access to mdelay routine
#include <linux/errno.h>
#include <asm/uaccess.h>        // user access routines (get_user, put_user)
#include <asm/io.h>             // outb and inb prototypes

#include "lcm.h"                // our own ioctl codes (external interface)
#include "lcm_module.h"

MODULE_AUTHOR("Vincente Tsou <vincente@nexcom.com.tw>");
MODULE_DESCRIPTION("NEXCOM LCM Device Driver");
MODULE_LICENSE("GPL");

#define LCM_MODULE_VERSION "LCM V0.2"

//
// Static Global Variables
//
static int lcm_major  = LCM_MAJOR;    // 0 - dynamically assigned, >0 - statically assigned
static int lcm_opened = 0;            // track whether device is opened or not

static int lcm_disp;

//
// Prototypes
//
static void lcm_init_lcd(void);
static void lcm_write_control(unsigned char data);
static void lcm_write_data (unsigned char data);

//mapping from offset value to Address Counter, for 2*16 display
static char offToDDr[32]={0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
                    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f};

/*******************************************************************************
 * Function: lcm_open()
 *******************************************************************************
 *
 * Description: Handler for calls to open LCM LCD device.
 *
 * Input:       Nothing
 *
 * Output:      0 on success; <0 on error
 *
 ******************************************************************************/
int lcm_open(struct inode *minode, struct file *mfile)
{
#if 0
    if (lcm_opened == 1)
    {
        printk(KERN_ERR "LCM: Device already opened\n");
        return (-EBUSY);
    }
#endif

    DPRINT("LCM: device open: file(0x%x)\n", (int)mfile);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
    MOD_INC_USE_COUNT;  // increment module reference count,
                        // in case unload before file is closed
#endif

    lcm_opened = 1;             // lcd being opened, set usage to 1

#if 0
    mfile->f_pos = (loff_t) 0;  // initialize the current file position (0-31)
#endif

    return (0);
}
/* End of lcm_open() */


/*******************************************************************************
 * Function: lcm_release()
 *******************************************************************************
 *
 * Description: Handler for calls to close LCM LCD device.
 *
 * Input:       Nothing
 *
 * Output:      0 on success; <0 on error
 *
 ******************************************************************************/
int lcm_release(struct inode *minode, struct file *mfile)
{
    DPRINT("LCM: device release: file(0x%x)\n", (int)mfile);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
    MOD_DEC_USE_COUNT;  // decrement modules reference count in minode
#endif

    lcm_opened = 0; // lcd being closed, set usage to 0

    return (0);
}
/* End of lcm_release() */



/*******************************************************************************
 * Function: lcm_llseek()
 *******************************************************************************
 *
 * Description: Handler for seek operations on LCM LCD devices.
 *
 * Input:       *mfile - pointer to file (device object) for seek operation
 *              offset - the offset value to modify the current position
 *              whence - SEEK_SET, SEEK_CUR, SEEK_END.
 *                       indicate which how to use the offset to change the
 *                       the current positon.
 *
 * Output:      the position that being changed to.
 *
 ******************************************************************************/

/*To place the cursor at the requested location, offset is from 0 to 31
 * filp->f_pos will keep track of the current DDRAM address, so need to update the value when
 * 1. write
 * 2. clear display
 * 3. return home
 *
*/
loff_t lcm_llseek(struct file *mfile, loff_t offset, int whence)
{
    loff_t newpos;
    int ddram_addr;

    DPRINT("LCM: device llseek: file(0x%x)\n", (int)mfile);

    if ((mfile->f_pos < 0) || (mfile->f_pos > LCD_SIZE))
        {
            DPRINT("LCM: device llseek: file position out of range (0x%x)\n", (int)mfile->f_pos);
            return (-EINVAL);
        }

    switch(whence)
    {
        case 0: /* SEEK_SET */
            newpos = offset;
            break;
        case 1: /* SEEK_CUR */
            newpos = mfile->f_pos + offset;
            break;
        case 2: /* SEEK_END */
            newpos = LCD_SIZE - offset;  // max size - offset
            break;
        default: /* can't happen */
            return (-EINVAL);
    }

    if ((newpos < 0) || (newpos > LCD_SIZE))
    {
        DPRINT("LCM: device llseek: new position out of range (0x%x)\n", (int)newpos);
        return (-EINVAL);
    }

    mfile->f_pos = newpos;

    //map the corresponding Address Counter
    ddram_addr = offToDDr[newpos];

    lcm_write_control(0x80 | ddram_addr);   // 0x80 - set ddram address cmd

    DPRINT("LCM: device llseek: new position (0x%x)\n", (int)newpos);

    return (newpos);
}


/*******************************************************************************
 * Function: lcm_write()
 *******************************************************************************
 *
 * Description: Handler for all write operations on LCM LCD devices.
 *
 * Input:       *mfile - pointer to file (device object) for write operation
 *              *gdata - pointer to data passed from client to driver
 *              length - number of bytes in gdata buffer
 *              *f_pos - pointer to where current f_pos is stored
 *
 * Output:      bytes written on success; <0 on error
 *
 * note: llseek needs to used to position the location first
 ******************************************************************************/

#define	LCD_COLMASK	(LCD_COLS - 1)

ssize_t lcm_write(struct file *mfile, const char *gdata, size_t length, loff_t *f_pos)
{
    char c = 0;
    int i, j;

    DPRINT("LCM: device write: file(0x%x) gdata(0x%x) len(%d) offset(0x%x)\n",
            (int)mfile, (int)gdata, length, (int) *f_pos);

    if (((*f_pos) < 0) || ((*f_pos) >= LCD_SIZE))
	(*f_pos) = 0;

    /* Start processing bytes at current location
     */
    for (i = 0; i < length; i++)
    {
        /*Set real Address Counter to the current file position before write data */
        lcm_write_control(0x80 | offToDDr[*f_pos]);

        get_user(c, gdata+i);
        DPRINT("LCM: device write: get_user (%c)\n", c);

	switch (c) {
	case 0x1 /*CTRL-A*/:
		/* toggle cursor on/off */
		lcm_disp ^= 0x2;
    		lcm_write_control(lcm_disp);
		break;
	case 0x2 /*CTRL-B*/:
		/* toggle cursor blinking */
		lcm_disp ^= 0x1;
    		lcm_write_control(lcm_disp);
		break;
	case 0x3 /*CTRL-C*/:
		/* home cursors position, no screen clear */
		lcm_write_control(0x2);
		udelay(1600);
		(*f_pos) = 0;
		break;
	case 0x5 /*CTRL-E*/:
		/* clear to end of line */
		for (j = ((*f_pos) & LCD_COLMASK); (j < LCD_COLS); j++)
			lcm_write_data(' ');
		break;
	case 0x6 /*CTRL-F*/:
		/* advance cursor one position */
		(*f_pos)++;
		break;
	case '\b' /*CTRL-H*/:
		/* backspace one position */
		if ((*f_pos) & LCD_COLMASK)
			(*f_pos)--;
		break;
	case '\t' /*CTRL-I*/:
		/* tab forward to tabstop */
		do {
			lcm_write_data(' ');
			(*f_pos)++;
		} while ((*f_pos) % 8);
		break;
	case '\n' /*CTRL-J*/:
		/* new line */
		for (j = ((*f_pos) & LCD_COLMASK); (j < LCD_COLS); j++) {
			lcm_write_data(' ');
			(*f_pos)++;
		}
		break;	
	case '\v' /*CTRL-K*/:
		/* vertical tab (change lines!) */
		(*f_pos) ^= LCD_COLS;
		break;
	case '\f' /*CTRL-L*/:
		/* clear and home */
		lcm_write_control(0x1);
		udelay(1600);
		(*f_pos) = 0;
		break;
	case '\r' /*CTRL-M*/:
		/* return to start of current line */
		(*f_pos) &= ~LCD_COLMASK;
		break;
	default:
        	lcm_write_data(c);
        	(*f_pos)++;
		break;
	}

	/* keep position within boundary of LCD display */
	if (((*f_pos) < 0) || ((*f_pos) >= LCD_SIZE))
		(*f_pos) = 0;

    }

    DPRINT("LCM: device write: current file position (0x%x)\n", (int) *f_pos);
    DPRINT("LCM: device write: current file position address (0x%x)\n", (int) offToDDr[*f_pos]);
    return (length);        // return number of bytes written
}
/* End of lcm_write() */


/*******************************************************************************
 * Function: lcm_read()
 *******************************************************************************
 *
 * Description: Handler for all read operations on LCM LCD devices.
 *
 * Input:       *mfile - pointer to file (device object) for read operation
 *              *gdata - pointer to client's data buffer to put data into
 *              length - number of bytes to put into gdata buffer
 *              *f_pos - position within data stream
 *
 * Output:      bytes read on success; <0 on error
 *
 ******************************************************************************/
ssize_t lcm_read(struct file *mfile, char *gdata, size_t length, loff_t *f_pos)
{
    DPRINT("LCM: device read: file(0x%x) to write-only device\n", (int)mfile);

    return (-EACCES);
}
/* End of lcm_read() */


/*******************************************************************************
 * Function: lcm_ioctl()
 *******************************************************************************
 *
 * Description: Handler for all ioctl operations on LCM LCD devices.
 *
 * Input:       *minode - pointer to inode for module
 *              *mfile - pointer to file (device object) for read operation
 *              ioctl_num - number of ioctl operation being requested
 *              ioctl_param - parameter for ioctl operation (can be anything)
 *
 * Output:      > 0  on success; <0 on error
 *
 ******************************************************************************/
long lcm_ioctl(struct file *mfile, unsigned int cmd, unsigned long ioctl_param)
{
    unsigned char c;

    DPRINT("LCM: device ioctl: file(0x%x) ioctl(0x%x) param(0x%x)\n",
            (int)mfile, cmd, (int)ioctl_param);

    switch(cmd){
        case IOCTL_CLEAR_DISPLAY:
            //clear display - write 0x20 to DDRAM and set its address to AC
            lcm_write_control(0x01);
            //move current position to the head
            mfile->f_pos = 0;
            break;

	case IOCTL_KEYPAD_ACTION:
	    c = inb(LCD_STATUS_ADDRESS);
	    return c;

        default:
            return -ENOTTY; //no such ioctl for device
    }

    DPRINT("LCM: device ioctl: current position (0x%x)\n", (int) mfile->f_pos);

    return (0);
}
/* End of lcm_ioctl() */



static struct file_operations lcm_fops =
{
    llseek:		lcm_llseek,
    read:		lcm_read,      // read
    write:		lcm_write,     // write
    unlocked_ioctl:	lcm_ioctl,     // ioctl
    open:		lcm_open,      // open
    release:		lcm_release    // release
};

/*******************************************************************************
 * Function: init_module()
 *******************************************************************************
 *
 * Description: Initialization routine called by linux OS when driver is
 *              loaded. Setups up driver callback functions.
 *
 * Input:       Nothing
 *
 * Output:      0 on success; <0 on error
 *
 ******************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
int init_module(void)
#else
static int __init lcm_init_module(void)
#endif
{
    int result;

    DPRINT("LCM: init module for %s driver\n", LCM_MODULE_VERSION);
    /* Register driver with OS. Pass lcm_major device number of devices
     * this drive manages. If lcm_major is 0, a major num will dynamically
     * be assigned to driver and returned in result. Register struct
     * of callback functions too.
     */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
    SET_MODULE_OWNER(&lcm_fops);
#endif

    /*if major number is 0(dynamically assign), return the requested major#
     *if major > 0(predefined), return 0 when success*/
    result = register_chrdev(lcm_major, LCM_DEVICE_NAME, &lcm_fops);
    if (result < 0)
    {
        printk(KERN_ERR "LCM: Unable to get major device %d for LCM device\n",
                lcm_major);

        return (-EBUSY);
    }
    lcm_major = (lcm_major != 0) ? lcm_major : result;

    lcm_opened = 0;

    lcm_init_lcd();     // init lcd at module load time

    return (0);
}
/* End of init_module() */


/*******************************************************************************
 * Function: cleanup_module()
 *******************************************************************************
 *
 * Description: Unload routine called by linux OS when module is unloaded.
 *              Cleans up all allocated resources and such.
 *
 * Input:       Nothing
 *
 * Output:      Nothing
 *
 ******************************************************************************/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
void cleanup_module(void)
#else
static void __exit lcm_cleanup_module(void)
#endif
{
    DPRINT("LCM: cleanup module for %s driver\n", LCM_MODULE_VERSION);

    unregister_chrdev(lcm_major, LCM_DEVICE_NAME);
}
/* End of cleanup_module() */





/******************************************************************************
 * Low level LCD routines
 *****************************************************************************/


/*
** lcm_init_lcd: Initialize the LCD.
*/
static void lcm_init_lcd(void)
{
#if 0
    lcm_write_control(0x38);//function set- 5*8, 2 lines, 8 bits
    lcm_write_control(0x38);
    lcm_write_control(0x38);

    lcm_write_control(0x01);//clear display
    lcm_write_control(0x06);//entry mode- increment, NO display shit
    lcm_write_control(0x0c);//0x1100, display on, cursor off, blanking off
#endif

    lcm_write_control(0x30);
    mdelay(5);               // delay >4.1ms
    lcm_write_control(0x30);
    mdelay(1);               // delay >100us
    lcm_write_control(0x30);

    lcm_write_control(0x3e); // 0x1110, 8 bits xfer, 2 lines and 5x10 dots

    //lcm_write_control(0x08); // display off
    lcm_write_control(0x01); // clear display
    lcm_write_control(0x06); // entry mode - increment on, no display shift
    lcm_disp = 0xc; // display on, cursor off, blanking off
    lcm_write_control(lcm_disp);
    lcm_write_control(0x02); // return cursor home (from sample code)
}


/* Mapping of Control bits to LCD device
 *
 *  bit 7: N/A
 *  bit 6: N/A
 *  bit 5: N/A
 *  bit 4: N/A
 *  bit 3: LCD RS (HW INV)  0 -> DATA, 1 -> CMD
 *  bit 2: LCD R/W  0 -> WRITE, 1 -> READ
 *  bit 1: LCD Enable (HW INV) 0 -> HIGH, 1 -> LO
 *  bit 0: N/A
 */

/*
** lcm_write_control: Write one byte control instruction to the LCD
*/
static void lcm_write_control(unsigned char cmd)
{
    outb(0x08, LCD_CONTROL_ADDRESS); //command mode, RS = 1
    udelay(600);

    outb(cmd, LCD_DATA_ADDRESS);
    udelay(600);

    outb(0x08, LCD_CONTROL_ADDRESS); //cmmand mode plus the E -> 0 (hi)
    udelay(600);

    outb(0x0A, LCD_CONTROL_ADDRESS); //command mode but E -> 1 (lo)
    udelay(600);

 #if 0
    outb(0x0A, LCD_CONTROL_ADDRESS);    // CMD, WRITE, E_LO, 0000 1010
    udelay(500);
    outb(cmd, LCD_DATA_ADDRESS);
    udelay(500);
    outb(0x08, LCD_CONTROL_ADDRESS);    // CMD, WRITE, E_HI  0000 1000
    udelay(500);
    outb(0x0A, LCD_CONTROL_ADDRESS);    // CMD, WRITE, E_LO
    udelay(500);
#endif

#if 0
    outb(0x03, LCD_CONTROL_ADDRESS);    // RS=0, R/W=0, E=0

    outb(data, LCD_DATA_ADDRESS);

    outb(0x02, LCD_CONTROL_ADDRESS);    // RS=0, R/W=0, E=1
    outb(0x03, LCD_CONTROL_ADDRESS);    // RS=0, R/W=0, E=0
    outb(0x01, LCD_CONTROL_ADDRESS);    // RS=0, R/W=1, E=0

    mdelay(10);         // 10ms delay
#endif

}


/*
** lcm_write_data: Write one byte of data to the LCD
*/
static void lcm_write_data (unsigned char data)
{

    outb(0x02, LCD_CONTROL_ADDRESS); //data mode, E -> 1 (lo)
    udelay(600);

    outb(data, LCD_DATA_ADDRESS);
    udelay(600);

    outb(0x00, LCD_CONTROL_ADDRESS); //cmmand mode plus the E -> 0 (hi)
    udelay(600);

    outb(0x02, LCD_CONTROL_ADDRESS); //command mode but E -> 1 (lo)
    udelay(600);


#if 0
    outb(0x02, LCD_CONTROL_ADDRESS);    // DATA, WRITE, E_LO  0000 0010
    outb(data, LCD_DATA_ADDRESS);
    outb(0x00, LCD_CONTROL_ADDRESS);    // DATA, WRITE, E_HI  0000 0000
    outb(0x02, LCD_CONTROL_ADDRESS);    // DATA, WRITE, E_LO
#endif

#if 0
    outb(0x07, LCD_CONTROL_ADDRESS);    // RS=1, R/W=0, E=0

    outb(data, LCD_DATA_ADDRESS);

    outb(0x06, LCD_CONTROL_ADDRESS);    // RS=1, R/W=0, E=1
    outb(0x07, LCD_CONTROL_ADDRESS);    // RS=1, R/W=0, E=0
    outb(0x05, LCD_CONTROL_ADDRESS);    // RS=1, R/W=1, E=0

#endif

    //mdelay(1);          // 1ms delay
}


#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0)
module_init(lcm_init_module);
module_exit(lcm_cleanup_module);
#endif
