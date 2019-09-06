/*
 * Copyright (c) 2014 Arturo Borrero Gonzalez <arturo@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>
#include <net/netfilter/nf_tables.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/ipv4/nf_nat_masquerade.h>
#include <net/netfilter/ipv6/nf_nat_masquerade.h>

struct nft_masq {
	u32			flags;
	enum nft_registers      sreg_proto_min:8;
	enum nft_registers      sreg_proto_max:8;
};

static const struct nla_policy nft_masq_policy[NFTA_MASQ_MAX + 1] = {
	[NFTA_MASQ_FLAGS]		= { .type = NLA_U32 },
	[NFTA_MASQ_REG_PROTO_MIN]	= { .type = NLA_U32 },
	[NFTA_MASQ_REG_PROTO_MAX]	= { .type = NLA_U32 },
};

static int nft_masq_validate(const struct nft_ctx *ctx,
			     const struct nft_expr *expr,
			     const struct nft_data **data)
{
	int err;

	err = nft_chain_validate_dependency(ctx->chain, NFT_CHAIN_T_NAT);
	if (err < 0)
		return err;

	return nft_chain_validate_hooks(ctx->chain,
				        (1 << NF_INET_POST_ROUTING));
}

static int nft_masq_init(const struct nft_ctx *ctx,
			 const struct nft_expr *expr,
			 const struct nlattr * const tb[])
{
	u32 plen = FIELD_SIZEOF(struct nf_nat_range, min_addr.all);
	struct nft_masq *priv = nft_expr_priv(expr);
	int err;

	if (tb[NFTA_MASQ_FLAGS]) {
		priv->flags = ntohl(nla_get_be32(tb[NFTA_MASQ_FLAGS]));
		if (priv->flags & ~NF_NAT_RANGE_MASK)
			return -EINVAL;
	}

	if (tb[NFTA_MASQ_REG_PROTO_MIN]) {
		priv->sreg_proto_min =
			nft_parse_register(tb[NFTA_MASQ_REG_PROTO_MIN]);

		err = nft_validate_register_load(priv->sreg_proto_min, plen);
		if (err < 0)
			return err;

		if (tb[NFTA_MASQ_REG_PROTO_MAX]) {
			priv->sreg_proto_max =
				nft_parse_register(tb[NFTA_MASQ_REG_PROTO_MAX]);

			err = nft_validate_register_load(priv->sreg_proto_max,
							 plen);
			if (err < 0)
				return err;
		} else {
			priv->sreg_proto_max = priv->sreg_proto_min;
		}
	}

	return nf_ct_netns_get(ctx->net, ctx->family);
}

static int nft_masq_dump(struct sk_buff *skb, const struct nft_expr *expr)
{
	const struct nft_masq *priv = nft_expr_priv(expr);

	if (priv->flags != 0 &&
	    nla_put_be32(skb, NFTA_MASQ_FLAGS, htonl(priv->flags)))
		goto nla_put_failure;

	if (priv->sreg_proto_min) {
		if (nft_dump_register(skb, NFTA_MASQ_REG_PROTO_MIN,
				      priv->sreg_proto_min) ||
		    nft_dump_register(skb, NFTA_MASQ_REG_PROTO_MAX,
				      priv->sreg_proto_max))
			goto nla_put_failure;
	}

	return 0;

nla_put_failure:
	return -1;
}

static void nft_masq_ipv4_eval(const struct nft_expr *expr,
			       struct nft_regs *regs,
			       const struct nft_pktinfo *pkt)
{
	struct nft_masq *priv = nft_expr_priv(expr);
	struct nf_nat_range2 range;

	memset(&range, 0, sizeof(range));
	range.flags = priv->flags;
	if (priv->sreg_proto_min) {
		range.min_proto.all = (__force __be16)nft_reg_load16(
			&regs->data[priv->sreg_proto_min]);
		range.max_proto.all = (__force __be16)nft_reg_load16(
			&regs->data[priv->sreg_proto_max]);
	}
	regs->verdict.code = nf_nat_masquerade_ipv4(pkt->skb, nft_hook(pkt),
						    &range, nft_out(pkt));
}

static void
nft_masq_ipv4_destroy(const struct nft_ctx *ctx, const struct nft_expr *expr)
{
	nf_ct_netns_put(ctx->net, NFPROTO_IPV4);
}

static struct nft_expr_type nft_masq_ipv4_type;
static const struct nft_expr_ops nft_masq_ipv4_ops = {
	.type		= &nft_masq_ipv4_type,
	.size		= NFT_EXPR_SIZE(sizeof(struct nft_masq)),
	.eval		= nft_masq_ipv4_eval,
	.init		= nft_masq_init,
	.destroy	= nft_masq_ipv4_destroy,
	.dump		= nft_masq_dump,
	.validate	= nft_masq_validate,
};

static struct nft_expr_type nft_masq_ipv4_type __read_mostly = {
	.family		= NFPROTO_IPV4,
	.name		= "masq",
	.ops		= &nft_masq_ipv4_ops,
	.policy		= nft_masq_policy,
	.maxattr	= NFTA_MASQ_MAX,
	.owner		= THIS_MODULE,
};

#ifdef CONFIG_NF_TABLES_IPV6
static void nft_masq_ipv6_eval(const struct nft_expr *expr,
			       struct nft_regs *regs,
			       const struct nft_pktinfo *pkt)
{
	struct nft_masq *priv = nft_expr_priv(expr);
	struct nf_nat_range2 range;

	memset(&range, 0, sizeof(range));
	range.flags = priv->flags;
	if (priv->sreg_proto_min) {
		range.min_proto.all = (__force __be16)nft_reg_load16(
			&regs->data[priv->sreg_proto_min]);
		range.max_proto.all = (__force __be16)nft_reg_load16(
			&regs->data[priv->sreg_proto_max]);
	}
	regs->verdict.code = nf_nat_masquerade_ipv6(pkt->skb, &range,
						    nft_out(pkt));
}

static void
nft_masq_ipv6_destroy(const struct nft_ctx *ctx, const struct nft_expr *expr)
{
	nf_ct_netns_put(ctx->net, NFPROTO_IPV6);
}

static struct nft_expr_type nft_masq_ipv6_type;
static const struct nft_expr_ops nft_masq_ipv6_ops = {
	.type		= &nft_masq_ipv6_type,
	.size		= NFT_EXPR_SIZE(sizeof(struct nft_masq)),
	.eval		= nft_masq_ipv6_eval,
	.init		= nft_masq_init,
	.destroy	= nft_masq_ipv6_destroy,
	.dump		= nft_masq_dump,
	.validate	= nft_masq_validate,
};

static struct nft_expr_type nft_masq_ipv6_type __read_mostly = {
	.family		= NFPROTO_IPV6,
	.name		= "masq",
	.ops		= &nft_masq_ipv6_ops,
	.policy		= nft_masq_policy,
	.maxattr	= NFTA_MASQ_MAX,
	.owner		= THIS_MODULE,
};

static int __init nft_masq_module_init_ipv6(void)
{
	int ret = nft_register_expr(&nft_masq_ipv6_type);

	if (ret)
		return ret;

	ret = nf_nat_masquerade_ipv6_register_notifier();
	if (ret < 0)
		nft_unregister_expr(&nft_masq_ipv6_type);

	return ret;
}

static void nft_masq_module_exit_ipv6(void)
{
	nft_unregister_expr(&nft_masq_ipv6_type);
	nf_nat_masquerade_ipv6_unregister_notifier();
}
#else
static inline int nft_masq_module_init_ipv6(void) { return 0; }
static inline void nft_masq_module_exit_ipv6(void) {}
#endif

static int __init nft_masq_module_init(void)
{
	int ret;

	ret = nft_masq_module_init_ipv6();
	if (ret < 0)
		return ret;

	ret = nft_register_expr(&nft_masq_ipv4_type);
	if (ret < 0) {
		nft_masq_module_exit_ipv6();
		return ret;
	}

	ret = nf_nat_masquerade_ipv4_register_notifier();
	if (ret < 0) {
		nft_masq_module_exit_ipv6();
		nft_unregister_expr(&nft_masq_ipv4_type);
		return ret;
	}

	return ret;
}

static void __exit nft_masq_module_exit(void)
{
	nft_masq_module_exit_ipv6();
	nft_unregister_expr(&nft_masq_ipv4_type);
	nf_nat_masquerade_ipv4_unregister_notifier();
}

module_init(nft_masq_module_init);
module_exit(nft_masq_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arturo Borrero Gonzalez <arturo@debian.org>");
MODULE_ALIAS_NFT_AF_EXPR(AF_INET6, "masq");
MODULE_ALIAS_NFT_AF_EXPR(AF_INET, "masq");
