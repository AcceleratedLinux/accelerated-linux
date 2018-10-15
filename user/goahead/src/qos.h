#ifndef __QOS_H__
#define __QOS_H__


#define QOS_MANGLE_CHAIN	"qos_mangle_chain"
#define QOS_REMARK_CHAIN	"qos_remark_chain"

#define CLASSIFIER_RULE		0
#define REMARK_RULE			1
#define RETURN_RULE			2

#define HARD_LIMIT_RATE_MIN			2.0			// kbits

#define AF1_MAX_LATENCY				500			// ms
#define AF2_MAX_LATENCY				500			// ms
#define AF3_MAX_LATENCY				500			// ms
#define AF4_MAX_LATENCY				500			// ms
#define AF5_MAX_LATENCY				300			// ms (EF)
#define AF6_MAX_LATENCY				500			// ms (BE)

struct entry_s{
	char *name;
	char *value;
};

#define QOS_PROFILE_ENTRYS_MAX 16

void formDefineQoS(void);
inline void QoSInit(void);



#endif
