// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024, Digi International Inc
 */
#include <linux/device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <linux/nvmem-provider.h>
#include <linux/platform_device.h>

/*
 * Offsets for the HD efuse region of the Armada 380.
 */
#define EFUSE_HD_BASE	0xf9000

#define EFUSE_HD_DATA	24
#define EFUSE_HD_DIGEST	26
#define EFUSE_HD_CSK	31
#define EFUSE_HD_FLASH	47
#define EFUSE_HD_BOX	48

#define AES_KEY_BYTES	32
#define DIGEST_BYTES	32

u8 mvebu_efuse_hash[AES_KEY_BYTES];

/*
 * Check if the secure boot efuse bit is set or not.
 */
static int mvebu_efuse_secureboot(void __iomem *base)
{
	void __iomem *p;

	p = base + EFUSE_HD_BASE + EFUSE_HD_DATA * 16;
	return (readl(p) & 0x1);
}

/*
 * The digest is stored at HD efuse lines 26 through 30. The efuse
 * region is made of rows of 16 bytes. The first 56 bits of first two
 * 32-bit data words are part of the digest itself. The following
 * 8 bits are Marvell's ECC bits. After that is the lock bit.
 */
static void mvebu_efuse_get_digest(void __iomem *base)
{
	void __iomem *p;
	u8 digest[DIGEST_BYTES];
	int i, pos;
	u32 v;

	p = base + EFUSE_HD_BASE + EFUSE_HD_DIGEST * 16;

	for (i = 0, pos = 0; i < 5; i++, p += 0x10) {
		v = readl(p);
		digest[pos++] = v & 0xff;
		digest[pos++] = (v >> 8) & 0xff;
		digest[pos++] = (v >> 16) & 0xff;
		digest[pos++] = (v >> 24) & 0xff;

		if (pos >= DIGEST_BYTES)
			break;

		v = readl(p + 4);
		digest[pos++] = v & 0xff;
		digest[pos++] = (v >> 8) & 0xff;
		digest[pos++] = (v >> 16) & 0xff;
	}

	memcpy(mvebu_efuse_hash, digest, sizeof(mvebu_efuse_hash));
}

void get_hw_hash(u8 *hash, int len);
void get_hw_hash(u8 *hash, int len)
{
	if (len > sizeof(mvebu_efuse_hash)) {
		memset(hash, 0, len);
		len = sizeof(mvebu_efuse_hash);
	}
	memcpy(hash, mvebu_efuse_hash, len);
}

static int mvebu_efuse_probe(struct platform_device *pdev)
{
	struct resource *res;
	void __iomem *base;

	base = devm_platform_get_and_ioremap_resource(pdev, 0, &res);
        if (IS_ERR(base))
                return PTR_ERR(base);

	mvebu_efuse_get_digest(base);

	pr_info("MVEBU efuse: secure boot %s",
		(mvebu_efuse_secureboot(base) ? "enabled" : "disabled"));
	return 0;
}

static const struct of_device_id mvebu_efuse_of_match[] = {
	{ .compatible = "marvell,efuse", },
	{ /* sentinal */ },
};

static struct platform_driver mvebu_efuse_driver = {
	.probe = mvebu_efuse_probe,
	.driver = {
		.name = "mvebu-efuse",
		.of_match_table = mvebu_efuse_of_match,
	},
};
module_platform_driver(mvebu_efuse_driver);

MODULE_AUTHOR("Greg Ungerer <greg.ungerere@digi.com>");
MODULE_DESCRIPTION("Marvell Armada efuse reader");
MODULE_LICENSE("GPL");
