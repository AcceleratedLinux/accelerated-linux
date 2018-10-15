

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/netlink.h>
#include <net/xfrm.h>
#include <net/ah.h>
int xfrm_user_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh, int *errp);

#include <asm/arch/kszipsec/elpcrypto.h>

#if 1
#define SIM_TRANSPORT
#define NUM_OF_POLICY  2
#else
#if 1
#define SIM_TUNNEL_AH
#else
#define SIM_TUNNEL_ESP
#endif
#define NUM_OF_POLICY  1
#endif

#if 0
#define TRANSPORT_AH_SPI       0x55555555
#define TRANSPORT_AH_OUT_DST   0x6401A8C0
#define TRANSPORT_AH_OUT_SRC   0x0201A8C0

#define TUNNEL_AH_SPI          0x44444444
#define TUNNEL_AH_OUT_DST      0x6401A8C0
#define TUNNEL_AH_OUT_SRC      0x0401A8C0

#define TRANSPORT_ESP_SPI      0x12345678
#define TRANSPORT_ESP_OUT_DST  0x6401A8C0
#define TRANSPORT_ESP_OUT_SRC  0x0101A8C0

#define TUNNEL_ESP_SPI         0x33333333
#define TUNNEL_ESP_OUT_DST     0x6401A8C0
#define TUNNEL_ESP_OUT_SRC     0x0301A8C0
#else
#define TRANSPORT_AH_SPI       0x55555555
#define TRANSPORT_AH_OUT_DST   0x0201A8C0
#define TRANSPORT_AH_OUT_SRC   0x6401A8C0

#define TUNNEL_AH_SPI          0x44444444
#define TUNNEL_AH_OUT_DST      0x0401A8C0
#define TUNNEL_AH_OUT_SRC      0x6401A8C0

#define TRANSPORT_ESP_SPI      0x12345678
#define TRANSPORT_ESP_OUT_DST  0x0101A8C0
#define TRANSPORT_ESP_OUT_SRC  0x6401A8C0

#define TUNNEL_ESP_SPI         0x33333333
#define TUNNEL_ESP_OUT_DST     0x0301A8C0
#define TUNNEL_ESP_OUT_SRC     0x6401A8C0
#endif


#define AH_KEY_SIZE   16
#if 1
#define AH_ALG        "hmac(md5)"
#endif
#if 0
#define AH_ALG        "hmac(sha1)"
#endif

#if 1
#define ESP_KEY_SIZE  8
#endif
#if 0
#define ESP_KEY_SIZE  16
#endif
#if 0
#define ESP_KEY_SIZE  24
#endif
#if 0
#define ESP_KEY_SIZE  32
#endif
#if 0
#define ESP_ALG       "cbc(aes)"
#endif
#if 1
#define ESP_ALG       "cbc(des)"
#endif
#if 0
#define ESP_ALG       "cbc(des3_ede)"
#endif


static unsigned char ah_key[ 16 ] = {
0x6C,
0x88,
0x3F,
0xD9,
0x17,
0xA1,
0xBF,
0x9B,
0x32,
0xE0,
0x49,
0xD7,
0x71,
0xE3,
0xC0,
0x9F
};

static unsigned char alg_key[ 32 ] = {
0x79,
0x8D,
0xC8,
0x7D,
0x30,
0xEB,
0x1E,
0x29,
0x29,
0x4E,
0x90,
0x7E,
0x7E,
0x82,
0xC2,
0xD5,
0x6B,
0xD7,
0xC3,
0x5F,
0x23,
0x3D,
0x7E,
0xDA,
0x4C,
0x5E,
0x43,
0x32,
0x47,
0x38,
0xC8,
0x14
};


void test_ipsec ( void )
{
    struct ah_data* ahp;
    struct sk_buff* skb;
    struct nlmsghdr* nlh;
    struct xfrm_usersa_info* info;
    struct xfrm_algo *algp;
    struct xfrm_userpolicy_info* policy;
    struct xfrm_user_tmpl* ut;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    struct nlattr* nla;
#else
    struct rtattr* attr;
#endif
    int attrlen;
    u8 auth_data[ 20 ];
    int handle;
    int i;
    int err;
    int outsize;

#ifdef SIM_TRANSPORT
    printk( "  Transport ESP: %08X -> %08X; AH: %08X -> %08X\n",
        TRANSPORT_ESP_OUT_SRC, TRANSPORT_ESP_OUT_DST,
        TRANSPORT_AH_OUT_SRC, TRANSPORT_AH_OUT_DST );
#elif defined( SIM_TUNNEL_AH )
    printk( "  Tunnel AH: %08X -> %08X\n",
        TUNNEL_AH_OUT_SRC, TUNNEL_AH_OUT_DST );
#else
    printk( "  Tunnel ESP: %08X -> %08X\n",
        TUNNEL_ESP_OUT_SRC, TUNNEL_ESP_OUT_DST );
#endif

    skb = dev_alloc_skb( 1500 );
    skb->len = 128;
    skb->data[ 0 ] = 0x45;
    skb->data[ 1 ] = 0x00;
    skb->data[ 2 ] = 0x00;
    skb->data[ 3 ] = 0x80;
    skb->data[ 4 ] = 0x00;
    skb->data[ 5 ] = 0x00;
    skb->data[ 6 ] = 0x00;
    skb->data[ 7 ] = 0x00;
    skb->data[ 8 ] = 0x00;
    skb->data[ 9 ] = 0x33;
    skb->data[ 10 ] = 0x00;
    skb->data[ 11 ] = 0x00;
    skb->data[ 12 ] = 0xC0;
    skb->data[ 13 ] = 0xA8;
    skb->data[ 14 ] = 0x08;
    skb->data[ 15 ] = 0x0E;
    skb->data[ 16 ] = 0xC0;
    skb->data[ 17 ] = 0xA8;
    skb->data[ 18 ] = 0x08;
    skb->data[ 19 ] = 0x85;
    skb->data[ 20 ] = 0x04;
    skb->data[ 21 ] = 0x04;
    skb->data[ 22 ] = 0x00;
    skb->data[ 23 ] = 0x00;
    skb->data[ 24 ] = 0x67;
    skb->data[ 25 ] = 0xA3;
    skb->data[ 26 ] = 0x79;
    skb->data[ 27 ] = 0x7F;
    skb->data[ 28 ] = 0x00;
    skb->data[ 29 ] = 0x00;
    skb->data[ 30 ] = 0x00;
    skb->data[ 31 ] = 0x01;
    skb->data[ 32 ] = 0x00;
    skb->data[ 33 ] = 0x00;
    skb->data[ 34 ] = 0x00;
    skb->data[ 35 ] = 0x00;
    skb->data[ 36 ] = 0x00;
    skb->data[ 37 ] = 0x00;
    skb->data[ 38 ] = 0x00;
    skb->data[ 39 ] = 0x00;
    skb->data[ 40 ] = 0x00;
    skb->data[ 41 ] = 0x00;
    skb->data[ 42 ] = 0x00;
    skb->data[ 43 ] = 0x00;
    skb->data[ 44 ] = 0x45;
    skb->data[ 45 ] = 0x00;
    skb->data[ 46 ] = 0x00;
    skb->data[ 47 ] = 0x54;
    skb->data[ 48 ] = 0x00;
    skb->data[ 49 ] = 0x00;
    skb->data[ 50 ] = 0x40;
    skb->data[ 51 ] = 0x00;
    skb->data[ 52 ] = 0x40;
    skb->data[ 53 ] = 0x01;
    skb->data[ 54 ] = 0xD7;
    skb->data[ 55 ] = 0xF2;
    skb->data[ 56 ] = 0xAC;
    skb->data[ 57 ] = 0x10;
    skb->data[ 58 ] = 0x05;
    skb->data[ 59 ] = 0x0E;
    skb->data[ 60 ] = 0xAC;
    skb->data[ 61 ] = 0x10;
    skb->data[ 62 ] = 0x05;
    skb->data[ 63 ] = 0x88;
    skb->data[ 64 ] = 0x08;
    skb->data[ 65 ] = 0x00;
    skb->data[ 66 ] = 0x6B;
    skb->data[ 67 ] = 0x1D;
    skb->data[ 68 ] = 0x0D;
    skb->data[ 69 ] = 0x0C;
    skb->data[ 70 ] = 0x00;
    skb->data[ 71 ] = 0x01;
    skb->data[ 72 ] = 0xC9;
    skb->data[ 73 ] = 0x0D;
    skb->data[ 74 ] = 0x3C;
    skb->data[ 75 ] = 0x43;
    skb->data[ 76 ] = 0x83;
    skb->data[ 77 ] = 0x81;
    skb->data[ 78 ] = 0x0C;
    skb->data[ 79 ] = 0x00;
    skb->data[ 80 ] = 0x08;
    skb->data[ 81 ] = 0x09;
    skb->data[ 82 ] = 0x0A;
    skb->data[ 83 ] = 0x0B;
    skb->data[ 84 ] = 0x0C;
    skb->data[ 85 ] = 0x0D;
    skb->data[ 86 ] = 0x0E;
    skb->data[ 87 ] = 0x0F;
    skb->data[ 88 ] = 0x10;
    skb->data[ 89 ] = 0x11;
    skb->data[ 90 ] = 0x12;
    skb->data[ 91 ] = 0x13;
    skb->data[ 92 ] = 0x14;
    skb->data[ 93 ] = 0x15;
    skb->data[ 94 ] = 0x16;
    skb->data[ 95 ] = 0x17;
    skb->data[ 96 ] = 0x18;
    skb->data[ 97 ] = 0x19;
    skb->data[ 98 ] = 0x1A;
    skb->data[ 99 ] = 0x1B;
    skb->data[ 100 ] = 0x1C;
    skb->data[ 101 ] = 0x1D;
    skb->data[ 102 ] = 0x1E;
    skb->data[ 103 ] = 0x1F;
    skb->data[ 104 ] = 0x20;
    skb->data[ 105 ] = 0x21;
    skb->data[ 106 ] = 0x22;
    skb->data[ 107 ] = 0x23;
    skb->data[ 108 ] = 0x24;
    skb->data[ 109 ] = 0x25;
    skb->data[ 110 ] = 0x26;
    skb->data[ 111 ] = 0x27;
    skb->data[ 112 ] = 0x28;
    skb->data[ 113 ] = 0x29;
    skb->data[ 114 ] = 0x2A;
    skb->data[ 115 ] = 0x2B;
    skb->data[ 116 ] = 0x2C;
    skb->data[ 117 ] = 0x2D;
    skb->data[ 118 ] = 0x2E;
    skb->data[ 119 ] = 0x2F;
    skb->data[ 120 ] = 0x30;
    skb->data[ 121 ] = 0x31;
    skb->data[ 122 ] = 0x32;
    skb->data[ 123 ] = 0x33;
    skb->data[ 124 ] = 0x34;
    skb->data[ 125 ] = 0x35;
    skb->data[ 126 ] = 0x36;
    skb->data[ 127 ] = 0x37;
    ahp = kmalloc(sizeof(*ahp), GFP_KERNEL);

    memset(ahp, 0, sizeof(*ahp));

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    ahp->key_len = AH_KEY_SIZE;
    ahp->key = kmalloc( ahp->key_len, GFP_KERNEL );
#endif
    ahp->tfm = crypto_alloc_hash(AH_ALG, 0, CRYPTO_ALG_ASYNC);
    ahp->icv_full_len = 16;
    ahp->icv_trunc_len = 12;
    ahp->work_icv = kmalloc(ahp->icv_full_len, GFP_KERNEL);
    crypto_hash_setkey(ahp->tfm, ah_key, AH_KEY_SIZE);
    ah_mac_digest( ahp, skb, auth_data );
    for ( i = 0; i < 16; i++ ) {
        printk( "%02X ", ahp->work_icv[ i ]);
    }
    printk( "\n" );

    handle = crypto_open( CRYPTO_CONTEXT_HMAC, CRYPTO_MODE_HMAC_MD5 );
    err = crypto_set_context( handle, ah_key, AH_KEY_SIZE, NULL, 0, 0 );
    outsize = 20;
    memset( auth_data, 0, outsize );
    err = crypto_hmac_wait_( handle, skb->data, skb->len, auth_data,
        &outsize, 1 );
    crypto_close( handle );

    for ( i = 0; i < 16; i++ ) {
        printk( "%02X ", auth_data[ i ]);
    }
    printk( "\n" );

/* Outbound policy */
    nlh = ( struct nlmsghdr* ) skb->data;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    attrlen =
        nla_total_size( sizeof( struct xfrm_user_tmpl ) * NUM_OF_POLICY );
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_userpolicy_info ) + attrlen;
    
#else
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_userpolicy_info ) +
        sizeof( struct rtattr ) * 2 +
        sizeof( struct xfrm_user_tmpl ) * NUM_OF_POLICY;
#endif
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = XFRM_MSG_NEWPOLICY;

    policy = ( struct xfrm_userpolicy_info* ) NLMSG_DATA( nlh );
    memset( policy, 0, sizeof( struct xfrm_userpolicy_info ));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = ( struct nlattr* )( policy + 1 );
    nla->nla_len = nla_attr_size( sizeof( struct xfrm_user_tmpl ) *
        NUM_OF_POLICY );
    nla->nla_type = XFRMA_TMPL;

    ut = nla_data( nla );

#else
    attr = ( struct rtattr* )( policy + 1 );
    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_user_tmpl ) *
        NUM_OF_POLICY );
    attr->rta_type = XFRMA_TMPL;

    ut = RTA_DATA( attr );
#endif

#ifdef SIM_TRANSPORT
/* Transport ESP */
    memset( ut, 0, sizeof( struct xfrm_user_tmpl ));
    ut->id.daddr.a4 = TRANSPORT_ESP_OUT_DST;
    ut->id.spi = TRANSPORT_ESP_SPI;
    ut->id.proto = IPPROTO_ESP;
#if 1
    ut->optional = 1;
#endif
    ut->saddr.a4 = TRANSPORT_ESP_OUT_SRC;

/* Transport AH */
    ut++;
    memset( ut, 0, sizeof( struct xfrm_user_tmpl ));
    ut->id.daddr.a4 = TRANSPORT_AH_OUT_DST;
    ut->id.spi = TRANSPORT_AH_SPI;
    ut->id.proto = IPPROTO_AH;
#if 1
    ut->optional = 1;
#endif
    ut->saddr.a4 = TRANSPORT_AH_OUT_SRC;
    ut++;
#endif

#ifdef SIM_TUNNEL_ESP
/* Tunnel ESP */
    memset( ut, 0, sizeof( struct xfrm_user_tmpl ));
    ut->id.daddr.a4 = TUNNEL_ESP_OUT_DST;
    ut->id.spi = 0;
    ut->id.proto = IPPROTO_ESP;
#if 1
    ut->optional = 1;
#endif
    ut->mode = 1;
    ut->saddr.a4 = TUNNEL_ESP_OUT_SRC;
    ut++;
#endif

#ifdef SIM_TUNNEL_AH
/* Tunnel AH */
    memset( ut, 0, sizeof( struct xfrm_user_tmpl ));
    ut->id.daddr.a4 = TUNNEL_AH_OUT_DST;
    ut->id.spi = 0;
    ut->id.proto = IPPROTO_AH;
#if 1
    ut->optional = 1;
#endif
    ut->mode = 1;
    ut->saddr.a4 = TUNNEL_AH_OUT_SRC;
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);
    attr->rta_len = 0;
#endif

    policy->sel.family = AF_INET;
    policy->dir = XFRM_POLICY_OUT;
    policy->action = XFRM_POLICY_ALLOW;
    policy->share = XFRM_SHARE_ANY;

    policy->lft.hard_byte_limit = 400000000;
    policy->lft.hard_packet_limit = 40000;
    policy->lft.soft_byte_limit = 200000000;
    policy->lft.soft_packet_limit = 20000;

    skb->len = nlh->nlmsg_len;

    if ( xfrm_user_rcv_msg( skb, nlh, &err ) )
        goto setup_error;
    skb->data = skb->tail;

/* Inbound policy */
    nlh = ( struct nlmsghdr* ) skb->data;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    attrlen =
        nla_total_size( sizeof( struct xfrm_user_tmpl ) * NUM_OF_POLICY );
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_userpolicy_info ) + attrlen;
    
#else
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_userpolicy_info ) +
        sizeof( struct rtattr ) * 2 +
        sizeof( struct xfrm_user_tmpl ) * NUM_OF_POLICY;
#endif
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = XFRM_MSG_NEWPOLICY;

    policy = ( struct xfrm_userpolicy_info* ) NLMSG_DATA( nlh );
    memset( policy, 0, sizeof( struct xfrm_userpolicy_info ));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = ( struct nlattr* )( policy + 1 );
    nla->nla_len = nla_attr_size( sizeof( struct xfrm_user_tmpl ) *
        NUM_OF_POLICY );
    nla->nla_type = XFRMA_TMPL;

    ut = nla_data( nla );

#else
    attr = ( struct rtattr* )( policy + 1 );
    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_user_tmpl ) *
        NUM_OF_POLICY );
    attr->rta_type = XFRMA_TMPL;

    ut = RTA_DATA( attr );
#endif

#ifdef SIM_TRANSPORT
/* Transport ESP */
    memset( ut, 0, sizeof( struct xfrm_user_tmpl ));
    ut->id.daddr.a4 = TRANSPORT_ESP_OUT_SRC;
    ut->id.spi = TRANSPORT_ESP_SPI;
    ut->id.proto = IPPROTO_ESP;
#if 1
    ut->optional = 1;
#endif
    ut->saddr.a4 = TRANSPORT_ESP_OUT_DST;

/* Transport AH */
    ut++;
    memset( ut, 0, sizeof( struct xfrm_user_tmpl ));
    ut->id.daddr.a4 = TRANSPORT_AH_OUT_SRC;
    ut->id.spi = TRANSPORT_AH_SPI;
    ut->id.proto = IPPROTO_AH;
#if 1
    ut->optional = 1;
#endif
    ut->saddr.a4 = TRANSPORT_AH_OUT_DST;
    ut++;
#endif

#ifdef SIM_TUNNEL_ESP
/* Tunnel ESP */
    memset( ut, 0, sizeof( struct xfrm_user_tmpl ));
    ut->id.daddr.a4 = TUNNEL_ESP_OUT_SRC;
    ut->id.spi = 0;
    ut->id.proto = IPPROTO_ESP;
#if 1
    ut->optional = 1;
#endif
    ut->mode = 1;
    ut->saddr.a4 = TUNNEL_ESP_OUT_DST;
    ut++;
#endif

#ifdef SIM_TUNNEL_AH
/* Tunnel AH */
    memset( ut, 0, sizeof( struct xfrm_user_tmpl ));
    ut->id.daddr.a4 = TUNNEL_AH_OUT_SRC;
    ut->id.spi = 0;
    ut->id.proto = IPPROTO_AH;
#if 1
    ut->optional = 1;
#endif
    ut->mode = 1;
    ut->saddr.a4 = TUNNEL_AH_OUT_DST;
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);
    attr->rta_len = 0;
#endif

    policy->sel.family = AF_INET;
    policy->dir = XFRM_POLICY_IN;
    policy->action = XFRM_POLICY_ALLOW;
    policy->share = XFRM_SHARE_ANY;

    policy->lft.hard_byte_limit = 400000000;
    policy->lft.hard_packet_limit = 40000;
    policy->lft.soft_byte_limit = 200000000;
    policy->lft.soft_packet_limit = 20000;

    skb->len = nlh->nlmsg_len;

    if ( xfrm_user_rcv_msg( skb, nlh, &err ) )
        goto setup_error;
    skb->data = skb->tail;

/* Outbound Transport AH */
    nlh = ( struct nlmsghdr* ) skb->data;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    attrlen =
        nla_total_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) + attrlen;
    
#else
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) +
        sizeof( struct rtattr ) * 2 +
        sizeof( struct xfrm_algo ) + AH_KEY_SIZE;
#endif
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = XFRM_MSG_NEWSA;

    info = ( struct xfrm_usersa_info* ) NLMSG_DATA( nlh );
    memset( info, 0, sizeof( struct xfrm_usersa_info ));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = ( struct nlattr* )( info + 1 );
    nla->nla_len = nla_attr_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    nla->nla_type = XFRMA_ALG_AUTH;

    algp = nla_data( nla );

#else
    attr = ( struct rtattr* )( info + 1 );
    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    attr->rta_type = XFRMA_ALG_AUTH;

    algp = RTA_DATA( attr );
#endif
    strcpy( algp->alg_name, AH_ALG );
    algp->alg_key_len = AH_KEY_SIZE * 8;
    memcpy( algp->alg_key, ah_key, AH_KEY_SIZE );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);
    attr->rta_len = 0;
#endif

    info->family = AF_INET;

    info->id.daddr.a4 = TRANSPORT_AH_OUT_DST;
    info->id.spi = TRANSPORT_AH_SPI;
    info->id.proto = IPPROTO_AH;

    info->lft.hard_byte_limit = 400000000;
    info->lft.hard_packet_limit = 40000;
    info->lft.soft_byte_limit = 200000000;
    info->lft.soft_packet_limit = 20000;

    info->saddr.a4 = TRANSPORT_AH_OUT_SRC;

    skb->len = nlh->nlmsg_len;

#if 1
    if ( xfrm_user_rcv_msg( skb, nlh, &err ) )
        goto setup_error;
    skb->data = skb->tail;
#endif

/* Outbound Tunnel AH */
    nlh = ( struct nlmsghdr* ) skb->data;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    attrlen =
        nla_total_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) + attrlen;
    
#else
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) +
        sizeof( struct rtattr ) * 2 +
        sizeof( struct xfrm_algo ) + AH_KEY_SIZE;
#endif
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = XFRM_MSG_NEWSA;

    info = ( struct xfrm_usersa_info* ) NLMSG_DATA( nlh );
    memset( info, 0, sizeof( struct xfrm_usersa_info ));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = ( struct nlattr* )( info + 1 );
    nla->nla_len = nla_attr_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    nla->nla_type = XFRMA_ALG_AUTH;

    algp = nla_data( nla );

#else
    attr = ( struct rtattr* )( info + 1 );
    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    attr->rta_type = XFRMA_ALG_AUTH;

    algp = RTA_DATA( attr );
#endif
    strcpy( algp->alg_name, AH_ALG );
    algp->alg_key_len = AH_KEY_SIZE * 8;
    memcpy( algp->alg_key, ah_key, AH_KEY_SIZE );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);
    attr->rta_len = 0;
#endif

    info->family = AF_INET;
    info->mode = 1;

    info->id.daddr.a4 = TUNNEL_AH_OUT_DST;
    info->id.spi = TUNNEL_AH_SPI;
    info->id.proto = IPPROTO_AH;

    info->lft.hard_byte_limit = 400000000;
    info->lft.hard_packet_limit = 40000;
    info->lft.soft_byte_limit = 200000000;
    info->lft.soft_packet_limit = 20000;

    info->saddr.a4 = TUNNEL_AH_OUT_SRC;

    skb->len = nlh->nlmsg_len;

#if 1
    if ( xfrm_user_rcv_msg( skb, nlh, &err ) )
        goto setup_error;
    skb->data = skb->tail;
#endif

/* Outbound Transport ESP */
    nlh = ( struct nlmsghdr* ) skb->data;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    attrlen =
        nla_total_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE ) +
        nla_total_size( sizeof( struct xfrm_algo ) + ESP_KEY_SIZE );
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) + attrlen;
    
#else
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) +
        sizeof( struct rtattr ) * 3 +
        sizeof( struct xfrm_algo ) + AH_KEY_SIZE +
        sizeof( struct xfrm_algo ) + ESP_KEY_SIZE;
#endif
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = XFRM_MSG_NEWSA;

    info = ( struct xfrm_usersa_info* ) NLMSG_DATA( nlh );
    memset( info, 0, sizeof( struct xfrm_usersa_info ));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = ( struct nlattr* )( info + 1 );

    nla->nla_len = nla_attr_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    nla->nla_type = XFRMA_ALG_AUTH;
    algp = nla_data( nla );

#else
    attr = ( struct rtattr* )( info + 1 );

    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    attr->rta_type = XFRMA_ALG_AUTH;
    algp = RTA_DATA( attr );
#endif
    strcpy( algp->alg_name, AH_ALG );
    algp->alg_key_len = AH_KEY_SIZE * 8;
    memcpy( algp->alg_key, ah_key, AH_KEY_SIZE );

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = nla_next( nla, &attrlen );

    nla->nla_len = nla_attr_size( sizeof( struct xfrm_algo ) + ESP_KEY_SIZE );
    nla->nla_type = XFRMA_ALG_CRYPT;
    algp = nla_data( nla );

#else
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);

    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_algo ) + ESP_KEY_SIZE );
    attr->rta_type = XFRMA_ALG_CRYPT;
    algp = RTA_DATA( attr );
#endif
    strcpy( algp->alg_name, ESP_ALG );
    algp->alg_key_len = ESP_KEY_SIZE * 8;
    memcpy( algp->alg_key, alg_key, ESP_KEY_SIZE );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);
    attr->rta_len = 0;
#endif

    info->family = AF_INET;

    info->id.daddr.a4 = TRANSPORT_ESP_OUT_DST;
    info->id.spi = TRANSPORT_ESP_SPI;
    info->id.proto = IPPROTO_ESP;

    info->lft.hard_byte_limit = 400000000;
    info->lft.hard_packet_limit = 40000;
    info->lft.soft_byte_limit = 200000000;
    info->lft.soft_packet_limit = 20000;

    info->saddr.a4 = TRANSPORT_ESP_OUT_SRC;

    skb->len = nlh->nlmsg_len;

#if 1
    if ( xfrm_user_rcv_msg( skb, nlh, &err ) )
        goto setup_error;
    skb->data = skb->tail;
#endif

/* Outbound Tunnel ESP */
    nlh = ( struct nlmsghdr* ) skb->data;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    attrlen =
        nla_total_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE ) +
        nla_total_size( sizeof( struct xfrm_algo ) + ESP_KEY_SIZE );
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) + attrlen;
    
#else
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) +
        sizeof( struct rtattr ) * 3 +
        sizeof( struct xfrm_algo ) + AH_KEY_SIZE +
        sizeof( struct xfrm_algo ) + ESP_KEY_SIZE;
#endif
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = XFRM_MSG_NEWSA;

    info = ( struct xfrm_usersa_info* ) NLMSG_DATA( nlh );
    memset( info, 0, sizeof( struct xfrm_usersa_info ));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = ( struct nlattr* )( info + 1 );

    nla->nla_len = nla_attr_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    nla->nla_type = XFRMA_ALG_AUTH;
    algp = nla_data( nla );

#else
    attr = ( struct rtattr* )( info + 1 );

    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    attr->rta_type = XFRMA_ALG_AUTH;
    algp = RTA_DATA( attr );
#endif
    strcpy( algp->alg_name, AH_ALG );
    algp->alg_key_len = AH_KEY_SIZE * 8;
    memcpy( algp->alg_key, ah_key, AH_KEY_SIZE );

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = nla_next( nla, &attrlen );

    nla->nla_len = nla_attr_size( sizeof( struct xfrm_algo ) + ESP_KEY_SIZE );
    nla->nla_type = XFRMA_ALG_CRYPT;
    algp = nla_data( nla );

#else
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);

    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_algo ) + ESP_KEY_SIZE );
    attr->rta_type = XFRMA_ALG_CRYPT;
    algp = RTA_DATA( attr );
#endif
    strcpy( algp->alg_name, ESP_ALG );
    algp->alg_key_len = ESP_KEY_SIZE * 8;
    memcpy( algp->alg_key, alg_key, ESP_KEY_SIZE );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);
    attr->rta_len = 0;
#endif

    info->family = AF_INET;
    info->mode = 1;

    info->id.daddr.a4 = TUNNEL_ESP_OUT_DST;
    info->id.spi = TUNNEL_ESP_SPI;
    info->id.proto = IPPROTO_ESP;

    info->lft.hard_byte_limit = 400000000;
    info->lft.hard_packet_limit = 40000;
    info->lft.soft_byte_limit = 200000000;
    info->lft.soft_packet_limit = 20000;

    info->saddr.a4 = TUNNEL_ESP_OUT_SRC;

    skb->len = nlh->nlmsg_len;

#if 1
    if ( xfrm_user_rcv_msg( skb, nlh, &err ) )
        goto setup_error;
    skb->data = skb->tail;
#endif

/* Inbound Transport AH */
    nlh = ( struct nlmsghdr* ) skb->data;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    attrlen =
        nla_total_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) + attrlen;
    
#else
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) +
        sizeof( struct rtattr ) * 2 +
        sizeof( struct xfrm_algo ) + AH_KEY_SIZE;
#endif
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = XFRM_MSG_NEWSA;

    info = ( struct xfrm_usersa_info* ) NLMSG_DATA( nlh );
    memset( info, 0, sizeof( struct xfrm_usersa_info ));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = ( struct nlattr* )( info + 1 );
    nla->nla_len = nla_attr_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    nla->nla_type = XFRMA_ALG_AUTH;

    algp = nla_data( nla );

#else
    attr = ( struct rtattr* )( info + 1 );
    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    attr->rta_type = XFRMA_ALG_AUTH;

    algp = RTA_DATA( attr );
#endif
    strcpy( algp->alg_name, AH_ALG );
    algp->alg_key_len = AH_KEY_SIZE * 8;
    memcpy( algp->alg_key, ah_key, AH_KEY_SIZE );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);
    attr->rta_len = 0;
#endif

    info->family = AF_INET;
#if 0
    info->replay_window = 1;
#endif

    info->id.daddr.a4 = TRANSPORT_AH_OUT_SRC;
    info->id.spi = TRANSPORT_AH_SPI;
    info->id.proto = IPPROTO_AH;

    info->lft.hard_byte_limit = 400000000;
    info->lft.hard_packet_limit = 40000;
    info->lft.soft_byte_limit = 200000000;
    info->lft.soft_packet_limit = 20000;

    info->saddr.a4 = TRANSPORT_AH_OUT_DST;

    skb->len = nlh->nlmsg_len;

#if 1
    if ( xfrm_user_rcv_msg( skb, nlh, &err ) )
        goto setup_error;
    skb->data = skb->tail;
#endif

/* Inbound Tunnel AH */
    nlh = ( struct nlmsghdr* ) skb->data;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    attrlen =
        nla_total_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) + attrlen;
    
#else
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) +
        sizeof( struct rtattr ) * 2 +
        sizeof( struct xfrm_algo ) + AH_KEY_SIZE;
#endif
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = XFRM_MSG_NEWSA;

    info = ( struct xfrm_usersa_info* ) NLMSG_DATA( nlh );
    memset( info, 0, sizeof( struct xfrm_usersa_info ));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = ( struct nlattr* )( info + 1 );
    nla->nla_len = nla_attr_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    nla->nla_type = XFRMA_ALG_AUTH;

    algp = nla_data( nla );

#else
    attr = ( struct rtattr* )( info + 1 );
    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    attr->rta_type = XFRMA_ALG_AUTH;

    algp = RTA_DATA( attr );
#endif
    strcpy( algp->alg_name, AH_ALG );
    algp->alg_key_len = AH_KEY_SIZE * 8;
    memcpy( algp->alg_key, ah_key, AH_KEY_SIZE );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);
    attr->rta_len = 0;
#endif

    info->family = AF_INET;
    info->mode = 1;
#if 0
    info->replay_window = 1;
#endif

    info->id.daddr.a4 = TUNNEL_AH_OUT_SRC;
    info->id.spi = TUNNEL_AH_SPI;
    info->id.proto = IPPROTO_AH;

    info->lft.hard_byte_limit = 400000000;
    info->lft.hard_packet_limit = 40000;
    info->lft.soft_byte_limit = 200000000;
    info->lft.soft_packet_limit = 20000;

    info->saddr.a4 = TUNNEL_AH_OUT_DST;

    skb->len = nlh->nlmsg_len;

#if 1
    if ( xfrm_user_rcv_msg( skb, nlh, &err ) )
        goto setup_error;
    skb->data = skb->tail;
#endif

/* Inbound Transport ESP */
    nlh = ( struct nlmsghdr* ) skb->data;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    attrlen =
        nla_total_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE ) +
        nla_total_size( sizeof( struct xfrm_algo ) + ESP_KEY_SIZE );
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) + attrlen;
    
#else
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) +
        sizeof( struct rtattr ) * 3 +
        sizeof( struct xfrm_algo ) + AH_KEY_SIZE +
        sizeof( struct xfrm_algo ) + ESP_KEY_SIZE;
#endif
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = XFRM_MSG_NEWSA;

    info = ( struct xfrm_usersa_info* ) NLMSG_DATA( nlh );
    memset( info, 0, sizeof( struct xfrm_usersa_info ));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = ( struct nlattr* )( info + 1 );

    nla->nla_len = nla_attr_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    nla->nla_type = XFRMA_ALG_AUTH;
    algp = nla_data( nla );

#else
    attr = ( struct rtattr* )( info + 1 );

    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    attr->rta_type = XFRMA_ALG_AUTH;
    algp = RTA_DATA( attr );
#endif
    strcpy( algp->alg_name, AH_ALG );
    algp->alg_key_len = AH_KEY_SIZE * 8;
    memcpy( algp->alg_key, ah_key, AH_KEY_SIZE );

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = nla_next( nla, &attrlen );

    nla->nla_len = nla_attr_size( sizeof( struct xfrm_algo ) + ESP_KEY_SIZE );
    nla->nla_type = XFRMA_ALG_CRYPT;
    algp = nla_data( nla );

#else
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);

    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_algo ) + ESP_KEY_SIZE );
    attr->rta_type = XFRMA_ALG_CRYPT;
    algp = RTA_DATA( attr );
#endif
    strcpy( algp->alg_name, ESP_ALG );
    algp->alg_key_len = ESP_KEY_SIZE * 8;
    memcpy( algp->alg_key, alg_key, ESP_KEY_SIZE );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);
    attr->rta_len = 0;
#endif

    info->family = AF_INET;
#if 0
    info->replay_window = 1;
#endif

    info->id.daddr.a4 = TRANSPORT_ESP_OUT_SRC;
    info->id.spi = TRANSPORT_ESP_SPI;
    info->id.proto = IPPROTO_ESP;

    info->lft.hard_byte_limit = 400000000;
    info->lft.hard_packet_limit = 40000;
    info->lft.soft_byte_limit = 200000000;
    info->lft.soft_packet_limit = 20000;

    info->saddr.a4 = TRANSPORT_ESP_OUT_DST;

    skb->len = nlh->nlmsg_len;

#if 1
    if ( xfrm_user_rcv_msg( skb, nlh, &err ) )
        goto setup_error;
    skb->data = skb->tail;
#endif

/* Inbound Tunnel ESP */
    nlh = ( struct nlmsghdr* ) skb->data;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    attrlen =
        nla_total_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE ) +
        nla_total_size( sizeof( struct xfrm_algo ) + ESP_KEY_SIZE );
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) + attrlen;
    
#else
    nlh->nlmsg_len = sizeof( struct nlmsghdr ) +
        sizeof( struct xfrm_usersa_info ) +
        sizeof( struct rtattr ) * 3 +
        sizeof( struct xfrm_algo ) + AH_KEY_SIZE +
        sizeof( struct xfrm_algo ) + ESP_KEY_SIZE;
#endif
    nlh->nlmsg_flags = NLM_F_REQUEST;
    nlh->nlmsg_type = XFRM_MSG_NEWSA;

    info = ( struct xfrm_usersa_info* ) NLMSG_DATA( nlh );
    memset( info, 0, sizeof( struct xfrm_usersa_info ));

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = ( struct nlattr* )( info + 1 );

    nla->nla_len = nla_attr_size( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    nla->nla_type = XFRMA_ALG_AUTH;
    algp = nla_data( nla );

#else
    attr = ( struct rtattr* )( info + 1 );

    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_algo ) + AH_KEY_SIZE );
    attr->rta_type = XFRMA_ALG_AUTH;
    algp = RTA_DATA( attr );
#endif
    strcpy( algp->alg_name, AH_ALG );
    algp->alg_key_len = AH_KEY_SIZE * 8;
    memcpy( algp->alg_key, ah_key, AH_KEY_SIZE );

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
    nla = nla_next( nla, &attrlen );

    nla->nla_len = nla_attr_size( sizeof( struct xfrm_algo ) + ESP_KEY_SIZE );
    nla->nla_type = XFRMA_ALG_CRYPT;
    algp = nla_data( nla );

#else
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);

    attr->rta_len = RTA_LENGTH( sizeof( struct xfrm_algo ) + ESP_KEY_SIZE );
    attr->rta_type = XFRMA_ALG_CRYPT;
    algp = RTA_DATA( attr );
#endif
    strcpy( algp->alg_name, ESP_ALG );
    algp->alg_key_len = ESP_KEY_SIZE * 8;
    memcpy( algp->alg_key, alg_key, ESP_KEY_SIZE );

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
    attrlen = attr->rta_len;
    attr = RTA_NEXT(attr, attrlen);
    attr->rta_len = 0;
#endif

    info->family = AF_INET;
    info->mode = 1;
#if 0
    info->replay_window = 1;
#endif

    info->id.daddr.a4 = TUNNEL_ESP_OUT_SRC;
    info->id.spi = TUNNEL_ESP_SPI;
    info->id.proto = IPPROTO_ESP;

    info->lft.hard_byte_limit = 400000000;
    info->lft.hard_packet_limit = 40000;
    info->lft.soft_byte_limit = 200000000;
    info->lft.soft_packet_limit = 20000;

    info->saddr.a4 = TUNNEL_ESP_OUT_DST;

    skb->len = nlh->nlmsg_len;

#if 1
    if ( xfrm_user_rcv_msg( skb, nlh, &err ) )
        goto setup_error;
    skb->data = skb->tail;
#endif

setup_error:
    dev_kfree_skb( skb );
	if (ahp) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
	    kfree( ahp->key );
#endif
		if (ahp->work_icv)
			kfree(ahp->work_icv);
		if (ahp->tfm)
			crypto_free_hash(ahp->tfm);
		kfree(ahp);
	}
}
