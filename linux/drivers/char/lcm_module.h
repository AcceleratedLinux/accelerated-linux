#ifndef LCMMODULE_H
#define LCMMODULE_H

#ifdef LCM_DEBUG
#define DPRINT(fmt, args...)    printk(KERN_ERR fmt, ## args)
#else
#define DPRINT(fmt, args...)
#endif

#define LCM_MAJOR	120

/* Default IO port addresses for lpt ports 0, 1 and 2.
 */
#define LPT0_ADDRESS        0x03bc          // corresponds to lpt0: on most PCs
#define LPT1_ADDRESS        0x0378          // corresponds to lpt1: on most PCs
#define LPT2_ADDRESS        0x0278     	     // corresponds to lpt2: on most PCs

/* IO port base address of your parallel port connect to LCD panel.
 */
#define LCD_ADDRESS         LPT1_ADDRESS    // corresponds to lpt port with LCD


/* IO port address for data, status and control bytes of LPT port.
 */
#define LCD_DATA_ADDRESS    LCD_ADDRESS+0   // Data Port at base address + 0
#define LCD_STATUS_ADDRESS  LCD_ADDRESS+1   // Status Port at base address + 1
#define LCD_CONTROL_ADDRESS LCD_ADDRESS+2   // Control Port at base address + 2


/* LCD Display specific information
 */
#define LCD_ROWS        2       // number of rows (lines) displayed
#define LCD_COLS        16      // number of columns (characters) per row
#define LCD_SIZE        (LCD_ROWS * LCD_COLS)   // total chars displayed


#endif  /* end LCMMODULE_H */

