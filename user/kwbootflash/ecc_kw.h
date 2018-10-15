#ifndef ECC_KW_H
#define ECC_KW_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

int nand_calculate_ecc_kw(const uint8_t *data, uint8_t *ecc);

#endif /* ECC_KW_H */
