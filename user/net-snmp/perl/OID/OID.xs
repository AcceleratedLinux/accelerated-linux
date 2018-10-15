/* -*- C -*- */
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

/* pulled from Dave's, yet-to-be-used, net-snmp library rewrite.
   autocompatibility for the future? */

typedef struct netsnmp_oid_s {
    oid                 *name;
    size_t               len;
    oid                  namebuf[ MAX_OID_LEN ];
} netsnmp_oid;

static int
not_here(char *s)
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant(char *name, int len, int arg)
{
    errno = EINVAL;
    return 0;
}

netsnmp_oid *
nso_newarrayptr(oid *name, size_t name_len) 
{
    netsnmp_oid *RETVAL;
    RETVAL = SNMP_MALLOC_TYPEDEF(netsnmp_oid);
    RETVAL->name = RETVAL->namebuf;
    RETVAL->len = name_len;
    memcpy(RETVAL->name, name, name_len * sizeof(oid));
    return RETVAL;
}

static int __sprint_num_objid _((char *, oid *, int));

/* stolen from SNMP.xs.  Ug, this needs merging to snmplib */
/* XXX: this is only here because snmplib forces quotes around the
   data and won't return real binary data or a numeric string.  Every
   app must do its own switch() to get around it.  Ug. */
#define USE_BASIC 0
#define USE_ENUMS 1
#define USE_SPRINT_VALUE 2
static int
__snprint_value (buf, buf_len, var, tp, type, flag)
char * buf;
size_t buf_len;
netsnmp_variable_list * var;
struct tree * tp;
int type;
int flag;
{
   int len = 0;
   u_char* ip;
   struct enum_list *ep;


   buf[0] = '\0';
   if (flag == USE_SPRINT_VALUE) {
	snprint_value(buf, buf_len, var->name, var->name_length, var);
	len = strlen(buf);
   } else {
     switch (var->type) {
        case ASN_INTEGER:
           if (flag == USE_ENUMS) {
              for(ep = tp->enums; ep; ep = ep->next) {
                 if (ep->value == *var->val.integer) {
                    strcpy(buf, ep->label);
                    len = strlen(buf);
                    break;
                 }
              }
           }
           if (!len) {
              sprintf(buf,"%ld", *var->val.integer);
              len = strlen(buf);
           }
           break;

        case ASN_GAUGE:
        case ASN_COUNTER:
        case ASN_TIMETICKS:
        case ASN_UINTEGER:
           sprintf(buf,"%lu", (unsigned long) *var->val.integer);
           len = strlen(buf);
           break;

        case ASN_OCTET_STR:
        case ASN_OPAQUE:
           memcpy(buf, (char*)var->val.string, var->val_len);
           len = var->val_len;
           break;

        case ASN_IPADDRESS:
          ip = (u_char*)var->val.string;
          sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
          len = strlen(buf);
          break;

        case ASN_NULL:
           break;

        case ASN_OBJECT_ID:
          __sprint_num_objid(buf, (oid *)(var->val.objid),
                             var->val_len/sizeof(oid));
          len = strlen(buf);
          break;

	case SNMP_ENDOFMIBVIEW:
          sprintf(buf,"%s", "ENDOFMIBVIEW");
	  break;
	case SNMP_NOSUCHOBJECT:
	  sprintf(buf,"%s", "NOSUCHOBJECT");
	  break;
	case SNMP_NOSUCHINSTANCE:
	  sprintf(buf,"%s", "NOSUCHINSTANCE");
	  break;

        case ASN_COUNTER64:
          printU64(buf,(struct counter64 *)var->val.counter64);
          len = strlen(buf);
          break;

        case ASN_BIT_STR:
            snprint_bitstring(buf, sizeof(buf), var, NULL, NULL, NULL);
            len = strlen(buf);
            break;

        case ASN_NSAP:
        default:
           warn("snprint_value: asn type not handled %d\n",var->type);
     }
   }
   return(len);
}

static int
__sprint_num_objid (buf, objid, len)
char *buf;
oid *objid;
int len;
{
   int i;
   buf[0] = '\0';
   for (i=0; i < len; i++) {
	sprintf(buf,".%lu",*objid++);
	buf += strlen(buf);
   }
   return SNMPERR_SUCCESS;
}

static int
__tp_sprint_num_objid (buf, tp)
char *buf;
struct tree *tp;
{
   oid newname[MAX_OID_LEN], *op;
   /* code taken from get_node in snmp_client.c */
   for (op = newname + MAX_OID_LEN - 1; op >= newname; op--) {
      *op = tp->subid;
      tp = tp->parent;
      if (tp == NULL) break;
   }
   return __sprint_num_objid(buf, op, newname + MAX_OID_LEN - op);
}

MODULE = NetSNMP::OID		PACKAGE = NetSNMP::OID		PREFIX=nso_

netsnmp_oid *
nso_newptr(initstring)
    char *initstring
    CODE:
        if (get_tree_head() == NULL)
            netsnmp_init_mib();
        RETVAL = SNMP_MALLOC_TYPEDEF(netsnmp_oid);
        RETVAL->name = RETVAL->namebuf;
        RETVAL->len = sizeof(RETVAL->namebuf)/sizeof(RETVAL->namebuf[0]);
        if (!snmp_parse_oid(initstring, (oid *) RETVAL->name, &RETVAL->len)) {
            snmp_log(LOG_ERR, "Can't parse: %s\n", initstring);
            RETVAL->len = 0;
            RETVAL = NULL;
        }
    OUTPUT:
        RETVAL

double
constant(sv,arg)
    PREINIT:
	STRLEN		len;
    INPUT:
	SV *		sv
	char *		s = SvPV(sv, len);
	int		arg
    CODE:
	RETVAL = constant(s,len,arg);
    OUTPUT:
	RETVAL

int
_snmp_oid_compare(oid1, oid2)
    netsnmp_oid *oid1;
    netsnmp_oid *oid2;
    CODE:
        RETVAL = snmp_oid_compare((oid *) oid1->name, oid1->len,
                                  (oid *) oid2->name, oid2->len);
    OUTPUT:
        RETVAL

MODULE = NetSNMP::OID  PACKAGE = netsnmp_oidPtr  PREFIX = nsop_

void
nsop_DESTROY(oid1)
	netsnmp_oid *oid1
    CODE:
{
    if (oid1->name != oid1->namebuf) {
	free(oid1->name);
    }
    free(oid1);
}
        
char *
nsop_to_string(oid1)
    	netsnmp_oid *oid1
    PREINIT:
        static char mystr[SNMP_MAXBUF];
    CODE:
        {
            if (oid1->len == 0)
                snprintf(mystr, sizeof(mystr), "Illegal OID");
            else
                snprint_objid(mystr, sizeof(mystr),
                              (oid *) oid1->name, oid1->len);
            RETVAL = mystr;
        }
                
    OUTPUT:
        RETVAL

void
nsop_to_array(oid1)
    netsnmp_oid *oid1;
    PREINIT:
        int i;

    PPCODE:
        EXTEND(SP, oid1->len);
        for(i=0; i < (int)oid1->len; i++) {
            PUSHs(sv_2mortal(newSVnv(oid1->name[i])));
        }

SV *
nsop_get_indexes(oid1)
        netsnmp_oid *oid1;
    PREINIT:
        int i, nodecount;
        struct tree    *tp, *tpe, *tpnode, *indexnode;
        struct index_list *index;
        netsnmp_variable_list vbdata;
        u_char         *buf = NULL;
        size_t          buf_len = 256, out_len = 0;
        oid name[MAX_OID_LEN];
        size_t name_len = MAX_OID_LEN;
        oid *oidp;
        size_t oidp_len;
        AV *myret;
        int is_private;

    CODE:
        {
            memset(&vbdata, 0, sizeof(vbdata));
            if (NULL == (tp = get_tree(oid1->name, oid1->len,
                                       get_tree_head()))) {
                RETVAL = NULL;
                return;
            }
                
            if ((buf = (u_char *) calloc(buf_len, 1)) == NULL) {
                RETVAL = NULL;
                return;
            }

            nodecount = 0;
            for(tpnode = tp; tpnode; tpnode = tpnode->parent) {
                nodecount++;
                if (nodecount == 2)
                    tpe = tpnode;
                if (nodecount == 3 &&
                    (strlen(tpnode->label) < 6 ||
                     strcmp(tpnode->label + strlen(tpnode->label) - 5,
                            "Table"))) {
                    /* we're not within a table.  bad logic, little choice */
                    RETVAL = NULL;
                    return;
                }
            }

            if (tpe->augments && strlen(tpe->augments) > 0) {
                /* we're augmenting another table, so use that entry instead */
                if (!snmp_parse_oid(tpe->augments, name, &name_len) ||
                    (NULL ==
                     (tpe = get_tree(name, name_len,
                                     get_tree_head())))) {
                    RETVAL = NULL;
                    return; /* XXX: better error recovery needed? */
                }
            }
            
            i = 0;
            for(index = tpe->indexes; index; index = index->next) {
                i++;
            }

            myret = (AV *) sv_2mortal((SV *) newAV());

            oidp = oid1->name + nodecount;
            oidp_len = oid1->len - nodecount;

            for(index = tpe->indexes; index; index = index->next) {
                /* XXX: NOT efficient! */
                name_len = MAX_OID_LEN;
                if (!snmp_parse_oid(index->ilabel, name, &name_len) ||
                    (NULL ==
                     (indexnode = get_tree(name, name_len,
                                           get_tree_head())))) {
                    RETVAL = NULL;
                    return;             /* xxx mem leak */
                }
                vbdata.type = mib_to_asn_type(indexnode->type);

                if (vbdata.type == (u_char) -1) {
                    RETVAL = NULL;
                    return; /* XXX: not good.  half populated stack? */
                }

                /* check for fixed length strings */
                if (vbdata.type == ASN_OCTET_STR &&
                    indexnode->ranges && !indexnode->ranges->next
                    && indexnode->ranges->low == indexnode->ranges->high) {
                    vbdata.val_len = indexnode->ranges->high;
                    vbdata.type |= ASN_PRIVATE;
                    is_private = 1;
                } else {
                    vbdata.val_len = 0;
                    if (index->isimplied) {
                        vbdata.type |= ASN_PRIVATE;
                        is_private = 1;
                    } else {
                        is_private = 0;
                    }
                }

                /* possible memory leak: vbdata.data should be freed later */
                if (parse_one_oid_index(&oidp, &oidp_len, &vbdata, 0)
                    != SNMPERR_SUCCESS) {
                    RETVAL = NULL;
                    return;
                }
                out_len = 0;
                if (is_private)
                    vbdata.type ^= ASN_PRIVATE;
                out_len =
                    __snprint_value (buf, buf_len, &vbdata, indexnode,
                                     vbdata.type, 0);
/*
                sprint_realloc_value(&buf, &buf_len, &out_len,
                                     1, name, name_len, &vbdata);
*/

                av_push(myret, newSVpv(buf, out_len));
            }
            RETVAL = newRV((SV *)myret);
        }
    OUTPUT:
        RETVAL

void
nsop_append(oid1, string)
    netsnmp_oid *oid1;
    char *string;
    PREINIT:
    oid name[MAX_OID_LEN];
    size_t name_len = MAX_OID_LEN;
    int i;
    CODE: 
    {
        if (!snmp_parse_oid(string, (oid *) name, &name_len)) {
            /* XXX */
        }
        if (oid1->len + name_len > MAX_OID_LEN) {
            /* XXX: illegal */
        }
        for(i = 0; i < (int)name_len; i++) {
            oid1->name[i+oid1->len] = name[i];
        }
        oid1->len += name_len;
    }

void
nsop_append_oid(oid1, oid2)
    netsnmp_oid *oid1;
    netsnmp_oid *oid2;
    PREINIT:
    int i;
    CODE: 
    {
        if (oid1->len + oid2->len > MAX_OID_LEN) {
            /* XXX: illegal */
        }
        for(i = 0; i < (int)oid2->len; i++) {
            oid1->name[i+oid1->len] = oid2->name[i];
        }
        oid1->len += oid2->len;
    }

int
nsop_length(oid1)
    netsnmp_oid *oid1;
    CODE: 
    {
        RETVAL = oid1->len;
    }
    OUTPUT:
    RETVAL
    
netsnmp_oid *
nsop_clone(oid1)
    netsnmp_oid *oid1;
    PREINIT:
    netsnmp_oid *oid2;
    CODE:
    {
        oid2 = nso_newarrayptr(oid1->name, oid1->len);
        RETVAL = oid2;
    }
OUTPUT:
    RETVAL
        
