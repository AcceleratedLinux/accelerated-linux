/* Nessus
 * Copyright (C) 1998 - 2006 Tenable Network Security, Inc.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * Users manager
 *
 */
 
#include <includes.h>
#include "log.h"
#include "users.h"
#include "rules.h"
#include "md5.h"
#ifdef HAVE_SSL
#include <openssl/md5.h>
#endif

char *
user_home(globals)
 struct arglist * globals;
{
 char * user = arg_get_value(globals, "user");
 char * ret;

 if(!user)
  return NULL;
 
 ret = emalloc(strlen(NESSUSD_LOGINS) + strlen(user) + 2);
 sprintf(ret, "%s/%s", NESSUSD_LOGINS, user);
 
 return ret;
}


/*
 * Add the rules to the current user, and return 
 * the name of the next user
 */
void 
users_add_rule(struct nessus_rules * rules, char * rule)
{
  struct nessus_rules * start = rules;
  int def = rules->def;
  char * t = rule;
  int len;
#ifdef DEBUG_RULES
 printf("parse %s\n", rule);
#endif
 while(rules->next)rules = rules->next;
 if(!strncmp(t, "default", 7))
 {
   if(!strncmp(t+8, "accept", 6))def = RULES_ACCEPT;
   else def = RULES_REJECT;
   rules_set_def(start, def);
   return;
 }
     
 if(!strncmp(t, "accept", 6))
       rules->rule = RULES_ACCEPT;
 else rules->rule = RULES_REJECT;
 rule = strchr(rule, ' ');
 if(rule)
 {
  rule+=sizeof(char);
  t = strchr(rule, '/');
  if(t)t[0]='\0';
  if(rule[0]=='!'){
    rules->not = 1;
    rule+=sizeof(char);
  }
  else rules->not = 0;
  
  len = strlen(rule);
  
  while(rule[len-1]==' ')
  {
   rule[len-1]='\0';
   len --;
  }
  
  
  if(!(inet_aton(rule,&rules->ip))) 
	 {
	  if(strcmp(rule, "client_ip"))
	  {
	  log_write("Parse error in the user rules : %s is not a valid IP\n",
	      			rule);
	  EXIT(1);
	  }
	  else
	  {
	   rules->client_ip = 1;
	   rules->ip.s_addr = -1;
	   }
	 }
	  else rules->client_ip = 0;
   rules->def = def;
   if(t)rules->mask = atoi(t+sizeof(char));
   else rules->mask = 32;
   if(rules->mask < 0 || rules->mask > 32)
   {
     /* The user may have tried to fool us by entering
	a bogus netmask. Just ignore this rule
	*/
     log_write("User entered an invalid netmask - %s/%d\n",
	 	inet_ntoa(rules->ip), rules->mask);
     bzero(&rules, sizeof(*rules));
   }
   else rules->next = emalloc(sizeof(*rules));
#ifdef DEBUG_RULES 
   printf("Added rule %s/%d\n",inet_ntoa(rules->ip), rules->mask); 
#endif   
 }
}


/*
 * Reads the rules file
 */
static struct nessus_rules * 
users_read_rules(struct nessus_rules * rules,  FILE * f,char * buffer,
			int len)
{
  char *t = buffer;
  bzero(buffer, len);
  if(!fgets(buffer, len, f))return rules;
  t[strlen(t)-1]='\0';
  while((t[0]==' ')||(t[0]=='\t'))t+=sizeof(char);
  if(t[0]=='#'||t[0] == '\0')return users_read_rules(rules, f, buffer,
  						   len);

   users_add_rule(rules, t);
   return users_read_rules(rules, f, buffer, len);
}


#ifndef MD5_DIGEST_LENGTH
# define MD5_DIGEST_LENGTH	16
#endif

struct nessus_rules * 
check_user(char * user, char * password, char * dname)
{
  struct nessus_rules* ret = NULL;
  char * buf;
  FILE * f;
  char	fname[MAXPATHLEN];
  int	check_pass = 1;


#ifdef NESSUS_MAX_USERNAME_LEN
  if (strlen(user) >=  NESSUS_MAX_USERNAME_LEN)
    return BAD_LOGIN_ATTEMPT;
#endif

#ifdef DEBUG_PASSWD
  fprintf(stderr, "user=%s pass=%s dname=%s\n",
	  user, password ? password : "(null)", dname ? dname : "(null)");
#endif

  /* Should we change this for certificate authentication? */
  if( password == NULL || 
      password[0] == '\0' )
   return BAD_LOGIN_ATTEMPT;

  if( strstr(user, "..") != NULL ||
       strchr(user, '/') != NULL )
   return BAD_LOGIN_ATTEMPT;
  
  if (dname != NULL && *dname != '\0')
    {
      snprintf(fname, sizeof(fname), "%s/%s/auth/dname", NESSUSD_LOGINS, user);
      if ((f = fopen(fname, "r")) == NULL)
	perror(fname);
      else
	{
	  char	dnameref[512], *p;

	  while (check_pass && fgets(dnameref, sizeof(dnameref) - 1, f) != NULL)
	    {
	      if ((p = strchr(dnameref, '\n')) != NULL)
		*p = '\0';
	      if (strcmp(dname, dnameref) == 0)
		check_pass = 0;
	    }
          if(check_pass)
	    log_write("check_user: Bad DN for user %s\nGiven DN=%s\nLast tried DN=%s\n", user, dname, dnameref);
	  (void) fclose(f);
	}
    }

  if (check_pass)
  {
   char orig[255];
   char	*p;

   snprintf(fname, sizeof(fname), "%s/%s/auth/hash", NESSUSD_LOGINS, user);
   if ((f = fopen(fname, "r")) != NULL)
     {
       char	seed[512], h1[MD5_DIGEST_LENGTH], h2[MD5_DIGEST_LENGTH];
       char	h16[MD5_DIGEST_LENGTH*2+1];
       int	i, x, n;

       /*
	* file contains one or two string: 
	* the 1st one is the hex encoded MD5 hash of the seed 
	* concatenated to the password (or just the password),
	* 2nd one (if any) is the seed, in ASCII, without space.
	*/
       *seed =  '\0';
       if (fscanf(f, "%32s%32s", h16, seed) < 1)
	 {
	   perror(fname);
	   fclose(f);
	   return BAD_LOGIN_ATTEMPT;
	 }
       (void) fclose(f);

       memset(h1, 0, sizeof(h1));
       memset(h2, 0, sizeof(h2));
       for (i = 0; i < MD5_DIGEST_LENGTH; i ++)
	 if (sscanf(h16 + 2 * i, "%02x", &x) < 1)
	   {
	     fprintf(stderr, "check_pass: incorrect hexadecimal string\n");
	     return BAD_LOGIN_ATTEMPT;
	   }
	 else
	   h1[i] = x;
           
       n = sizeof (seed) - 1;
       
       for (p = seed; *p != '\0' && *p != '\n' && *p != '\r'; p ++, n -- )
	 ;
         
       if ( n < strlen(password) )
            return BAD_LOGIN_ATTEMPT;
            
       strncpy(p, password, n);

       MD5((unsigned char*)seed, strlen(seed), (unsigned char*)h2);
       if (memcmp(h1, h2, MD5_DIGEST_LENGTH) != 0)
	 {
	   return BAD_LOGIN_ATTEMPT;
	 }
     }
   else
     {
   snprintf(fname, sizeof(fname), "%s/%s/auth/password", NESSUSD_LOGINS, user);
   if ((f = fopen(fname, "r")) == NULL)
     return BAD_LOGIN_ATTEMPT;
   
   bzero(orig, sizeof(orig));
   fgets(orig, sizeof ( orig ) - 1, f);
   fclose(f);
   if ( orig[0] != '\0' )orig[ strlen(orig) - 1 ]= '\0';
   if(strcmp(password, orig) != 0)
   	return BAD_LOGIN_ATTEMPT;
     }
  }
 
 snprintf(fname, sizeof(fname), "%s/%s/auth/rules", NESSUSD_LOGINS, user);
 if ((f = fopen(fname, "r")) == NULL)
   perror(fname);

 if ( f == NULL ) return BAD_LOGIN_ATTEMPT;
 
  
 buf = emalloc(1024);
 ret = emalloc(sizeof(*ret));
 ret = users_read_rules(ret, f, buf, 1024);
 
 fclose(f);
 efree(&buf);
 
 return ret;
}
 
 
 
