/*
 * Copyright 2005-2006 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*!
 * @file fs453.c
 * @brief Driver for FS453/4 TV encoder
 *
 * @ingroup Framebuffer
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/ioctl.h>
#include <linux/video_encoder.h>
#include <asm/uaccess.h>

#include "fs453.h"
#define DEBUGP(...) 

#if defined(CONFIG_UC53281EVM_1_1) 
extern int i2c_coldfire_gpio_setup(int address);
#endif
extern struct mutex i2c_sem;

/*
 * VGA mode is not defined by video_encoder.h
 * while FS453 supports VGA output.
 */
#ifndef VIDEO_ENCODER_VGA
#define VIDEO_ENCODER_VGA	32
#endif


/*!
 * This stucture contains the status of FS453.
 */
struct fs453_data {
	int norm;
	int input;
	int output;
	int enable;
};

/*!
 * This structure contains all the register values needed to program the
 * TV encoder chip.  This structure is instantiated and initialized for
 * each supported output standard.
 */
struct fs453_presets {
	u32 mode;		/*! Video mode */
	u16 qpr;		/*! Quick Program Register */
	u16 pwr_mgmt;		/*! Power Management */
	u16 iho;		/*! Input Horizontal Offset */
	u16 ivo;		/*! Input Vertical Offset */
	u16 ihw;		/*! Input Horizontal Width */
	u16 vsc;		/*! Vertical Scaling Coefficient */
	u16 hsc;		/*! Horizontal Scaling Coefficient */
	u16 bypass;		/*! Bypass */
	u16 misc;		/*! Miscellaneous Bits Register */
	u8 misc46;		/*! Miscellaneous Bits Register 46 */
	u8 misc47;		/*! Miscellaneous Bits Register 47 */
	u32 ncon;		/*! Numerator of NCO Word */
	u32 ncod;		/*! Denominator of NCO Word */
	u16 pllm;		/*! PLL M and Pump Control */
	u16 plln;		/*! PLL N */
	u16 pllpd;		/*! PLL Post-Divider */
	u16 vid_cntl0;		/*! Video Control 0 */
	u16 dac_cntl;		/*! DAC Control */
	u16 fifo_lat;		/*! FIFO Latency */
};

static struct fs453_presets fs453_vga_presets = {
	.mode = VIDEO_ENCODER_VGA,
        .iho = 0x0002,
        .ivo = 0x0000,
#ifdef CONFIG_LCD_640x480
        .ihw = 0x0280,
#elif defined CONFIG_LCD_320x240 
        .ihw = 0x0140,
#endif
	.qpr = 0x9d70,
	.pwr_mgmt = 0x0408,
	.misc = 0x0103,
	.ncon = 0x00000000,
	.ncod = 0x00000000,
	.misc46 = 0xa9,
	.misc47 = 0x00,
	.pllm = 0x317f,
	.plln = 0x008e,
	.pllpd = 0x0202,
	.vid_cntl0 = 0x3816,  /* 4006,*/
	.dac_cntl = 0x00e1,
	.fifo_lat = 0x0082,
};

static struct fs453_presets fs453_ntsc_presets = {
	.mode = VIDEO_ENCODER_NTSC,
	.qpr = 0x9c48,
	.pwr_mgmt = 0x0200,
	.misc = 0x0103,
	.ncon = 0x00000001,
	.ncod = 0x00000001,
	.misc46 = 0x01,
	.misc47 = 0x00,
	.pllm = 0x4000 | (296 - 17),
	.plln = 30 - 1,
	.pllpd = ((10 - 1) << 8) | (10 - 1),
	.iho = 0,
	.ivo = 40,
	.ihw = 768,
	.vsc = 789,
	.hsc = 0x0000,
	.bypass = 0x000a,
	.vid_cntl0 = 0x0340,
	.dac_cntl = 0x00e4,
	.fifo_lat = 0x0082,
};

static struct fs453_presets fs453_pal_presets = {
	.mode = VIDEO_ENCODER_PAL,
	.qpr = 0x9c41,
	.pwr_mgmt = 0x0200,
	.misc = 0x0103,
	.ncon = 0x00000001,
	.ncod = 0x00000001,
	.misc46 = 0x01,
	.misc47 = 0x00,
	.pllm = 0x4000 | (296 - 17),
	.plln = 30 - 1,
	.pllpd = ((10 - 1) << 8) | (10 - 1),
	.iho = 0,
	.ivo = 19,
	.ihw = 768,
	.vsc = 8200,
	.hsc = 0x0000,
	.bypass = 0x000a,
	.vid_cntl0 = 0x0340,
	.dac_cntl = 0x00e4,
	.fifo_lat = 0x0082,
};

static int fs453_preset(struct i2c_client *client,
			struct fs453_presets *presets);
static int fs453_enable(struct i2c_client *client, int enable);

static int fs453_write(struct i2c_client *client, u8 reg, u32 value, u32 len);
static int fs453_read(struct i2c_client *client, u8 reg, u32 * value, u32 len);

static struct i2c_driver fs453_driver;
/*
 * FIXME: fs453_client will represent the first FS453 device found by
 * the I2C subsystem, which means fs453_ioctl() always works on the
 * first FS453 device.
 */
static struct i2c_client *fs453_client = 0;

static int fs453_command(struct i2c_client *client, unsigned int cmd, void *arg)
{
	int val;
	char *smode = 0;
	struct video_encoder_capability *cap;
	struct fs453_data *data = i2c_get_clientdata(client);
	int ret = 0;

	pr_debug("FS453: cmd=0x%02x\n", cmd);

	switch (cmd) {
	case ENCODER_GET_CAPABILITIES:
		cap = arg;
		cap->flags =
		    VIDEO_ENCODER_PAL | VIDEO_ENCODER_NTSC | VIDEO_ENCODER_VGA;
		cap->inputs = 1;
		cap->outputs = 1;
		break;
	case ENCODER_SET_NORM:
		val = *(int *)arg;
		switch (val) {
		case VIDEO_ENCODER_PAL:
			ret = fs453_preset(client, &fs453_pal_presets);
			smode = "PAL";
			break;
		case VIDEO_ENCODER_NTSC:
			ret = fs453_preset(client, &fs453_ntsc_presets);
			smode = "NTSC";
			break;
		case VIDEO_ENCODER_VGA:
			ret = fs453_preset(client, &fs453_vga_presets);
			smode = "VGA";
			break;
		default:
			ret = -EINVAL;
			break;
		}
		if (!ret) {
			data->norm = val;
			data->enable = 1;
			pr_debug("FS453: switched to %s\n", smode);
		}
		break;
	case ENCODER_SET_INPUT:
		val = *(int *)arg;
		/* We have only one input */
		if (val != 0)
			return -EINVAL;
		data->input = val;
		break;
	case ENCODER_SET_OUTPUT:
		val = *(int *)arg;
		/* We have only one output */
		if (val != 0)
			return -EINVAL;
		data->output = val;
		break;
	case ENCODER_ENABLE_OUTPUT:
		val = *(int *)arg;
		if ((ret = fs453_enable(client, val)) == 0)
			data->enable = val;
		break;
	case ENCODER_SET_REG:
		{   struct ve_set_reg tmp_val ;
                    copy_from_user(&tmp_val, (void *)arg, sizeof(tmp_val));

		   DEBUGP("reg=[0x%04x] val=[0x%x] size=[%d]\n", tmp_val.reg, tmp_val.val, tmp_val.size);

		   if(mutex_lock_interruptible(&i2c_sem))
		      return -ERESTARTSYS;
#if defined(CONFIG_UC53281EVM_1_1) 
	           i2c_coldfire_gpio_setup(FS453_I2C_ADDR);
#endif

		   fs453_write(client, tmp_val.reg, tmp_val.val, tmp_val.size);
	           fs453_read(client, tmp_val.reg, &val, tmp_val.size);

#if defined(CONFIG_UC53281EVM_1_1) 
		   i2c_coldfire_gpio_setup(0);
#endif
		   mutex_unlock(&i2c_sem);

	           DEBUGP("0x%04x: 0x%08x\n", tmp_val.reg, val);
                }
		break;
	default:
		return -EINVAL;
	}

	return ret;
}

static int i2c_fs453_detect_client(struct i2c_adapter *adapter, int address,
				   int kind)
{
	int chip_id;
	struct i2c_client *client;
	struct fs453_data *data;
	const char *client_name = "FS453 I2C dev";

	DEBUGP("FS453: i2c-bus: %s; address: 0x%x\n", adapter->name, address);

	/* Let's see whether this adapter can support what we need */
	if (!i2c_check_functionality(adapter,
				     I2C_FUNC_SMBUS_WORD_DATA |
				     I2C_FUNC_SMBUS_BYTE_DATA)) {
		DEBUGP("FS453: SMBUS word/byte operations not permited.\n");
		return 0;
	}

	client =
	    kmalloc(sizeof(struct i2c_client) + sizeof(struct fs453_data),
		    GFP_KERNEL);
	if (!client)
		return -ENOMEM;

	data = (struct fs453_data *)(client + 1);
	client->addr = address;
	client->adapter = adapter;
	client->driver = &fs453_driver;
	client->flags = 0;

	/*
	 * The generic detection, that is skipped if any force
	 * parameter was used.
	 */

	if (kind < 0) {
		if(mutex_lock_interruptible(&i2c_sem))
			return -ERESTARTSYS;
#if defined(CONFIG_UC53281EVM_1_1) 
		i2c_coldfire_gpio_setup(FS453_I2C_ADDR);
#endif
		chip_id = i2c_smbus_read_word_data(client, FS453_ID);

#if defined(CONFIG_UC53281EVM_1_1) 
		i2c_coldfire_gpio_setup(0);
#endif
		mutex_unlock(&i2c_sem);

		if (chip_id != FS453_CHIP_ID) {
			pr_info("FS453: TV encoder not present, ID=0x%x\n", chip_id);
			kfree(client);
			return 0;
		} else
			pr_info("FS453: TV encoder present, ID=0x%04X\n",
				chip_id);
	}
	strcpy(client->name, client_name);

	/* FS453 default status */
	data->input = 0;
	data->output = 0;
	data->norm = 0;
	data->enable = 0;
	i2c_set_clientdata(client, data);

        pr_debug("%s: client=[%p]\n", __FUNCTION__, client);

	fs453_preset(client, &fs453_vga_presets);
	fs453_enable(client, 1);

	fs453_client = client;

	return 0;
}

static unsigned short normal_i2c[] = { FS453_I2C_ADDR, I2C_CLIENT_END };

/* Magic definition of all other variables and things */
I2C_CLIENT_INSMOD;

static int i2c_fs453_attach(struct i2c_adapter *adap)
{
	return i2c_probe(adap, &addr_data, i2c_fs453_detect_client);
}

static int i2c_fs453_detach(struct i2c_client *client)
{
	int err;

	pr_debug("FS453: i2c_detach_client()\n");

	if ((err = i2c_detach_client(client))) {
		pr_debug("FS453: i2c_detach_client() failed\n");
		return err;
	}

	if (fs453_client == client)
		fs453_client = 0;

	kfree(client);
	return 0;
}

static struct i2c_driver fs453_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = "FS453 driver",
		   },
	.attach_adapter = &i2c_fs453_attach,
	.detach_client = &i2c_fs453_detach,
	.command = fs453_command,
};

/*!
 * @brief Function to read TV encoder registers on the i2c bus
 * @param     client	I2C client structure
 * @param     reg	The register number
 * @param     value	Pointer to buffer to receive the read data
 * @param     len	Number of 16-bit register words to read
 * @return    0 on success, others on failure
 */
static int fs453_read(struct i2c_client *client, u8 reg, u32 * value, u32 len)
{
	int ret = 0;

	if (len == 1)
		*value = i2c_smbus_read_byte_data(client, reg);
	else if (len == 2)
		*value = i2c_smbus_read_word_data(client, reg);
	else if (len == 4) {
		*(u16 *) value = i2c_smbus_read_word_data(client, reg);
		*((u16 *) value + 1) =
		    i2c_smbus_read_word_data(client, reg + 2);
	} else
		ret = -EINVAL;

	return ret;
}

/*!
 * @brief Function to write a TV encoder register on the i2c bus
 * @param     client	I2C client structure
 * @param     reg	The register number
 * @param     value	The value to write
 * @param     len	Number of words to write (must be 1)
 * @return    0 on success, others on failure
 */
static int fs453_write(struct i2c_client *client, u8 reg, u32 value, u32 len)
{
	int ret = -EINVAL;
	DEBUGP("Entered reg-[%d] val-[0x%x], len=[%u]\n", reg, value, len);
	
	if (len == 1) {
		ret = i2c_smbus_write_byte_data(client, reg, (u8) value);
	}
	else if (len == 2){
		ret = i2c_smbus_write_word_data(client, reg, (u16) value);
	}
	else if (len == 4){
		ret = i2c_smbus_write_block_data(client, reg, len,
						  (u8 *) & value);
	}

	return ret;
}

/*!
 * @brief Function to initialize the TV encoder
 * @param     client	I2C client structure
 * @param     presets	FS453 pre-defined register values
 * @return    0 on success; ENODEV if the encoder wasn't found
 */
static int fs453_preset(struct i2c_client *client,
			struct fs453_presets *presets)
{
	u32 data;

	if (!client)
		return -ENODEV;

	if(mutex_lock_interruptible(&i2c_sem))
		return -ERESTARTSYS;
#if defined(CONFIG_UC53281EVM_1_1) 
	i2c_coldfire_gpio_setup(FS453_I2C_ADDR);
#endif

	/* set the clock level */
	fs453_write(client, FS453_CR, CR_GCC_CK_LVL, 2);

	/* soft reset the encoder */
	fs453_read(client, FS453_CR, &data, 2);
	fs453_write(client, FS453_CR, data | CR_SRESET, 2);
	fs453_write(client, FS453_CR, data & ~CR_SRESET, 2);


	/* customize */
	fs453_write(client, FS453_PWR_MGNT, presets->pwr_mgmt, 2);

	if (presets->mode != VIDEO_ENCODER_VGA) {
		/* TBD: set up the NCO and PLL */
	}

	fs453_write(client, FS453_IHO, presets->iho, 2);
	fs453_write(client, FS453_IVO, presets->ivo, 2);
	fs453_write(client, FS453_IHW, presets->ihw, 2);
	fs453_write(client, FS453_VSC, presets->vsc, 2);
	fs453_write(client, FS453_HSC, presets->hsc, 2);
	fs453_write(client, FS453_BYPASS, presets->bypass, 2);
	fs453_write(client, FS453_NCON, presets->ncon, 4);
	fs453_write(client, FS453_NCOD, presets->ncod, 4);
	fs453_write(client, FS453_PLL_M_PUMP, presets->pllm, 2);
	fs453_write(client, FS453_PLL_N, presets->plln, 2);
	fs453_write(client, FS453_PLL_PDIV, presets->pllpd, 2);
	fs453_write(client, FS453_MISC_46, presets->misc46, 1);
	fs453_write(client, FS453_MISC_47, presets->misc47, 1);

	/* latch the NCO and PLL settings */
	fs453_read(client, FS453_CR, &data, 2);
	fs453_write(client, FS453_CR, data | CR_NCO_EN, 2);

	if(presets->mode == VIDEO_ENCODER_VGA)
		fs453_write(client, FS453_CR, (data & ~CR_NCO_EN) | 0x2000, 2);
	else
		fs453_write(client, FS453_CR, data & ~CR_NCO_EN, 2);

	/* Write the QPR (Quick Programming Register). */
	fs453_write(client, FS453_QPR, presets->qpr, 2);

	fs453_write(client, FS453_MISC, presets->misc, 2);
	fs453_write(client, FS453_VID_CNTL0, presets->vid_cntl0, 2);

	if(presets->mode == VIDEO_ENCODER_VGA) {

		fs453_write(client, FS453_RED_SCL, 0xf0, 2);
		fs453_write(client, FS453_GRN_SCL, 0xf0, 2);
		fs453_write(client, FS453_BLU_SCL, 0xf0, 2);
		fs453_write(client, FS453_IHW, presets->ihw, 2);
	}

	fs453_write(client, FS453_DAC_CNTL, presets->dac_cntl, 2);
	fs453_write(client, FS453_FIFO_LAT, presets->fifo_lat, 2);

#if defined(CONFIG_UC53281EVM_1_1) 
	i2c_coldfire_gpio_setup(0);
#endif
	mutex_unlock(&i2c_sem);

	return 0;
}

#define FS453_DUMP_BYTE(val)                                         \
do {                                                            \
        u32 data;                                               \
                                                                \
        fs453_read(fs453_client, val, &data, 1);                \
        printk("%s(%02x)\t = 0x%08X\t%d\n", #val, val, data, data);        \
} while (0)

#define FS453_DUMP(val)                                         \
do {                                                            \
        u32 data;                                               \
                                                                \
        fs453_read(fs453_client, val, &data, 2);                \
        printk("%s(%02x)\t = 0x%08X\t%d\n", #val, val, data, data);        \
} while (0)

#define FS453_DUMP_LONG(val)                                         \
do {                                                            \
        u32 data;                                               \
                                                                \
        fs453_read(fs453_client, val, &data, 4);                \
        printk("%s(%02x)\t = 0x%08X\t%d\n", #val, val, data, data);        \
} while (0)

void fs453_dump(void)
{

        if(!fs453_client) return;

	if(mutex_lock_interruptible(&i2c_sem))
		return ;
#if defined(CONFIG_UC53281EVM_1_1) 
	i2c_coldfire_gpio_setup(FS453_I2C_ADDR);
#endif

	FS453_DUMP(FS453_IHO);
	FS453_DUMP(FS453_IVO);
	FS453_DUMP(FS453_IHW);
	FS453_DUMP(FS453_VSC);
	FS453_DUMP(FS453_HSC);
	FS453_DUMP(FS453_BYPASS);
	FS453_DUMP(FS453_CR);
	FS453_DUMP(FS453_MISC);
	FS453_DUMP_LONG(FS453_NCON);
	FS453_DUMP_LONG(FS453_NCOD);
	FS453_DUMP(FS453_PLL_M_PUMP);
	FS453_DUMP(FS453_PLL_N);
	FS453_DUMP(FS453_PLL_PDIV);
	FS453_DUMP(FS453_SHP);
	FS453_DUMP(FS453_FLK);
	FS453_DUMP(FS453_GPIO);
	FS453_DUMP(FS453_ID);
	FS453_DUMP(FS453_STATUS);
	FS453_DUMP(FS453_FIFO_SP);
	FS453_DUMP(FS453_FIFO_LAT);
	FS453_DUMP_LONG(FS453_CHR_FREQ );
	FS453_DUMP_BYTE(FS453_CHR_PHASE);
	FS453_DUMP_BYTE(FS453_MISC_45 );
	FS453_DUMP_BYTE(FS453_MISC_46 );
	FS453_DUMP_BYTE(FS453_MISC_47 );
	FS453_DUMP_BYTE(FS453_HSYNC_WID);
	FS453_DUMP_BYTE(FS453_BURST_WID );
	FS453_DUMP_BYTE(FS453_BPORCH    );
	FS453_DUMP_BYTE(FS453_CB_BURST  );
	FS453_DUMP_BYTE(FS453_CR_BURST  );
	FS453_DUMP_BYTE(FS453_MISC_4D   );
	FS453_DUMP(FS453_BLACK_LVL );
	FS453_DUMP(FS453_BLANK_LVL );
	FS453_DUMP(FS453_NUM_LINES );
	FS453_DUMP(FS453_WHITE_LVL );
	FS453_DUMP_BYTE(FS453_CB_GAIN   );
	FS453_DUMP_BYTE(FS453_CR_GAIN   );
	FS453_DUMP_BYTE(FS453_TINT      );
	FS453_DUMP_BYTE(FS453_BR_WAY    );
	FS453_DUMP_BYTE(FS453_FR_PORCH  );
	FS453_DUMP(FS453_NUM_PIXELS);
	FS453_DUMP_BYTE(FS453_1ST_LINE  );
	FS453_DUMP_BYTE(FS453_MISC_74   );
	FS453_DUMP_BYTE(FS453_SYNC_LVL  );
	FS453_DUMP(FS453_VBI_BL_LVL);
	FS453_DUMP_BYTE(FS453_SOFT_RST  );
	FS453_DUMP_BYTE(FS453_ENC_VER   );
	FS453_DUMP_BYTE(FS453_WSS_CONFIG);
	FS453_DUMP(FS453_WSS_CLK   );
	/*FS453_DUMP(FS453_WSS_DATAF1); 24 bits*/
	/*FS453_DUMP(FS453_WSS_DATAF0); 24 bits*/
	FS453_DUMP_BYTE(FS453_WSS_LNF1  );
	FS453_DUMP_BYTE(FS453_WSS_LNF0  );
	FS453_DUMP(FS453_WSS_LVL   );
	FS453_DUMP_BYTE(FS453_MISC_8D   );
	FS453_DUMP(FS453_VID_CNTL0 );
	FS453_DUMP(FS453_HD_FP_SYNC);
	FS453_DUMP(FS453_HD_YOFF_BP);
	FS453_DUMP(FS453_SYNC_DL   );
	FS453_DUMP(FS453_LD_DET    );
	FS453_DUMP(FS453_DAC_CNTL  );
	FS453_DUMP(FS453_PWR_MGNT  );
	FS453_DUMP(FS453_RED_MTX   );
	FS453_DUMP(FS453_GRN_MTX   );
	FS453_DUMP(FS453_BLU_MTX   );
	FS453_DUMP(FS453_RED_SCL   );
	FS453_DUMP(FS453_GRN_SCL   );
	FS453_DUMP(FS453_BLU_SCL   );
	FS453_DUMP(FS453_CC_FIELD_1);
	FS453_DUMP(FS453_CC_FIELD_2);
	FS453_DUMP(FS453_CC_CONTROL);
	FS453_DUMP(FS453_CC_BLANK_VALUE);
	FS453_DUMP(FS453_CC_BLANK_SAMPLE);
	FS453_DUMP(FS453_HACT_ST        );
	FS453_DUMP(FS453_HACT_WD        );
	FS453_DUMP(FS453_VACT_ST        );
	FS453_DUMP(FS453_VACT_HT        );
	FS453_DUMP(FS453_PR_PB_SCALING  );
	FS453_DUMP(FS453_LUMA_BANDWIDTH );
	FS453_DUMP(FS453_QPR            );

#if defined(CONFIG_UC53281EVM_1_1) 
	i2c_coldfire_gpio_setup(0);
#endif
	mutex_unlock(&i2c_sem);
}

/*!
 * @brief Function to enable/disable the TV encoder
 * @param     client	I2C client structure
 * @param     enable	0 to disable, others to enable
 * @return    0 on success; ENODEV if the encoder wasn't found
 */
static int fs453_enable(struct i2c_client *client, int enable)
{
	struct fs453_data *data;

	if (!client)
		return -ENODEV;

	data = i2c_get_clientdata(client);

	if (enable)
		return fs453_command(client, ENCODER_SET_NORM, &data->norm);
	else {
		int ret;
		if(mutex_lock_interruptible(&i2c_sem))
			return -ERESTARTSYS;
#if defined(CONFIG_UC53281EVM_1_1) 
		i2c_coldfire_gpio_setup(FS453_I2C_ADDR);
#endif
		ret = fs453_write(client, FS453_PWR_MGNT, 0x3BFF, 2);

#if defined(CONFIG_UC53281EVM_1_1) 
		i2c_coldfire_gpio_setup(0);
#endif
		mutex_unlock(&i2c_sem);

		return ret;
	}
}

#ifdef CONFIG_ARCH_MX27
extern void gpio_fs453_reset_low(void);
extern void gpio_fs453_reset_high(void);
#endif

/*!
 * @brief FS453 control routine
 * @param	cmd	Control command
 * @param	arg	Control argument
 * @return	0 on success, others on failure
 */
int fs453_ioctl(unsigned int cmd, void *arg)
{
	/* check for deferred I2C registration */
	pr_debug("FS453: fs453_ioctl\n");
	if (!fs453_client) {
		int err;
#ifdef CONFIG_ARCH_MX27
		/* reset the FS453 via the CLS/GPIOA25 line */
		gpio_fs453_reset_low();
		gpio_fs453_reset_high();
#endif
		if ((err = i2c_add_driver(&fs453_driver))) {
			pr_info("FS453: driver registration failed\n");
		}
	}
	
	if (!fs453_client)
		return -ENODEV;

	pr_debug("FS453: calling cmd 0x%02x\n", cmd);
	return fs453_command(fs453_client, cmd, arg);
}

/*!
 * @brief Probe for the TV enocder and initialize the driver
 * @return    0 on success, others on failure
 */
static int __init fs453_init(void)
{
	pr_info("FS453/4 driver, (c) 2005 Freescale Semiconductor, Inc.\n");
	return 0;
}

/*!
 * @brief Module exit routine
 */
static void __exit fs453_exit(void)
{
	i2c_del_driver(&fs453_driver);
}

module_init(fs453_init);
module_exit(fs453_exit);

EXPORT_SYMBOL(fs453_ioctl);

MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_DESCRIPTION("FS453/4 TV encoder driver");
MODULE_LICENSE("GPL");
