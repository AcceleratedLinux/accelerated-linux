// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) 2019 Facebook */

#include <linux/bpf.h>
#include <netinet/in.h>
#include <stdbool.h>

#include "bpf_helpers.h"
#include "bpf_endian.h"

enum bpf_addr_array_idx {
	ADDR_SRV_IDX,
	ADDR_CLI_IDX,
	__NR_BPF_ADDR_ARRAY_IDX,
};

enum bpf_result_array_idx {
	EGRESS_SRV_IDX,
	EGRESS_CLI_IDX,
	INGRESS_LISTEN_IDX,
	__NR_BPF_RESULT_ARRAY_IDX,
};

enum bpf_linum_array_idx {
	EGRESS_LINUM_IDX,
	INGRESS_LINUM_IDX,
	__NR_BPF_LINUM_ARRAY_IDX,
};

struct bpf_map_def SEC("maps") addr_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(__u32),
	.value_size = sizeof(struct sockaddr_in6),
	.max_entries = __NR_BPF_ADDR_ARRAY_IDX,
};

struct bpf_map_def SEC("maps") sock_result_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(__u32),
	.value_size = sizeof(struct bpf_sock),
	.max_entries = __NR_BPF_RESULT_ARRAY_IDX,
};

struct bpf_map_def SEC("maps") tcp_sock_result_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(__u32),
	.value_size = sizeof(struct bpf_tcp_sock),
	.max_entries = __NR_BPF_RESULT_ARRAY_IDX,
};

struct bpf_map_def SEC("maps") linum_map = {
	.type = BPF_MAP_TYPE_ARRAY,
	.key_size = sizeof(__u32),
	.value_size = sizeof(__u32),
	.max_entries = __NR_BPF_LINUM_ARRAY_IDX,
};

static bool is_loopback6(__u32 *a6)
{
	return !a6[0] && !a6[1] && !a6[2] && a6[3] == bpf_htonl(1);
}

static void skcpy(struct bpf_sock *dst,
		  const struct bpf_sock *src)
{
	dst->bound_dev_if = src->bound_dev_if;
	dst->family = src->family;
	dst->type = src->type;
	dst->protocol = src->protocol;
	dst->mark = src->mark;
	dst->priority = src->priority;
	dst->src_ip4 = src->src_ip4;
	dst->src_ip6[0] = src->src_ip6[0];
	dst->src_ip6[1] = src->src_ip6[1];
	dst->src_ip6[2] = src->src_ip6[2];
	dst->src_ip6[3] = src->src_ip6[3];
	dst->src_port = src->src_port;
	dst->dst_ip4 = src->dst_ip4;
	dst->dst_ip6[0] = src->dst_ip6[0];
	dst->dst_ip6[1] = src->dst_ip6[1];
	dst->dst_ip6[2] = src->dst_ip6[2];
	dst->dst_ip6[3] = src->dst_ip6[3];
	dst->dst_port = src->dst_port;
	dst->state = src->state;
}

static void tpcpy(struct bpf_tcp_sock *dst,
		  const struct bpf_tcp_sock *src)
{
	dst->snd_cwnd = src->snd_cwnd;
	dst->srtt_us = src->srtt_us;
	dst->rtt_min = src->rtt_min;
	dst->snd_ssthresh = src->snd_ssthresh;
	dst->rcv_nxt = src->rcv_nxt;
	dst->snd_nxt = src->snd_nxt;
	dst->snd_una = src->snd_una;
	dst->mss_cache = src->mss_cache;
	dst->ecn_flags = src->ecn_flags;
	dst->rate_delivered = src->rate_delivered;
	dst->rate_interval_us = src->rate_interval_us;
	dst->packets_out = src->packets_out;
	dst->retrans_out = src->retrans_out;
	dst->total_retrans = src->total_retrans;
	dst->segs_in = src->segs_in;
	dst->data_segs_in = src->data_segs_in;
	dst->segs_out = src->segs_out;
	dst->data_segs_out = src->data_segs_out;
	dst->lost_out = src->lost_out;
	dst->sacked_out = src->sacked_out;
	dst->bytes_received = src->bytes_received;
	dst->bytes_acked = src->bytes_acked;
}

#define RETURN {						\
	linum = __LINE__;					\
	bpf_map_update_elem(&linum_map, &linum_idx, &linum, 0);	\
	return 1;						\
}

SEC("cgroup_skb/egress")
int egress_read_sock_fields(struct __sk_buff *skb)
{
	__u32 srv_idx = ADDR_SRV_IDX, cli_idx = ADDR_CLI_IDX, result_idx;
	struct sockaddr_in6 *srv_sa6, *cli_sa6;
	struct bpf_tcp_sock *tp, *tp_ret;
	struct bpf_sock *sk, *sk_ret;
	__u32 linum, linum_idx;

	linum_idx = EGRESS_LINUM_IDX;

	sk = skb->sk;
	if (!sk || sk->state == 10)
		RETURN;

	sk = bpf_sk_fullsock(sk);
	if (!sk || sk->family != AF_INET6 || sk->protocol != IPPROTO_TCP ||
	    !is_loopback6(sk->src_ip6))
		RETURN;

	tp = bpf_tcp_sock(sk);
	if (!tp)
		RETURN;

	srv_sa6 = bpf_map_lookup_elem(&addr_map, &srv_idx);
	cli_sa6 = bpf_map_lookup_elem(&addr_map, &cli_idx);
	if (!srv_sa6 || !cli_sa6)
		RETURN;

	if (sk->src_port == bpf_ntohs(srv_sa6->sin6_port))
		result_idx = EGRESS_SRV_IDX;
	else if (sk->src_port == bpf_ntohs(cli_sa6->sin6_port))
		result_idx = EGRESS_CLI_IDX;
	else
		RETURN;

	sk_ret = bpf_map_lookup_elem(&sock_result_map, &result_idx);
	tp_ret = bpf_map_lookup_elem(&tcp_sock_result_map, &result_idx);
	if (!sk_ret || !tp_ret)
		RETURN;

	skcpy(sk_ret, sk);
	tpcpy(tp_ret, tp);

	RETURN;
}

SEC("cgroup_skb/ingress")
int ingress_read_sock_fields(struct __sk_buff *skb)
{
	__u32 srv_idx = ADDR_SRV_IDX, result_idx = INGRESS_LISTEN_IDX;
	struct bpf_tcp_sock *tp, *tp_ret;
	struct bpf_sock *sk, *sk_ret;
	struct sockaddr_in6 *srv_sa6;
	__u32 linum, linum_idx;

	linum_idx = INGRESS_LINUM_IDX;

	sk = skb->sk;
	if (!sk || sk->family != AF_INET6 || !is_loopback6(sk->src_ip6))
		RETURN;

	srv_sa6 = bpf_map_lookup_elem(&addr_map, &srv_idx);
	if (!srv_sa6 || sk->src_port != bpf_ntohs(srv_sa6->sin6_port))
		RETURN;

	if (sk->state != 10 && sk->state != 12)
		RETURN;

	sk = bpf_get_listener_sock(sk);
	if (!sk)
		RETURN;

	tp = bpf_tcp_sock(sk);
	if (!tp)
		RETURN;

	sk_ret = bpf_map_lookup_elem(&sock_result_map, &result_idx);
	tp_ret = bpf_map_lookup_elem(&tcp_sock_result_map, &result_idx);
	if (!sk_ret || !tp_ret)
		RETURN;

	skcpy(sk_ret, sk);
	tpcpy(tp_ret, tp);

	RETURN;
}

char _license[] SEC("license") = "GPL";
