/* linux/spi/spi_ks8692.h */

/*
 * struct ks8692_spi is for ks8692 SPI controller driver which
 * handled by the "spi_ks8692.c". This structures can be as interface between 
 * KS8692 SPI controller driver and SPI device driver. 

 * @operation: spi device must update it to tell ks8692 SPI controller that
 *             the content of 'tx_buf' is to set up read or write operation.

 *             SPI_PREPARE_TO_READ:  set up read operation,
 *             SPI_PREPARE_TO_WRITE: set up write operation,
 *             SPI_WRITE_RAW_DATA:   raw data to write to spi device.
 *
 * read_len: if 'operation' is SPI_PREPARE_TO_READ, spi device must update it 
 *           to tell ks8692 SPI controller that number of bytes is going to read.
 */

#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>


#define SPI_WRITE_CMD        0x02    /* KS8695M SPI device write Command */
#define SPI_READ_CMD         0x03    /* KS8695M SPI device read  Command */
#define SPI_WRDI_CMD         0x04    /* SPI device write protect Command */
#define SPI_RDSR_CMD         0x05    /* SPI device read status Command */
#define SPI_WREN_CMD         0x06    /* SPI device write enable Command */

#define SPI_PREPARE_TO_READ     1
#define SPI_PREPARE_TO_WRITE    2
#define SPI_WRITE_RAW_DATA      0
#define SPI_READ_RAW_DATA       SPI_WRITE_RAW_DATA


struct ks8692_spi {
	/* bitbang has to be first */
	struct spi_bitbang	 bitbang;
	struct completion	 done;

	unsigned long  regBase;
	int			   irq;
	int			   len;
	int			   tx_count;
	int			   rx_count;
    int            operation;  
    int            read_len;  
    int            chipselect;  /* SPI enable status (1: enabled, 0: disabled) */

    /* rx & tx data buffers from\to the spi_transfer */
    const void *tx;
    void       *rx;

	/* spi tx & rx functions to deal with different sized buffers */
	void (*spi_rx) (struct ks8692_spi *);
	int  (*spi_tx) (struct ks8692_spi *);

    /* issure read & write command from\to the spi_transfer */
	void (*issue_write) (struct ks8692_spi *);
	int  (*issue_read) (struct ks8692_spi *);

	u32                sysclk;
	struct resource   *ioarea;
	struct spi_master *master;
	struct spi_device *curdev;
	struct device     *dev;

	struct spi_board_info	info[1];

};


