/* Copyright (C) Hansi Reiser, dl9rdz
 *
 * partially based on dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <stdio.h>
#include <WString.h>
#include <stdlib.h>
//#include <arpa/inet.h>
//#include <sys/socket.h>
#include <math.h>
#include <unistd.h>
#include <inttypes.h>
#include "aprs.h"


void aprsstr_append(char *b, const char *data)
{
	int blen=strlen(b);
	int len=strlen(data);
	if(blen+len>APRS_MAXLEN) len=APRS_MAXLEN-blen;
	strncat(b, data, len);
}

uint32_t realcard(float x) {
	if(x<0) return 0;
	else return (uint32_t)x;
}


/* CRC for AXUDP frames */

#define APRSCRC_POLY 0x8408 
static uint8_t CRCL[256];
static uint8_t CRCH[256];

void aprs_gencrctab(void)
{
   uint32_t c;
   uint32_t crc;
   uint32_t i;
   for (c = 0UL; c<=255UL; c++) {
      crc = 255UL-c;
      for (i = 0UL; i<=7UL; i++) {
         if ((crc&1)) crc = (uint32_t)((uint32_t)(crc>>1)^APRSCRC_POLY);
         else crc = crc>>1;
      } /* end for */
      CRCL[c] = (uint8_t)crc;
      CRCH[c] = (uint8_t)(255UL-(crc>>8));
   } /* end for */
} /* end Gencrctab() */

static void aprsstr_appcrc(char frame[], uint32_t frame_len, int32_t size)
{
   uint8_t h;
   uint8_t l;
   uint8_t b;
   int32_t i;
   int32_t tmp;
   l = 0U;
   h = 0U;
   tmp = size-1L;
   i = 0L;
   if (i<=tmp) for (;; i++) {
      b = (uint8_t)((uint8_t)(uint8_t)frame[i]^l);
      l = CRCL[b]^h;
      h = CRCH[b];
      if (i==tmp) break;
   } /* end for */
   frame[size] = (char)l;
   frame[size+1L] = (char)h;
} /* end aprsstr_appcrc() */


static int mkaprscall(int32_t * p, char raw[],
                uint32_t * i, const char mon[],
                char sep1, char sep2, char sep3,
                uint32_t sbase)
{
   uint32_t s;
   uint32_t l;
   l = 0UL;
   while ((((mon[*i] && mon[*i]!=sep1) && mon[*i]!=sep2) && mon[*i]!=sep3)
                && mon[*i]!='-') {
      s = (uint32_t)(uint8_t)mon[*i]*2UL&255UL;
      if (s<=64UL) return 0;
      raw[*p] = (char)s;
      ++*p;
      ++*i;
      ++l;
      if (l>=7UL) return 0;
   }
   while (l<6UL) {
      raw[*p] = '@';
      ++*p;
      ++l;
   }
   s = 0UL;
   if (mon[*i]=='-') {
      ++*i;
      while ((uint8_t)mon[*i]>='0' && (uint8_t)mon[*i]<='9') {
         s = (s*10UL+(uint32_t)(uint8_t)mon[*i])-48UL;
         ++*i;
      }
      if (s>15UL) return 0;
   }
   raw[*p] = (char)((s+sbase)*2UL);
   ++*p;
   return 1;
} /* end call() */



// returns raw len, 0 in case of error
extern int aprsstr_mon2raw(const char *mon, char raw[], int raw_len)
{
   uint32_t r;
   uint32_t n;
   uint32_t i;
   uint32_t tmp;
   int p = 7L;
   i = 0UL;
   //fprintf(stderr,"mon2raw for %s\n", mon);
   if (!mkaprscall(&p, raw, &i, mon, '>', 0, 0, 48UL)) {
      return 0;
   }
   p = 0L;
   if (mon[i]!='>') return 0;
   /* ">" */
   ++i;
   if (!mkaprscall(&p, raw, &i, mon, ':', ',', 0, 112UL)) {
      return 0;
   }
   p = 14L;
   n = 0UL;
   while (mon[i]==',') {
      ++i;
      if (!mkaprscall(&p, raw, &i, mon, ':', ',', '*', 48UL)) {
         return 0;
      }
      ++n;
      if (n>8UL) {
         return 0;
      }
      if (mon[i]=='*') {
         /* "*" has repeatet sign */
         ++i;
         r = (uint32_t)p;
         if (r>=21UL) for (tmp = (uint32_t)(r-21UL)/7UL;;) {
            raw[r-1UL] = (char)((uint32_t)(uint8_t)raw[r-1UL]+128UL);
                 /* set "has repeated" flags */
            if (!tmp) break;
            --tmp;
            r -= 7UL;
         } /* end for */
      }
   }
   //fprintf(stderr,"p=%d, i=%d, moni=%c \n",p, i, mon[i]);
 if (p==0L || mon[i]!=':') {
	return 0;
   }
   raw[p-1L] = (char)((uint32_t)(uint8_t)raw[p-1L]+1UL);
                /* end address field mark */
   //int ii;
   //for(ii=0; ii<p; ii++) { fprintf(stderr,"%02x ", raw[ii]); } fprintf(stderr,"\n");
   raw[p] = '\003';
   ++p;
   raw[p] = '\360';
   ++p;
   ++i;
   n = 256UL;
   while (mon[i]) {
      /* copy info part */
      if (p>=(int32_t)(raw_len-1)-2L || n==0UL) {
         return 0;
      }
      raw[p] = mon[i];
      ++p;
      ++i;
      --n;
   }
   aprsstr_appcrc(raw, raw_len, p);
   //fprintf(stderr,"results in %s\n",raw);
   return p+2;
} /* end mon2raw() */

extern int aprsstr_mon2kiss(const char *mon, char raw[], int raw_len)
{
	char tmp[201];
	int len = aprsstr_mon2raw(mon, tmp, 201);
	if(len==0) return 0;
	int idx=0;
	raw[idx++] = '\xC0';
        raw[idx++] = 0;
	for(int i=0; i<len-2; i++) { // -2: discard CRC, not used in KISS
		if(tmp[i]=='\xC0') {	
			raw[idx++] = '\xDB';
			raw[idx++] = '\xDC';
		} else if (tmp[i]=='\xDB') {
			raw[idx++] = '\xDB';
			raw[idx++] = '\xDD';
		} else {
			raw[idx++] = tmp[i];
		}
		if(idx>=raw_len)
			return 0;
	}
	raw[idx++] = '\xC0';
	raw[idx++] = '\xC0';
	return idx;
}

#define FEET (1.0/0.3048)
#define KNOTS (1.851984)

#define X2C_max_longcard		0xFFFFFFFFUL
static uint32_t X2C_TRUNCC(double x, uint32_t min0, uint32_t max0)
{
        uint32_t i;

        if (x < (double)min0)
                i = (uint32_t)min0;
        if (x > (double)max0)
                i = (uint32_t)max0;

        i = (uint32_t)x;
        if ((double)i > x)
                --i;
        return i;
}


static uint32_t truncc(double r)
{
   if (r<=0.0) return 0UL;
   else if (r>=2.E+9) return 2000000000UL;
   else return (uint32_t)X2C_TRUNCC(r,0UL,X2C_max_longcard);
   return 0;
} /* end truncc() */



static uint32_t dao91(double x)
/* radix91(xx/1.1) of dddmm.mmxx */
{
   double a;
   a = fabs(x);
   return ((truncc((a-(double)(float)truncc(a))*6.E+5)%100UL)
                *20UL+11UL)/22UL;
} /* end dao91() */



#if 0
int main(int argc, char *argv[])
{
	Gencrctab();

	struct sockaddr_in si;
	int fd = openudp("127.0.0.1",9002,&si);
	if(fd<0) { fprintf(stderr,"open failed\n"); return 1; }

	float lat=48, lon=10;
	while(1) {
	const char *str = aprs_senddata(lat, lon, 543, 5, 180, 1.5, "RS41", "TE0ST", "TE1ST", "EO");
	int rawlen = aprsstr_mon2raw(str, raw, APRS_MAXLEN);
	sendudp(fd, raw, rawlen);

	str = "OE3XKC>APMI06,qAR,OE3XLR:;ER-341109*111111z4803.61NE01532.39E0145.650MHz R15k OE3XPA";
	rawlen = aprsstr_mon2raw(str, raw, APRS_MAXLEN);
	sendudp(fd, &si, raw, rawlen);
	lat += 0.002; lon += 0.01;
	sleep(5);
	}
}
#endif
