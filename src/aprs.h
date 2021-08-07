
#ifndef _aprs_h
#define _aprs_h


#define APRS_MAXLEN 201
void aprs_gencrctab(void);
int aprsstr_mon2raw(const char *mon, char raw[], int raw_len);
int aprsstr_mon2kiss(const char *mon, char raw[], int raw_len);


#endif
