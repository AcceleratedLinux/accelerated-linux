/* linux/drivers/spi/spi_ks8692.c
 *
 * Copyright (c) 2007-2008 Micrel, Inc.
 *
 * SPI driver for Micrel Pegasus SPI controller.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/


#include <linux/init.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <mach/hardware.h>
#include <linux/spi/spi_ks8692.h>
#include <mach/platform.h>

#if (0)
#define CONFIG_SPI_DEBUG
#endif

#ifdef CONFIG_SPI_DEBUG
#define DBG_PRINT(f, x...) \
        printk(KS8692_SPI_DRIVER " [%s()]: " f, __func__, ## x)
#else
#define DBG_PRINT(f, x...)	do { } while (0)
#endif

#define KS8692_SPI_DRIVER	"pegasus-spi"

#define KS_TX_FIFO_SIZE    16
#define KS_RX_FIFO_SIZE    16
#define KS_TX_THRESHOLD    6
#define KS_RX_THRESHOLD    8
#define KS_TX_FIFO_ALLOW   (KS_TX_FIFO_SIZE - KS_TX_THRESHOLD)



/*
 * Reset SPI controller.
 */
static inline void ks_spi_reset
(
	struct ks8692_spi *ks_spi
) 
{
    DBG_PRINT("\n");

    /* reset */
	writel(SPI_RESET, ks_spi->regBase + KS8692_SPI_CTRL);
    mdelay(1);
}

/* 
 * Init SPI Control register and enable it.
 */
static inline void ks_spi_init 
(
	struct ks8692_spi *ks_spi
) 
{
    unsigned long  regData;

    /* Set 1 delay between consecutive transfers, and between chip selects.
       Transmit buffer threshold for 1 byte */
	regData = readl(ks_spi->regBase + KS8692_SPI_CS);
    //regData |= ( SPI_DATA_DELAY_1 | SPI_CS_DELAY_1);
    regData &= ~( SPI_TX_THRESHOLD_MASK);
    regData |= ( KS_TX_THRESHOLD << 20 );                            
    regData &= ~( SPI_RX_THRESHOLD_MASK);
    regData |= ( KS_RX_THRESHOLD << 16 );                            

	writel(regData, ks_spi->regBase + KS8692_SPI_CS);

    DBG_PRINT("(SPI_CSR)0x%08X: %08x\n", (int)(ks_spi->regBase + KS8692_SPI_CS), 
               (int)(int)readl(ks_spi->regBase + KS8692_SPI_CS) );

    /* Enable SPI to transfer and receive data */
    //writel(SPI_ENABLE, ks_spi->regBase + KS8692_SPI_CTRL);  
}

/* 
 * Get System clock  SYSCLK.
 */
static inline void ks_spi_getclk 
(
	struct ks8692_spi *ks_spi
) 
{
    unsigned long  regData;
    unsigned long  fAPB;

    /* Get APB system clock in Hz */
    regData = KS8692_READ_REG( KS8692_SYSTEM_BUS_CLOCK ); 
    fAPB = regData & SYSTEM_BUS_CLOCK_MASK;

    switch ( fAPB ) {
        case SYSTEM_BUS_CLOCK_200:
            ks_spi->sysclk = 200 * 1000 * 1000;
            ks_spi->info[0].max_speed_hz = 12500 * 1000;
            break;
        case SYSTEM_BUS_CLOCK_166:
            ks_spi->sysclk = 166 * 1000 * 1000;
            ks_spi->info[0].max_speed_hz = 10375 * 1000;
            break;
        case SYSTEM_BUS_CLOCK_125:
            ks_spi->sysclk = 125 * 1000 * 1000;
            ks_spi->info[0].max_speed_hz = 9375 * 1000;
            break;
        case SYSTEM_BUS_CLOCK_50:
        default:
            ks_spi->sysclk = 50 * 1000 * 1000;
            ks_spi->info[0].max_speed_hz = 7813 * 1000;
            break;
    }

    DBG_PRINT("sysclk=%d, max_speed_hz=%d\n", ks_spi->sysclk, ks_spi->info[0].max_speed_hz );
}

/* 
 * WaitForTDBU: Wait for TxBuf empty (SPI_ISR). 
 */
static __inline__ int WaitForTDBU (struct ks8692_spi *ks_spi)
{
    int    timeOut=10;
	u32 txUntxData;						

    /* Wait for SPI transmit buffer empty */
	txUntxData = readl(ks_spi->regBase + KS8692_SPI_BUF_STATUS) & SPI_UNTX_LEN_MASK;
    while ( (--timeOut > 0 ) && (txUntxData > 0) ) {
        mdelay(1);
	    txUntxData = readl(ks_spi->regBase + KS8692_SPI_BUF_STATUS) & SPI_UNTX_LEN_MASK;
    };

    if ( txUntxData > 0 ) 
    {
       return ( -EINVAL );
    }
    
    return ( 0 ) ;
}

/*
 * spi_issue_read_cmd
 *
 * Send SPI read command, and address offset to spi target.
 */
static int spi_issue_read_cmd_u8(struct ks8692_spi *ks_spi)
{
    u32       regData;
    int       count;
	const u8 *cp = ks_spi->tx;

    /* Transmit READ command to SPI controller SPI_TDR */
    for (count=0; count < (ks_spi->len-1); count++) {
        regData = (*cp++ << 16);
	    writel(regData, ks_spi->regBase + KS8692_SPI_TX_DATA);	
        DBG_PRINT("(SPI_TDR): regData=0x%08x\n", (int)regData);

        ks_spi->tx_count += 1;
    }

    /* Transmit the last SPI device command, and configure reading length 
       to SPI controller SPI_TDR */
    regData = ((*cp++ << 16) |
                SPI_TX_CS_END | 
                SPI_TX_HIZEXT_ENABLE | 
               (ks_spi->read_len & SPI_TX_CEXT_MASK) );
	writel(regData, ks_spi->regBase + KS8692_SPI_TX_DATA);	
    DBG_PRINT("(SPI_TDR): regData=0x%08x\n", (int)regData);

    ks_spi->tx_count += 1;

    return ( 0 );
}

static int spi_issue_read_cmd_u16(struct ks8692_spi *ks_spi)
{
    u32       regData;
    int       count;
	const u8 *cp = ks_spi->tx;

    /* Transmit READ command to SPI controller SPI_TDR */
    for (count=0; count < (ks_spi->len-2); count++) {
        regData = (*cp++ << 24);
        regData |= (*cp++ << 16);
	    writel(regData, ks_spi->regBase + KS8692_SPI_TX_DATA);	
        DBG_PRINT("(SPI_TDR): regData=0x%08x\n", (int)regData);

        ks_spi->tx_count += 2;
    }

    /* Transmit the last SPI device command, and configure reading length 
       to SPI controller SPI_TDR */
    regData = (*cp++ << 24);
    regData |= ((*cp++ << 16) |
                SPI_TX_CS_END | 
                SPI_TX_HIZEXT_ENABLE | 
                SPI_TX_16_BITS | 
               ((ks_spi->read_len >> 1) & SPI_TX_CEXT_MASK) );
	writel(regData, ks_spi->regBase + KS8692_SPI_TX_DATA);	
    DBG_PRINT("(SPI_TDR): regData=0x%08x\n", (int)regData);

    ks_spi->tx_count += 2;

    return ( 0 );
}


/* 
 * Read data from the SPI device through ks8692 SPI interface to spi_transfer
 * by one BYTE.
 */
static void ks8692_spi_rx_buf_u8(struct ks8692_spi *ks_spi) 
{									  
	u32 regData;						
	u8 *rx = ks_spi->rx;					  
#ifdef CONFIG_SPI_DEBUG
	u8 *tmp = ks_spi->rx;					  
#endif

	regData = readl(ks_spi->regBase + KS8692_SPI_RX_DATA);
	*rx++ = (u8)( (regData >> 16) & 0x000000ff );						  
    DBG_PRINT("regData=0x%08x\n", regData);
    DBG_PRINT("rx=0x%08x, *rx=0x%02x\n", (int)tmp,  *tmp);

	ks_spi->rx = rx;						  
    ks_spi->rx_count += 1;
}

/* 
 * Read data from the SPI device through ks8692 SPI interface to spi_transfer
 * by one WORD.
 */
static void ks8692_spi_rx_buf_u16(struct ks8692_spi *ks_spi) 
{									  
	u32 regData;						
	u8 *rx = ks_spi->rx;					  
#ifdef CONFIG_SPI_DEBUG
	u16 *tmp = ks_spi->rx;					  
#endif

	regData = readl(ks_spi->regBase + KS8692_SPI_RX_DATA);
	*rx++ = (u8)( (regData >> 24) & 0x000000ff );						  
	*rx++ = (u8)( (regData >> 16) & 0x000000ff );						  
    DBG_PRINT("regData=0x%08x\n", regData);
    DBG_PRINT("rx=0x%08x, *rx=0x%04x\n", (int)tmp, *tmp);

	ks_spi->rx = rx;						  
    ks_spi->rx_count += 2;
}


/* 
 * Write data from spi_transfer to the SPI device through ks8692 SPI interface 
 * by one BYTE.
 */
static int ks8692_spi_tx_buf_u8(struct ks8692_spi *ks_spi)	
{								
	u32 data;						
	u32 regData;						
	u32 txUntxData;						
	const u8 *tx = ks_spi->tx;			

    DBG_PRINT("tx=0x%08x, *tx=0x%02x\n", (int)tx, *tx);

    /* return error if SPI transmit buffer is not available */
	txUntxData = readl(ks_spi->regBase + KS8692_SPI_BUF_STATUS) & SPI_UNTX_LEN_MASK;
    if ( (KS_TX_FIFO_ALLOW - txUntxData) < 0 )
    {
        printk("SPI transmit buffer is not available! txUntxData=0x%08x\n", (int)txUntxData);
        return -EINVAL;
    }

	data = *tx++;						
	ks_spi->tx = tx;
    ks_spi->tx_count += 1;
	
    /* Write data to SPI_TDR */
    regData = (data << 16);
	if (ks_spi->tx_count == ks_spi->len)
          regData |= SPI_TX_CS_END;
	writel(regData, ks_spi->regBase + KS8692_SPI_TX_DATA);	
    DBG_PRINT("(SPI_TDR): regData=0x%08x\n", (int)regData);

	return 0;						
}

/* 
 * Write data from spi_transfer to the SPI device through ks8692 SPI interface
 * by one WORD.
 */
static int ks8692_spi_tx_buf_u16(struct ks8692_spi *ks_spi)	
{								
	u32 regData=0;						
	u32 txUntxData;						
	const u8 *tx = ks_spi->tx;			
#ifdef CONFIG_SPI_DEBUG
	const u16 *tmp = ks_spi->tx;			
#endif

    DBG_PRINT("tx=0x%08x, *tx=0x%04x\n", (int)tmp, *tmp);

    /* return error if SPI transmit buffer is not available */
	txUntxData = readl(ks_spi->regBase + KS8692_SPI_BUF_STATUS) & SPI_UNTX_LEN_MASK;
    if ( (KS_TX_FIFO_ALLOW - txUntxData) < 0 )
    {
        printk("SPI transmit buffer is not available! txUntxData=0x%08x\n", (int)txUntxData);
        return -EINVAL;
    }

    ks_spi->tx_count += 2;

    /* Write data to SPI_TDR */
    regData = (*tx++ << 24);
    regData |= ((*tx++ << 16) | SPI_TX_16_BITS);
	if (ks_spi->tx_count == ks_spi->len)
       regData |= SPI_TX_CS_END;
	writel(regData, ks_spi->regBase + KS8692_SPI_TX_DATA);	
    DBG_PRINT("(SPI_TDR): regData=0x%08x\n", (int)regData);

	ks_spi->tx = tx;

	return 0;						
}


static inline struct ks8692_spi *to_hw (struct spi_device *sdev)
{
	return spi_master_get_devdata(sdev->master);
}

static void ks8692_spi_chipsel
(
    struct spi_device *spi, 
    int value
)
{
	struct ks8692_spi *ks_spi = to_hw(spi);
    unsigned long  regData;
    u32  intISR;

    DBG_PRINT("value=%d, mode=%02x\n", (int)value, (int)spi->mode);

	switch (value) {
	case BITBANG_CS_INACTIVE:
        if ( WaitForTDBU (ks_spi) < 0 ) {
		    dev_dbg(ks_spi->dev, "ERROR: some data still left in SPI transmit buffer before CS inactived.\n");
		    printk("ks8692_spi_chipsel: ERROR: some data still left in SPI transmit buffer before SPI disabled.\n");
        }

        if (ks_spi->chipselect == 1) {
            mdelay(1);
		    writel(0, ks_spi->regBase + KS8692_SPI_CTRL);
            ks_spi->chipselect = 0;
        }
		break;

	case BITBANG_CS_ACTIVE:  /* normally nCS, active low */
		regData = readl(ks_spi->regBase + KS8692_SPI_CS);

        /* Clock Phase Select (CPHA) */
		if (spi->mode & SPI_CPHA)
			regData |= SPI_DATA_LEADING_EDGE;
		else
			regData &= ~SPI_DATA_LEADING_EDGE;

        /* Clock polarity select (CPOL) */
		if (spi->mode & SPI_CPOL)
			regData &= ~SPI_SPCK_INACTIVE_HIGH;
		else
			regData |= SPI_SPCK_INACTIVE_HIGH;

        /* Active level of chip select */
		if (spi->mode & SPI_CS_HIGH)
			regData |= SPI_CS_ACTIVE_HIGH;  /* chipselect active high? */
		else
			regData &= ~SPI_CS_ACTIVE_HIGH;

		/* write new configration */
		writel(regData, ks_spi->regBase + KS8692_SPI_CS);

        /* Enable SPI to transfer and receive data */
	    writel(SPI_ENABLE, ks_spi->regBase + KS8692_SPI_CTRL);  
        ks_spi->chipselect = 1;

        /* clear interrupt status */
        intISR = readl(ks_spi->regBase + KS8692_SPI_INT_STATUS);
        DBG_PRINT("(SPI_ISR)0x%08X: %08x\n", 
               (int)(ks_spi->regBase + KS8692_SPI_INT_STATUS), (int)intISR);

		break;
	}

    DBG_PRINT("(SPI_CSR)0x%08X: %08x\n", (int)(ks_spi->regBase + KS8692_SPI_CS),
              (int)readl(ks_spi->regBase + KS8692_SPI_CS));

    DBG_PRINT("(SPI_CTR)0x%08X: %08x\n", (int)(ks_spi->regBase + KS8692_SPI_CTRL),
              (int)readl(ks_spi->regBase + KS8692_SPI_CTRL));

}

static int ks8692_spi_setupxfer
(
    struct spi_device *spi,
    struct spi_transfer *t
)
{
	struct ks8692_spi *ks_spi = to_hw(spi);
	unsigned int bpw;
	unsigned int hz;
	unsigned int div;
    unsigned long  regData;
    unsigned long  sysclkDivider;

    if (t) 
       DBG_PRINT("bpw %d, hz %d\n", t->bits_per_word, t->speed_hz);
    else
       DBG_PRINT("spi_bpw %d, spi_hz %d\n", spi->bits_per_word, spi->max_speed_hz);

	bpw = (t && t->bits_per_word) ? t->bits_per_word : spi->bits_per_word;
	hz  = (t && t->speed_hz) ? t->speed_hz : spi->max_speed_hz;

	/* Make sure its a bit width we support [8, 16] */
	if ((bpw != 8) && (bpw != 16)) {
		dev_err(&spi->dev, "invalid bits-per-word (%d)\n", bpw);
		return -EINVAL;
    }

	if (bpw == 8) {
		ks_spi->spi_rx = ks8692_spi_rx_buf_u8;
		ks_spi->spi_tx = ks8692_spi_tx_buf_u8;

		ks_spi->issue_read = spi_issue_read_cmd_u8;
	} 
    else if (bpw == 16) {
		ks_spi->spi_rx = ks8692_spi_rx_buf_u16;
		ks_spi->spi_tx = ks8692_spi_tx_buf_u16;

		ks_spi->issue_read = spi_issue_read_cmd_u16;
	} 
    else
		return -EINVAL;

    /* Program Bits Per Transfer to the device */
	regData = readl(ks_spi->regBase + KS8692_SPI_TX_DATA);
    if (bpw == 16) {
	   regData |= SPI_TX_16_BITS;
	   regData &= ~SPI_MICREL_MODE;
    }
    else if (bpw == 32) {
	   regData &= ~SPI_TX_BITS_MASK;
	   regData |= SPI_MICREL_MODE;
    }
    else  {
	   regData &= ~SPI_TX_BITS_MASK;
    }

	writel(regData, ks_spi->regBase + KS8692_SPI_TX_DATA);

    DBG_PRINT("(SPI_TDR)0x%08X: %08x\n", (int)(ks_spi->regBase + KS8692_SPI_TX_DATA),
              (int)readl(ks_spi->regBase + KS8692_SPI_TX_DATA));

    /* Get the clock divider */
	div = ks_spi->sysclk / hz;

    DBG_PRINT("max_speed_hz=%d, sysclk=%d, hz=%d, div=%d\n", 
               spi->max_speed_hz, ks_spi->sysclk, hz, div);

	if (div <= 16)
		sysclkDivider = SPI_SYSCLK_BY16;
    else if ((div > 16) && (div <= 32))
		sysclkDivider = SPI_SYSCLK_BY32;
    else if ((div > 32) && (div <= 64))
		sysclkDivider = SPI_SYSCLK_BY64;
    else if ((div > 64) && (div <= 128))
		sysclkDivider = SPI_SYSCLK_BY128;
    else if ((div > 128) && (div <= 256))
		sysclkDivider = SPI_SYSCLK_BY256;
    else if ((div > 256) && (div <= 1024))
		sysclkDivider = SPI_SYSCLK_BY1024;
    else if ((div > 1024) && (div <= 8192))
		sysclkDivider = SPI_SYSCLK_BY8192;
    else if ((div > 8192) && (div <= 65536))
		sysclkDivider = SPI_SYSCLK_BY65536;
    else
		sysclkDivider = SPI_SYSCLK_BY65536;

    DBG_PRINT("sysclkDivider=%08x\n", (int)sysclkDivider);

    /* Program SPCK clock rate to the device */
	regData = readl(ks_spi->regBase + KS8692_SPI_CS);
	regData &= ~SPI_SYSCLK_MASK;
	regData |= sysclkDivider;
	writel(regData, ks_spi->regBase + KS8692_SPI_CS);
    DBG_PRINT("(SPI_CSR)0x%08X: %08x\n", (int)(ks_spi->regBase + KS8692_SPI_CS),
              (int)readl(ks_spi->regBase + KS8692_SPI_CS));

	spin_lock(&ks_spi->bitbang.lock);
	if (!ks_spi->bitbang.busy) {
		ks_spi->bitbang.chipselect(spi, BITBANG_CS_INACTIVE);
		/* need to ndelay for 0.5 clocktick ? */
	}
	spin_unlock(&ks_spi->bitbang.lock);

	return 0;
}

/* the spi->mode bits understood by this driver: */
#define MODEBITS	(SPI_CPOL | SPI_CPHA | SPI_CS_HIGH | \
			 SPI_LSB_FIRST | SPI_3WIRE)

static int ks8692_spi_setup
(
    struct spi_device *spi
)
{
	int ret;

    DBG_PRINT("mode %d, %u bpw, %d hz\n", 
              spi->mode, spi->bits_per_word, spi->max_speed_hz);

	if (spi->mode & ~MODEBITS) {
		return -EINVAL;
	}

	if (!spi->bits_per_word)
		//spi->bits_per_word = 8;
		spi->bits_per_word = 16;

	if ((spi->mode & SPI_LSB_FIRST) != 0) {
        DBG_PRINT("ERROR: spi mode %d is SPI_LSB_FIRST\n", spi->mode);
		return -EINVAL;
    }

	ret = ks8692_spi_setupxfer(spi, NULL);
	if (ret < 0) {
		dev_err(&spi->dev, "setupxfer returned %d\n", ret);
		return ret;
	}

	return 0;
}


static int ks8692_spi_txrx
(
    struct spi_device *spi, 
    struct spi_transfer *t
)
{
    struct ks8692_spi *ks_spi = to_hw(spi);
    u32  intIER;
    u32  intISR, tdr;
    u32  intMask=0;

    DBG_PRINT("tx %p, rx %p, len %d, operation %d\n", 
               t->tx_buf, t->rx_buf, t->len, ks_spi->operation );

    ks_spi->tx = t->tx_buf;
    ks_spi->rx = t->rx_buf;
    ks_spi->len = t->len;
    ks_spi->tx_count = 0;
    ks_spi->rx_count = 0;
    INIT_COMPLETION(ks_spi->done);

    /* Read SPI IER */
	intIER = readl(ks_spi->regBase + KS8692_SPI_INT_ENABLE);

    /* fill the transmit FIFO */
    if ( ks_spi->tx ) {

        /* Issue SPI READ cmd */
        if (ks_spi->operation == SPI_PREPARE_TO_READ) {
            ks_spi->read_len = (ks_spi->read_len > KS_RX_FIFO_SIZE) ? 
                               KS_RX_FIFO_SIZE : ks_spi->read_len;
            ks_spi->issue_read(ks_spi);
        }

        /* write raw data to SPI device */
        else { 
            while (ks_spi->tx_count < ks_spi->len) {

                DBG_PRINT(" len=%d, tx_count=%d\n", ks_spi->len, ks_spi->tx_count);

                /* if SPI transmit FIFO is full */
	            if ( ks_spi->spi_tx(ks_spi) < 0) {

                    /* enable SPI TB_TH interrupt */
                    intMask = (SPI_INT_TDTH | SPI_INT_TDBU);
	                intIER |= intMask;
                    DBG_PRINT("(SPI_IER)0x%08X: %08x\n", 
                               (int)(ks_spi->regBase + KS8692_SPI_INT_ENABLE), 
                               (int)intIER);

	                writel(intIER, ks_spi->regBase + KS8692_SPI_INT_ENABLE);
                    break;
                }
            }
       
            if (ks_spi->tx_count < ks_spi->len) 
	            wait_for_completion(&ks_spi->done);

        } /* write raw data to SPI device */
    } /* if ( ks_spi->tx ) { */

    else if ( ks_spi->rx ) {

        ks_spi->len = (ks_spi->len > KS_RX_FIFO_SIZE) ? 
                               KS_RX_FIFO_SIZE : ks_spi->len;
	    /* Get interrupt status */
	    intISR = readl(ks_spi->regBase + KS8692_SPI_INT_STATUS);
        DBG_PRINT("intISR: 0x%08x\n", intISR);
        
	    if (spi->mode & SPI_3WIRE) {
		tdr = readl(ks_spi->regBase + KS8692_SPI_TX_DATA);
		tdr |= SPI_TX_HIZ_ENABLE;
		writel(tdr, ks_spi->regBase + KS8692_SPI_TX_DATA);
	    }

	    while ( (intISR & SPI_INT_RDRDY) || (intISR & SPI_INT_RDBU) ) {
            DBG_PRINT(" len=%d, rx_count=%d\n", ks_spi->len, ks_spi->rx_count);

            /* read data from SPI device to rx FIFO */
            if (ks_spi->rx_count < ks_spi->len) 
                ks_spi->spi_rx(ks_spi);

	        intISR = readl(ks_spi->regBase + KS8692_SPI_INT_STATUS);
        }

        /* still more data are expected to read from rx FIFO */
        if (ks_spi->rx_count < ks_spi->len) {
             /* enable rx ready interrupt */
             intMask = (SPI_INT_RDRDY | SPI_INT_RDBU);
             intIER |= intMask;
             DBG_PRINT("(SPI_IER)0x%08X: %08x\n", 
                 (int)(ks_spi->regBase + KS8692_SPI_INT_ENABLE), (int)intIER);

             writel(intIER, ks_spi->regBase + KS8692_SPI_INT_ENABLE);
             wait_for_completion(&ks_spi->done);

	    if (spi->mode & SPI_3WIRE) {
		tdr = readl(ks_spi->regBase + KS8692_SPI_TX_DATA);
		tdr &= ~SPI_TX_HIZ_ENABLE;
		writel(tdr, ks_spi->regBase + KS8692_SPI_TX_DATA);
	    }
        }

    } /* else if ( ks_spi->rx ) { */

	/* disable spi interrupts */
	intIER &= ~(intMask);
	writel(intIER, ks_spi->regBase + KS8692_SPI_INT_ENABLE);

    DBG_PRINT("(SPI_IER)0x%08X: %08x\n", (int)(ks_spi->regBase + KS8692_SPI_INT_ENABLE),
              (int)readl(ks_spi->regBase + KS8692_SPI_INT_ENABLE));

    return ks_spi->rx_count > ks_spi->tx_count ? ks_spi->rx_count : ks_spi->tx_count;
}

static irqreturn_t ks8692_spi_irq
(
    int irq,
    void *dev
)
{
	struct ks8692_spi *ks_spi = dev;
    u32  intStatus;
 
	/* Get interrupt status */
	intStatus = readl(ks_spi->regBase + KS8692_SPI_INT_STATUS);

    DBG_PRINT(" 0x%08x\n", intStatus);

	/* spi receive data ready, we need handle RX first */
	if ( (intStatus & SPI_INT_RDRDY) || (intStatus & SPI_INT_RDBU) ) {

        DBG_PRINT(" len=%d, rx_count=%d\n", ks_spi->len, ks_spi->rx_count);

        /* read data from SPI device to rx FIFO */
        if (ks_spi->rx_count < ks_spi->len) {
           if (ks_spi->rx) 
              ks_spi->spi_rx(ks_spi);
        }
        else {
           u32  dumpData;

          /* Clear RX fifo */
	       dumpData = readl(ks_spi->regBase + KS8692_SPI_RX_DATA);
           DBG_PRINT("dumpData=0x%08x\n", dumpData);
        }

        /* finished read data from SPI rx FIFO */
        if (ks_spi->rx_count >= ks_spi->len) 
           complete(&ks_spi->done);
	}

	/* spi transmit buffer level is lower than the threshole */
	if ( intStatus & SPI_INT_TDTH ) {

        /* write data from tx FIFO to SPI device */
        if (ks_spi->tx_count < ks_spi->len) {
            if (ks_spi->tx)
                ks_spi->spi_tx(ks_spi);
        }
        /* finished write from tx FIFO */
        else {
            complete(&ks_spi->done);
        }
    }

	return( IRQ_HANDLED );
}

static int ks8692_spi_probe(struct platform_device *pdev)
{
	struct ks8692_spi *ks_spi;
	struct spi_master *master;
	struct resource *res;
	int err = 0;

	master = spi_alloc_master(&pdev->dev, sizeof(struct ks8692_spi));
	if (master == NULL) {
		dev_err(&pdev->dev, "No memory for spi_master\n");
		err = -ENOMEM;
		goto err_nomem;
	}

	ks_spi = spi_master_get_devdata(master);
	memset(ks_spi, 0, sizeof(struct ks8692_spi));

	ks_spi->master = spi_master_get(master);
	ks_spi->dev = &pdev->dev;

    DBG_PRINT("pdev=0x%08x, ks_spi=0x%08x, ks_spi->dev=0x%08x\n", 
               (int)pdev, (int)ks_spi, (int)ks_spi->dev);

	platform_set_drvdata(pdev, ks_spi);
	init_completion(&ks_spi->done);

	/* setup the state for the bitbang driver */

	ks_spi->bitbang.master         = ks_spi->master;
	ks_spi->bitbang.setup_transfer = ks8692_spi_setupxfer;
	ks_spi->bitbang.chipselect     = ks8692_spi_chipsel;
	ks_spi->bitbang.txrx_bufs      = ks8692_spi_txrx;
	ks_spi->bitbang.master->setup  = ks8692_spi_setup;
	ks_spi->bitbang.master->mode_bits = MODEBITS;
	ks_spi->bitbang.master->num_chipselect = 2;
	ks_spi->spi_rx = ks8692_spi_rx_buf_u8;
	ks_spi->spi_tx = ks8692_spi_tx_buf_u8;
	ks_spi->issue_read = spi_issue_read_cmd_u8;

    DBG_PRINT("bitbang at %p\n", &ks_spi->bitbang);

	/* find and map our resources */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");
		err = -ENOENT;
		goto err_no_iores;
	}

	ks_spi->ioarea = request_mem_region(res->start, (res->end - res->start)+1,
					pdev->name);
	if (ks_spi->ioarea == NULL) {
		dev_err(&pdev->dev, "Cannot reserve region\n");
		err = -ENXIO;
		goto err_no_iores;
	}

	ks_spi->regBase = VIO(KS8692_SPI_BASE);    /* SPI controller virtual address */

	ks_spi->irq = platform_get_irq(pdev, 0);
	if (ks_spi->irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		err = -ENOENT;
		goto err_no_iomap;
	}

	err = request_irq(ks_spi->irq, ks8692_spi_irq, 0, "spi", ks_spi);
	if (err) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		goto err_no_iomap;
	}

	/* program defaults into the registers */

    ks_spi_reset(ks_spi);
	ks_spi_init(ks_spi);

	/* register our spi controller */

	err = spi_bitbang_start(&ks_spi->bitbang);
	if (err) {
		dev_err(&pdev->dev, "Failed to register SPI master\n");
		goto err_register;
	}

#if (1)
	printk(KERN_INFO "%s at 0x%08x ioaddr 0x%08x irq %d\n", pdev->name,
           (int)ks_spi->ioarea, (int)ks_spi->regBase, ks_spi->irq );
#endif

    /* Get System clock  SYSCLK */
    ks_spi_getclk(ks_spi);

	/* register all the devices associated */
	strcpy(ks_spi->info[0].modalias, "spidev");
	ks_spi->info[0].chip_select = 1;
	ks_spi->info[0].controller_data = ks_spi;

	if ( spi_new_device(ks_spi->master, &ks_spi->info[0]) ) {
	    DBG_PRINT("registering %s at bus %d.\n", 
                   ks_spi->info[0].modalias, ks_spi->info[0].bus_num);
    }

	return 0;

 err_register:
	free_irq(ks_spi->irq, ks_spi);

 err_no_iomap:
    if (ks_spi->ioarea) {
	   release_resource(ks_spi->ioarea);
	   kfree(ks_spi->ioarea);
    }

 err_no_iores:
	spi_master_put(ks_spi->master);;

 err_nomem:
	return err;
}

static int ks8692_spi_remove(struct platform_device *dev)
{
	struct ks8692_spi *ks_spi = platform_get_drvdata(dev);

	platform_set_drvdata(dev, NULL);

	spi_unregister_master(ks_spi->master);

	free_irq(ks_spi->irq, ks_spi);

	release_resource(ks_spi->ioarea);
	kfree(ks_spi->ioarea);

	spi_master_put(ks_spi->master);
	return 0;
}


static struct platform_driver ks8692_spidrv = {
	.probe		= ks8692_spi_probe,
	.remove		= ks8692_spi_remove,
	.driver		= {
		.name	= KS8692_SPI_DRIVER,
		.owner	= THIS_MODULE,
	},
};

static int __init ks8692_spi_init(void)
{
    DBG_PRINT("Micrel Pegasus KS8692 SPI Driver\n");
    return platform_driver_register(&ks8692_spidrv);
}

static void __exit ks8692_spi_exit(void)
{
    platform_driver_unregister(&ks8692_spidrv);
}

module_init(ks8692_spi_init);
module_exit(ks8692_spi_exit);

MODULE_DESCRIPTION("Micrel Pegasus SPI Driver");
MODULE_LICENSE("GPL");
