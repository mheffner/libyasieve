#include <stdio.h>

#include "libconfig.h"

enum {
	SIEVEOPT_TYPE_SWITCH = 0,
};

struct sieveopt_data {
	int	 opt_type;
	void	*opt_val;
};

static struct sieveopt_data sieve_options[] = {
	{ SIEVEOPT_TYPE_SWITCH,		(void *)1 },
	{ SIEVEOPT_SIEVE_UTF8FILEINTO,	(void *)1 },
};

/* these will assert() if they're called on the wrong type of
   option (imapopts.c) */

#define GET_OPTS(opt)						\
do {								\
	if (sieve_options[opt].opt_type == SIEVEOPT_TYPE_SWITCH)\
		return (int)sieve_options[opt].opt_val;		\
} while (0)

void config_read(const char *alt_config) {
  printf("calling %s\n", __func__);
}

const char *config_getstring(enum imapopt opt)
{  printf("calling %s\n", __func__);
}

int config_getint(enum imapopt opt)
{  printf("calling %s\n", __func__);
}

int config_getswitch(enum imapopt opt)
{
	printf("calling %s\n", __func__);
	GET_OPTS(opt);
}

enum enum_value config_getenum(enum imapopt opt)
{  printf("calling %s\n", __func__);
}

/* these work on additional strings that are not defined in the
 * imapoptions table */
const char *config_getoverflowstring(const char *key, const char *def){};
void config_foreachoverflowstring(
    void (*func)(const char *, const char *, void *), void *rock){};
const char *config_partitiondir(const char *partition){};
const char *config_metapartitiondir(const char *partition){};

/* cached configuration variables accessable to external world */
const char *config_filename;
const char *config_dir;
const char *config_defpartition;
const char *config_servername;
enum enum_value config_serverinfo;
const char *config_mupdate_server;
const char *config_defdomain;
const char *config_ident;
int config_hashimapspool;
int config_implicitrights;
enum enum_value config_virtdomains;
enum enum_value config_mupdate_config;
int config_maxquoted;
int config_maxword;

/* config requirement flags */
#define CONFIG_NEED_PARTITION_DATA (1<<0)
