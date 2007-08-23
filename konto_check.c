/* C prolog +§§§1
   vim:cindent:ft=c:foldmethod=marker:fmr=+§§§,-§§§
   Copyright-Notitz +§§§2 */
/*
 * ##########################################################################
 * #  Dies ist konto_check, ein Programm zum Testen der Prüfziffern         #
 * #  von deutschen Bankkonten. Es kann als eigenständiges Programm         #
 * #  (z.B. mit der beigelegten main() Routine) oder als Library zur        #
 * #  Verwendung in anderen Programmen bzw. Programmiersprachen benutzt     #
 * #  werden.                                                               #
 * #                                                                        #
 * #  Die einleitenden Beschreibungen zu den einzelnen Methoden wurden      #
 * #  wurden aus der aktuellen BLZ-Datei der Deutschen Bundesbank           #
 * #  übernommen.                                                           #
 * #                                                                        #
 * #  Copyright (C) 2002-2007 Michael Plugge <m.plugge@hs-mannheim.de>      #
 * #                                                                        #
 * #  Dieses Programm ist freie Software; Sie dürfen es unter den           #
 * #  Bedingungen der GNU Lesser General Public License, wie von der Free   #
 * #  Software Foundation veröffentlicht, weiterverteilen und/oder          #
 * #  modifizieren; entweder gemäß Version 2.1 der Lizenz oder (nach Ihrer  #
 * #  Option) jeder späteren Version.                                       #
 * #                                                                        #
 * #  Die GNU LGPL ist weniger infektiös als die normale GPL; Code, der von #
 * #  Ihnen hinzugefügt wird, unterliegt nicht der Offenlegungspflicht      #
 * #  (wie bei der normalen GPL); außerdem müssen Programme, die diese      #
 * #  Bibliothek benutzen, nicht (L)GPL lizensiert sein, sondern können     #
 * #  beliebig kommerziell verwertet werden. Die Offenlegung des Sourcecodes#
 * #  bezieht sich bei der LGPL *nur* auf geänderten Bibliothekscode.       #
 * #                                                                        #
 * #  Dieses Programm wird in der Hoffnung weiterverbreitet, daß es         #
 * #  nützlich sein wird, jedoch OHNE IRGENDEINE GARANTIE, auch ohne die    #
 * #  implizierte Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR       #
 * #  EINEN BESTIMMTEN ZWECK. Mehr Details finden Sie in der GNU Lesser     #
 * #  General Public License.                                               #
 * #                                                                        #
 * #  Sie sollten eine Kopie der GNU Lesser General Public License          #
 * #  zusammen mit diesem Programm erhalten haben; falls nicht,             #
 * #  schreiben Sie an die Free Software Foundation, Inc., 59 Temple        #
 * #  Place, Suite 330, Boston, MA 02111-1307, USA. Sie können sie auch     #
 * #  von                                                                   #
 * #                                                                        #
 * #       http://www.gnu.org/licenses/lgpl.html                            #
 * #                                                                        #
 * # im Internet herunterladen.                                             #
 * ##########################################################################
 */

/* Definitionen und Includes  */
#define BLZ_BIG_JUMP 0  /* noch optimalen Wert suchen */

#define VERSION "2.2"
#define VERSION_DATE "2007-08-21"

#ifndef INCLUDE_KONTO_CHECK_DE
#define INCLUDE_KONTO_CHECK_DE 1
#endif

#if INCLUDE_KONTO_CHECK_DE
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef O_BINARY
#define O_BINARY 0	/* reserved by dos */
#endif

#define KONTO_CHECK_VARS
#include "konto_check.h"

/* Definitionen für die Hashtabelle +§§§2 */
   /* fixme: FIX_HASH_SHIFT fest, ohne Variablen */

   /* Das Makro FIX_HASH_SHIFT wird benutzt, um einen festen SHIFT-Wert
    * für die Hashtabelle zu erzeugen. Der Code ist kein Lehrbeispiel,
    * jedoch einfach. Falls FIX_HASH_SHIFT gesetzt ist, werden erst die
    * Variablen deklariert (für evl. noch vorhandene externen Referenzen),
    * und dann die Variablen auf die Makros abgebildet. Eine Definition
    * mit const-Variablen läßt sich nicht durchführen, da diese bei
    * vorhandenen externen Referenzen einen Absturz bewirken (wegen Schreiben
    * in Read-Only Bereich).
    * Auf einem Athlon ergeben sich die besten Resultate mit HASH_SHIFT=13.
    * Das Makro ist nur zur Entwicklung und zum Test gedacht, deshalb ist
    * es nicht in konto_check.h definiert.
    */

#define FIX_HASH_SHIFT 1

#define HASH_SHIFT 13

   /* Hash-Array:
    *
    * shift low_limit high_limit n_elements
    *  10    9765       97656      87891
    *  11    4882       48828      43946
    *  12    2441       24414      21973
    *  13    1220       12207      10987
    *  14     610        6103       5493
    *  15     305        3051       2746
    */

#if HASH_SHIFT<=10
#   define HASH_BASE   9700
#   define HASH_SIZE  88000
#elif HASH_SHIFT==11
#   define HASH_BASE   4880
#   define HASH_SIZE  45000
#elif HASH_SHIFT==12
#   define HASH_BASE   2400
#   define HASH_SIZE  23000
#elif HASH_SHIFT==13
#   define HASH_BASE   1200
#   define HASH_SIZE  12000
#elif HASH_SHIFT==14
#   define HASH_BASE    600
#   define HASH_SIZE   5700
#elif HASH_SHIFT>=15
#   define HASH_BASE    300
#   define HASH_SIZE   2800
#endif

    /* fixme: noch optimalen Wert für die Hashtabelle testen */
#if FIX_HASH_SHIFT
int hash_shift,
    hash_base,
    hash_size;

#   define hash_shift   HASH_SHIFT
#   define hash_base    HASH_BASE
#   define hash_size    HASH_SIZE
#else
int hash_shift=HASH_SHIFT,
    hash_base=HASH_BASE,
    hash_size=HASH_SIZE;
#endif

#define SET_VARS prev=blz_array[i]; j=(prev>>hash_shift)-hash_base; \
   if(j<0)j=0; else if(j>HASH_SIZE)j=HASH_SIZE; \
   if(!blz_hash_low[j])blz_hash_low[j]=i; if(blz_hash_high[j]<i)blz_hash_high[j]=i; \
   uptr++; if(*uptr==0xff){invalid[i]=1; uptr++;} prev_pz=pz_array[i++]= *uptr

/* Divisions-Makros (ab Version 2.0) +§§§2 */

   /* Makros zur Modulo-Bildung über iterierte Subtraktionen.
    * auf Intel-Hardware ist dies schneller als eine direkte Modulo-Operation;
    * Auf Alpha ist Modulo allerdings schneller (gute FPU).
    */

#ifdef __ALPHA
#   define MOD_11_352   pz%=11
#   define MOD_11_176   pz%=11
#   define MOD_11_88    pz%=11
#   define MOD_11_44    pz%=11
#   define MOD_11_22    pz%=11
#   define MOD_11_11    pz%=11

#   define MOD_10_320   pz%=10
#   define MOD_10_160   pz%=10
#   define MOD_10_80    pz%=10
#   define MOD_10_40    pz%=10
#   define MOD_10_20    pz%=10
#   define MOD_10_10    pz%=10

#   define MOD_9_288    pz%=9
#   define MOD_9_144    pz%=9
#   define MOD_9_72     pz%=9
#   define MOD_9_36     pz%=9
#   define MOD_9_18     pz%=9
#   define MOD_9_9      pz%=9

#   define MOD_7_224    pz%=7
#   define MOD_7_112    pz%=7
#   define MOD_7_56     pz%=7
#   define MOD_7_28     pz%=7
#   define MOD_7_14     pz%=7
#   define MOD_7_7      pz%=7

#   define SUB1_22      p1%=11
#   define SUB1_11      p1%=11
#else
#   define SUB(x) if(pz>=x)pz-=x
#   define MOD_11_352 SUB(352); MOD_11_176
#   define MOD_11_176 SUB(176); MOD_11_88
#   define MOD_11_88  SUB(88);  MOD_11_44
#   define MOD_11_44  SUB(44);  MOD_11_22
#   define MOD_11_22  SUB(22);  MOD_11_11
#   define MOD_11_11  SUB(11)

#   define MOD_10_320 SUB(320); MOD_10_160
#   define MOD_10_160 SUB(160); MOD_10_80
#   define MOD_10_80  SUB(80);  MOD_10_40
#   define MOD_10_40  SUB(40);  MOD_10_20
#   define MOD_10_20  SUB(20);  MOD_10_10
#   define MOD_10_10  SUB(10)

#   define MOD_9_288 SUB(288);  MOD_9_144
#   define MOD_9_144 SUB(144);  MOD_9_72
#   define MOD_9_72  SUB(72);   MOD_9_36
#   define MOD_9_36  SUB(36);   MOD_9_18
#   define MOD_9_18  SUB(18);   MOD_9_9
#   define MOD_9_9   SUB(9)

#   define MOD_7_224 SUB(224);  MOD_7_112
#   define MOD_7_112 SUB(112);  MOD_7_56
#   define MOD_7_56  SUB(56);   MOD_7_28
#   define MOD_7_28  SUB(28);   MOD_7_14
#   define MOD_7_14  SUB(14);   MOD_7_7
#   define MOD_7_7   SUB(7)

#   define SUB1(x)   if(p1>=x)p1-=x
#   define SUB1_22   SUB1(22);  SUB1_11
#   define SUB1_11   SUB1(11)
#endif

/* Testmakros für die Prüfziffermethoden +§§§2 */
#define CHECK_PZ6     return (*(kto+5)-'0'==pz ? OK : FALSE)
#define CHECK_PZ7     return (*(kto+6)-'0'==pz ? OK : FALSE)
#define CHECK_PZ8     return (*(kto+7)-'0'==pz ? OK : FALSE)
#define CHECK_PZ9     return (*(kto+8)-'0'==pz ? OK : FALSE)
#define CHECK_PZ10    return (*(kto+9)-'0'==pz ? OK : FALSE)

/*
 * ######################################################################
 * # Anmerkung zur DEBUG-Variante:                                      #
 * # Die Debug-Version enthält einige Dinge, die für Puristen tabu sind #
 * # (z.B. das goto im default-Zweig der case-Anweisung, oder (noch     #
 * # schlimmer) die case-Anweisungen in if/else Strukturen (z.B. in     #
 * # Methode 93). Da der Code jedoch nur für Debugzwecke gedacht ist    #
 * # (Verifizierung der Methoden mit anderen Programmen, Generierung    #
 * # Testkonten) und nicht als Lehrbeispiel, mag es angehen ;-)))       #
 * ######################################################################
 */

#if DEBUG
#define CASE0(nr)     case nr: pz_str[0]=nr/10+'0'; pz_str[1]=nr%10+'0';
#define CASE1(nr)     case nr: pz_str[0]=nr/10-10+'a'; pz_str[1]=nr%10+'0';
#define CASE_U(nr,u)  case u*1000+nr: pz_str[0]=nr/10+'0'; pz_str[1]=nr%10+'0'; pz_str[2]=u-1+'a';  /* Untermethode */
#define CASE_U1(nr,u) case u*1000+nr: pz_str[0]=nr/10-10+'a'; pz_str[1]=nr%10+'0'; pz_str[2]=u-1+'a';  /* Untermethode */
#define CHECK_PZX7    if(*(kto+6)-'0'==pz)return OK; if(untermethode)return FALSE;
#define CHECK_PZX8    if(*(kto+7)-'0'==pz)return OK; if(untermethode)return FALSE;
#define CHECK_PZX9    if(*(kto+8)-'0'==pz)return OK; if(untermethode)return FALSE;
#define CHECK_PZX10   if(*(kto+9)-'0'==pz)return OK; if(untermethode)return FALSE;
#else
#define CASE0(nr)     case nr: pz_str[0]=nr/10+'0'; pz_str[1]=nr%10+'0';
#define CASE1(nr)     case nr: pz_str[0]=nr/10-10+'a'; pz_str[1]=nr%10+'0';
#define CASE_U(nr,u)  pz_str[2]=u-1+'a';
#define CASE_U1(nr,u) pz_str[2]=u-1+'a';
#define CHECK_PZX7    if(*(kto+6)-'0'==pz)return OK;
#define CHECK_PZX8    if(*(kto+7)-'0'==pz)return OK;
#define CHECK_PZX9    if(*(kto+8)-'0'==pz)return OK;
#define CHECK_PZX10   if(*(kto+9)-'0'==pz)return OK;
#endif

/* noch einige Makros +§§§2 */
#define EXTRACT(feld) do{ \
      for(sptr=zeile+feld.pos-1,dptr=buffer,j=feld.len;j>0;j--)*dptr++= *sptr++; \
      *dptr=0; \
      while(*--dptr==' ')*dptr=0; \
   }while(0)

#define WRITE_LONG(var) fputc((var)&255,out); fputc(((var)>>8)&255,out); fputc(((var)>>16)&255,out); fputc(((var)>>24)&255,out)
#define READ_LONG(var) var= *uptr+(*(uptr+1)<<8)+(*(uptr+2)<<16)+(*(uptr+3)<<24); uptr+=4
#define ISDIGIT(x) ((x)>='0' && (x)<='9')
#define UCP (unsigned char*)
#define SCP (char*)

/* globale Variablen +§§§2 */
/*
 * ######################################################################
 * #               globale Variablen der externen Schnittstelle         #
 * ######################################################################
 */

DLL_EXPORT_V char *kto_check_msg,pz_str[4];
DLL_EXPORT_V int pz_methode;

#if DEBUG
DLL_EXPORT
#else
static
#endif
int pz;    /* Prüfziffer (bei DEBUG als globale Variable für Testzwecke) */

/* interne Funktionen und Variablen +§§§2 */
/*
 * ######################################################################
 * #               interne Funktionen und Variablen                     #
 * ######################################################################
 */

#define E_START(x)
#define E_END(x)

static int kto_check_int(char *pz_or_blz,char *kto,char *lut_name);
static int read_lut(char *filename);
static UINT4 adler32(UINT4 adler,const char *buf,unsigned int len);
static void set_msg(int retval);
static void init_atoi_table(void);

#if THREAD_SAFE
static int kto_check_int_t(char *pz_or_blz,char *kto,char *lut_name,KTO_CHK_CTX *ctx);
static int read_lut_t(char *filename,KTO_CHK_CTX *ctx);
static void set_msg_t(int retval,KTO_CHK_CTX *ctx);
static void init_atoi_table_t(KTO_CHK_CTX *ctx);
#else
#define kto_check_int_t(pz_or_blz,kto,lut_name,ctx) kto_check_int(pz_or_blz,kto,lut_name)
#define read_lut_t(filename,ctx) read_lut(filename)
#define set_msg_t(retval,ctx) set_msg(retval)
#define init_atoi_table_t(ctx) init_atoi_table()

static char lut_info[1024];
INT4 cnt_blz;
static UINT4 *blz_array,*pz_array,*blz_hash_low,*blz_hash_high,*invalid;
static UINT4 b1[256],b2[256],b3[256],b4[256],b5[256],b6[256],b7[256],b8[256],bx1[256],by1[256],bx2[256],by4[256];
#endif


   /* Variable für die Methoden 27, 29 und 69 */
const static int m10h_digits[4][10]={
   {0,1,5,9,3,7,4,8,2,6},
   {0,1,7,6,9,8,3,2,5,4},
   {0,1,8,4,6,2,9,5,7,3},
   {0,1,2,3,4,5,6,7,8,9}
};


#if !THREAD_SAFE
   /* Variablen für Methode 87 */
static int c2,d2,a5,p,konto[11];
#endif
const static int tab1[]={0,4,3,2,6},
   tab2[]={7,1,5,9,8};

/*  einige Hacks für THREAD_SAFE +§§§1 */
#if THREAD_SAFE
/*
 * ######################################################################
 * # Die nicht threadfeste Version der Library ist etwas schneller als  #
 * # die threadfeste; deshalb gibt es beide Varianten. Für die Version  #
 * # mit threadfesten Funktionen wird hier zusätzlich eine Variable     #
 * # global_ctx angelegt, die als ctx Parameter bei den diversen        #
 * # Funktionen benutzt wird.                                           #
 * # Die kritischen Variablen werden per #define auf Strukturelemente   #
 * # der KTO_CHK_CTX Struktur umgetauft. Die Funktion set_globals setzt #
 * # (für die nicht threadfesten Varianten) die globalen Variablen auf  #
 * # die Werte der ctx-Variablen (normalerweise wird das global_ctx     #
 * # sein). Damit kann die library auch in der threadfesten Variante    #
 * # mit dem alten API aufgerufen werden; die Funktionen sind natürlich #
 * # nicht threadfest.                                                  #
 * ######################################################################
 */

static KTO_CHK_CTX global_ctx;

static void set_globals(KTO_CHK_CTX *ctx)
{
   kto_check_msg=ctx->kto_check_msg;
   memcpy(pz_str,ctx->pz_str,4);
   pz_methode=ctx->pz_methode;
   pz=ctx->pz;
}

#define c2 ctx->c2
#define d2 ctx->d2
#define a5 ctx->a5
#define p ctx->p
#define konto ctx->konto
#define kto_check_msg ctx->kto_check_msg
#define pz_str ctx->pz_str
#define pz_methode ctx->pz_methode
#define pz ctx->pz
#define cnt_blz ctx->cnt_blz
#define blz_array ctx->blz_array
#define pz_array ctx->pz_array
#define blz_hash_low ctx->blz_hash_low
#define blz_hash_high ctx->blz_hash_high
#define invalid ctx->invalid
#define lut_info ctx->lut_info
#define b1 ctx->b1
#define b2 ctx->b2
#define b3 ctx->b3
#define b4 ctx->b4
#define b5 ctx->b5
#define b6 ctx->b6
#define b7 ctx->b7
#define b8 ctx->b8
#endif

   /* Gewichtsvariablen für Methoden 24, 52, 53 und 93 */
   /* für Methode 52 sind ein paar Gewichtsfaktoren mehr als spezifiziert, falls die Kontonummer zu lang wird */
const static int w52[] = { 2, 4, 8, 5,10, 9, 7, 3, 6, 1, 2, 4, 0, 0, 0, 0},
   w24[]={ 1, 2, 3, 1, 2, 3, 1, 2, 3 },
   w93[]= { 2, 3, 4, 5, 6, 7, 2, 3, 4 };

/* Funktion adler32()+§§§1 */
/* ##########################################################################
 * # Die Funktion adler32.c wurde aus der zlib entnommen                    #
 * #                                                                        #
 * # adler32.c -- compute the Adler-32 checksum of a data stream            #
 * # Copyright (C) 1995-1998 Mark Adler                                     #
 * # For conditions of distribution and use, see copyright notice in zlib.h #
 * ##########################################################################
 */

#define BASE 65521L /* largest prime smaller than 65536 */
#define NMAX 5552
   /* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {s1+=buf[i]; s2+=s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

static UINT4 adler32(UINT4 adler,const char *buf,unsigned int len)
{
   UINT4 s1=adler&0xffff;
   UINT4 s2=(adler>>16)&0xffff;
   int k;

   if(buf==NULL)return 1L;

   while(len>0){
      k=len<NMAX ? len:NMAX;
      len-=k;
      while(k>=16){
         DO16(buf);
	      buf+=16;
         k-=16;
      }
      if(k!=0)do{
         s1+=*buf++;
         s2+=s1;
      } while(--k);
      s1%=BASE;
      s2%=BASE;
   }
   return (s2<<16)|s1;
}

/* Funktion generate_lut() +§§§1 */
/* ###########################################################################
 * # Die Funktion generate_lut() generiert aus der BLZ-Datei der deutschen   #
 * # Bundesbank (knapp 4 MB) eine kleine Datei (14 KB), in der nur die       #
 * # Bankleitzahlen und Prüfziffermethoden gespeichert sind. Um die Datei    #
 * # klein zu halten, werden normalerweise nur Differenzen zur Vorgänger-BLZ #
 * # gespeichert (meist mit 1 oder 2 Byte); die Prüfziffermethode wird in    #
 * # einem Byte kodiert. Diese kleine Datei läßt sich dann natürlich viel    #
 * # schneller einlesen als die große Bundesbank-Datei.                      #
 * #                                                                         #
 * # Bugs: es wird für eine BLZ nur eine Prüfziffermethode unterstützt.      #
 * #      (nach Bankfusionen finden sich für eine Bank manchmal zwei         #
 * #      Prüfziffermethoden; das Problem wird mit dem neuen Dateiformat     #
 * #      noch einmal angegangen.                                            #
 * #                                                                         #
 * # Copyright (C) 2002-2005 Michael Plugge <m.plugge@hs-mannheim.de>        #
 * ###########################################################################
 */

#if THREAD_SAFE
DLL_EXPORT int generate_lut(char *inputname,char *outputname,char *user_info,int lut_version)
{
   generate_lut_t(inputname,outputname,user_info,lut_version,&global_ctx);
}

DLL_EXPORT int generate_lut_t(char *inputname,char *outputname,char *user_info,int lut_version,KTO_CHK_CTX *ctx)
#else
#define generate_lut_t(inputname,outputname,user_info,lut_version,global_ctx) generate_lut(inputname,outputname,user_info,lut_version)
DLL_EXPORT int generate_lut(char *inputname,char *outputname,char *user_info,int lut_version)
#endif
{
   unsigned char *outbuffer,*uptr,*methoden_array,*valid_array,*sptr,*dptr,buffer[256],zeile[256];
   int i,j,blz,prev_blz,diff,last_index,cnt,m,valid,*array,blz_file_format,line;
   UINT4 adler;
   time_t t;
   struct tm *lt;
   FILE *in,*out;

   struct{int pos; int len;}
      old_bankleitzahl={1,8},
      old_pruefziffer={182,2},
      old_geloeschte_blz={10,8};

   struct{int pos; int len;}
      bankleitzahl={1,8},
      pruefziffer={151,2};

   if(!(in=fopen(inputname,"r"))){
#if THREAD_SAFE
      set_msg_t(NO_BLZ_FILE,ctx);
      set_globals(&global_ctx);
#else
      set_msg(NO_BLZ_FILE);
#endif
      cnt_blz=0;
      return(NO_BLZ_FILE);
   }

   blz_array=calloc(MAX_BLZ_CNT,sizeof(int));   /* Bankleitzahlen */
   methoden_array=calloc(MAX_BLZ_CNT,1);        /* Prüfziffermethoden */
   valid_array=calloc(MAX_BLZ_CNT,1);           /* BLZ gültig oder nicht */
   outbuffer=calloc(2*MAX_BLZ_CNT,sizeof(int)); /* Ausgabepuffer (nicht zu knapp bemessen) */

   for(prev_blz=0,i= -1,line=1;!feof(in);line++){
      if(!fgets((char *)zeile,512,in) || !ISDIGIT(*zeile))continue;

         /* Zeilenenden entfernen, um eine eindeutige Zeilenlänge zu bekommen
          * (sonst könnte am Zeilenende CR/LF oder nur LF kommen).
          */
      for(sptr=zeile+strlen(SCP zeile)-1;*sptr=='\r' || *sptr=='\n';sptr--)*sptr=0;
      switch(strlen(SCP zeile)){
         case 188: blz_file_format=1; break; /* altes BLZ-Dateiformat (vor 7/2006) */
         case 168: blz_file_format=2; break; /* neues BLZ-Dateiformat (ab 7/2006) */
         default: blz_file_format=0; break;  /* falsche Datensatzlänge, Fehler in der blz.txt Datei */
      }

      if(!blz_file_format){
#if THREAD_SAFE
         set_msg_t(INVALID_BLZ_FILE,ctx);
         set_globals(&global_ctx);
#else
         set_msg(INVALID_BLZ_FILE);
#endif
         fprintf(stderr,"falsche Zeilenlänge in Zeile %d von %s; Abbruch\n",line,inputname);
         return INVALID_BLZ_FILE;
      }

      if(blz_file_format==1){ /* altes Dateiformat */
            /* BLZ holen */
         EXTRACT(old_bankleitzahl);
         blz=atoi((char *)buffer);
         if(blz==0){ /* gelöschter Eintrag */
            EXTRACT(old_geloeschte_blz); /* alte BLZ holen */
            blz=atoi((char *)buffer);
            valid=2;
         }
         else
            valid=1;
         diff=blz-prev_blz;
         if(diff){
            i++;
            prev_blz=blz;
         }

            /* Manchmal sind bei einer Löschung noch gültige Bankleitzahlen
             * desselben Instituts vorhanden; daher wird hier nur eine
             * OR-Verknüpfung mit dem valid-Wert gemacht, und beim Speichern
             * dann überprüft, ob valid==2 ist (nur dann gibt es kein Institut
             * mit der BLZ mehr).
             */
         valid_array[i]|=valid;

            /* Prüfziffer-Methode holen */
         EXTRACT(old_pruefziffer);
      }
      else{ /* neues Dateiformat */
            /* BLZ holen */
         EXTRACT(bankleitzahl);
         blz=atoi((char *)buffer);
         diff=blz-prev_blz;
         if(diff){
            i++;
            prev_blz=blz;
         }
         valid_array[i]=1;

            /* Prüfziffer-Methode holen */
         EXTRACT(pruefziffer);
      }
      if(ISDIGIT(j= *buffer))
         m=(j-'0')*10;
      else if(j>='A' && j<='Z')
         m=(j-'A'+10)*10;
      else if(j>='a' && j<='z')
         m=(j-'a'+10)*10;
      else
         m=0;

      if(ISDIGIT(j= *(buffer+1)))
         m+=j-'0';
      else if(j>='A' && j<='Z')
         m+= j-'A'+10;
      else if(j>='a' && j<='z')
         m+=j-'a'+10;

      blz_array[i]=blz;
      methoden_array[i]=m;
   }
   last_index=i;
   fclose(in);

      /* Format der Lookup-Datei für blz:
       *    - Signatur und Version
       *    - (ab Version 1.1) Infozeile mit Erstellungsdatum und Source-Dateiname
       *                       evl. User-Infozeile
       *    - 4 Byte Anzahl Bankleitzahlen
       *    - 4 Byte Prüfsumme
       *    - Bankleitzahlen (komprimiert):
       *       - bei Differenz zur letzten BLZ
       *            1...250         : *kein* Kennbyte, 1 Byte Differenz
       *            251...65535     : Kennbyte 254, 2 Byte Differenz
       *            >65536/<-65535  : Kennbyte 253, 4 Byte Wert der BLZ, nicht Differenz
       *            -1...-255       : Kennbyte 252, 1 Byte Differenz (Absolutwert)
       *            -256...-65535   : Kennbyte 251, 2 Byte Differenz (Absolutwert)
       *                              Kennbyte 255 ist reserviert
       *       - Falls die BLZ ungültig ist (Feld 1 Nullen), wird zuerst ein Byte FF geschrieben.
       *       - ein Byte mit der zugehörigen Methode.
       */

   for(i=cnt=prev_blz=0,array=blz_array,uptr=outbuffer;i<=last_index && i<MAX_BLZ_CNT;i++){
      blz=array[i];
      diff=blz-prev_blz;
      m=methoden_array[i];
      prev_blz=blz;
      if(diff==0)
         continue;
      else if(diff>0 && diff<=250){
         *uptr++=diff&255;
      }
      else if(diff>250 && diff<65536){   /* 2 Byte, positiv */
         *uptr++=254;
         *uptr++=diff&255;
         diff>>=8;
         *uptr++=diff&255;
      }
      else if(diff<-65535 || diff>65535){   /* Wert direkt eintragen */
         *uptr++=253;
         *uptr++=blz&255;
         blz>>=8;
         *uptr++=blz&255;
         blz>>=8;
         *uptr++=blz&255;
         blz>>=8;
         *uptr++=blz&255;
      }
      else if(diff<-255){   /* 2 Byte, negativ */
         *uptr++=251;
         diff*= -1;
         *uptr++=diff&255;
         diff>>=8;
         *uptr++=diff&255;
      }
      else if(diff<0){   /* 1 Byte, negativ */
         *uptr++=252;
         *uptr++=(-diff)&255;
      }

      if(valid_array[i]==2)*uptr++=0xff;   /* invalid flag schreiben */
      *uptr++=methoden_array[i];              /* Methode */
      cnt++;
   }
   if(!(out=fopen(outputname,"wb"))){
      perror(outputname);
#if THREAD_SAFE
      set_msg_t(FILE_WRITE_ERROR,ctx);
      set_globals(&global_ctx);
#else
      set_msg(FILE_WRITE_ERROR);
#endif
      return(FILE_WRITE_ERROR);
   }
   switch(lut_version){   /* Datei-Signatur */
      case 1:
         fprintf(out,"BLZ Lookup Table/Format 1.0\n");
         break;
      case 2:
         fprintf(out,"BLZ Lookup Table/Format 1.1\n");
         break;
      default:
#if DEBUG
         fprintf(stderr,"Das Dateiformat %d ist nicht definiert; benutze Version 2\n",lut_version);
#endif
         lut_version=2;
         fprintf(out,"BLZ Lookup Table/Format 1.1\n");
         break;
   }
   if(lut_version>1){
      t=time(NULL);
      lt=localtime(&t);
      fprintf(out,"LUT-Datei generiert am %d.%d.%d, %d:%02d aus %s",
         lt->tm_mday,lt->tm_mon+1,lt->tm_year+1900,lt->tm_hour,
         lt->tm_min,inputname);
      if(user_info && *user_info){

            /* newlines sind in der user_info Zeile nicht zulässig; in Blanks umwandeln */
         for(sptr=UCP user_info;*sptr;sptr++)if(*sptr=='\n')*sptr=' ';
         fprintf(out,"\\\n%s\n",user_info);
      }
      else
         fputc('\n',out);
   }
   if(lut_version<3){   /* Prüfziffer-Block direkt schreiben, ohne Header */
      adler=adler32(0,(char *)outbuffer,uptr-outbuffer)^cnt;  /* Prüfsumme */
      WRITE_LONG(cnt);
      WRITE_LONG(adler);
      fwrite((char *)outbuffer,1,uptr-outbuffer,out);       /* BLZ-Liste */
   }
   fclose(out);

   free(blz_array);
   free(methoden_array);
   free(outbuffer);
   cnt_blz=0;
#if THREAD_SAFE
   set_msg_t(OK,ctx);
   set_globals(&global_ctx);
#else
   set_msg(OK);
#endif
   return OK;
}

/* Funktion read_lut() +§§§1 */
/* ###########################################################################
 * # Die Funktion read_lut() liest die Lookup-Tabelle mit Bankleitzahlen und #
 * # Prüfziffermethoden ein und führt einige Konsistenz-Tests durch.         #
 * #                                                                         #
 * # Bugs: für eine BLZ wird nur eine Prüfziffermethode unterstützt (s.o.).  #
 * #                                                                         #
 * # Copyright (C) 2002-2005 Michael Plugge <m.plugge@hs-mannheim.de>        #
 * ###########################################################################
 */

#if THREAD_SAFE
static int read_lut(char *filename)
{
   read_lut_t(filename,&global_ctx);
}

static int read_lut_t(char *filename,KTO_CHK_CTX *ctx)
#else
static int read_lut(char *filename)
#endif
{
   unsigned char *inbuffer,*uptr,*dptr;
   int i,j,prev,cnt,prev_pz,lut_version;
   UINT4 adler1,adler2;
   struct stat s_buf;
   int in;

#if THREAD_SAFE
   if(!b1['1'])init_atoi_table_t(ctx);
#else
   if(!b1['1'])init_atoi_table();
#endif
   if(stat(filename,&s_buf)==-1)return NO_LUT_FILE;
   if(!(inbuffer=calloc(s_buf.st_size+128,1))){
      perror("inbuffer");
#if THREAD_SAFE
      set_msg_t(ERROR_MALLOC,ctx);
#else
      set_msg(ERROR_MALLOC);
#endif
      return ERROR_MALLOC;
   }

   /* fixme: noch optimalen Wert für die Hashtabelle testen */
   /* Werte für die Hashtabelle setzen
    */
#if !FIX_HASH_SHIFT
   if(hash_shift<10)
      hash_shift=10;
   else if(hash_shift>18)
      hash_shift=18;
   switch(hash_shift){
      case 10: hash_base=9700; hash_size=88000; break;
      case 11: hash_base=4880; hash_size=45000; break;
      case 12: hash_base=2400; hash_size=23000; break;
      case 13: hash_base=1200; hash_size=12000; break;
      case 14: hash_base= 600; hash_size= 5700; break;
      case 15:
      default: hash_base= 300; hash_size= 2800; break;
   }
#endif
   if((in=open(filename,O_RDONLY|O_BINARY))<0){
      perror(filename);
#if THREAD_SAFE
      set_msg_t(NO_LUT_FILE,ctx);
#else
      set_msg(NO_LUT_FILE);
#endif
      return NO_LUT_FILE;
   }
   if(!(cnt=read(in,inbuffer,s_buf.st_size))){
      fprintf(stderr,"Problem beim Lesen der LUT-Datei (fread); Abbruch\n%s\n",
         strerror(errno));
#if THREAD_SAFE
      set_msg_t(FATAL_ERROR,ctx);
#else
      set_msg(FATAL_ERROR);
#endif
      return FATAL_ERROR;
   }
   close(in);
   lut_version= -1;
   if(!strncmp((char *)inbuffer,"BLZ Lookup Table/Format 1.0\n",28))
      lut_version=1;
   if(!strncmp((char *)inbuffer,"BLZ Lookup Table/Format 1.1\n",28))
      lut_version=2;
   if(lut_version==-1){
      fprintf(stderr,"falsche Dateisignatur für Lookup Table Datei\n");
#if THREAD_SAFE
      set_msg_t(INVALID_LUT_FILE,ctx);
#else
      set_msg(INVALID_LUT_FILE);
#endif
      return INVALID_LUT_FILE;
   }
   for(uptr=inbuffer,i=cnt;*uptr++!='\n';i--);  /* Signatur */
   if(lut_version==2){  /* LUT-Info kopieren, Info-Zeile überspringen */
      for(i--,j=0,dptr=UCP lut_info;*uptr!='\n';i--,uptr++)
         if(++j<1024)*dptr++= *uptr;
      if(*(uptr-1)=='\\'){
         *--dptr='\n';
         for(i--,uptr++,dptr++;*uptr!='\n';i--,uptr++)
            if(++j<1024)*dptr++= *uptr;
      }
      *dptr=0;
      uptr++;
   }
   else
      *lut_info=0;
   i-=9;
   READ_LONG(cnt);
   READ_LONG(adler1);
   adler2=adler32(0,(char *)uptr,i)^cnt;
   if(cnt>(i-8)/2 || adler1!=adler2)return INVALID_LUT_FILE;

   if(!(blz_array=calloc(j=cnt+100,sizeof(int)))){
      perror("blz");
#if THREAD_SAFE
      set_msg_t(ERROR_MALLOC,ctx);
#else
      set_msg(ERROR_MALLOC);
#endif
      return ERROR_MALLOC;
   }
   if(!(pz_array=calloc(j,sizeof(int)))){
      perror("pz_array");
#if THREAD_SAFE
      set_msg_t(ERROR_MALLOC,ctx);
#else
      set_msg(ERROR_MALLOC);
#endif
      return ERROR_MALLOC;
   }
   if(!(invalid=calloc(j,sizeof(int)))){
      perror("invalid");
#if THREAD_SAFE
   set_msg_t(ERROR_MALLOC,ctx);
#else
   set_msg(ERROR_MALLOC);
#endif
      return ERROR_MALLOC;
   }
   if(!(blz_hash_low=calloc(hash_size+1,sizeof(int)))){
      perror("blz_hash_low");
#if THREAD_SAFE
      set_msg_t(ERROR_MALLOC,ctx);
#else
      set_msg(ERROR_MALLOC);
#endif
      return ERROR_MALLOC;
   }
   if(!(blz_hash_high=calloc(hash_size+1,sizeof(int)))){
      perror("blz_hash_high");
#if THREAD_SAFE
      set_msg_t(ERROR_MALLOC,ctx);
#else
      set_msg(ERROR_MALLOC);
#endif
      return ERROR_MALLOC;
   }
   for(prev=i=0;i<cnt;uptr++){
      if(*uptr<251){
         blz_array[i]=prev+*uptr;
         SET_VARS;
      }
      else switch(*uptr){
         case 251:   /* 2 Byte Differenz (Absolutwert) */
            uptr++;
            blz_array[i]= prev-(*uptr+(*(uptr+1)<<8));
            uptr++;
            SET_VARS;
            break;
         case 252:   /* 1 Byte Differenz (Absolutwert) */
            uptr++;
            blz_array[i]= prev-*uptr;
            SET_VARS;
            break;
         case 253:   /* 4 Byte Wert der BLZ, nicht Differenz */
            uptr++;
            blz_array[i]= *uptr+(*(uptr+1)<<8)+(*(uptr+2)<<16)+(*(uptr+3)<<24);
            uptr+=3;
            SET_VARS;
            break;
         case 254:   /* 2 Byte Differenz */
            uptr++;
            blz_array[i]=prev+*uptr+(*(uptr+1)<<8);
            uptr++;
            SET_VARS;
            break;
         case 255:   /* reserviert */
            return INVALID_LUT_FILE;
      }
   }
   for(;i<cnt+50;i++)blz_array[i]=100000000+i;
   free(inbuffer);
   return cnt;
}

/* Funktion init_atoi_table() +§§§1 */
/* ###########################################################################
 * # Die Funktion init_atoi_table initialisiert die Arrays b1 bis b8, die    #
 * # zur schnellen Umwandlung des Bankleitzahlstrings in eine Zahl dienen.   #
 * # Dazu werden 8 Arrays aufgebaut (für jede Stelle eines); die Umwandlung  #
 * # von String nach int läßt sich damit auf Arrayoperationen und Summierung #
 * # zurückführen, was wesentlich schneller ist, als die sonst nötigen acht  #
 * # Multiplikationen mit dem jeweiligen Stellenwert.                        #
 * #                                                                         #
 * # Copyright (C) 2002-2005 Michael Plugge <m.plugge@hs-mannheim.de>        #
 * ###########################################################################
 */

#if THREAD_SAFE
static void init_atoi_table(void)
{
   init_atoi_table_t(&global_ctx);
}

static void init_atoi_table_t(KTO_CHK_CTX *ctx)
#else
static void init_atoi_table(void)
#endif
{
   int i,ziffer;

   /* ungültige Ziffern (Blanks und Tabs werden ebenfalls als ungültig
    * angesehen(!), da die Stellenzuordnung sonst nicht mehr stimmt)
    */
   for(i=0;i<256;i++)b1[i]=b2[i]=b3[i]=b4[i]=b5[i]=b6[i]=b7[i]=b8[i]=bx1[i]=by1[i]=bx2[i]=by4[i]=0xfffffff;

      /* eigentliche Ziffern belegen */
   for(i='0';i<='9';i++){
      ziffer=i-'0';
      bx1[i]=by1[i]=b1[i]=ziffer; ziffer*=10;
      bx2[i]=b2[i]=ziffer; ziffer*=10;
      b3[i]=ziffer; ziffer*=10;
      by4[i]=b4[i]=ziffer; ziffer*=10;
      b5[i]=ziffer; ziffer*=10;
      b6[i]=ziffer; ziffer*=10;
      b7[i]=ziffer; ziffer*=10;
      b8[i]=ziffer;
   }
   for(i='a';i<'z';i++){ /* Sonderfall für bx1, bx2 und by4: Buchstaben a-z => 10...36 (Prüfziffermethoden) */
      bx1[i]=(i-'a'+10);
      by1[i]=(i-'a'+1);
      bx2[i]=bx1[i]*10;
      by4[i]=(i-'a'+1)*1000;
   }
   for(i='A';i<'Z';i++){ /* Sonderfall für bx1, bx2 und by4: Buchstaben A-Z => 10...36 (Untermethoden) */
      bx1[i]=(i-'A'+10);
      by1[i]=(i-'A'+1);
      bx2[i]=bx1[i]*10;
      by4[i]=(i-'A'+1)*1000;
   }
}

/* Funktion kto_check_int() +§§§1
   Prolog +§§§2 */
/* ###########################################################################
 * # Die Funktion kto_check_int() ist die interne Funktion zur Überprüfung   #
 * # einer Kontonummer. Falls die Lookup-Table noch nicht eingelesen wurde,  #
 * # wird sie durch diese Funktion eingelesen.                               #
 * #                                                                         #
 * # Parameter:                                                              #
 * #    pz_or_blz:  Prüfziffer (2-stellig) oder BLZ (8-stellig)              #
 * #    kto:        Kontonummer                                              #
 * #    lut_name:   Name der Lookup-Datei oder NULL (für DEFAULT_LUT_NAME)   #
 * #                                                                         #
 * #                                                                         #
 * # Copyright (C) 2002-2005 Michael Plugge <m.plugge@hs-mannheim.de>        #
 * ###########################################################################
 */

#if THREAD_SAFE
static int kto_check_int(char *pz_or_blz,char *kto,char *lut_name)
{
   kto_check_int_t(pz_or_blz,kto,lut_name,&global_ctx);
}

static int kto_check_int_t(char *pz_or_blz,char *kto,char *lut_name,KTO_CHK_CTX *ctx)
#else
static int kto_check_int(char *pz_or_blz,char *kto,char *lut_name)
#endif
{
   char *ptr,*dptr,kto_alt[32],xkto[32];
   int i,p1,h,ret_val,tmp,kto_len,pz1;
#if DEBUG
	int untermethode;
#endif

/* Initialisierung +§§§2 */
   if(!lut_name || !*lut_name)lut_name=DEFAULT_LUT_NAME;
   kto_len=strlen(kto);
   if(kto_len>10 || kto_len==0)return INVALID_KTO_LENGTH;
   if(!*pz_or_blz)return INVALID_BLZ;
#if THREAD_SAFE
   if(!b1['1'])init_atoi_table_t(ctx);
#else
   if(!b1['1'])init_atoi_table();
#endif

#if DEBUG
         /* zwei Ziffern: Prüfziffermethode angegeben */
   untermethode=0;
   if(!*(pz_or_blz+2))
      pz_methode=bx2[(int)*pz_or_blz]+bx1[(int)*(pz_or_blz+1)];

      /* drei Ziffern: Prüfziffermethode + Untermethode (3. Stelle) */
   else if(!*(pz_or_blz+3)){
      pz_methode=bx2[(int)*pz_or_blz]+bx1[(int)*(pz_or_blz+1)]+by4[(int)*(pz_or_blz+2)];
      untermethode=by1[(int)*(pz_or_blz+2)];
   }
   else{
#endif

         /* BLZ angegeben; entsprechende Prüfmethode suchen.
          *
          * Falls die LUT-Tabelle noch nicht initialisiert ist, muß das
          * jetzt geschehen; u.U. Fehler zurückgeben (ret-Wert<0)
          */

#if THREAD_SAFE
      if(cnt_blz==0 && (cnt_blz=read_lut_t(lut_name,ctx))<=0){
         ret_val=cnt_blz;
         cnt_blz=0;
         return ret_val;
      }
#else
      if(cnt_blz==0 && (cnt_blz=read_lut(lut_name))<=0){
         ret_val=cnt_blz;
         cnt_blz=0;
         return ret_val;
      }
#endif

         /* die Bankleitzahl muß genau 8 stellen haben, sonst Fehlermeldung */
      tmp = b1[(unsigned int)pz_or_blz[7]]
          + b2[(unsigned int)pz_or_blz[6]]
          + b3[(unsigned int)pz_or_blz[5]]
          + b4[(unsigned int)pz_or_blz[4]]
          + b5[(unsigned int)pz_or_blz[3]]
          + b6[(unsigned int)pz_or_blz[2]]
          + b7[(unsigned int)pz_or_blz[1]]
          + b8[(unsigned int)pz_or_blz[0]];
      if(tmp>=0xfffffff || pz_or_blz[8])return INVALID_BLZ_LENGTH;
      h=(tmp>>hash_shift)-hash_base; /* Hashwert für schnelle Suche im BLZ-Array */
      if(h<0)
         h=0;
      else if(h>HASH_SIZE)
         h=HASH_SIZE;

#if BLZ_BIG_JUMP
      for(i=blz_hash_low[h];blz_array[i]<tmp;i+=4);
      if(blz_array[i]!=tmp && ((i>0 && blz_array[--i]!=tmp && blz_array[--i]!=tmp && blz_array[--i]!=tmp || i==0)))return INVALID_BLZ;
#else
      for(i=blz_hash_low[h];blz_array[i]<tmp;i+=2);
      if(blz_array[i]!=tmp && (i>0 && blz_array[--i]!=tmp || i==0))return INVALID_BLZ;
#endif
      pz_methode=pz_array[i];
#if DEBUG
   }
#endif

      /* Konto links mit Nullen auf 10 Stellen auffüllen,
       * und in die lokale Variable xkto umkopieren.
       * Es wird die Funktion memset() benutzt, da diese (beim
       * gcc mit Optimierung) in drei mov-Befehle umgesetzt wird.
       */
   memset(xkto,'0',10);
   dptr=xkto+10-kto_len;
   for(ptr=kto;*dptr++= *ptr++;);
   kto=xkto;
   *((UINT4*)pz_str)=0;

/* Methoden der Prüfzifferberechnung +§§§2
   Prolog +§§§3 */
/*
 * ######################################################################
 * #               Methoden der Prüfzifferberechnung                    #
 * ######################################################################
 */

   switch(pz_methode){

/* Berechnungsmethoden 00 bis 09 +§§§3
   Berechnung nach der Methode 00 +§§§4 */
/*
 * ######################################################################
 * #               Berechnung nach der Methode 00                       #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2.                  #
 * # Die einzelnen Stellen der Kontonummer sind von rechts nach         #
 * # links mit den Ziffern 2, 1, 2, 1, 2 usw. zu multiplizieren.        #
 * # Die jeweiligen Produkte werden addiert, nachdem jeweils aus        #
 * # den zweistelligen Produkten die Quersumme gebildet wurde           #
 * # (z.B. Produkt 16 = Quersumme 7). Nach der Addition bleiben         #
 * # außer der Einerstelle alle anderen Stellen unberücksichtigt.       #
 * # Die Einerstelle wird von dem Wert 10 subtrahiert. Das Ergebnis     #
 * # ist die Prüfziffer. Ergibt sich nach der Subtraktion der           #
 * # Rest 10, ist die Prüfziffer 0.                                     #
 * ######################################################################
 */

      CASE0(0)

            /* Alpha: etwas andere Berechnung, wesentlich schneller
             * (benötigt nur 75 statt 123 Takte, falls Berechnung wie Intel.
             * Die Intel-Methode wäre somit sogar langsamer als die alte Version
             * mit 105 Takten)
             */
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;


/*  Berechnung nach der Methode 01 +§§§4 */
/*
 * ######################################################################
 * #               Berechnung nach der Methode 01                       #
 * ######################################################################
 * # Modulus 10, Gewichtung 3, 7, 1, 3, 7, 1, 3, 7, 1.                  #
 * # Die einzelnen Stellen der Kontonummer sind von rechts nach         #
 * # links mit den Ziffern 3, 7, 1, 3, 7, 1 usw. zu multiplizieren.     #
 * # Die jeweiligen Produkte werden addiert. Nach der Addition          #
 * # bleiben außer der Einerstelle alle anderen Stellen                 #
 * # Unberücksichtigt. Die Einerstelle wird von dem Wert 10             #
 * # subtrahiert. Das Ergebnis ist die Prüfziffer. Ergibt sich nach     #
 * # der Subtraktion der Rest 10, ist die Prüfziffer 0.                 #
 * ######################################################################
 */
      CASE0(1)
         pz = (kto[0]-'0')
            + (kto[1]-'0') * 7
            + (kto[2]-'0') * 3
            + (kto[3]-'0')
            + (kto[4]-'0') * 7
            + (kto[5]-'0') * 3
            + (kto[6]-'0')
            + (kto[7]-'0') * 7
            + (kto[8]-'0') * 3;

         MOD_10_160;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 02 +§§§4 */
/*
 * ######################################################################
 * #               Berechnung nach der Methode 02                       #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 2.                  #
 * # Die einzelnen Stellen der Kontonummer sind von rechts nach         #
 * # links mit den Ziffern 2, 3, 4, 5, 6, 7, 8, 9, 2 zu                 #
 * # multiplizieren. Die jeweiligen Produkte werden addiert.            #
 * # Die Summe ist durch 11 zu dividieren. Der verbleibende Rest        #
 * # wird vom Divisor (11) subtrahiert. Das Ergebnis ist die            #
 * # Prüfziffer. Verbleibt nach der Division durch 11 kein Rest,        #
 * # ist die Prüfziffer 0. Ergibt sich als Rest 1, ist die              #
 * # Prüfziffer zweistellig und kann nicht verwendet werden.            #
 * # Die Kontonummer ist dann nicht verwendbar.                         #
 * ######################################################################
 */
      CASE0(2)
         pz = (kto[0]-'0') * 2
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz)pz=11-pz;
         if(pz==10)return INVALID_KTO;
         CHECK_PZ10;

/*  Berechnung nach der Methode 03 +§§§4 */
/*
 * ######################################################################
 * #               Berechnung nach der Methode 03                       #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2.                  #
 * # Die Berechnung erfolgt wie bei Verfahren 01.                       #
 * ######################################################################
 */
      CASE0(3)
         pz = (kto[0]-'0') * 2
            + (kto[1]-'0')
            + (kto[2]-'0') * 2
            + (kto[3]-'0')
            + (kto[4]-'0') * 2
            + (kto[5]-'0')
            + (kto[6]-'0') * 2
            + (kto[7]-'0')
            + (kto[8]-'0') * 2;

         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 04 +§§§4 */
/*
 * ######################################################################
 * #               Berechnung nach der Methode 04                       #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 2, 3, 4.                  #
 * # Die Berechnung erfolgt wie bei Verfahren 02.                       #
 * ######################################################################
 */
      CASE0(4)
         pz = (kto[0]-'0') * 4
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 2
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz)pz=11-pz;
         if(pz==10)return INVALID_KTO;
         CHECK_PZ10;

/*  Berechnung nach der Methode 05 +§§§4 */
/*
 * ######################################################################
 * #               Berechnung nach der Methode 05                       #
 * ######################################################################
 * # Modulus 10, Gewichtung 7, 3, 1, 7, 3, 1, 7, 3, 1.                  #
 * # Die Berechnung erfolgt wie bei Verfahren 01.                       #
 * ######################################################################
 */
      CASE0(5)
         pz = (kto[0]-'0')
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 7
            + (kto[3]-'0')
            + (kto[4]-'0') * 3
            + (kto[5]-'0') * 7
            + (kto[6]-'0')
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 7;

         MOD_10_160;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 06 +§§§4 */
/*
 * ######################################################################
 * #               Berechnung nach der Methode 06                       #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7 (modifiziert)              #
 * # Die einzelnen Stellen der Kontonummer sind von rechts nach         #
 * # links mit den Ziffern 2, 3, 4, 5, 6, 7, 2, 3 ff. zu multiplizieren.#
 * # Die jeweiligen Produkte werden addiert. Die Summe ist              #
 * # durch 11 zu dividieren. Der verbleibende Rest wird vom             #
 * # Divisor (11) subtrahiert. Das Ergebnis ist die Prüfziffer.         #
 * # Ergibt sich als Rest 1, findet von dem Rechenergebnis 10           #
 * # nur die Einerstelle (0) als Prüfziffer Verwendung. Verbleibt       #
 * # nach der Division durch 11 kein Rest, dann ist auch die            #
 * # Prüfziffer 0. Die Stelle 10 der Kontonummer ist die Prüfziffer.    #
 * ######################################################################
 */
      CASE0(6)
         pz = (kto[0]-'0') * 4
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 2
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 07 +§§§4 */
/*
 * ######################################################################
 * #               Berechnung nach der Methode 07                       #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 10.                 #
 * # Die Berechnung erfolgt wie bei Verfahren 02.                       #
 * ######################################################################
 */
      CASE0(7)
         pz = (kto[0]-'0') * 10
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz)pz=11-pz;
         if(pz==10)return INVALID_KTO;
         CHECK_PZ10;

/*  Berechnung nach der Methode 08 +§§§4 */
/*
 * ######################################################################
 * #               Berechnung nach der Methode 08                       #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2 (modifiziert).    #
 * # Die Berechnung erfolgt wie bei Verfahren 00, jedoch erst           #
 * # ab der Kontonummer 60 000.                                         #
 * ######################################################################
 */
      CASE0(8)
         if(strcmp(kto,"0000060000")<0)  /* Ausnahmen: keine Prüfzifferberechnung */
            return OK_NO_CHK;
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 09 +§§§4 */
/*
 * ######################################################################
 * #               Berechnung nach der Methode 09                       #
 * ######################################################################
 * # Keine Prüfziffernberechung (es wird immer richtig zurückgegeben).  #
 * ######################################################################
 */
      CASE0(9)
         return OK_NO_CHK;

/* Berechnungsmethoden 10 bis 19 +§§§3
   Berechnung nach der Methode 10 +§§§4 */
/*
 * ######################################################################
 * #               Berechnung nach der Methode 10                       #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 10 (modifiziert).   #
 * # Die Berechnung erfolgt wie bei Verfahren 06.                       #
 * ######################################################################
 */
      CASE0(10)
         pz = (kto[0]-'0') * 10
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 11 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 11                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 10 (modifiziert).   #
 * # Die Berechnung erfolgt wie bei Verfahren 06. Beim Rechenergebnis   #
 * # 10 wird die Null jedoch durch eine 9 ersetzt.                      #
 * ######################################################################
 */
      CASE0(11)
         pz = (kto[0]-'0') * 10
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz==1)
            pz=9;
         else if(pz>1)
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 12 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 12                        #
 * ######################################################################
 * # frei/nicht definiert                                               #
 * # Beim Aufruf dieser Methode wird grundsätzlich ein Fehler zurück-   #
 * # gegeben, um nicht eine (evl. falsche) Implementation vorzutäuschen.#
 * ######################################################################
 */
      CASE0(12) /* frei */
         return NOT_DEFINED;

/*  Berechnung nach der Methode 13 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 13                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1.                           #
 * # Die Berechnung erfolgt wie bei Verfahren 00. Es ist jedoch zu      #
 * # beachten, daß die zweistellige Unterkonto-Nummer (Stellen 9        #
 * # und 10) nicht in das Prüfziffernberechnungsverfahren mit           #
 * # einbezogen werden darf. Die für die Berechnung relevante           #
 * # sechsstellige Grundnummer befindet sich in den Stellen 2 bis 7,    #
 * # die Prüfziffer in Stelle 8. Die Kontonummer ist neunstellig,       #
 * # Stelle 1 ist also unbenutzt.                                       #
 * # Ist die obengenannte Unternummer = 00 kommt es vor, daß sie        #
 * # auf den Zahlungsverkehrsbelegen nicht angegeben ist. Ergibt        #
 * # die erste Berechnung einen Prüfziffernfehler, wird empfohlen,      #
 * # die Prüfziffernberechnung ein zweites Mal durchzuführen und        #
 * # dabei die "gedachte" Unternummer 00 zu berücksichtigen.            #
 * ######################################################################
 */
      CASE0(13)
      CASE_U(13,1)
#ifdef __ALPHA
         pz =  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9);
#else
         pz =(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0');
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX8;

      CASE_U(13,2)
#ifdef __ALPHA
         pz =  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz =(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif

         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 14 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 14                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7.                           #
 * # Die Berechnung erfolgt wie bei Verfahren 02. Es ist jedoch zu      #
 * # beachten, daß die zweistellige Kontoart nicht in das Prüfziffern-  #
 * # berechnungsverfahren mit einbezogen wird. Die Kontoart belegt      #
 * # die Stellen 2 und 3, die zu berechnende Grundnummer die Stellen    #
 * # 4 bis 9. Die Prüfziffer befindet sich in Stelle 10.                #
 * ######################################################################
 */
      CASE0(14)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz)pz=11-pz;
         if(pz==10)return INVALID_KTO;
         CHECK_PZ10;

/*  Berechnung nach der Methode 15 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 15                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5.                                 #
 * # Die Berechnung erfolgt wie bei Verfahren 06. Es ist jedoch zu      #
 * # beachten, daß nur die vierstellige Kontonummer in das              #
 * # Prüfziffernberechnungsverfahren einbezogen wird. Sie befindet      #
 * # sich in den Stellen 6 bis 9, die Prüfziffer in Stelle 10           #
 * # der Kontonummer.                                                   #
 * ######################################################################
 */
      CASE0(15)
         pz = (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_88;    /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 16 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 16                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 2, 3, 4                   #
 * # Die Berechnung erfolgt wie bei Verfahren 06. Sollte sich jedoch    #
 * # nach der Division der Rest 1 ergeben, so ist die Kontonummer       #
 * # unabhängig vom eigentlichen Berechnungsergebnis                    #
 * # richtig, wenn die Ziffern an 10. und 9. Stelle identisch sind.     #
 * ######################################################################
 */
      CASE0(16)
         pz = (kto[0]-'0') * 4
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 2
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz)pz=11-pz;
         if(pz==10){
            if(*(kto+8)== *(kto+9)){
#if DEBUG
               pz= *(kto+9)-'0';
#endif
               return OK;
            }
            else
               pz=0;
         }
         CHECK_PZ10;

/*  Berechnung nach der Methode 17 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 17 (neu)                  #
 * ######################################################################
 * # Modulus 11, Gewichtung 1, 2, 1, 2, 1, 2                            #
 * # Die Kontonummer ist 10-stellig mit folgendem Aufbau:               #
 * #                                                                    #
 * #     KSSSSSSPUU                                                     #
 * #     K = Kontoartziffer                                             #
 * #     S = Stammnummer                                                #
 * #     P = Prüfziffer                                                 #
 * #     U = Unterkontonummer                                           #
 * #                                                                    #
 * # Die für die Berechnung relevante 6-stellige Stammnummer            #
 * # (Kundennummer) befindet sich in den Stellen 2 bis 7 der            #
 * # Kontonummer, die Prüfziffer in der Stelle 8. Die einzelnen         #
 * # Stellen der Stammnummer (S) sind von links nach rechts mit         #
 * # den Ziffern 1, 2, 1, 2, 1, 2 zu multiplizieren. Die                #
 * # jeweiligen Produkte sind zu addieren, nachdem aus eventuell        #
 * # zweistelligen Produkten der 2., 4. und 6. Stelle der               #
 * # Stammnummer die Quersumme gebildet wurde. Von der Summe ist        #
 * # der Wert "1" zu subtrahieren. Das Ergebnis ist dann durch          #
 * # 11 zu dividieren. Der verbleibende Rest wird von 10                #
 * # subtrahiert. Das Ergebnis ist die Prüfziffer. Verbleibt            #
 * # nach der Division durch 11 kein Rest, ist die Prüfziffer 0.        #
 * ######################################################################
 */

      CASE0(17)
#ifdef __ALPHA
         pz =  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9);
#else
         pz =(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0');
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
#endif

         pz-=1;
         MOD_11_44;   /* pz%=11 */
         if(pz)pz=10-pz;
         CHECK_PZ8;

/*  Berechnung nach der Methode 18 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 18                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 3, 9, 7, 1, 3, 9, 7, 1, 3.                  #
 * # Die Berechnung erfolgt wie bei Verfahren 01.                       #
 * ######################################################################
 */
      CASE0(18)
         pz = (kto[0]-'0') * 3
            + (kto[1]-'0')
            + (kto[2]-'0') * 7
            + (kto[3]-'0') * 9
            + (kto[4]-'0') * 3
            + (kto[5]-'0')
            + (kto[6]-'0') * 7
            + (kto[7]-'0') * 9
            + (kto[8]-'0') * 3;

         MOD_10_320;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 19 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 19                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 1.                  #
 * # Die Berechnung erfolgt wie bei Verfahren 06.                       #
 * ######################################################################
 */
      CASE0(19)
         pz = (kto[0]-'0')
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/* Berechnungsmethoden 20 bis 29 +§§§3
   Berechnung nach der Methode 20 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 20                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 3 (modifiziert).    #
 * # Die Berechnung erfolgt wie bei Verfahren 06.                       #
 * ######################################################################
 */
      CASE0(20)
         pz = (kto[0]-'0') * 3
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 21 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 21                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2 (modifiziert).    #
 * # Die Berechnung erfolgt wie bei Verfahren 00. Nach der Addition     #
 * # der Produkte werden neben der Einerstelle jedoch alle Stellen      #
 * # berücksichtigt, indem solange Quersummen gebildet werden, bis      #
 * # ein einstelliger Wert verbleibt. Die Differenz zwischen diesem     #
 * # Wert und dem Wert 10 ist die Prüfziffer.                           #
 * ######################################################################
 */
      CASE0(21)
         pz = (kto[0]-'0') * 2
            + (kto[1]-'0')
            + (kto[2]-'0') * 2
            + (kto[3]-'0')
            + (kto[4]-'0') * 2
            + (kto[5]-'0')
            + (kto[6]-'0') * 2
            + (kto[7]-'0')
            + (kto[8]-'0') * 2;

         if(pz>=80)pz=pz-80+8;
         if(pz>=40)pz=pz-40+4;
         if(pz>=20)pz=pz-20+2;
         if(pz>=10)pz=pz-10+1;
         if(pz>=10)pz=pz-10+1;
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 22 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 22                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 3, 1, 3, 1, 3, 1, 3, 1, 3.                  #
 * # Die einzelnen Stellen der Kontonummer sind von rechts nach         #
 * # links mit den Ziffern 3, 1, 3, 1 usw. zu multiplizieren.           #
 * # Von den jeweiligen Produkten bleiben die Zehnerstellen             #
 * # unberücksichtigt. Die verbleibenden Zahlen (Einerstellen)          #
 * # werden addiert. Die Differenz bis zum nächsten Zehner ist          #
 * # die Prüfziffer.                                                    #
 * ######################################################################
 */
      CASE0(22)
         pz = (kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');

            if(kto[0]<'4')
               pz+=(kto[0]-'0')*3;
            else if(kto[0]<'7')
               pz+=(kto[0]-'0')*3-10;
            else
               pz+=(kto[0]-'0')*3-20;

            if(kto[2]<'4')
               pz+=(kto[2]-'0')*3;
            else if(kto[2]<'7')
               pz+=(kto[2]-'0')*3-10;
            else
               pz+=(kto[2]-'0')*3-20;

            if(kto[4]<'4')
               pz+=(kto[4]-'0')*3;
            else if(kto[4]<'7')
               pz+=(kto[4]-'0')*3-10;
            else
               pz+=(kto[4]-'0')*3-20;

            if(kto[6]<'4')
               pz+=(kto[6]-'0')*3;
            else if(kto[6]<'7')
               pz+=(kto[6]-'0')*3-10;
            else
               pz+=(kto[6]-'0')*3-20;

            if(kto[8]<'4')
               pz+=(kto[8]-'0')*3;
            else if(kto[8]<'7')
               pz+=(kto[8]-'0')*3-10;
            else
               pz+=(kto[8]-'0')*3-20;

         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 23 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 23                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7.                           #
 * # Das Prüfziffernverfahren entspricht dem der Kennziffer 16,         #
 * # wird jedoch nur auf die ersten sechs Ziffern der Kontonummer       #
 * # angewandt. Die Prüfziffer befindet sich an der 7. Stelle der       #
 * # Kontonummer. Die drei folgenden Stellen bleiben ungeprüft.         #
 * # Sollte sich nach der Division der Rest 1 ergeben, so ist           #
 * # die Kontonummer unabhängig vom eigentlichen Berechnungsergebnis    #
 * # richtig, wenn die Ziffern an 6. und 7. Stelle identisch sind.      #
 * ######################################################################
 */
      CASE0(23)
         pz = (kto[0]-'0') * 7
            + (kto[1]-'0') * 6
            + (kto[2]-'0') * 5
            + (kto[3]-'0') * 4
            + (kto[4]-'0') * 3
            + (kto[5]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz)pz=11-pz;
         if(pz==10){
            if(*(kto+5)== *(kto+6)){
#if DEBUG
               pz= *(kto+6)-'0';
#endif
               return OK;
            }
            else
               return INVALID_KTO;
         }
         CHECK_PZ7;

/*  Berechnung nach der Methode 24 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 24                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 1, 2, 3, 1, 2, 3, 1, 2, 3                   #
 * # Die für die Berechnung relevanten Stellen der Kontonummer          #
 * # befinden sich in den Stellen 1 - 9 und die Prüfziffer in           #
 * # Stelle 10 des zehnstelligen Kontonummernfeldes. Eine evtl.         #
 * # in Stelle 1 vorhandene Ziffer 3, 4, 5, 6 wird wie 0 gewertet.      #
 * # Eine ggf. in Stelle 1 vorhandene Ziffer 9 wird als 0 gewertet und  #
 * # führt dazu, dass auch die beiden nachfolgenden Ziffern in den      #
 * # Stellen 2 und 3 der Kontonummer als 0 gewertet werden müssen. Der  #
 * # o. g. Prüfalgorithmus greift in diesem Fall also erst ab Stelle 4  #
 * # der 10stelligen Kontonummer. Die Stelle 4 ist ungleich 0.          #
 * #                                                                    #
 * # Die einzelnen Stellen der Kontonummer sind von                     #
 * # links nach rechts, beginnend mit der ersten signifikanten          #
 * # Ziffer (Ziffer ungleich 0) in den Stellen 1 - 9, mit den           #
 * # Ziffern 1, 2, 3, 1, 2, 3, 1, 2, 3 zu multiplizieren. Zum           #
 * # jeweiligen Produkt ist die entsprechende Gewichtungsziffer         #
 * # zu addieren (zum ersten Produkt + 1; zum zweiten Produkt + 2;      #
 * # zum dritten Produkt +3; zum vierten Produkt + 1 usw.).             #
 * # Die einzelnen Rechenergebnisse sind durch 11 zu dividieren.        #
 * # Die sich nach der Division ergebenden Reste sind zu summieren.     #
 * # Die Summe der Reste ist durch 10 zu dividieren. Der sich           #
 * # danach ergebende Rest ist die Prüfziffer.                          #
 * ######################################################################
 */
      CASE0(24)
         if(*kto>='3' && *kto<='6')*kto='0';
         if(*kto=='9')*kto= *(kto+1)= *(kto+2)='0';
         for(ptr=kto;*ptr=='0';ptr++);
         for(i=0,pz=0;ptr<kto+9;ptr++,i++){
            p1=(*ptr-'0')*w24[i]+w24[i];
            SUB1_22; /* p1%=11 */
            pz+=p1;
         }
         MOD_10_80;     /* pz%=10 */
         CHECK_PZ10;

/*  Berechnung nach der Methode 25 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 25                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9 ohne Quersumme.      #
 * # Die einzelnen Stellen der Kontonummer sind von rechts nach         #
 * # links mit den Ziffern 2, 3, 4, 5, 6, 7, 8, 9 zu multiplizieren.    #
 * # Die jeweiligen Produkte werden addiert. Die Summe ist durch        #
 * # 11 zu dividieren. Der verbleibende Rest wird vom Divisor (11)      #
 * # subtrahiert. Das Ergebnis ist die Prüfziffer. Verbleibt nach       #
 * # der Division durch 11 kein Rest, ist die Prüfziffer 0.             #
 * # Ergibt sich als Rest 1, ist die Prüfziffer immer 0 und kann        #
 * # nur für die Arbeitsziffer 8 und 9 verwendet werden. Die            #
 * # Kontonummer ist für die Arbeitsziffer 0, 1, 2, 3, 4, 5, 6          #
 * # und 7 dann nicht verwendbar.                                       #
 * # Die Arbeitsziffer (Geschäftsbereich oder Kontoart) befindet        #
 * # sich in der 2. Stelle (von links) des zehnstelligen                #
 * # Kontonummernfeldes.                                                #
 * ######################################################################
 */
      CASE0(25)
         pz = (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz)pz=11-pz;
         if(pz==10){
            pz=0;
            if(*(kto+1)!='8' && *(kto+1)!='9')return INVALID_KTO;
         }
         CHECK_PZ10;

/*  Berechnung nach der Methode 26 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 26                        #
 * ######################################################################
 * # Modulus 11. Gewichtung 2, 3, 4, 5, 6, 7, 2                         #
 * # Die Kontonummer ist 10-stellig. Sind Stelle 1 und 2 mit            #
 * # Nullen gefüllt ist die Kontonummer um 2 Stellen nach links         #
 * # zu schieben und Stelle 9 und 10 mit Nullen zu füllen. Die          #
 * # Berechnung erfolgt wie bei Verfahren 06 mit folgender              #
 * # Modifizierung: für die Berechnung relevant sind die Stellen 1      #
 * # - 7; die Prüfziffer steht in Stelle 8. Bei den Stellen 9 und 10    #
 * # handelt es sich um eine Unterkontonummer, welche für die           #
 * # Berechnung nicht berücksichtigt wird.                              #
 * ######################################################################
 */
      CASE0(26)
         if(*kto=='0' && *(kto+1)=='0'){
         pz = (kto[2]-'0') * 2
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZ10;
         }
         else{
            pz = (kto[0]-'0') * 2
               + (kto[1]-'0') * 7
               + (kto[2]-'0') * 6
               + (kto[3]-'0') * 5
               + (kto[4]-'0') * 4
               + (kto[5]-'0') * 3
               + (kto[6]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZ8;
         }

/*  Berechnung nach der Methode 27 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 27                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2 (modifiziert).    #
 * # Die Berechnung erfolgt wie bei Verfahren 00, jedoch nur für        #
 * # die Kontonummern von 1 bis 999.999.999. Ab Konto 1.000.000.000     #
 * # kommt das Prüfziffernverfahren M10H (Iterierte Transformation)     #
 * # zum Einsatz.                                                       #
 * # Es folgt die Beschreibung der iterierten Transformation:           #
 * # Die Position der einzelnen Ziffer von rechts nach links            #
 * # innerhalb der Kontonummer gibt die Zeile 1 bis 4 der               #
 * # Transformationstabelle an. Aus ihr sind die Übersetzungswerte      #
 * # zu summieren. Die Einerstelle wird von 10 subtrahiert und          #
 * # stellt die Prüfziffer dar.                                         #
 * # Transformationstabelle:                                            #
 * #    Ziffer    0 1 2 3 4 5 6 7 8 9                                   #
 * #    Ziffer 1  0 1 5 9 3 7 4 8 2 6                                   #
 * #    Ziffer 2  0 1 7 6 9 8 3 2 5 4                                   #
 * #    Ziffer 3  0 1 8 4 6 2 9 5 7 3                                   #
 * #    Ziffer 4  0 1 2 3 4 5 6 7 8 9                                   #
 * ######################################################################
 */
      CASE0(27)
         if(*kto=='0'){ /* Kontonummern von 1 bis 999.999.999 */
#ifdef __ALPHA
         pz =  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz =(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         }
         else{
            pz = m10h_digits[0][(unsigned int)(kto[0]-'0')]
               + m10h_digits[3][(unsigned int)(kto[1]-'0')]
               + m10h_digits[2][(unsigned int)(kto[2]-'0')]
               + m10h_digits[1][(unsigned int)(kto[3]-'0')]
               + m10h_digits[0][(unsigned int)(kto[4]-'0')]
               + m10h_digits[3][(unsigned int)(kto[5]-'0')]
               + m10h_digits[2][(unsigned int)(kto[6]-'0')]
               + m10h_digits[1][(unsigned int)(kto[7]-'0')]
               + m10h_digits[0][(unsigned int)(kto[8]-'0')];
            MOD_10_80;   /* pz%=10 */
         }
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 28 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 28                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8.                        #
 * # Innerhalb der 10stelligen Kontonummer ist die 8. Stelle die        #
 * # Prüfziffer. Die 9. und 10. Stelle der Kontonummer sind             #
 * # Unterkontonummern, die nicht in die Prüfziffernberechnung          #
 * # einbezogen sind.                                                   #
 * # Jede Stelle der Konto-Stamm-Nummer wird mit einem festen           #
 * # Stellenfaktor (Reihenfolge 8, 7, 6, 5, 4, 3, 2) multipliziert.     #
 * # Die sich ergebenden Produkte werden addiert. Die aus der           #
 * # Addition erhaltene Summe wird durch 11 dividiert. Der Rest         #
 * # wird von 11 subtrahiert. Die Differenz wird der Konto-Stamm-       #
 * # Nummer als Prüfziffer beigefügt. Wird als Rest eine 0 oder         #
 * # eine 1 ermittelt, so lautet die Prüfziffer 0.                      #
 * ######################################################################
 */
      CASE0(28)
         pz = (kto[0]-'0') * 8
            + (kto[1]-'0') * 7
            + (kto[2]-'0') * 6
            + (kto[3]-'0') * 5
            + (kto[4]-'0') * 4
            + (kto[5]-'0') * 3
            + (kto[6]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ8;

/*  Berechnung nach der Methode 29 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 29                        #
 * ######################################################################
 * # Modulus 10, Iterierte Transformation.                              #
 * # Die einzelnen Ziffern der Kontonummer werden über eine Tabelle     #
 * # in andere Werte transformiert. Jeder einzelnen Stelle der          #
 * # Kontonummer ist hierzu eine der Zeilen 1 bis 4 der Transforma-     #
 * # tionstabelle fest zugeordnet. Die Transformationswerte werden      #
 * # addiert. Die Einerstelle der Summe wird von 10 subtrahiert.        #
 * # Das Ergebnis ist die Prüfziffer. Ist das Ergebnis = 10, ist        #
 * # die Prüfziffer = 0.                                                #
 * # Transformationstabelle:                                            #
 * #    Ziffer    0 1 2 3 4 5 6 7 8 9                                   #
 * #    Ziffer 1  0 1 5 9 3 7 4 8 2 6                                   #
 * #    Ziffer 2  0 1 7 6 9 8 3 2 5 4                                   #
 * #    Ziffer 3  0 1 8 4 6 2 9 5 7 3                                   #
 * #    Ziffer 4  0 1 2 3 4 5 6 7 8 9                                   #
 * ######################################################################
 */
      CASE0(29)
         pz = m10h_digits[0][(unsigned int)(kto[0]-'0')]
            + m10h_digits[3][(unsigned int)(kto[1]-'0')]
            + m10h_digits[2][(unsigned int)(kto[2]-'0')]
            + m10h_digits[1][(unsigned int)(kto[3]-'0')]
            + m10h_digits[0][(unsigned int)(kto[4]-'0')]
            + m10h_digits[3][(unsigned int)(kto[5]-'0')]
            + m10h_digits[2][(unsigned int)(kto[6]-'0')]
            + m10h_digits[1][(unsigned int)(kto[7]-'0')]
            + m10h_digits[0][(unsigned int)(kto[8]-'0')];
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/* Berechnungsmethoden 30 bis 39 +§§§3
   Berechnung nach der Methode 30 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 30                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 0, 0, 0, 0, 1, 2, 1, 2.                  #
 * # Die letzte Stelle ist per Definition die Prüfziffer. Die           #
 * # einzelnen Stellen der Kontonummer sind ab der ersten Stelle von    #
 * # links nach rechts mit den Ziffern 2, 0, 0, 0, 0, 1, 2, 1, 2 zu     #
 * # multiplizieren. Die jeweiligen Produkte werden addiert (ohne       #
 * # Quersummenbildung). Die weitere Berechnung erfolgt wie bei         #
 * # Verfahren 00.                                                      #
 * ######################################################################
 */
      CASE0(30)
         pz = (kto[0]-'0') * 2
            + (kto[5]-'0')
            + (kto[6]-'0') * 2
            + (kto[7]-'0')
            + (kto[8]-'0') * 2;

         MOD_10_40;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 31 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 31                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 9, 8, 7, 6, 5, 4, 3, 2, 1                   #
 * # Die Kontonummer ist 10-stellig. Die Stellen 1 bis 9 der            #
 * # Kontonummer sind von rechts nach links mit den Ziffern 9,          #
 * # 8, 7, 6, 5, 4, 3, 2, 1 zu multiplizieren. Die jeweiligen Produkte  #
 * # werden addiert. Die Summe ist durch 11 zu dividieren. Der          #
 * # verbleibende Rest ist die Prüfziffer. Verbleibt nach der           #
 * # Division durch 11 kein Rest, ist die Prüfziffer 0. Ergibt sich ein #
 * # Rest 10, ist die Kontonummer falsch.Die Prüfziffer  befindet sich  #
 * # in der 10. Stelle der Kontonummer.                                 #
 * ######################################################################
 */
      CASE0(31)
         pz = (kto[0]-'0')
            + (kto[1]-'0') * 2
            + (kto[2]-'0') * 3
            + (kto[3]-'0') * 4
            + (kto[4]-'0') * 5
            + (kto[5]-'0') * 6
            + (kto[6]-'0') * 7
            + (kto[7]-'0') * 8
            + (kto[8]-'0') * 9;

         MOD_11_352;   /* pz%=11 */
         if(pz==10){
            pz= *(kto+9)+1;
            return INVALID_KTO;
         }
         CHECK_PZ10;

/*  Berechnung nach der Methode 32 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 32                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7.                           #
 * # Die Kontonummer ist 10-stellig. Die einzelnen Stellen der          #
 * # Kontonummer werden von links nach rechts aufsteigend von           #
 * # 1 bis 10 durchnumeriert. Die Stelle 10 der Kontonummer ist         #
 * # per Definition die Prüfziffer.                                     #
 * # Es wird das Berechnungsverfahren 06 in modifizierter Form          #
 * # auf die Stellen 4 bis 9 angewendet. Die Gewichtung ist             #
 * # 2, 3, 4, 5, 6, 7. Die genannten Stellen werden von rechts          #
 * # nach links mit diesen Faktoren multipliziert. Die restliche        #
 * # Berechnung und mögliche Ergebnisse entsprechen dem Verfahren 06.   #
 * ######################################################################
 */
      CASE0(32)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 33 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 33                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6.                              #
 * # Die Kontonummer ist 10-stellig. Die einzelnen Stellen der          #
 * # Kontonummer werden von links nach rechts aufsteigend von           #
 * # 1 bis 10 durchnumeriert. Die Stelle 10 der Kontonummer ist         #
 * # per Definition die Prüfziffer.                                     #
 * # Es wird das Berechnungsverfahren 06 in modifizierter Form          #
 * # auf die Stellen 5 bis 9 angewendet. Die Gewichtung ist             #
 * # 2, 3, 4, 5, 6. Die genannten Stellen werden von rechts             #
 * # nach links mit diesen Faktoren multipliziert. Die restliche        #
 * # Berechnung und mögliche Ergebnisse entsprechen dem Verfahren 06.   #
 * ######################################################################
 */
      CASE0(33)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 34 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 34                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 4, 8, 5, A, 9, 7.                        #
 * # Die Kontonummer ist 10-stellig. Es wird das Berechnungsverfahren   #
 * # 28 mit modifizierter Gewichtung angewendet. Die Gewichtung         #
 * # lautet: 2, 4, 8, 5, A, 9, 7. Dabei steht der Buchstabe A für       #
 * # den Wert 10.                                                       #
 * ######################################################################
 */
      CASE0(34)
         pz = (kto[0]-'0') * 7
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 10
            + (kto[3]-'0') * 5
            + (kto[4]-'0') * 8
            + (kto[5]-'0') * 4
            + (kto[6]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ8;

/*  Berechnung nach der Methode 35 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 35                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 10                  #
 * # Die Kontonummer ist ggf. durch linksbündige Nullenauffüllung       #
 * # 10-stellig darzustellen. Die 10. Stelle der Kontonummer ist die    #
 * # Prüfziffer. Die Stellen 1 bis 9 der Kontonummer werden von         #
 * # rechts nach links mit den Ziffern 2, 3, 4, ff. multipliziert. Die  #
 * # jeweiligen Produkte werden addiert. Die Summe der Produkte         #
 * # ist durch 11 zu dividieren. Der verbleibende Rest ist die          #
 * # Prüfziffer. Sollte jedoch der Rest 10 ergeben, so ist die          #
 * # Kontonummer unabhängig vom eigentlichen Berechnungsergebnis        #
 * # richtig, wenn die Ziffern an 10. und 9. Stelle identisch           #
 * # sind.                                                              #
 * ######################################################################
 */
      CASE0(35)
         pz = (kto[0]-'0') * 10
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz==10){
            if(*(kto+8)== *(kto+9)){
               pz= *(kto+9)-'0';
               return OK;
            }
            else
               return INVALID_KTO;
         }
         CHECK_PZ10;

/*  Berechnung nach der Methode 36 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 36                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 4, 8, 5.                                 #
 * # Die Kontonummer ist 10-stellig. Die einzelnen Stellen der          #
 * # Kontonummer werden von links nach rechts aufsteigend von           #
 * # 1 bis 10 durchnumeriert. Die Stelle 10 der Kontonummer ist         #
 * # per Definition die Prüfziffer.                                     #
 * # Es wird das Berechnungsverfahren 06 in modifizierter Form          #
 * # auf die Stellen 6 bis 9 angewendet. Die Gewichtung ist             #
 * # 2, 4, 8, 5. Die genannten Stellen werden von rechts nach links     #
 * # mit diesen Faktoren multipliziert. Die restliche Berechnung        #
 * # und mögliche Ergebnisse entsprechen dem Verfahren 06.              #
 * ######################################################################
 */
      CASE0(36)
         pz = (kto[5]-'0') * 5
            + (kto[6]-'0') * 8
            + (kto[7]-'0') * 4
            + (kto[8]-'0') * 2;

         MOD_11_88;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 37 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 37                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 4, 8, 5, A.                              #
 * # Die Kontonummer ist 10-stellig. Die einzelnen Stellen der          #
 * # Kontonummer werden von links nach rechts aufsteigend von           #
 * # 1 bis 10 durchnumeriert. Die Stelle 10 der Kontonummer ist         #
 * # per Definition die Prüfziffer.                                     #
 * # Es wird das Berechnungsverfahren 06 in modifizierter Form          #
 * # auf die Stellen 5 bis 9 angewendet. Die Gewichtung ist             #
 * # 2, 4, 8, 5, A. Dabei steht der Buchstabe A für den                 #
 * # Wert 10. Die genannten Stellen werden von rechts nach links        #
 * # mit diesen Faktoren multipliziert. Die restliche Berechnung        #
 * # und mögliche Ergebnisse entsprechen dem Verfahren 06.              #
 * ######################################################################
 */
      CASE0(37)
         pz = (kto[4]-'0') * 10
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 8
            + (kto[7]-'0') * 4
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 38 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 38                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 4, 8, 5, A, 9.                           #
 * # Die Kontonummer ist 10-stellig. Die einzelnen Stellen der          #
 * # Kontonummer werden von links nach rechts aufsteigend von           #
 * # 1 bis 10 durchnumeriert. Die Stelle 10 der Kontonummer ist         #
 * # per Definition die Prüfziffer.                                     #
 * # Es wird das Berechnungsverfahren 06 in modifizierter Form          #
 * # auf die Stellen 4 bis 9 angewendet. Die Gewichtung ist             #
 * # 2, 4, 8, 5, A, 9. Dabei steht der Buchstabe A für den              #
 * # Wert 10. Die genannten Stellen werden von rechts nach links        #
 * # mit diesen Faktoren multipliziert. Die restliche Berechnung        #
 * # und mögliche Ergebnisse entsprechen dem Verfahren 06.              #
 * ######################################################################
 */
      CASE0(38)
         pz = (kto[3]-'0') * 9
            + (kto[4]-'0') * 10
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 8
            + (kto[7]-'0') * 4
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 39 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 39                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 4, 8, 5, A, 9, 7.                        #
 * # Die Kontonummer ist 10-stellig. Die einzelnen Stellen der          #
 * # Kontonummer werden von links nach rechts aufsteigend von           #
 * # 1 bis 10 durchnumeriert. Die Stelle 10 der Kontonummer ist         #
 * # per Definition die Prüfziffer.                                     #
 * # Es wird das Berechnungsverfahren 06 in modifizierter Form          #
 * # auf die Stellen 3 bis 9 angewendet. Die Gewichtung ist             #
 * # 2, 4, 8, 5, A, 9, 7. Dabei steht der Buchstabe A für den           #
 * # Wert 10. Die genannten Stellen werden von rechts nach links        #
 * # mit diesen Faktoren multipliziert. Die restliche Berechnung        #
 * # und mögliche Ergebnisse entsprechen dem Verfahren 06.              #
 * ######################################################################
 */
      CASE0(39)
         pz = (kto[2]-'0') * 7
            + (kto[3]-'0') * 9
            + (kto[4]-'0') * 10
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 8
            + (kto[7]-'0') * 4
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/* Berechnungsmethoden 40 bis 49 +§§§3
   Berechnung nach der Methode 40 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 40                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 4, 8, 5, A, 9, 7, 3, 6.                  #
 * # Die Kontonummer ist 10-stellig. Die einzelnen Stellen der          #
 * # Kontonummer werden von links nach rechts aufsteigend von           #
 * # 1 bis 10 durchnumeriert. Die Stelle 10 der Kontonummer ist         #
 * # per Definition die Prüfziffer.                                     #
 * # Es wird das Berechnungsverfahren 06 in modifizierter Form          #
 * # auf die Stellen 1 bis 9 angewendet. Die Gewichtung ist             #
 * # 2, 4, 8, 5, A, 9, 7, 3, 6. Dabei steht der Buchstabe A für den     #
 * # Wert 10. Die genannten Stellen werden von rechts nach links        #
 * # mit diesen Faktoren multipliziert. Die restliche Berechnung        #
 * # und mögliche Ergebnisse entsprechen dem Verfahren 06.              #
 * ######################################################################
 */
      CASE0(40)
         pz = (kto[0]-'0') * 6
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 7
            + (kto[3]-'0') * 9
            + (kto[4]-'0') * 10
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 8
            + (kto[7]-'0') * 4
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 41 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 41                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2 (modifiziert)     #
 * # Die Berechnung erfolgt wie bei Verfahren 00                        #
 * # Ausnahme:                                                          #
 * # Ist die 4. Stelle der Kontonummer (von links) = 9, so werden       #
 * # die Stellen 1 bis 3 nicht in die Prüfzifferberechnung ein-bezogen. #
 * ######################################################################
 */
      CASE0(41)
         if(*(kto+3)=='9'){
#ifdef __ALPHA
            pz =  (kto[3]-'0')
               + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
               +  (kto[7]-'0')
               + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else

            pz=(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
            if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
            MOD_10_80;   /* pz%=10 */
         }
         else{
#ifdef __ALPHA
            pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
               +  (kto[1]-'0')
               + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
               +  (kto[3]-'0')
               + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
               +  (kto[7]-'0')
               + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else

            pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
            if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
            if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
            if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
            MOD_10_80;   /* pz%=10 */
         }
         pz=pz%10;
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 42 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 42                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9.                     #
 * # Die Kontonummer ist 10-stellig. Die einzelnen Stellen der          #
 * # Kontonummer werden von links nach rechts aufsteigend von           #
 * # 1 bis 10 durchnumeriert. Die Stelle 10 der Kontonummer ist         #
 * # per Definition die Prüfziffer.                                     #
 * # Es wird das Berechnungsverfahren 06 in modifizierter Form          #
 * # auf die Stellen 2 bis 9 angewendet. Die Gewichtung ist             #
 * # 2, 3, 4, 5, 6, 7, 8, 9. Die genannten Stellen werden von           #
 * # rechts nach links mit diesen Faktoren multipliziert. Die           #
 * # restliche Berechnung und mögliche Ergebnisse entsprechen           #
 * # dem Verfahren 06.                                                  #
 * ######################################################################
 */
      CASE0(42)
         pz = (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 43 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 43                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 1, 2, 3, 4, 5, 6, 7, 8, 9.                  #
 * # Die Kontonummer ist 10-stellig. Die einzelnen Stellen der          #
 * # Kontonummer werden von links nach rechts aufsteigend von           #
 * # 1 bis 10 durchnumeriert. Die Stelle 10 der Kontonummer ist         #
 * # per Definition die Prüfziffer.                                     #
 * # Das Verfahren wird auf die Stellen 1 bis 9 angewendet.             #
 * # Die genannten Stellen werden von rechts nach links                 #
 * # mit den Gewichtungsfaktoren multipliziert. Die Summe der           #
 * # Produkte wird durch den Wert 10 dividiert. Der Rest der            #
 * # Division wird vom Divisor subtrahiert. Die Differenz               #
 * # ist die Prüfziffer. Ergibt die Berechnung eine Differenz           #
 * # von 10, lautet die Prüfziffer 0.                                   #
 * ######################################################################
 */
      CASE0(43)
         pz = (kto[0]-'0') * 9
            + (kto[1]-'0') * 8
            + (kto[2]-'0') * 7
            + (kto[3]-'0') * 6
            + (kto[4]-'0') * 5
            + (kto[5]-'0') * 4
            + (kto[6]-'0') * 3
            + (kto[7]-'0') * 2
            + (kto[8]-'0');

         MOD_10_320;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 44 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 44                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 4, 8, 5, A, 0, 0, 0, 0  (A = 10)         #
 * #                                                                    #
 * # Die Berechnung erfolgt wie bei Verfahren 33.                       #
 * # Stellennr.:    1 2 3 4 5 6 7 8 9 10                                #
 * # Kontonr.:      x x x x x x x x x P                                 #
 * # Gewichtung:    0 0 0 0 A 5 8 4 2    (A = 10)                       #
 * ######################################################################
 */
      CASE0(44)
         pz = (kto[4]-'0') * 10
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 8
            + (kto[7]-'0') * 4
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 45 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 45                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2                   #
 * # Die Berechnung erfolgt wie bei Verfahren 00                        #
 * # Ausnahme:                                                          #
 * # Kontonummern, die an Stelle 1 (von links) eine 0 enthalten,        #
 * # und Kontonummern, die an Stelle 5 eine 1 enthalten,                #
 * # beinhalten keine Prüfziffer.                                       #
 * # Testkontonummern:                                                  #
 * # 3545343232, 4013410024                                             #
 * # Keine Prüfziffer enthalten:                                        #
 * # 0994681254, 0000012340 (da 1. Stelle = 0)                          #
 * # 1000199999, 0100114240 (da 5. Stelle = 1)                          #
 * ######################################################################
 */
      CASE0(45)
         if(*kto=='0' || *(kto+4)=='1'){
#if DEBUG
            pz= *(kto+9)-'0';
#endif
            return OK_NO_CHK;
         }
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 46 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 46                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6.                              #
 * # Die Kontonummer ist 10-stellig. Die einzelnen Stellen der          #
 * # Kontonummer werden von links nach rechts aufsteigend von           #
 * # 1 bis 10 durchnumeriert. Die Stelle 8 der Kontonummer ist          #
 * # per Definition die Prüfziffer.                                     #
 * # Es wird das Berechnungsverfahren 06 in modifizierter Form          #
 * # auf die Stellen 3 bis 7 angewendet. Die Gewichtung ist             #
 * # 2, 3, 4, 5, 6. Die genannten Stellen werden von                    #
 * # rechts nach links mit diesen Faktoren multipliziert. Die           #
 * # restliche Berechnung und mögliche Ergebnisse entsprechen           #
 * # dem Verfahren 06.                                                  #
 * ######################################################################
 */
      CASE0(46)
         pz = (kto[2]-'0') * 6
            + (kto[3]-'0') * 5
            + (kto[4]-'0') * 4
            + (kto[5]-'0') * 3
            + (kto[6]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ8;

/*  Berechnung nach der Methode 47 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 47                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6.                              #
 * # Die Kontonummer ist 10-stellig. Die einzelnen Stellen der          #
 * # Kontonummer werden von links nach rechts aufsteigend von           #
 * # 1 bis 10 durchnumeriert. Die Stelle 9 der Kontonummer ist          #
 * # per Definition die Prüfziffer.                                     #
 * # Es wird das Berechnungsverfahren 06 in modifizierter Form          #
 * # auf die Stellen 4 bis 8 angewendet. Die Gewichtung ist             #
 * # 2, 3, 4, 5, 6. Die genannten Stellen werden von                    #
 * # rechts nach links mit diesen Faktoren multipliziert. Die           #
 * # restliche Berechnung und mögliche Ergebnisse entsprechen           #
 * # dem Verfahren 06.                                                  #
 * ######################################################################
 */
      CASE0(47)
         pz = (kto[3]-'0') * 6
            + (kto[4]-'0') * 5
            + (kto[5]-'0') * 4
            + (kto[6]-'0') * 3
            + (kto[7]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ9;

/*  Berechnung nach der Methode 48 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 48                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7.                           #
 * # Die Kontonummer ist 10-stellig. Die einzelnen Stellen der          #
 * # Kontonummer werden von links nach rechts aufsteigend von           #
 * # 1 bis 10 durchnumeriert. Die Stelle 9 der Kontonummer ist          #
 * # per Definition die Prüfziffer.                                     #
 * # Es wird das Berechnungsverfahren 06 in modifizierter Form          #
 * # auf die Stellen 3 bis 8 angewendet. Die Gewichtung ist             #
 * # 2, 3, 4, 5, 6, 7. Die genannten Stellen werden von                 #
 * # rechts nach links mit diesen Faktoren multipliziert. Die           #
 * # restliche Berechnung und mögliche Ergebnisse entsprechen           #
 * # dem Verfahren 06.                                                  #
 * ######################################################################
 */
      CASE0(48)
         pz = (kto[2]-'0') * 7
            + (kto[3]-'0') * 6
            + (kto[4]-'0') * 5
            + (kto[5]-'0') * 4
            + (kto[6]-'0') * 3
            + (kto[7]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ9;

/*  Berechnung nach der Methode 49 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 49                        #
 * ######################################################################
 * # Variante 1                                                         #
 * # Die Prüfzifferberechnung ist nach Kennziffer 00                    #
 * # durchzuführen. Führt die Berechnung nach Variante 1 zu             #
 * # einem Prüfzifferfehler, so ist die Berechnung nach                 #
 * # Variante 2 vorzunehmen.                                            #
 * #                                                                    #
 * # Variante 2                                                         #
 * # Die Prüfzifferberechnung ist nach Kennziffer 01                    #
 * # durchzuführen.                                                     #
 * ######################################################################
 */
      CASE0(49)
      CASE_U(49,1)
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

      CASE_U(49,2)
         pz = (kto[0]-'0')
            + (kto[1]-'0') * 7
            + (kto[2]-'0') * 3
            + (kto[3]-'0')
            + (kto[4]-'0') * 7
            + (kto[5]-'0') * 3
            + (kto[6]-'0')
            + (kto[7]-'0') * 7
            + (kto[8]-'0') * 3;

         MOD_10_160;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/* Berechnungsmethoden 50 bis 59 +§§§3
   Berechnung nach der Methode 50 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 50                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7.                           #
 * # Die Kontonummer ist 10-stellig. Die einzelnen Stellen der          #
 * # Kontonummer werden von links nach rechts aufsteigend von           #
 * # 1 bis 10 durchnumeriert. Die Stelle 7 der Kontonummer ist          #
 * # per Definition die Prüfziffer.                                     #
 * # Es wird das Berechnungsverfahren 06 in modifizierter Form          #
 * # auf die Stellen 1 bis 6 angewendet. Die Gewichtung ist             #
 * # 2, 3, 4, 5, 6, 7. Die genannten Stellen werden von                 #
 * # rechts nach links mit diesen Faktoren multipliziert. Die           #
 * # restliche Berechnung und mögliche Ergebnisse entsprechen           #
 * # dem Verfahren 06.                                                  #
 * # Ergibt die erste Berechnung einen Prüfziffernfehler, wird          #
 * # empfohlen, die Prüfziffernberechnung ein zweites Mal durch-        #
 * # zuführen und dabei die "gedachte" Unternummer 000 an die           #
 * # Stellen 8 bis 10 zu setzen und die vorhandene Kontonummer          #
 * # vorher um drei Stellen nach links zu verschieben                   #
 * ######################################################################
 */
      CASE0(50)
      CASE_U(50,1)
         pz = (kto[0]-'0') * 7
            + (kto[1]-'0') * 6
            + (kto[2]-'0') * 5
            + (kto[3]-'0') * 4
            + (kto[4]-'0') * 3
            + (kto[5]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX7;

      CASE_U(50,2)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 51 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 51 (geändert 6.9.04)      #
 * ######################################################################
 * # Die Kontonummer ist immer 10-stellig. Die für die Berechnung       #
 * # relevante Kundennummer (K) befindet sich bei der Methode A         #
 * # in den Stellen 4 bis 9 der Kontonummer und bei den Methoden        #
 * # B + C in den Stellen 5 bis 9, die Prüfziffer in Stelle 10          #
 * # der Kontonummer.                                                   #
 * #                                                                    #
 * # Methode A:                                                         #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7                            #
 * # Die Berechnung und mögliche Ergebnisse entsprechen dem             #
 * # Verfahren 06.                                                      #
 * # Stellennr.:    1 2 3 4 5 6 7 8 9 A (A = 10)                        #
 * # Kontonr.:      x x x K K K K K K P                                 #
 * # Gewichtung:          7 6 5 4 3 2                                   #
 * #                                                                    #
 * # Ergibt die Berechnung der Prüfziffer nach der Methode A            #
 * # einen Prüfzifferfehler, ist eine weitere Berechnung mit der        #
 * # Methode B vorzunehmen.                                             #
 * #                                                                    #
 * # Methode B:                                                         #
 * # Modulus     11, Gewichtung 2, 3, 4, 5, 6                           #
 * # Die Berechnung und mögliche Ergebnisse entsprechen dem             #
 * # Verfahren 33.                                                      #
 * # Stellennr.:    1 2 3 4 5 6 7 8 9 A (A = 10)                        #
 * # Kontonr.:      x x x x K K K K K P                                 #
 * # Gewichtung:            6 5 4 3 2                                   #
 * #                                                                    #
 * # Ergibt auch die Berechnung der Prüfziffer nach Methode B           #
 * # einen Prüfzifferfehler, ist eine weitere Berechnung mit der        #
 * # Methode C vorzunehmen.                                             #
 * #                                                                    #
 * # Methode C:                                                         #
 * # Kontonummern, die bis zur Methode C gelangen und in der 10.        #
 * # Stelle eine 7, 8 oder 9 haben, sind ungültig.                      #
 * # Modulus 7, Gewichtung 2, 3, 4, 5, 6                                #
 * # Das Berechnungsverfahren entspricht Methode B. Die Summe der       #
 * # Produkte ist jedoch durch 7 zu dividieren. Der verbleibende        #
 * # Rest wird vom Divisor (7) subtrahiert. Das Ergebnis ist die        #
 * # Prüfziffer. Verbleibt kein Rest, ist die Prüfziffer 0.             #
 * #                                                                    #
 * # Ausnahme:                                                          #
 * # Ist nach linksbündiger Auffüllung mit Nullen auf 10 Stellen die    #
 * # 3. Stelle der Kontonummer = 9 (Sachkonten), so erfolgt die         #
 * # Berechnung wie folgt:                                              #
 * #                                                                    #
 * # Variante 1 zur Ausnahme                                            #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8                         #
 * # Die für die Berechnung relevanten Stellen 3 bis 9 werden von       #
 * # rechts nach links mit den Ziffern 2, 3, 4, 5, 6, 7, 8              #
 * # multipliziert. Die Produkte werden addiert. Die Summe ist durch 11 #
 * # zu dividieren. Der verbleibende Rest wird vom Divisor (11)         #
 * # subtrahiert. Das Ergebnis ist die Prüfziffer. Ergibt sich als Rest #
 * # 1 oder 0, ist die Prüfziffer 0.                                    #
 * #                                                                    #
 * # Stellennr.:   1 2 3 4 5 6 7 8 9 A (A=10)                           #
 * # Kontonr.;     x x 9 x x x x x x P                                  #
 * # Gewichtung:       8 7 6 5 4 3 2                                    #
 * #                                                                    #
 * # Führt die Variante 1 zur Ausnahme zu einem Prüfzifferfehler, ist   #
 * # eine weitere Berechnung nach der Variante 2 vorzunehmen.           #
 * #                                                                    #
 * # Variante 2 zur Ausnahme                                            #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 10                  #
 * # Berechnung und Ergebnisse entsprechen der Variante 1 zur Ausnahme. #
 * ######################################################################
 */
      CASE0(51)
         if(*(kto+2)=='9'){   /* Ausnahme */

            /* Variante 1 */
      CASE_U(51,4)
            pz =         9 * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZX10;

            /* Variante 2 */
      CASE_U(51,5)
               pz = (kto[0]-'0') * 10
               + (kto[1]-'0') * 9
               +            9 * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

               MOD_11_352;   /* pz%=11 */
               if(pz<=1)
                  pz=0;
               else
                  pz=11-pz;
               CHECK_PZ10;
            }

            /* Methode A */
      CASE_U(51,1)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

            /* Methode B */
      CASE_U(51,2)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

            /* Methode C */
      CASE_U(51,3)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_7_112;   /* pz%=7 */
         if(pz)pz=7-pz;
         CHECK_PZ10;


/*  Berechnung nach der Methode 52 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 52                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 4, 8, 5, 10, 9, 7, 3, 6, 1, 2, 4.        #
 * # Zur Berechnung der Prüfziffer muß zunächst aus der angegebenen     #
 * # achtstelligen Kontonummer die zugehörige Kontonummer des ESER-     #
 * # Altsystems (maximal 12stellig) ermittelt werden. Die einzelnen     #
 * # Stellen dieser Alt-Kontonummer sind von rechts nach links mit      #
 * # den Ziffern 2, 4, 8, 5, 10, 9, 7, 3, 6, 1, 2, 4 zu multipli-       #
 * # zieren. Dabei ist für die Prüfziffer, die sich immer an der        #
 * # 6. Stelle von links der Alt-Kontonummer befindet, 0 zu setzen.     #
 * # Die jeweiligen Produkte werden addiert und die Summe durch 11      #
 * # dividiert. Zum Divisionsrest (ggf. auch 0) ist das Gewicht         #
 * # oder ein Vielfaches des Gewichtes über der Prüfziffer zu           #
 * # addieren. Die Summe wird durch 11 dividiert; der Divisionsrest     #
 * # muß 10 lauten. Die Prüfziffer ist der verwendete Faktor des        #
 * # Gewichtes. Kann bei der Division kein Rest 10 erreicht werden,     #
 * # ist die Konto-Nr. nicht verwendbar.                                #
 * # Bildung der Konto-Nr. des ESER-Altsystems aus angegebener          #
 * # Bankleitzahl und Konto-Nr.: XXX5XXXX XPXXXXXX (P=Prüfziffer)       #
 * # Kontonummer des Altsystems: XXXX-XP-XXXXX (XXXXX = variable        #
 * # Länge, da evtl.vorlaufende Nullen eliminiert werden).              #
 * # Bei 10stelligen, mit 9 beginnenden Kontonummern ist die            #
 * # Prüfziffer nach Kennziffer 20 zu berechnen.                        #
 * ######################################################################
 */
      CASE0(52)

            /* Berechnung nach Methode 20 */
      CASE_U(52,1)
         if(*kto=='9'){
            pz = (kto[0]-'0') * 3
               + (kto[1]-'0') * 9
               + (kto[2]-'0') * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            MOD_11_352;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZ10;
         }

               /* nur Prüfziffer angegeben; Test-BLZ einsetzen */
      CASE_U(52,2)
         if(strlen(pz_or_blz)!=8){
            fprintf(stderr,"Warnung: für Methode 52 wird die BLZ benötigt!!; benutze zum Test 13051172\n");
            pz_or_blz="13051172";
         }

            /* Generieren der Konto-Nr. des ESER-Altsystems */
         for(ptr=kto;*ptr=='0';ptr++);
         if(ptr>kto+2)return INVALID_KTO;
         kto_alt[0]=pz_or_blz[4];
         kto_alt[1]=pz_or_blz[5];
         kto_alt[2]=pz_or_blz[6];
         kto_alt[3]=pz_or_blz[7];
         kto_alt[4]= *ptr++;
         kto_alt[5]= *ptr++;
         while(*ptr=='0' && *ptr)ptr++;
         for(dptr=kto_alt+6;*dptr= *ptr++;dptr++);
         p1=kto_alt[5];   /* Prüfziffer */
         kto_alt[5]='0';
         for(pz=0,ptr=dptr-1,i=0;ptr>=kto_alt;ptr--,i++)
            pz+=(*ptr-'0')*w52[i];
         kto_alt[5]=p1;
         pz=pz%11;
         p1=w52[i-6];

            /* passenden Faktor suchen */
         tmp=pz;
         for(i=0;i<10;i++){
            pz=tmp+p1*i;
            MOD_11_88;
            if(pz==10)break;
         }
         pz=i;
         if(i==10)return INVALID_KTO;
         if(*(kto_alt+5)-'0'==pz)
            return OK;
         else
            return FALSE;

/*  Berechnung nach der Methode 53 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 53                        #
 * ######################################################################
 * # Analog Kennziffer 52, jedoch für neunstellige Kontonummern.        #
 * # Bildung der Kontonummern des ESER-Altsystems aus angegebener       #
 * # Bankleitzahl und angegebener neunstelliger Kontonummer:            #
 * # XXX5XXXX XTPXXXXXX (P=Prüfziffer)                                  #
 * # Kontonummer des Altsystems: XXTX-XP-XXXXX (XXXXX = variable        #
 * # Länge, da evtl.vorlaufende Nullen eliminiert werden).              #
 * # Die Ziffer T ersetzt die 3. Stelle von links der nach              #
 * # Kennziffer 52 gebildeten Kontonummer des ESER-Altsystems.          #
 * # Bei der Bildung der Kontonummer des ESER-Altsystems wird die       #
 * # Ziffer T nicht in den Kundennummernteil (7.-12. Stelle der         #
 * # Kontonummer) übernommen.                                           #
 * # Bei 10stelligen, mit 9 beginnenden Kontonummern ist die            #
 * # Prüfziffer nach Kennziffer 20 zu berechnen.                        #
 * ######################################################################
 */
      CASE0(53)

            /* Berechnung nach Methode 20 */
         if(*kto=='9'){
      CASE_U(53,1)
         pz = (kto[0]-'0') * 3
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZ10;
         }

               /* nur Prüfziffer angegeben; Test-BLZ einsetzen */
      CASE_U(53,2)
         if(strlen(pz_or_blz)!=8){
            fprintf(stderr,"Warnung: für Methode 53 wird die BLZ benötigt!!; benutze zum Test 16052072\n");
            pz_or_blz="16052072";
         }

            /* Generieren der Konto-Nr. des ESER-Altsystems */
         for(ptr=kto;*ptr=='0';ptr++);
         if(*kto!='0' || *(kto+1)=='0'){  /* Kto-Nr. muß neunstellig sein */
#if DEBUG
            pz= -2;
#endif
            return INVALID_KTO;
         }
         kto_alt[0]=pz_or_blz[4];
         kto_alt[1]=pz_or_blz[5];
         kto_alt[2]=kto[2];         /* T-Ziffer */
         kto_alt[3]=pz_or_blz[7];
         kto_alt[4]=kto[1];
         kto_alt[5]=kto[3];
         for(ptr=kto+4;*ptr=='0' && *ptr;ptr++);
         for(dptr=kto_alt+6;*dptr= *ptr++;dptr++);
         kto=kto_alt;
         p1=kto_alt[5];   /* Prüfziffer merken */
         kto_alt[5]='0';
         for(pz=0,ptr=kto_alt+strlen(kto_alt)-1,i=0;ptr>=kto_alt;ptr--,i++)
            pz+=(*ptr-'0')*w52[i];
         kto_alt[5]=p1;   /* Prüfziffer zurückschreiben */
         pz=pz%11;
         p1=w52[i-6];

            /* passenden Faktor suchen */
         tmp=pz;
         for(i=0;i<10;i++){
            pz=tmp+p1*i;
            MOD_11_88;
            if(pz==10)break;
         }
         pz=i;
         if(i==10)return INVALID_KTO;
         CHECK_PZ6;

/*  Berechnung nach der Methode 54 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 54                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 2                         #
 * # Die Kontonummer ist 10-stellig, wobei die Stellen 1 u. 2           #
 * # generell mit 49 belegt sind. Die einzelnen Stellen der             #
 * # Kontonummer sind von rechts nach links mit den Ziffern 2, 3,       #
 * # 4, 5, 6, 7, 2 zu multiplizieren. Die jeweiligen Produkte werden    #
 * # addiert. Die Summe ist durch 11 zu dividieren. Der verbleibende    #
 * # Rest wird vom Divisor (11) subtrahiert. Das Ergebnis ist die       #
 * # Prüfziffer. Ergibt sich als Rest 0 oder 1, ist die Prüfziffer      #
 * # zweistellig und kann nicht verwendet werden. Die Kontonummer       #
 * # ist dann nicht verwendbar.                                         #
 * ######################################################################
 */
      CASE0(54)
         if(*kto!='4' && *(kto+1)!='9')return INVALID_KTO;
         pz = (kto[2]-'0') * 2
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         pz=11-pz;
         if(pz>9)return INVALID_KTO;
         CHECK_PZ10;

/*  Berechnung nach der Methode 55 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 55                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 7, 8 (modifiziert).    #
 * # Die Berechnung erfolgt wie bei Verfahren 06.                       #
 * # Die einzelnen Stellen der Kontonummer sind von rechts nach         #
 * # links mit den Ziffern 2, 3, 4, 5, 6, 7, 8, 7, 8 zu                 #
 * # multiplizieren. Die jeweiligen Produkte werden addiert.            #
 * # Die Summe ist durch 11 zu dividieren. Der verbleibende Rest        #
 * # wird vom Divisor (11) subtrahiert. Das Ergebnis ist die            #
 * # Prüfziffer. Verbleibt nach der Division durch 11 kein Rest,        #
 * # ist die Prüfziffer 0. Ergibt sich als Rest 1, entsteht bei         #
 * # der Subtraktion 11 - 1 = 10. Das Rechenergebnis ist                #
 * # nicht verwendbar und muß auf eine Stelle reduziert werden.         #
 * # Die linke Seite wird eliminiert, und nur die rechte Stelle         #
 * # (Null) findet als Prüfziffer Verwendung.                           #
 * ######################################################################
 */
      CASE0(55)
         pz = (kto[0]-'0') * 8
            + (kto[1]-'0') * 7
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 56 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 56                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 2, 3, 4.                  #
 * # Prüfziffer ist die letzte Stelle der Kontonummer.                  #
 * # Von rechts beginnend werden die einzelnen Ziffern der              #
 * # Kontonummer mit den Gewichten multipliziert. Die Produkte der      #
 * # Multiplikation werden addiert und diese Summe durch 11             #
 * # dividiert. Der Rest wird von 11 abgezogen, das Ergebnis ist        #
 * # die Prüfziffer, die an die Kontonummer angehängt wird.             #
 * # 1. Bei dem Ergebnis 10 oder 11 ist die Kontonummer ungültig.       #
 * # 2. Beginnt eine zehnstellige Kontonummer mit 9, so wird beim       #
 * # Ergebnis 10 die Prüfziffer 7 und beim Ergebnis 11 die              #
 * # Prüfziffer 8 gesetzt.                                              #
 * ######################################################################
 */
      CASE0(56)
         pz = (kto[0]-'0') * 4
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 2
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         pz=11-pz;
         if(pz>9){
            if(*kto=='9'){
               if(pz==10)
                  pz=7;
               else
                  pz=8;
            }
            else
               return INVALID_KTO;
         }
         CHECK_PZ10;

/*  Berechnung nach der Methode 57 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 57                        #
 * ######################################################################
 * # (geändert ab 04.03.2002)                                           #
 * # Modulus 10, Gewichtung 1, 2, 1, 2, 1, 2, 1, 2, 1                   #
 * # Die Kontonummern sind zur Berechnung der Prüfziffer durch          #
 * # linksbündige Auffüllung mit Nullen immer 10stellig                 #
 * # darzustellen. Die ersten 9 Stellen sind von links                  #
 * # entsprechend  Modulus 10 zu multiplizieren. Ergeben sich           #
 * # zweistellige Produkte, ist die Quersumme zu bilden. Alle           #
 * # Werte werden addiert, vom Ergebnis wird dann nur die               #
 * # Einerstelle berücksichtigt und von dem Wert 10 subtrahiert.        #
 * # Ergibt sich nach der Subtraktion der Wert 10, so ist die           #
 * # Prüfziffer = 0. Die 10. Stelle der Kontonummer ist dann die        #
 * # entsprechende Prüfziffer.                                          #
 * # Ausnahmen zu diesem Verfahren stellen die Kontonummern             #
 * # dar, die ­ ggf. nach der linksbündigen Auffüllung mit Nullen -     #
 * # mit einer Zahl zwischen 00 und 50, 91 oder 96 bis 99               #
 * # beginnen bzw. linksbündig von der ersten bis zur sechsten          #
 * # Stelle durchgehend eine 7 oder 8 aufweisen. Für diese              #
 * # Kontonummern gibt es keine Prüfziffernkontrolle, die               #
 * # Kontonummern sind aber als richtig anzusehen.                      #
 * ######################################################################
 */
      CASE0(57)
         p1=(*kto-'0')*10+(*(kto+1)-'0');
         if(p1<=50 || p1==91 || p1>95 || !strncmp(kto,"777777",6)
                   || !strncmp(kto,"888888",6)){
            pz= *(kto+9)-'0';
            return OK_NO_CHK;
         }
#ifdef __ALPHA
         pz =  (kto[0]-'0')
            + ((kto[1]<'5') ? (kto[1]-'0')*2 : (kto[1]-'0')*2-9)
            +  (kto[2]-'0')
            + ((kto[3]<'5') ? (kto[3]-'0')*2 : (kto[3]-'0')*2-9)
            +  (kto[4]-'0')
            + ((kto[5]<'5') ? (kto[5]-'0')*2 : (kto[5]-'0')*2-9)
            +  (kto[6]-'0')
            + ((kto[7]<'5') ? (kto[7]-'0')*2 : (kto[7]-'0')*2-9)
            +  (kto[8]-'0');
#else
         pz=(kto[0]-'0')+(kto[2]-'0')+(kto[4]-'0')+(kto[6]-'0')+(kto[8]-'0');
         if(kto[1]<'5')pz+=(kto[1]-'0')*2; else pz+=(kto[1]-'0')*2-9;
         if(kto[3]<'5')pz+=(kto[3]-'0')*2; else pz+=(kto[3]-'0')*2-9;
         if(kto[5]<'5')pz+=(kto[5]-'0')*2; else pz+=(kto[5]-'0')*2-9;
         if(kto[7]<'5')pz+=(kto[7]-'0')*2; else pz+=(kto[7]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 58 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 58                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6.                              #
 * # Es wird das Berechnungsverfahren 02 in modifizierter Form auf      #
 * # die Stellen 5 bis 9 angewendet.                                    #
 * ######################################################################
 */
      CASE0(58)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz)pz=11-pz;
         if(pz==10)return INVALID_KTO;
         CHECK_PZ10;

/*  Berechnung nach der Methode 59 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 59                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2.                  #
 * # Die Berechnung erfolgt wie bei Verfahren 00; es ist jedoch         #
 * # zu beachten, daß Kontonummern, die kleiner als 9-stellig sind,     #
 * # nicht in die Prüfziffernberechnung einbezogen werden.              #
 * ######################################################################
 */
      CASE0(59)
         if(*kto=='0' && *(kto+1)=='0')return OK_NO_CHK;
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else

         pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/* Berechnungsmethoden 60 bis 69 +§§§3
   Berechnung nach der Methode 60 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 60                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2.                        #
 * # Die Berechnung erfolgt wie bei Verfahren 00. Es ist jedoch zu      #
 * # beachten, daß die zweistellige Unterkontonummer (Stellen 1 und     #
 * # 2) nicht in das Prüfziffernverfahren mit einbezogen werden darf.   #
 * # Die für die Berechnung relevante siebenstellige Grundnummer        #
 * # befindet sich in den Stellen 3 bis 9, die Prüfziffer in der        #
 * # Stelle 10.                                                         #
 * ######################################################################
 */
      CASE0(60)
#ifdef __ALPHA
         pz = ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz=(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 61 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 61                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2.                        #
 * # Darstellung der Kontonummer: B B B S S S S P A U (10-stellig).     #
 * # B = Betriebsstellennummer                                          #
 * # S = Stammnummer                                                    #
 * # P = Prüfziffer                                                     #
 * # A = Artziffer                                                      #
 * # U = Unternummer                                                    #
 * # Die Berechnung erfolgt wie bei Verfahren 00 über Betriebs-         #
 * # stellennummer und Stammnummer mit der Gewichtung 2, 1, 2, 1,       #
 * # 2, 1, 2.                                                           #
 * # Ist die Artziffer (neunte Stelle der Kontonummer) eine 8, so       #
 * # werden die neunte und zehnte Stelle der Kontonummer in die         #
 * # Prüfziffernermittlung einbezogen. Die Berechnung erfolgt dann      #
 * # über Betriebsstellennummer, Stammnummer, Artziffer und Unter-      #
 * # nummer mit der Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2.               #
 * ######################################################################
 */
      CASE0(61)
         if(*(kto+8)=='8'){
      CASE_U(61,2)
#ifdef __ALPHA
            pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
               +  (kto[1]-'0')
               + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
               +  (kto[3]-'0')
               + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
               +  (kto[8]-'0')
               + ((kto[9]<'5') ? (kto[9]-'0')*2 : (kto[9]-'0')*2-9);
#else

            pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[8]-'0');
            if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
            if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
            if(kto[9]<'5')pz+=(kto[9]-'0')*2; else pz+=(kto[9]-'0')*2-9;
#endif
            MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ8;
         }
         else{
      CASE_U(61,1)
#ifdef __ALPHA
            pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
               +  (kto[1]-'0')
               + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
               +  (kto[3]-'0')
               + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9);
#else
            pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0');
            if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
            if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
#endif
            MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ8;
         }

/*  Berechnung nach der Methode 62 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 62                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2.                              #
 * # Die beiden ersten und die beiden letzten Stellen sind              #
 * # nicht zu berücksichtigen. Die Stellen drei bis sieben              #
 * # sind von rechts nach links mit den Ziffern 2, 1, 2, 1, 2           #
 * # zu multiplizieren. Aus zweistelligen Einzelergebnissen             #
 * # ist eine Quersumme zu bilden. Alle Ergebnisse sind dann            #
 * # zu addieren. Die Differenz zum nächsten Zehner ergibt die          #
 * # Prüfziffer auf Stelle acht. Ist die Differenz 10, ist die          #
 * # Prüfziffer 0.                                                      #
 * ######################################################################
 */
      CASE0(62)
#ifdef __ALPHA
         pz = ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9);
#else

         pz=(kto[3]-'0')+(kto[5]-'0');
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
#endif
         MOD_10_40;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ8;

/*  Berechnung nach der Methode 63 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 63                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1.                           #
 * # Die für die Berechnung relevante 6-stellige Grundnummer            #
 * # (Kundennummer) befindet sich in den Stellen 2-7, die Prüfziffer    #
 * # in Stelle 8 der Kontonummer. Die zweistellige Unterkontonummer     #
 * # (Stellen 9-10) ist nicht in das Prüfziffernverfahren einzu-        #
 * # beziehen. Die einzelnen Stellen der Grundnummer sind von rechts    #
 * # nach links mit den Ziffern 2, 1, 2, 1, 2 1 zu multiplizieren.      #
 * # Die jeweiligen Produkte werden addiert, nachdem jeweils aus        #
 * # den zweistelligen Produkten die Quersumme gebildet wurde           #
 * # (z.B. Produkt 16 = Quersumme 7). Nach der Addition bleiben         #
 * # außer der Einerstelle alle anderen Stellen unberücksichtigt.       #
 * # Die Einerstelle wird von dem Wert 10 subtrahiert. Das Ergebnis     #
 * # ist die Prüfziffer (Stelle 8). Hat die Einerstelle den Wert 0,     #
 * # ist die Prüfziffer 0. Ausnahmen:                                   #
 * # Ist die Ziffer in Stelle 1 vor der sechsstelligen Grundnummer      #
 * # nicht 0, ist das Ergebnis als falsch zu werten.                    #
 * # Ist die Unterkontonummer 00, kann es vorkommen, daß sie auf        #
 * # den Zahlungsverkehrsbelegen nicht angegeben ist, die Kontonummer   #
 * # jedoch um führende Nullen ergänzt wurde. In diesem Fall sind       #
 * # z.B. die Stellen 1-3 000, die Prüfziffer ist an der Stelle 10.     #
 * ######################################################################
 */
      CASE0(63)
         if(*kto!='0')return INVALID_KTO;
         if(*(kto+1)=='0' && *(kto+2)=='0'){
#ifdef __ALPHA
            pz =   (kto[3]-'0')
               +  ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +   (kto[5]-'0')
               +  ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
               +   (kto[7]-'0')
               +  ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
            pz=(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
            if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
            MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }
         else{
#ifdef __ALPHA
            pz =   (kto[1]-'0')
               +  ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
               +   (kto[3]-'0')
               +  ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +   (kto[5]-'0')
               +  ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9);
#else
            pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0');
            if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
#endif
            MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ8;
         }
/*  Berechnung nach der Methode 64 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 64                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 9, 10, 5, 8, 4, 2.                          #
 * #  Die Kontonummer ist 10-stellig. Die für die Berechnung relevanten #
 * #  Stellen der Kontonummer befinden sich in den Stellen 1 bis 6 und  #
 * #  werden von links nach rechts mit den Ziffern 9, 10, 5, 8, 4, 2    #
 * #  multipliziert. Die weitere Berechnung und Ergebnisse entsprechen  #
 * #  dem Verfahren 06. Die Prüfziffer befindet sich in Stelle 7 der    #
 * #  Kontonummer.                                                      #
 * ######################################################################
 */
      CASE0(64)
         pz = (kto[0]-'0') * 9
            + (kto[1]-'0') * 10
            + (kto[2]-'0') * 5
            + (kto[3]-'0') * 8
            + (kto[4]-'0') * 4
            + (kto[5]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ7;

/*  Berechnung nach der Methode 65 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 65                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2.                        #
 * # Die Kontonummer ist zehnstellig: G G G S S S S P K U               #
 * # G = Geschäftstellennummer                                          #
 * # S = Stammnummer                                                    #
 * # P = Prüfziffer                                                     #
 * # K = Kontenartziffer                                                #
 * # U = Unterkontonummer                                               #
 * # Die Berechnung erfolgt wie bei Verfahren 00 über Geschäfts-        #
 * # stellennummer und Stammnummer mit der Gewichtung 2, 1, 2,...       #
 * # Ausnahme: Ist die Kontenartziffer (neunte Stelle der Konto-        #
 * # nummer) eine 9, so werden die neunte und zehnte Stelle der         #
 * # Kontonummer in die Prüfziffernermittlung einbezogen. Die           #
 * # Berechnung erfolgt dann über die Geschäftsstellennummer,           #
 * # Stammnummer, Kontenartziffer und Unterkontonummer mit der          #
 * # Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2.                              #
 * ######################################################################
 */
      CASE0(65)
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9);
#else

         pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(kto[8]=='9'){
            p1=(kto[9]-'0')*2;
            if(p1>9)p1-=9;
            pz+=p1+9;
         }
         pz=pz%10;
         if(pz)pz=10-pz;
         CHECK_PZ8;

/*  Berechnung nach der Methode 66 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 66                        #
 * ######################################################################
 * # Aufbau der 9-stelligen Kontonummer (innerhalb des                  #
 * # zwischenbetrieblich 10-stelligen Feldes)                           #
 * # Stelle    1    = gehört nicht zur Kontonummer, muss                #
 * #                  daher 0 sein                                      #
 * #           2    = Stammnunmmer                                      #
 * #           3-4  = Unterkontonummer, wird bei der Prüfziffer-        #
 * #                  berechnung nicht berücksichtigt                   #
 * #           5-9  = Stammnummer                                       #
 * #           10   = Prüfziffer                                        #
 * # Der 9-stelligen Kontonummer wird für die Prüfzifferberechnung      #
 * # eine 0 vorangestellt. Die Prüfziffer steht in Stelle 10. Die für   #
 * # die Berechnung relevante 6-stellige Stammnummer (Kundenummer)      #
 * # befindet sich in den Stellen 2 und  5 bis 9. Die zweistellige      #
 * # Unterkontonummer (Stellen 3 und 4) wird nicht in das               #
 * # Prüfzifferberechnungsverfahren mit einbezogen und daher mit 0      #
 * # gewichtet. Die einzelnen Stellen der Stammnummer sind von rechts   #
 * # nach links mit den Ziffern 2, 3, 4, 5, 6, 0, 0, 7 zu               #
 * # multiplizieren. Die jeweiligen Produkte werden addiert. Die        #
 * # Summe ist durch 11 zu dividieren. Bei einem verbleibenden Rest     #
 * # von 0 ist die Prüfziffer 1. Bei einem Rest von 1 ist die           #
 * # Prüfziffer 0 Verbleibt ein Rest von 2 bis 10, so wird dieser vom   #
 * # Divison (11) subtrahiert. Die Differenz ist dann die Prüfziffer.   #
 * ######################################################################
 */
      CASE0(66)
         if(*kto!='0')return INVALID_KTO;
         pz = (kto[1]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<2)
            pz=1-pz;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 67 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 67                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2.                        #
 * # Die Kontonummer ist zehnstellig. Die Berechnung erfolgt wie bei    #
 * # Verfahren 00. Es ist jedoch zu beachten, daß die zweistellige      #
 * # Unterkontonummer (Stellen 9 und 10) nicht in das Prüfziffern-      #
 * # verfahren mit einbezogen werden darf. Die für die Berechnung       #
 * # relevante siebenstellige Stammnummer befindet sich in den          #
 * # Stellen 1 bis 7, die Prüfziffer in der Stelle 8.                   #
 * ######################################################################
 */
      CASE0(67)
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9);
#else

         pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ8;

/*  Berechnung nach der Methode 68 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 68                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2.                  #
 * # Die Kontonummern sind 6 bis 10stellig und enthalten keine          #
 * # führenden Nullen. Die erste Stelle von rechts ist die              #
 * # Prüfziffer. Die Berechnung erfolgt wie bei Verfahren 00,           #
 * # hierbei sind jedoch folgende Besonderheiten zu beachten:           #
 * # Bei 10stelligen Kontonummern erfolgt die Berechnung für die        #
 * # 2. bis 7. Stelle (von rechts!). Stelle 7 muß eine 9 sein.          #
 * # 6 bis 9stellige Kontonummern sind in zwei Varianten prüfbar.       #
 * # Variante 1: voll prüfbar.                                          #
 * # Ergibt die Berechnung nach Variante 1 einen Prüfziffernfehler,     #
 * # muß Variante 2 zu einer korrekten Prüfziffer führen.               #
 * # Variante 2: Stellen 7 und 8 werden nicht geprüft.                  #
 * # 9stellige Kontonummern im Nummerenbereich 400000000 bis            #
 * # 4999999999 sind nicht prüfbar, da diese Nummern keine              #
 * # Prüfziffer enthalten.                                              #
 * ######################################################################
 */
      CASE0(68)

            /* Sonderfall: keine Prüfziffer */
         if(*kto=='0' && *(kto+1)=='4'){
#if DEBUG
            pz= *(kto+9)-'0';
#endif
            return OK_NO_CHK;
         }

            /* 10stellige Kontonummern */
      CASE_U(68,1)
         if(*kto!='0'){
            if(*(kto+3)!='9')return INVALID_KTO;
#ifdef __ALPHA
         pz = 9           /* 7. Stelle von rechts */
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz=9+(kto[5]-'0')+(kto[7]-'0');
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_40;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }

            /* 6 bis 9stellige Kontonummern: Variante 1 */
      CASE_U(68,2)
#ifdef __ALPHA
         pz =  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

            /* 6 bis 9stellige Kontonummern: Variante 2 */
      CASE_U(68,3)
#ifdef __ALPHA
         pz =  (kto[1]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else

         pz=(kto[1]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_40;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 69 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 69                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8.                        #
 * # Die Berechnung erfolgt wie bei Verfahren 28. Ergibt die            #
 * # Berechnung einen Prüfziffernfehler, so ist die Prüfziffer          #
 * # nach Variante II zu ermitteln (s.u.).                              #
 * # Ausnahmen:                                                         #
 * # Für den Kontonummernkreis 9300000000 - 9399999999 ist keine        #
 * # Prüfziffernberechnung möglich = Kennziffer 09.                     #
 * # Für den Kontonummernkreis 9700000000 - 9799999999 ist die          #
 * # Prüfziffernberechnung wie folgt vorzunehmen (Variante II):         #
 * # Die Position der einzelnen Ziffern von rechts nach links           #
 * # innerhalb der Kontonummer gibt die Zeile 1 bis 4 der Trans-        #
 * # formationstabelle an. Aus ihr sind die Übersetzungswerte zu        #
 * # summieren. Die Einerstelle wird von 10 subtrahiert und stellt      #
 * # die Prüfziffer dar.                                                #
 * # Transformationstabelle:                                            #
 * # Ziffer    : 0123456789                                             #
 * # Zeile 1   : 0159374826                                             #
 * # Zeile 2   : 0176983254                                             #
 * # Zeile 3   : 0184629573                                             #
 * # Zeile 4   : 0123456789                                             #
 * ######################################################################
 */
      CASE0(69)
            /* Sonderfall 93xxxxxxxx: Keine Prüfziffer */
         if(*kto=='9' && *(kto+1)=='3')return OK_NO_CHK;

            /* Variante 1 */
      CASE_U(69,1)
            /* Sonderfall 97xxxxxxxx nur über Variante 2 */
         if(*kto!='9' || *(kto+1)!='7'){
            pz = (kto[0]-'0') * 8
               + (kto[1]-'0') * 7
               + (kto[2]-'0') * 6
               + (kto[3]-'0') * 5
               + (kto[4]-'0') * 4
               + (kto[5]-'0') * 3
               + (kto[6]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZX8;
         }

            /* Variante 2 */
      CASE_U(69,2)
         pz = m10h_digits[0][(unsigned int)(kto[0]-'0')]
            + m10h_digits[3][(unsigned int)(kto[1]-'0')]
            + m10h_digits[2][(unsigned int)(kto[2]-'0')]
            + m10h_digits[1][(unsigned int)(kto[3]-'0')]
            + m10h_digits[0][(unsigned int)(kto[4]-'0')]
            + m10h_digits[3][(unsigned int)(kto[5]-'0')]
            + m10h_digits[2][(unsigned int)(kto[6]-'0')]
            + m10h_digits[1][(unsigned int)(kto[7]-'0')]
            + m10h_digits[0][(unsigned int)(kto[8]-'0')];
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/* Berechnungsmethoden 70 bis 79 +§§§3
   Berechnung nach der Methode 70 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 70                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7.                           #
 * # Die Kontonummer ist zehnstellig. Die einzelnen Stellen der         #
 * # Kontonummer sind von rechts nach links mit den Ziffern             #
 * # 2,3,4,5,6,7,2,3,4 zu multiplizieren. Die Berechnung erfolgt wie    #
 * # bei Verfahren 06.                                                  #
 * # Ausnahme: Ist die 4. Stelle der Kontonummer = 5 oder die 4. -      #
 * # 5. Stelle der Kontonummer = 69, so werden die Stellen 1 - 3        #
 * # nicht in die Prüfziffernermittlung einbezogen.                     #
 * ######################################################################
 */
      CASE0(70)
         if(*(kto+3)=='5'){
            pz = 5 * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;
            MOD_11_176;   /* pz%=11 */
         }
         else if(*(kto+3)=='6' && *(kto+4)=='9'){
            pz = 6 * 7
               + 9 * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;
            MOD_11_176;   /* pz%=11 */
         }
         else{
            pz = (kto[0]-'0') * 4
               + (kto[1]-'0') * 3
               + (kto[2]-'0') * 2
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;
            MOD_11_176;   /* pz%=11 */
         }
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 71 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 71                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 6, 5, 4, 3, 2, 1.                           #
 * # Die Kontonummer ist immer 10-stellig. Die Stellen 2 bis 7          #
 * # sind von links nach rechts mit den Ziffern 6, 5, 4, 3, 2, 1 zu     #
 * # multiplizieren. Die Ergebnisse sind dann ohne Quersummenbildung    #
 * # zu addieren. Die Summe ist durch 11 zu dividieren.                 #
 * # Der verbleibende Rest wird vom Divisor (11) subtrahiert.           #
 * # Das Ergebnis ist die Prüfziffer.                                   #
 * # Ausnahmen: Verbleibt nach der Division durch 11 kein Rest, ist     #
 * # die Prüfziffer 0. Ergibt sich als Rest 1, entsteht bei der         #
 * # Subtraktion 11 ./. 1 = 10; die Zehnerstelle (1) ist dann           #
 * # die Prüfziffer.                                                    #
 * ######################################################################
 */
      CASE0(71)
         pz = (kto[1]-'0') * 6
            + (kto[2]-'0') * 5
            + (kto[3]-'0') * 4
            + (kto[4]-'0') * 3
            + (kto[5]-'0') * 2
            + (kto[6]-'0');

         MOD_11_176;   /* pz%=11 */
         if(pz>1)pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 72 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 72                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1.                           #
 * # Die Kontonummer ist zehnstellig. Die Berechnung erfolgt wie bei    #
 * # Verfahren 00. Es ist jedoch zu beachten, daß die zweistellige      #
 * # Unterkontonummer (Stellen 1 und 2) und die Artziffer (Stelle 3)    #
 * # nicht in das Prüfziffernverfahren mit einbezogen werden.           #
 * # Die für die Berechnung relevante sechsstellige Kundennummer        #
 * # befindet sich in den Stellen 4 bis 9, die Prüfziffer in der        #
 * # Stelle 10.                                                         #
 * ######################################################################
 */
      CASE0(72)
#ifdef __ALPHA
         pz =  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz=(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 73 (alt) +§§§4 */
/*
 * ######################################################################
 * #   Berechnung nach der Methode 73 alt (nur für Vergleichszwecke)    #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1.                           #
 * # Die Kontonummer ist 10-stellig. Die Stellen 4 bis 9 der            #
 * # Kontonummer werden von rechts nach links mit den Ziffern           #
 * # 2, 1, 2, 1, 2, 1 multipliziert. Die Berechnung der Ergebnisse      #
 * # entsprechen dem Verfahren 00. Die 10. Stelle der Kontonummer       #
 * # ist per Definition die Prüfziffer.                                 #
 * # Ausnahme:                                                          #
 * # Ist die 3. Stelle der Kontonummer = 9, werden die Stellen 1 bis    #
 * # 9 in die Prüfziffernberechnung einbezogen.                         #
 * # Die Berechnung erfolgt dann entsprechend Verfahren 06 mit          #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 10.                 #
 * ######################################################################
        Berechnung nach der Methode 73  (neu) +§§§4
 * #    Berechnung nach der Methode 73  (geändert zum 6.12.2004)        #
 * ######################################################################
 * #                                                                    #
 * # Die Kontonummer ist durch linksbündiges Auffüllen mit Nullen       #
 * # 10-stellig darzustellen. Die 10. Stelle der Kontonummer ist die    #
 * # Prüfziffer.                                                        #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1                            #
 * # Die Stellen 4 bis 9 der Kontonummer werden von rechts nach links   #
 * # mit den Ziffern 2, 1, 2, 1, 2, 1 multipliziert. Die Berechnung und #
 * # Ergebnisse entsprechen dem Verfahren 00.                           #
 * #                                                                    #
 * # Führt die Berechnung nach Variante 1 zu einem Prüfzifferfehler,    #
 * # ist eine weitere Berechnung nach Variante 2 vorzunehmen.           #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2                               #
 * # Das Berechnungsverfahren entspricht Variante 1, es ist jedoch zu   #
 * # beachten, dass nur die Stellen 5 bis 9 in das Prüfziffern-         #
 * # berechnungsverfahren einbezogen werden.                            #
 * #                                                                    #
 * # Führt die Berechnung auch nach Variante 2 zu einem Prüfziffer-     #
 * # fehler, ist die Berechnung nach Variante 3 vorzunehmen:            #
 * #                                                                    #
 * # Variante 3                                                         #
 * # Modulus 7, Gewichtung 2, 1, 2, 1, 2 Das Berechnungsverfahren       #
 * # entspricht Variante 2. Die Summe der Produkt-Quersummen ist jedoch #
 * # durch 7 zu dividieren. Der verbleibende Rest wird vom Divisor (7)  #
 * # subtrahiert. Das Ergebnis ist die Prüfziffer. Verbleibt nach der   #
 * # Division kein Rest, ist die Prüfziffer = 0                         #
 * #                                                                    #
 * # Ausnahme:                                                          #
 * # Ist nach linksbündiger Auffüllung mit Nullen auf 10 Stellen die 3. #
 * # Stelle der Kontonummer = 9 (Sachkonten), so erfolgt die Berechnung #
 * # gemäß der Ausnahme in Methode 51 mit den gleichen Ergebnissen und  #
 * # Testkontonummern.                                                  #
 * ######################################################################
 */

      CASE0(73)
                /* Ausnahme, Variante 1 */
         if(*(kto+2)=='9'){   /* Berechnung wie in Verfahren 51 */
      CASE_U(73,4)
            pz =         9 * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZX10;

            /* Ausnahme, Variante 2 */
      CASE_U(73,5)
               pz = (kto[0]-'0') * 10
               + (kto[1]-'0') * 9
               +            9 * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

               MOD_11_352;   /* pz%=11 */
               if(pz<=1)
                  pz=0;
               else
                  pz=11-pz;
               CHECK_PZ10;
            }

      CASE_U(73,1)
#ifdef __ALPHA
         pz1= ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else

         pz1=(kto[5]-'0')+(kto[7]-'0');
         if(kto[4]<'5')pz1+=(kto[4]-'0')*2; else pz1+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz1+=(kto[6]-'0')*2; else pz1+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz1+=(kto[8]-'0')*2; else pz1+=(kto[8]-'0')*2-9;
#endif
         pz=pz1+(kto[3]-'0');
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

      CASE_U(73,2)
#if DEBUG
#ifdef __ALPHA
         pz = ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else

         pz=(kto[5]-'0')+(kto[7]-'0');
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
#else
         pz=pz1;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

      CASE_U(73,3)
#if DEBUG
#ifdef __ALPHA
         pz = ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else

         pz=(kto[5]-'0')+(kto[7]-'0');
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
#else
         pz=pz1;
#endif
         MOD_7_56;   /* pz%=7 */
         if(pz)pz=7-pz;
         CHECK_PZ10;


/*  Berechnung nach der Methode 74 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 74                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2 ff.                           #
 * # Die Kontonummer (2- bis 10-stellig) ist durch linksbündige         #
 * # Nullenauffüllung 10-stellig darzustellen. Die 10. Stelle ist       #
 * # per Definition die Prüfziffer. Die für die Berechnung              #
 * # relevanten Stellen werden von rechts nach links mit den Ziffern    #
 * # 2, 1, 2, 1, 2 ff. multipliziert. Die weitere Berechnung und die    #
 * # Ergebnisse entsprechen dem Verfahren 00.                           #
 * # Bei 6-stelligen Kontonummern ist folgende Besonderheit zu          #
 * # beachten.                                                          #
 * # Ergibt die erste Berechnung der Prüfziffer nach dem Verfahren 00   #
 * # einen Prüfziffernfehler, so ist eine weitere Berechnung            #
 * # vorzunehmen. Hierbei ist die Summe der Produkte auf die nächste    #
 * # Halbdekade hochzurechnen. Die Differenz ist die Prüfziffer.        #
 * ######################################################################
 */
      CASE0(74)
      CASE_U(74,1)
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=pz1=(kto[0]-'0')*2; else pz+=pz1=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=pz1=(kto[2]-'0')*2; else pz+=pz1=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=pz1=(kto[4]-'0')*2; else pz+=pz1=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=pz1=(kto[6]-'0')*2; else pz+=pz1=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=pz1=(kto[8]-'0')*2; else pz+=pz1=(kto[8]-'0')*2-9;
#endif
         pz1=pz;  /* Summe merken für Fall b */
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

         if(*kto=='0' && *(kto+1)=='0' && *(kto+2)=='0' && *(kto+3)=='0'){

      CASE_U(74,2)
#if DEBUG
            if(untermethode){

                  /* pz wurde noch nicht berechnet; jetzt erledigen.
                   * Da dieser Code nur im DEBUG-Fall auftritt, wurde er
                   * für VMS nicht optimiert.
                   */
               pz1=(kto[5]-'0')+(kto[7]-'0');
               if(kto[4]<'5')pz1+=(kto[4]-'0')*2; else pz1+=(kto[4]-'0')*2-9;
               if(kto[6]<'5')pz1+=(kto[6]-'0')*2; else pz1+=(kto[6]-'0')*2-9;
               if(kto[8]<'5')pz1+=(kto[8]-'0')*2; else pz1+=(kto[8]-'0')*2-9;
            }
#endif
            pz=5-(pz1%5);  /* hochrechnen auf nächste Halbdekade */
//            if(pz==5)pz=0;  /* fraglich, ob in dem Fall 0 oder 5 genommen wird */
            CHECK_PZ10;
         }
         else
            return FALSE;

/*  Berechnung nach der Methode 75 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 75                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2.                              #
 * # Die Kontonummer (6-, 7- oder 9-stellig) ist durch linksbündige     #
 * # Nullenauffüllung 10-stellig darzustellen. Die für die Berech-      #
 * # nung relevante 5-stellige Stammnummer wird von links nach          #
 * # rechts mit den Ziffern 2, 1, 2, 1, 2 multipliziert. Die weitere    #
 * # Berechnung und die Ergebnisse entsprechen dem Verfahren 00.        #
 * # Bei 6- und 7-stelligen Kontonummern befindet sich die für die      #
 * # Berechnung relevante Stammnummer in den Stellen 5 bis 9, die       #
 * # Prüfziffer in Stelle 10 der Kontonummer.                           #
 * # Bei 9-stelligen Kontonummern befindet sich die für die Berech-     #
 * # nung relevante Stammnummer in den Stellen 2 bis 6, die Prüf-       #
 * # ziffer in der 7. Stelle der Kontonummer. Ist die erste Stelle      #
 * # der 9-stelligen Kontonummer = 9 (2. Stelle der "gedachten"         #
 * # Kontonummer), so befindet sich die für die Berechnung relevante    #
 * # Stammnummer in den Stellen 3 bis 7, die Prüfziffer in der 8.       #
 * # Stelle der Kontonummer.                                            #
 * ######################################################################
 */
      CASE0(75)
         if(*kto!='0')return INVALID_KTO;   /* 10-stellige Kontonummer */
         if(*kto=='0' && *(kto+1)=='0'){   /* 6/7-stellige Kontonummer */
      CASE_U(75,1)
            if(*(kto+2)!='0' || *(kto+2)=='0' && *(kto+3)=='0' && *(kto+4)=='0')
               return INVALID_KTO;   /* 8- oder <6-stellige Kontonummer */
#ifdef __ALPHA
            pz = ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
               +  (kto[7]-'0')
               + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
            pz=(kto[5]-'0')+(kto[7]-'0');
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
            if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }
         else if(*(kto+1)=='9'){   /* 9-stellige Kontonummer, Variante 2 */
      CASE_U(75,2)
#ifdef __ALPHA
            pz = ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
               +  (kto[3]-'0')
               + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9);
#else
            pz=(kto[3]-'0')+(kto[5]-'0');
            if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
#endif
         MOD_10_40;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ8;
         }
         else{   /* 9-stellige Kontonummer, Variante 1 */
      CASE_U(75,3)
#ifdef __ALPHA
            pz = ((kto[1]<'5') ? (kto[1]-'0')*2 : (kto[1]-'0')*2-9)
               +  (kto[2]-'0')
               + ((kto[3]<'5') ? (kto[3]-'0')*2 : (kto[3]-'0')*2-9)
               +  (kto[4]-'0')
               + ((kto[5]<'5') ? (kto[5]-'0')*2 : (kto[5]-'0')*2-9);
#else
            pz=(kto[2]-'0')+(kto[4]-'0');
            if(kto[1]<'5')pz+=(kto[1]-'0')*2; else pz+=(kto[1]-'0')*2-9;
            if(kto[3]<'5')pz+=(kto[3]-'0')*2; else pz+=(kto[3]-'0')*2-9;
            if(kto[5]<'5')pz+=(kto[5]-'0')*2; else pz+=(kto[5]-'0')*2-9;
#endif
         MOD_10_40;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ7;
         }

/*  Berechnung nach der Methode 76 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 76                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5 ff.                              #
 * # Die einzelnen Stellen der für die Berechnung der Prüfziffer        #
 * # relevanten 5-, 6- oder 7-stelligen Stammnummer sind von rechts     #
 * # nach links mit den Ziffern 2, 3, 4, 5 ff. zu multiplizieren.       #
 * # Die jeweiligen Produkte werden addiert. Die Summe ist durch 11     #
 * # zu dividieren. Der verbleibende Rest ist die Prüfziffer. Ist       #
 * # der Rest 10, kann die Kontonummer nicht verwendet werden.          #
 * # Darstellung der Kontonummer: ASSSSSSPUU.                           #
 * # Ist die Unterkontonummer "00", kann es vorkommen, daß sie auf      #
 * # Zahlungsbelegen nicht angegeben ist. Die Prüfziffer ist dann       #
 * # an die 10. Stelle gerückt.                                         #
 * # Die Kontoart (1. Stelle) kann den Wert 0, 4, 6, 7, 8 oder 9 haben. #
 * ######################################################################
 */
      CASE0(76)
      CASE_U(76,1)
         if((p1= *kto)=='1' || p1=='2' || p1=='3' || p1=='5'){
            pz= -3;
            return INVALID_KTO;
         }
         pz = (kto[1]-'0') * 7
            + (kto[2]-'0') * 6
            + (kto[3]-'0') * 5
            + (kto[4]-'0') * 4
            + (kto[5]-'0') * 3
            + (kto[6]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         CHECK_PZX8;

      CASE_U(76,2)
         if((p1=kto[2])=='1' || p1=='2' || p1=='3' || p1=='5'){
#if DEBUG
            pz= -3;
#endif
            return INVALID_KTO;
         }
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz==10){
            pz= -2;
            return INVALID_KTO;
         }
         CHECK_PZ10;

/*  Berechnung nach der Methode 77 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 77                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 1, 2, 3, 4, 5.                              #
 * # Die Kontonummer ist 10-stellig. Die für die Berechnung             #
 * # relevanten Stellen 6 bis 10 werden von rechts nach links mit       #
 * # den Ziffern 1, 2, 3, 4, 5 multipliziert. Die Produkte werden       #
 * # addiert. Die Summe ist durch 11 zu dividieren. Verbleibt nach      #
 * # der Division der Summe durch 11 ein Rest, ist folgende neue        #
 * # Berechnung durchzuführen:                                          #
 * # Modulus 11, Gewichtung 5, 4, 3, 4, 5.                              #
 * # Ergibt sich bei der erneuten Berechnung wiederum ein Rest,         #
 * # dann ist die Kontonummer falsch.                                   #
 * ######################################################################
 */
      CASE0(77)
      CASE_U(77,1)
         pz = (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2
            + (kto[9]-'0');

         MOD_11_88;   /* pz%=11 */
         if(pz==0)return OK;
#if DEBUG
         if(untermethode)return INVALID_KTO;
#endif
      CASE_U(77,2)
         pz = (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 4
            + (kto[9]-'0') * 5;

         MOD_11_176;   /* pz%=11 */
         if(pz==0)
            return OK;
         else
#if DEBUG
         if(untermethode)
            return INVALID_KTO;
         else
#endif
            return FALSE;

/*  Berechnung nach der Methode 78 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 78                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2.                  #
 * # Die Berechnung erfolgt wie bei Verfahren 00. Ausnahme:             #
 * # 8-stellige Kontonummern sind nicht prüfbar, da diese Nummern       #
 * # keine Prüfziffer enthalten.                                        #
 * ######################################################################
 */
      CASE0(78)
         if(*kto=='0' && *(kto+1)=='0' && *(kto+2)!='0')return OK_NO_CHK;
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz =(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif

         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 79 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 79                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2 ff.                     #
 * # Die Kontonummer ist 10-stellig. Die Berechnung und Ergebnisse      #
 * # entsprechen dem Verfahren 00. Es ist jedoch zu beachten, dass      #
 * # die Berechnung vom Wert der 1. Stelle der Kontonummer abhängig     #
 * # ist.                                                               #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Die 1. Stelle der Kontonummer hat die Ziffer 3, 4, 5, 6, 7         #
 * # oder 8                                                             #
 * # Die für die Berechnung relevanten Stellen der Kontonummer          #
 * # befinden sich in den Stellen 1 bis 9. Die 10. Stelle ist per       #
 * # Definition die Prüfziffer.                                         #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Die 1. Stelle der Kontonummer hat die Ziffer 1, 2 oder 9           #
 * # Die für die Berechnung relevanten Stellen der Kontonummer          #
 * # befinden sich in den Stellen 1 bis 8. Die 9. Stelle ist die        #
 * # Prüfziffer der 10-stelligen Kontonummer.                           #
 * #                                                                    #
 * # Kontonummern, die in der 1. Stelle eine 0 haben,                   #
 * # wurden nicht vergeben und gelten deshalb als falsch.               #
 * ######################################################################
 */
      CASE0(79)
         if(*kto==0)return INVALID_KTO;
         if(*kto=='1' || *kto=='2' || *kto=='9'){
#ifdef __ALPHA
         pz =  (kto[0]-'0')
            + ((kto[1]<'5') ? (kto[1]-'0')*2 : (kto[1]-'0')*2-9)
            +  (kto[2]-'0')
            + ((kto[3]<'5') ? (kto[3]-'0')*2 : (kto[3]-'0')*2-9)
            +  (kto[4]-'0')
            + ((kto[5]<'5') ? (kto[5]-'0')*2 : (kto[5]-'0')*2-9)
            +  (kto[6]-'0')
            + ((kto[7]<'5') ? (kto[7]-'0')*2 : (kto[7]-'0')*2-9);
#else
         pz =(kto[0]-'0')+(kto[2]-'0')+(kto[4]-'0')+(kto[6]-'0');
         if(kto[1]<'5')pz+=(kto[1]-'0')*2; else pz+=(kto[1]-'0')*2-9;
         if(kto[3]<'5')pz+=(kto[3]-'0')*2; else pz+=(kto[3]-'0')*2-9;
         if(kto[5]<'5')pz+=(kto[5]-'0')*2; else pz+=(kto[5]-'0')*2-9;
         if(kto[7]<'5')pz+=(kto[7]-'0')*2; else pz+=(kto[7]-'0')*2-9;
#endif

            MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ9;
         }
         else{
#ifdef __ALPHA
            pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
               +  (kto[1]-'0')
               + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
               +  (kto[3]-'0')
               + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
               +  (kto[7]-'0')
               + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
            pz =(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
            if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
            if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
            if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif

            MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }

/* Berechnungsmethoden 80 bis 89 +§§§3
   Berechnung nach der Methode 80 +§§§4 */
/*
 * ######################################################################
 * #          Berechnung nach der Methode 80 (geändert zum 8.6.04)      #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2.                              #
 * # Die Berechnung und die möglichen Ergebnisse entsprechen dem        #
 * # Verfahren 00; es ist jedoch  zu beachten, daß nur die Stellen      #
 * # 5 bis 9 in das Prüfziffernberechnungsverfahren einbezogen          #
 * # werden.                                                            #
 * # Führt die Berechnung zu einem Prüfziffernfehler, so ist die        #
 * # Berechnung nach Variante 2 vorzunehmen. Das Berechnungsverfahren   #
 * # entspricht Variante 1. Die Summe der Produkte ist jedoch durch     #
 * # 7 zu dividieren. Der verbleibende Rest wird vom Divisor (7)        #
 * # subtrahiert. Das Ergebnis ist die Prüfziffer. Verbleibt nach       #
 * # der Division kein Rest, ist die Prüfziffer 0.                      #
 * #                                                                    #
 * # Ausnahme (alt, als Referenz):                                      #
 * # Sind die 3. und 4. Stelle der Kontonummer = 99, so erfolgt die     #
 * # Berechnung nach Verfahren 10.                                      #
 * #                                                                    #
 * # Ausnahme (neu ab 8.6.04):                                          #
 * # Ist nach linksbündiger Auffüllung mit Nullen auf 10 Stellen die    #
 * # 3. Stelle der Kontonummer = 9 (Sachkonten), so erfolgt die         #
 * # Berechnung gemäß der Ausnahme in Methode 51 mit den gleichen       #
 * # Ergebnissen und Testkontonummern.                                  #
 * ######################################################################
 */

      CASE0(80)

                /* Ausnahme, Variante 1 */
         if(*(kto+2)=='9'){   /* Berechnung wie in Verfahren 51 */
      CASE_U(80,3)
            pz =         9 * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZX10;

            /* Ausnahme, Variante 2 */
      CASE_U(80,4)
               pz = (kto[0]-'0') * 10
               + (kto[1]-'0') * 9
               +            9 * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

               MOD_11_352;   /* pz%=11 */
               if(pz<=1)
                  pz=0;
               else
                  pz=11-pz;
               CHECK_PZ10;
            }

            /* Variante 1 */
      CASE_U(80,1)
#ifdef __ALPHA
         pz = ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz =(kto[5]-'0')+(kto[7]-'0');
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif

         MOD_10_40;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

            /* Variante 2 */
      CASE_U(80,2)
#ifdef __ALPHA
         pz = ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz =(kto[5]-'0')+(kto[7]-'0');
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif

         MOD_7_56;   /* pz%=10 */
         if(pz)pz=7-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 81 (alt) +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 81 (alt)                  #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7.                           #
 * # Ist die 3. Stelle der Kontonummer = 9, so erfolgt die              #
 * # Berechnung nach Verfahren 10, sonst nach Verfahren 32.             #
 * ######################################################################
        Berechnung nach der Methode 81  (neu) +§§§4
 * #    Berechnung nach der Methode 81  (geändert zum 6.9.2004)         #
 * ######################################################################
 * #                                                                    #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7                            #
 * # Die Kontonummer ist durch linksbündige Nullenauffüllung stellig    #
 * # darzustellen. Die 10. Stelle ist per Definition Prüfziffer. Die    #
 * # für die Berechnung relevanten Stellen werden von rechts nach       #
 * # links mit den Ziffern 2, 3, 4, 5, 6, multipliziert. Die weitere    #
 * # Berechnung und die möglichen Ergebnisse entsprechen dem Verfahren  #
 * # 32.                                                                #
 * #                                                                    #
 * # Ausnahme:                                                          #
 * # Ist nach linksbündiger Auffüllung mit Nullen auf 10 Stellen 3.     #
 * # Stelle der Kontonummer = 9 (Sachkonten), so erfolgt Berechnung     #
 * # gemäß der Ausnahme in Methode 51 mit gleichen Ergebnissen und      #
 * # Testkontonummern.                                                  #
 * ######################################################################
 */

      CASE0(81)
                /* Ausnahme, Variante 1 */
         if(*(kto+2)=='9'){   /* Berechnung wie in Verfahren 51 */
      CASE_U(81,2)
            pz =         9 * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZX10;

            /* Ausnahme, Variante 2 */
      CASE_U(81,3)
               pz = (kto[0]-'0') * 10
               + (kto[1]-'0') * 9
               +            9 * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

               MOD_11_352;   /* pz%=11 */
               if(pz<=1)
                  pz=0;
               else
                  pz=11-pz;
               CHECK_PZ10;
            }

      CASE_U(81,1)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 82 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 82                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7.                           #
 * # Sind die 3. und 4. Stelle der Kontonummer = 99, so erfolgt die     #
 * # Berechnung nach Verfahren 10, sonst nach Verfahren 33.             #
 * ######################################################################
 */
      CASE0(82)
      CASE_U(82,1)
            /* Verfahren 10 */
         if(*(kto+2)=='9' && *(kto+3)=='9'){
            pz = (kto[0]-'0') * 10
               + (kto[1]-'0') * 9
               + (kto[2]-'0') * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            MOD_11_352;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZ10;
         }

            /* Verfahren 33 */
      CASE_U(82,2)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 83 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 83                        #
 * ######################################################################
 * # 1. Kundenkonten                                                    #
 * # A. Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7                         #
 * # B. Modulus 11, Gewichtung 2, 3, 4, 5, 6                            #
 * # C. Modulus  7, Gewichtung 2, 3, 4, 5, 6                            #
 * # Gemeinsame Anmerkungen für die Berechnungsverfahren:               #
 * # Die Kontonummer ist immer 10-stellig. Die für die Berechnung       #
 * # relevante Kundennummer (K) befindet sich bei der Methode A in      #
 * # den Stellen 4 bis 9 der Kontonummer und bei den Methoden B + C     #
 * # in den Stellen 5 - 9, die Prüfziffer in Stelle 10 der              #
 * # Kontonummer.                                                       #
 * #                                                                    #
 * # Ergibt die erste Berechnung der Prüfziffer nach dem Verfahren A    #
 * # einen Prüfzifferfehler, so sind weitere Berechnungen mit den       #
 * # anderen Methoden vorzunehmen. Kontonummern, die nach               #
 * # Durchführung aller 3 Berechnungsmethoden nicht zu einem            #
 * # richtigen Ergebnis führen, sind nicht prüfbar.                     #
 * #                                                                    #
 * # Methode A:                                                         #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7                            #
 * # Die Berechnung und möglichen Ergebnisse entsprechen                #
 * # dem Verfahren 32.                                                  #
 * #                                                                    #
 * # Methode B:                                                         #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6                               #
 * # Die Berechnung und möglichen Ergebnisse entsprechen                #
 * # dem Verfahren 33.                                                  #
 * #                                                                    #
 * # Methode C:                                                         #
 * # Kontonummern, die bis zur Methode C gelangen und in der 10.        #
 * # Stelle eine 7, 8 oder 9 haben, sind ungültig. Modulus 7,           #
 * # Gewichtung 2, 3, 4, 5, 6 Das Berechnungsverfahren entspricht       #
 * # Methode B. Die Summe der Produkte ist jedoch durch 7 zu            #
 * # dividieren. Der verbleibende Rest wird vom Divisor (7)             #
 * # subtrahiert. Das Ergebnis ist die Prüfziffer. Verbleibt kein       #
 * # Rest, ist die Prüfziffer 0.                                        #
 * #                                                                    #
 * # 2. Sachkonten                                                      #
 * # Berechnungsmethode:                                                #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8                         #
 * # Die Sachkontonummer ist immer 10-stellig. Die für die Berechnung   #
 * # relevante  Sachkontontenstammnummer (S) befindet sich in den       #
 * # Stellen 3 bis 9 der Kontonummer, wobei die 3. und 4. Stelle        #
 * # immer jeweils 9 sein müssen; die Prüfziffer ist in Stelle 10 der   #
 * # Sachkontonummer. Führt die Berechnung nicht zu einem richtigen     #
 * # Ergebnis, ist die Nummer nicht prüfbar.                            #
 * # Berechnung:                                                        #
 * # Die einzelnen Stellen der Sachkontonummern sind von rechts nach    #
 * # links mit den Ziffern 2, 3, 4, 5, 6, 7, 8 zu multiplizieren. Die   #
 * # jeweiligen Produkte werden addiert. Die Summe ist durch 11 zu      #
 * # dividieren. Der verbleibende Rest wird vom Divisor (11)            #
 * # subtrahiert. Das Ergebnis ist die Prüfziffer. Verbleibt nach der   #
 * # Division durch die 11 kein Rest, ist die Prüfziffer "0". Das       #
 * # Rechenergebnis "10" ist nicht verwendbar und muss auf eine         #
 * # Stelle reduziert werden. Die rechte Stelle Null findet als         #
 * # Prüfziffer Verwendung.                                             #
 * ######################################################################
 */
      CASE0(83)
            /* Sachkonten */
      CASE_U(83,4)
         if(*(kto+2)=='9' && *(kto+3)=='9'){
            pz =      9       * 8
               +      9       * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZ10;
         }

            /* Methode A */
      CASE_U(83,1)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

            /* Methode B */
      CASE_U(83,2)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

            /* Methode C */
      CASE_U(83,3)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_7_112;   /* pz%=7 */
         if(pz)pz=7-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 84 +§§§4 */
/*
 * ######################################################################
 * #          Berechnung nach der Methode 84 (geändert zum 6.9.04)      #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6                               #
 * # Variante 1                                                         #
 * # Die Kontonummer ist durch linksbündige Nullenauffüllung            #
 * # 10-stellig darzustellen. Die 10. Stelle ist per Definition die     #
 * # Prüfziffer. Die für die Berechnung relevanten Stellen werden       #
 * # von rechts nach links mit den Ziffern 2, 3, 4, 5, 6                #
 * # multipliziert. Die weitere Berechnung und die möglichen            #
 * # Ergebnisse entsprechen dem Verfahren 33.                           #
 * # Führt die Berechnung nach Variante 1 zu einem Prüfziffer-          #
 * # fehler, ist die Berechnung nach Variante 2 vorzunehmen.            #
 * # Variante 2                                                         #
 * # Modulus 7, Gewichtung 2, 3, 4, 5, 6                                #
 * # Die Stellen 5 bis 9 der Kontonummer werden von rechts              #
 * # nach links mit den Gewichten multipliziert. Die jeweiligen         #
 * # Produkte werden addiert. Die Summe ist durch 7 zu                  #
 * # dividieren. Der verbleibende Rest wird vom Divisor (7)             #
 * # subtrahiert. Das Ergebnis ist die Prüfziffer. Verbleibt nach       #
 * # der Division kein Rest, ist die Prüfziffer = 0.                    #
 * #                                                                    #
 * # Ausnahme (alt, als Referenz):                                      #
 * # Sind die 3. und 4. Stelle der Kontonummer = 99, so erfolgt         #
 * # die Berechnung nach Verfahren 10.                                  #
 * #                                                                    #
 * # Ausnahme (neu ab 6.9.04):                                          #
 * # Ist nach linksbündiger Auffüllung mit Nullen auf 10 Stellen die    #
 * # 3. Stelle der Kontonummer = 9 (Sachkonten), so erfolgt die         #
 * # Berechnung gemäß der Ausnahme in Methode 51 mit den gleichen       #
 * # Ergebnissen und Testkontonummern.                                  #
 * ######################################################################
 */

      CASE0(84)

                /* Ausnahme, Variante 1 */
         if(*(kto+2)=='9'){   /* Berechnung wie in Verfahren 51 */
      CASE_U(84,3)
            pz =         9 * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZX10;

            /* Ausnahme, Variante 2 */
      CASE_U(84,4)
               pz = (kto[0]-'0') * 10
               + (kto[1]-'0') * 9
               +            9 * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

               MOD_11_352;   /* pz%=11 */
               if(pz<=1)
                  pz=0;
               else
                  pz=11-pz;
               CHECK_PZ10;
            }

            /* Variante 1 */
      CASE_U(84,1)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

            /* Variante 2 */
      CASE_U(84,2)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_7_112;   /* pz%=7 */
         if(pz)pz=7-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 85 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 85                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6.                              #
 * # Wie Verfahren 83, jedoch folgende Ausnahme:                        #
 * # Sind die 3. und 4. Stelle der Kontonummer = 99, so ist folgende    #
 * # Prüfziffernberechnung maßgebend:                                   #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8.                        #
 * # Die für die Berechnung relevanten Stellen 3 bis 9 der Kontonr      #
 * # werden von rechts nach links mit den Ziffern 2, 3, 4, 5, 6, 7, 8   #
 * # multipliziert. Die weitere Berechnung und möglichen Ergebnisse     #
 * # entsprechen dem Verfahren 02.                                      #
 * ######################################################################
 */
      CASE0(85)
      CASE_U(85,4)
            /* Sachkonten */
         if(*(kto+2)=='9' && *(kto+3)=='9'){
            pz =      9       * 8
               +      9       * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz)pz=11-pz;
            if(pz==10)return INVALID_KTO;
            CHECK_PZ10;
         }

            /* Methode A */
      CASE_U(85,1)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

            /* Methode B */
      CASE_U(85,2)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

            /* Methode C */
      CASE_U(85,3)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_7_112;   /* pz%=7 */
         if(pz)pz=7-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 86 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 86                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1.                           #
 * # Die Berechnung entspricht dem Verfahren 00, jedoch werden nur      #
 * # die Stellen 4 bis 9 einbezogen, Stelle 10 ist die Prüfziffer.      #
 * # Führt die Berechnung zu einem Prüfziffernfehler, so ist die        #
 * # Variante 2 anzuwenden:                                             #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7.                           #
 * # Die Stellen 4 bis 9 der Kontonummer werden von rechts nach links   #
 * # mit den Ziffern 2, 3, 4, 5, 6, 7 multipliziert. Die weitere        #
 * # Berechnung entspricht dem Verfahren 32. Die Stelle 10 ist die      #
 * # Prüfziffer.                                                        #
 * #                                                                    #
 * # Ausnahme (alt, als Referenz):                                      #
 * # Ist die 3. Stelle = 9, so erfolgt die Berechnung nach Verfahren 10 #
 * #                                                                    #
 * # Ausnahme (neu ab 6.9.04):                                          #
 * # Ist nach linksbündiger Auffüllung mit Nullen auf 10 Stellen die    #
 * # 3. Stelle der Kontonummer = 9 (Sachkonten), so erfolgt die         #
 * # Berechnung gemäß der Ausnahme in Methode 51 mit den gleichen       #
 * # Ergebnissen und Testkontonummern.                                  #
 * ######################################################################
 */

      CASE0(86)

                /* Ausnahme, Variante 1 */
         if(*(kto+2)=='9'){   /* Berechnung wie in Verfahren 51 */
      CASE_U(86,3)
            pz =         9 * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZX10;

            /* Ausnahme, Variante 2 */
      CASE_U(86,4)
               pz = (kto[0]-'0') * 10
               + (kto[1]-'0') * 9
               +            9 * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

               MOD_11_352;   /* pz%=11 */
               if(pz<=1)
                  pz=0;
               else
                  pz=11-pz;
               CHECK_PZ10;
            }

            /* Methode A */
      CASE_U(86,1)
#ifdef __ALPHA
            pz =  (kto[3]-'0')
               + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
               +  (kto[7]-'0')
               + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
            pz =(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
            if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif

            MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

            /* Methode B */
      CASE_U(86,2)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 87 +§§§4 */
/*
 * ######################################################################
 * #          Berechnung nach der Methode 87 (geändert zum 6.9.04)      #
 * ######################################################################
 * # Ausnahme: (neu zum 6.9.04, der Rest ist gleich geblieben)          #
 * # Ist nach linksbündiger Auffüllung mit Nullen auf 10 Stellen die    #
 * # 3. Stelle der Kontonummer = 9 (Sachkonten), so erfolgt die         #
 * # Berechnung gemäß der Ausnahme in Methode 51 mit den                #
 * # gleichen Ergebnissen und Testkontonummern.                         #
 * #                                                                    #
 * # Methode A:                                                         #
 * # Vorgegebener Pascalcode, anzuwenden auf Stellen 5 bis 9            #
 * # von links der Kontonummer, Prüfziffer in Stelle 10.                #
 * # Der vorgegebener Pseudocode (pascal-ähnlich) wurde nach C          #
 * # umgeschrieben. Eine Beschreibung des Berechnungsverfahrens findet  #
 * # sich in der Datei pz<mmyy>.pdf (z.B. pz0602.pdf) der  Deutschen    #
 * # Bundesbank.                                                        #
 * #                                                                    #
 * # Methode B:                                                         #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6                               #
 * # Die für die Berechnung relevanten Stellen werden von rechts        #
 * # nach links mit den Ziffern 2, 3, 4, 5, 6 multipliziert. Die        #
 * # weitere Berechnung und die möglichen Ergebnisse entsprechen dem    #
 * # Verfahren 33.                                                      #
 * # Führt die Berechnung nach Methode B wiederum zu einem              #
 * # Prüfzifferfehlen, ist eine weitere Berechnung nach Methode C       #
 * # vorzunehmen.                                                       #
 * #                                                                    #
 * # Methode C:                                                         #
 * # Modulus 7, Gewichtung 2, 3, 4, 5, 6                                #
 * # Die Stellen 5 bis 9 der Kontonummer werden von rechts nach         #
 * # links mit den Gewichten multipliziert. Die jeweiligen Produkte     #
 * # werden addiert. Die Summe ist durch 7 zu dividieren. Der           #
 * # verbleibende Rest wird vom Divisor (7) subtrahiert. Das            #
 * # Ergebnis ist die Prüfziffer. Verbleibt nach der Division kein      #
 * # Rest, ist die Prüfziffer = 0.                                      #
 * ######################################################################
 */

      CASE0(87)

                /* Ausnahme, Variante 1 */
         if(*(kto+2)=='9'){   /* Berechnung wie in Verfahren 51 */
      CASE_U(87,4)
            pz =         9 * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZX10;

            /* Ausnahme, Variante 2 */
      CASE_U(87,5)
               pz = (kto[0]-'0') * 10
               + (kto[1]-'0') * 9
               +            9 * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

               MOD_11_352;   /* pz%=11 */
               if(pz<=1)
                  pz=0;
               else
                  pz=11-pz;
               CHECK_PZ10;
            }

      CASE_U(87,1)
            /* Der Startindex für das Array konto[] ist 1, nicht wie in C
             * üblich 0; daher hat das es auch 11 Elemente (nicht wie in der
             * Beschreibung angegeben 10). Daß das so ist, sieht man an der
             * Initialisierung von i mit 4, an der Schleife [while(i<10)]
             * sowie am Ende des Verfahrens, wo p mit konto[10] verglichen
             * wird.
             * Konsequenterweise werden die beiden Arrays tab1 und tab2
             * mit dem Startindex 0 eingeführt ;-(((.
             */

         for(i=1,ptr=kto;i<11;)konto[i++]= *ptr++-'0';
         i=4;
         while(konto[i]==0)i++;
         c2=i%2;
         d2=0;
         a5=0;

         while(i<10){
            switch(konto[i]){
               case 0: konto[i]= 5; break;
               case 1: konto[i]= 6; break;
               case 5: konto[i]=10; break;
               case 6: konto[i]= 1; break;
            }

            if(c2==d2){
               if(konto[i]>5){
                  if(c2==0 && d2==0){
                     c2=d2=1;
                     a5=a5+6-(konto[i]-6);
                  }
                  else{
                     c2=d2=0;
                     a5=a5+konto[i];
                  }
               }
               else{
                  if(c2==0 && d2==0){
                     c2=1;
                     a5=a5+konto[i];
                  }
                  else{
                     c2=0;
                     a5=a5+konto[i];
                  }
               }
            }
            else{
               if(konto[i]>5){
                  if(c2==0){
                     c2=1;
                     d2=0;
                     a5=a5-6+(konto[i]-6);
                  }
                  else{
                     c2=0;
                     d2=1;
                     a5=a5-konto[i];
                  }
               }
               else{
                  if(c2==0){
                     c2=1;
                     a5=a5-konto[i];
                  }
                  else{
                     c2=0;
                     a5=a5-konto[i];
                  }
               }
            }
            i++;
         }
         while(a5<0 || a5>4){
            if(a5>4)
               a5=a5-5;
            else
               a5=a5+5;
         }
         if(d2==0)
            p=tab1[a5];
         else
            p=tab2[a5];
         if(p==konto[10])
            return OK; /* Prüfziffer ok; */
         else{
            if(konto[4]==0){
               if(p>4)
                  p=p-5;
               else
                  p=p+5;
               if(p==konto[10])return OK; /* Prüfziffer ok; */
            }
         }

            /* Methode B: Verfahren 33 */
      CASE_U(87,2)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

            /* Methode C: Verfahren 84 Variante 2 */
      CASE_U(87,3)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_7_112;   /* pz%=7 */
         if(pz)pz=7-pz;
         if(pz==*(kto+9)-'0')
            return OK;
         else
            return FALSE;


/*  Berechnung nach der Methode 88 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 88                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7.                           #
 * # Die Stellen 4 bis 9 werden von rechts nach links mit den           #
 * # Gewichten 2, 3, 4, 5, 6, 7 multipliziert. Die weitere Berechnung   #
 * # entspricht dem Verfahren 06.                                       #
 * # Ausnahme: Ist die 3. Stelle der Kontonummer = 9, so werden         #
 * # die Stellen 3 bis 9 von rechts nach links mit den Gewichten        #
 * # 2, 3, 4, 5, 6, 7, 8 multipliziert.                                 #
 * ######################################################################
 */
      CASE0(88)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;
         if(kto[2]=='9')pz+= 9*8;   /* Ausnahme */

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 89 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 89                        #
 * ######################################################################
 * # 8- und 9-stellige Kontonummern sind mit dem                        #
 * # Berechnungsverfahren 10 zu prüfen.                                 #
 * #                                                                    #
 * # 7-stellige Kontonummern sind wie folgt zu prüfen:                  #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7                            #
 * # Die Kontonummer ist durch linksbündige Nullenauffüllung            #
 * # 10-stellig darzustellen. Die für die Berechnung relevante          #
 * # 6-stellige Stammnummer (x) befindet sich in den Stellen 4 bis      #
 * # 9, die Prüfziffer in Stelle 10 der Kontonummer. Die einzelnen      #
 * # Stellen der Stammnummer sind von rechts nach links mit den         #
 * # Ziffern 2, 3, 4, 5, 6, 7 zu multiplizieren. Die jeweiligen         #
 * # Produkte werden addiert, nachdem jeweils aus den 2- stelligen      #
 * # Produkten Quersummen gebildet wurden. Die Summe ist durch 11       #
 * # zu dividieren. Die weiteren Berechnungen und Ergebnisse            #
 * # entsprechen dem Verfahren 06.                                      #
 * #                                                                    #
 * # 1- bis 6- und 10-stellige Kontonummern sind nicht zu               #
 * # prüfen, da diese keine Prüfziffer enthalten.                       #
 * # Testkontonummern: 1098506, 32028008, 218433000                     #
 * ######################################################################
 */
       CASE0(89)

            /* 8- und 9-stellige Kontonummern: Verfahren 10 */
       CASE_U(89,1)
         if(*kto=='0' && (*(kto+1)!='0' || *(kto+2)!='0')){
            pz = (kto[1]-'0') * 9
               + (kto[2]-'0') * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            MOD_11_352;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZ10;
         }

            /* 7-stellige Kontonummern */
       CASE_U(89,2)
         if(*kto=='0' && *(kto+1)=='0' && *(kto+2)=='0' && *(kto+3)!='0'){
            pz=(kto[3]-'0')*7;
            if(pz>=40){
               if(pz>=50){
                  if(pz>=60)
                     pz-=54;
                  else
                     pz-=45;
               }
               else
                  pz-=36;
            }
            else if(pz>=20){
               if(pz>=30)
                  pz-=27;
               else
                  pz-=18;
            }
            else if(pz>=10)
               pz-=9;


            p1=(kto[4]-'0')*6;
            if(p1>=40){
               if(p1>=50)
                  p1-=45;
               else
                  p1-=36;
            }
            else if(p1>=20){
               if(p1>=30)
                  p1-=27;
               else
                  p1-=18;
            }
            else if(p1>=10)
               p1-=9;
            pz+=p1;

            p1=(kto[5]-'0')*5;
            if(p1>=40)
               p1-=36;
            else if(p1>=20){
               if(p1>=30)
                  p1-=27;
               else
                  p1-=18;
            }
            else if(p1>=10)
               p1-=9;
            pz+=p1;

            p1=(kto[6]-'0')*4;
            if(p1>=20){
               if(p1>=30)
                  p1-=27;
               else
                  p1-=18;
            }
            else if(p1>=10)
               p1-=9;
            pz+=p1;

            p1=(kto[7]-'0')*3;
            if(p1>=20)
               p1-=18;
            else if(p1>=10)
               p1-=9;
            pz+=p1;

            p1 = (kto[8]-'0') * 2;
            if(p1>=10)p1-=9;
            pz+=p1;

            MOD_11_44;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZ10;
         }

            /* 1- bis 6- und 10-stellige Kontonummern */
       CASE_U(89,3)
         return OK_NO_CHK;


/* Berechnungsmethoden 90 bis 99 +§§§3
   Berechnung nach der Methode 90 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 90                        #
 * ######################################################################
 * # 1. Kundenkonten                                                    #
 * # A. Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7. -> Methode 32          #
 * # B. Modulus 11, Gewichtung 2, 3, 4, 5, 6.    -> Methode 33          #
 * # C. Modulus  7, Gewichtung 2, 3, 4, 5, 6.    -> Methode 33 mod7     #
 * # D. Modulus  9, Gewichtung 2, 3, 4, 5, 6.    -> Methode 33 mod9     #
 * # E. Modulus 10, Gewichtung 2, 1, 2, 1, 2.    -> Methode 33 mod10    #
 * #                                                                    #
 * # Die Kontonummer ist immer 10-stellig. Die für die Berechnung       #
 * # relevante Kundennummer befindet sich bei der Methode A in den      #
 * # Stellen 4 bis 9 der Kontonummer und bei den Methoden B - E in      #
 * # den Stellen 5 bis 9, die Prüfziffer in Stelle 10.                  #
 * #                                                                    #
 * # Ergibt die erste Berechnung der Prüfziffer nach dem Verfahren A    #
 * # einen Prüfziffernfehler, so sind weitere Berechnungen mit den      #
 * # anderen Methoden vorzunehmen.                                      #
 * # Die Methode A enstpricht Verfahren 32. Die Methoden B - E          #
 * # entsprechen Verfahren 33, jedoch mit Divisoren 11, 7, 9 und 10.    #
 * #                                                                    #
 * # Ausnahme: Ist nach linksbündigem Auffüllen mit Nullen auf 10       #
 * # Stellen die 3. Stelle der Kontonummer = 9 (Sachkonten) befindet    #
 * # sich die für die Berechnung relevante Sachkontonummer (S) in       #
 * # den Stellen 3 bis 9. Diese Kontonummern sind ausschließlich        #
 * # nach Methode F zu prüfen.                                          #
 * #                                                                    #
 * # 2. Sachkonten -> Methode 32 (modifiziert)                          #
 * # F. Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8.                     #
 * # Die 3. Stelle ist 9, die für die Berechnung relevanten  Stellen    #
 * # befinden sich in den Stellen 3 bis 9.                              #
 * ######################################################################
 */

      CASE0(90)

            /* Sachkonto */
         if(*(kto+2)=='9'){ /* geändert zum 6.6.2005; vorher waren 3. und 4. Stelle 9 */
      CASE_U(90,6)
         pz =      9       * 8 /* immer 9; kann vom Compiler optimiert werden */
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZ10;
         }

            /* Methode A */
      CASE_U(90,1)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

            /* Methode B */
      CASE_U(90,2)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

            /* Methode C */
      CASE_U(90,3)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_7_112;   /* pz%=7 */
         if(pz)pz=7-pz;
         CHECK_PZX10;

            /* Methode D */
      CASE_U(90,4)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_9_144;   /* pz%=9 */
         if(pz)pz=9-pz;
         CHECK_PZX10;

            /* Methode E */
      CASE_U(90,5)
         pz = (kto[4]-'0') * 2
            + (kto[5]-'0')
            + (kto[6]-'0') * 2
            + (kto[7]-'0')
            + (kto[8]-'0') * 2;

         MOD_10_40;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 91 (alt) +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 91                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7 (ohne Stellen 8 - 10).     #
 * # Modulus 11, Gewichtung 7, 6, 5, 4, 3, 2 (ohne Stellen 8 - 10).     #
 * # Modulus 11, Gewichtung 2, 3, 4, 0, 5, 6, 7, 8, 9, 10.              #
 * # Die Kontonummer ist immer 10-stellig. Die Stelle 7 ist die         #
 * # Prüfziffer. Die für die Berechnung relevanten Kundennummern        #
 * # sind von rechts nach links mit den jeweiligen Gewichten zu         #
 * # multiplizieren. Die restliche Berechnung enspricht dem             #
 * # Verfahren 06. Ergibt die erste Berechnung nach einer der           #
 * # beschriebenen Methoden einen Fehler, so sind weitere               #
 * # Berechnungen mit den anderen Methoden vorzunehmen.                 #
 * ######################################################################
       Berechnung nach der Methode 91 (neu)  +§§§4
 * #   Berechnung nach der Methode 91 (geändert zum 8.12.03)            #
 * ######################################################################
 * # 1. Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7                         #
 * # 2. Modulus 11, Gewichtung 7, 6, 5, 4, 3, 2                         #
 * # 3. Modulus 11, Gewichtung 2, 3, 4, 0, 5, 6, 7, 8, 9, A (A = 10)    #
 * # 4. Modulus 11, Gewichtung 2, 4, 8, 5, 10, 9                        #
 * #                                                                    #
 * # Gemeinsame Hinweise für die Berechnungsvarianten 1 bis 4:          #
 * #                                                                    #
 * # Die Kontonummer ist immer 10-stellig. Die einzelnen Stellen        #
 * # der Kontonummer werden von links nach rechts von 1 bis 10          #
 * # durchnummeriert. Die Stelle 7 der Kontonummer ist die              #
 * # Prüfziffer. Die für die Berechnung relevanten Kundennummern        #
 * # (K) sind von rechts nach links mit den jeweiligen Gewichten zu     #
 * # multiplizieren. Die restliche Berechnung und möglichen             #
 * # Ergebnisse entsprechen dem Verfahren 06.                           #
 * #                                                                    #
 * # Ergibt die Berechnung nach der ersten beschriebenen Variante       #
 * # einen Prüfzifferfehler, so sind in der angegebenen Reihenfolge     #
 * # weitere Berechnungen mit den anderen Varianten                     #
 * # vorzunehmen, bis die Berechnung keinen Prüfzifferfehler mehr       #
 * # ergibt. Kontonummern, die endgültig nicht zu einem richtigen       #
 * # Ergebnis führen, sind nicht prüfbar.                               #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7                            #
 * # Die Stellen 8 bis 10 werden nicht in die Berechnung                #
 * # einbezogen.                                                        #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 11, Gewichtung 7, 6, 5, 4, 3, 2                            #
 * # Die Stellen 8 bis 10 werden nicht in die Berechnung                #
 * # einbezogen.                                                        #
 * #                                                                    #
 * # Variante 3:                                                        #
 * # Modulus 11, Gewichtung 2, 3, 4, 0, 5, 6, 7, 8, 9, A (A = 10)       #
 * # Die Stellen 1 bis 10 werden in die Berechnung einbezogen.          #
 * #                                                                    #
 * # Variante 4:                                                        #
 * # Modulus 11, Gewichtung 2, 4, 8, 5, A, 9 (A = 10)                   #
 * # Die Stellen 8 bis 10 werden nicht in die Berechnung einbezogen.    #
 * ######################################################################
 */
      CASE0(91)

            /* Methode A */
      CASE_U(91,1)
         pz = (kto[0]-'0') * 7
            + (kto[1]-'0') * 6
            + (kto[2]-'0') * 5
            + (kto[3]-'0') * 4
            + (kto[4]-'0') * 3
            + (kto[5]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX7;

            /* Methode B */
      CASE_U(91,2)
         pz = (kto[0]-'0') * 2
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 4
            + (kto[3]-'0') * 5
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 7;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX7;

            /* Methode C */
      CASE_U(91,3)
         pz = (kto[0]-'0') * 10
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[7]-'0') * 4
            + (kto[8]-'0') * 3
            + (kto[9]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX7;

            /* Methode D */
      CASE_U(91,4)
         pz = (kto[0]-'0') * 9
            + (kto[1]-'0') * 10
            + (kto[2]-'0') * 5
            + (kto[3]-'0') * 8
            + (kto[4]-'0') * 4
            + (kto[5]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ7;

/*  Berechnung nach der Methode 92 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 92                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 3, 7, 1, 3, 7, 1.                           #
 * # Die Berechnung erfolgt wie bei Verfahren 01, jedoch werden nur     #
 * # die Stellen 4 bis 9 einbezogen. Stelle 10 ist die Prüfziffer.      #
 * ######################################################################
 */
      CASE0(92)
         pz = (kto[3]-'0')
            + (kto[4]-'0') * 7
            + (kto[5]-'0') * 3
            + (kto[6]-'0')
            + (kto[7]-'0') * 7
            + (kto[8]-'0') * 3;

         MOD_10_160;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 93 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 93                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6.                              #
 * # Die für die Berechnung relevante Kundennummer befindet sich        #
 * # entweder                                                           #
 * # a) in den Stellen 1 bis 5, die Prüfziffer in Stelle  6,            #
 * # b) in den Stellen 5 bis 9, die Prüfziffer in Stelle 10.            #
 * # Die 2-stellige Unternummer und die 2-stellige Kontoartnummer       #
 * # werden nicht in die Berechnung einbezogen. Sie befinden sich im    #
 * # Fall a) an Stelle 7 bis 10. Im Fall b) befinden Sie sich an        #
 * # Stelle 1 bis 4 und müssen "0000" lauten.                           #
 * # Die 5-stellige Kundennummer wird von rechts nach links mit den     #
 * # Gewichten multipliziert. Die weitere Berechnung und die            #
 * # möglichen Ergebnisse entsprechen dem Verfahren 06.                 #
 * # Führt die Berechnung zu einem Prüfziffernfehler, so ist die        #
 * # Berechnung nach Variante 2 vorzunehmen. Das Berechnungsverfahren   #
 * # entspricht Variante 1. Die Summe der Produkte ist jedoch durch     #
 * # 7 zu dividieren. Der verbleibende Rest wird vom Divisor (7)        #
 * # subtrahiert. Das Ergebnis ist die Prüfziffer.                      #
 * ######################################################################
 */
      CASE0(93)

            /* Variante 1 */
         if(*kto=='0' && *(kto+1)=='0' && *(kto+2)=='0' && *(kto+3)=='0'){   /* Fall b) */
      CASE_U(93,2)
            pz = (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            p1=pz;
            MOD_11_176;   /* pz%=11 */
         }
         else{
      CASE_U(93,1)
            pz = (kto[0]-'0') * 6
               + (kto[1]-'0') * 5
               + (kto[2]-'0') * 4
               + (kto[3]-'0') * 3
               + (kto[4]-'0') * 2;

            kto[9]=kto[5];  /* Prüfziffer nach Stelle 10 */
            p1=pz;
            MOD_11_176;   /* pz%=11 */
         }
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

            /* Variante 2 */
#if DEBUG
            /* nicht optimiert, da dieser Teil nur in der DEBUG-Version benutzt wird */

         if(untermethode){ /* pz wurde noch nicht berechnet */
            if(*kto=='0' && *(kto+1)=='0' && *(kto+2)=='0' && *(kto+3)=='0'){   /* Fall b) */
      CASE_U(93,4)
               for(p1=0,ptr=kto+8,i=0;i<5;ptr--,i++)
                  p1+=(*ptr-'0')*w93[i];
            }
            else{
      CASE_U(93,3)
               for(p1=0,ptr=kto+4,i=0;i<5;ptr--,i++)
                  p1+=(*ptr-'0')*w93[i];
                  *(kto+9)= *(kto+5);  /* Prüfziffer nach Stelle 10 */
            }
         }
#endif
         pz=p1%7;
         if(pz)pz=7-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 94 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 94                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 1, 2, 1, 2, 1, 2, 1, 2, 1.                  #
 * # Die Stellen 1 bis 9 der Kontonummer sind von rechts nach links     #
 * # mit den Gewichten zu multiplizieren. Die weitere Berechnung        #
 * # erfolgt wie bei Verfahren 00.                                      #
 * ######################################################################
 */
      CASE0(94)
#ifdef __ALPHA
         pz =  (kto[0]-'0')
            + ((kto[1]<'5') ? (kto[1]-'0')*2 : (kto[1]-'0')*2-9)
            +  (kto[2]-'0')
            + ((kto[3]<'5') ? (kto[3]-'0')*2 : (kto[3]-'0')*2-9)
            +  (kto[4]-'0')
            + ((kto[5]<'5') ? (kto[5]-'0')*2 : (kto[5]-'0')*2-9)
            +  (kto[6]-'0')
            + ((kto[7]<'5') ? (kto[7]-'0')*2 : (kto[7]-'0')*2-9)
            +  (kto[8]-'0');
#else
         pz =(kto[0]-'0')+(kto[2]-'0')+(kto[4]-'0')+(kto[6]-'0')+(kto[8]-'0');
         if(kto[1]<'5')pz+=(kto[1]-'0')*2; else pz+=(kto[1]-'0')*2-9;
         if(kto[3]<'5')pz+=(kto[3]-'0')*2; else pz+=(kto[3]-'0')*2-9;
         if(kto[5]<'5')pz+=(kto[5]-'0')*2; else pz+=(kto[5]-'0')*2-9;
         if(kto[7]<'5')pz+=(kto[7]-'0')*2; else pz+=(kto[7]-'0')*2-9;
#endif

         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 95 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 95                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 2, 3, 4                   #
 * # Die Berechnung erfolgt wie bei Verfahren 06.                       #
 * # Ausnahmen:                                                         #
 * # Kontonr.: 0000000001 bis 0001999999                                #
 * # Kontonr.: 0009000000 bis 0025999999                                #
 * # Kontonr.: 0396000000 bis 0499999999                                #
 * # Kontonr.: 0700000000 bis 0799999999                                #
 * # Für diese Kontonummernkreise ist keine Prüfzifferberechnung        #
 * # möglich. Sie sind als richtig anzusehen.                           #
 * ######################################################################
 */
     CASE0(95)
        if(   /* Ausnahmen: keine Prüfzifferberechnung */
            strcmp(kto,"0000000001")>=0 && strcmp(kto,"0001999999")<=0
         || strcmp(kto,"0009000000")>=0 && strcmp(kto,"0025999999")<=0
         || strcmp(kto,"0396000000")>=0 && strcmp(kto,"0499999999")<=0
         || strcmp(kto,"0700000000")>=0 && strcmp(kto,"0799999999")<=0)
            return OK_NO_CHK;
         pz = (kto[0]-'0') * 4
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 2
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 96 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 96                        #
 * ######################################################################
 * # A. Modulus 11, Gewichtung 2,3,4,5,6,7,8,9,1                        #
 * # B. Modulus 10, Gewichtung 2,1,2,1,2,1,2,1,2                        #
 * # Die Prüfziffernberechnung ist nach Kennziffer 19 durchzuführen.    #
 * # Führt die Berechnung zu einem Fehler, so ist sie nach Kennziffer   #
 * # 00 durchzuführen. Führen beide Varianten zu einem Fehler, so       #
 * # gelten Kontonummern zwischen 0001300000 und 0099399999 als         #
 * # richtig.                                                           #
 * ######################################################################
 */
      CASE0(96)

      CASE_U(96,3)
            /* die Berechnung muß in diesem Fall nicht gemacht werden */
         if(strcmp(kto,"0001300000")>=0 && strcmp(kto,"0099400000")<0)
            return OK_NO_CHK;

            /* Methode A */
      CASE_U(96,1)
         pz = (kto[0]-'0')
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

            /* Methode B */
      CASE_U(96,2)
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz =(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif

         MOD_10_80;   /* pz%=11 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

/*  Berechnung nach der Methode 97 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 97                        #
 * ######################################################################
 * # Modulus 11                                                         #
 * # Die 10. Stelle ist die Prüfziffer. Die Stellen 1 bis 9 (Wert X)    #
 * # sind durch 11 zu teilen. Das Ergebnis der Division ist ohne die    #
 * # Nachkommastellen mit 11 zu multiplizieren. Das Produkt ist vom     #
 * # Wert X zu subtrahieren. Ist das Ergebnis < 10, so entspricht       #
 * # das Ergebnis der Prüfziffer. Ist das Ergebnis = 10, so ist die     #
 * # Prüfziffer = 0.                                                    #
 * ######################################################################
 */
      CASE0(97)
         p1= *(kto+9);
         *(kto+9)=0;    /* Prüfziffer (temporär) löschen */
         pz=atoi(kto)%11;
         if(pz==10)pz=0;
         *(kto+9)=p1;   /* Prüfziffer wiederherstellen */
         CHECK_PZ10;

/*  Berechnung nach der Methode 98 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 98                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 3, 1, 7, 3, 1, 7, 3                         #
 * # Die Kontonummer ist 10-stellig. Die Berechnung erfolgt wie         #
 * # bei Verfahren 01. Es ist jedoch zu beachten, dass nur die          #
 * # Stellen 3 bis 9 in die Prüfzifferberechnung einbezogen             #
 * # werden. Die Stelle 10 der Kontonummer ist die Prüfziffer.          #
 * # Führt die Berechnung zu einem falschen Ergebnis, so ist            #
 * # alternativ das Verfahren 32 anzuwenden.                            #
 * ######################################################################
 */
      CASE0(98)
      CASE_U(98,1)
         pz = (kto[2]-'0') * 3
            + (kto[3]-'0') * 7
            + (kto[4]-'0')
            + (kto[5]-'0') * 3
            + (kto[6]-'0') * 7
            + (kto[7]-'0')
            + (kto[8]-'0') * 3;

         MOD_10_160;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

            /* alternativ: Verfahren 32 */
      CASE_U(98,2)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode 99 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode 99                        #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 2, 3, 4                   #
 * # Die Berechnung erfolgt wie bei Verfahren 06.                       #
 * # Ausnahmen: Kontonr.: 0396000000 bis 0499999999                     #
 * # Für diese Kontonummern ist keine Prüfzifferberechnung              #
 * # möglich.  Sie sind als richtig anzusehen.                          #
 * ######################################################################
 */
      CASE0(99)
         if(strcmp(kto,"0396000000")>=0 && strcmp(kto,"0500000000")<0){
            pz= *(kto+9)-'0';
            return OK;
         }
         pz = (kto[0]-'0') * 4
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 2
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/* Berechnungsmethoden A0 bis A9 +§§§3
   Berechnung nach der Methode A0 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode A0                       #
 * ######################################################################
 * #  Modulus 11, Gewichtung 2, 4, 8, 5, 10, 0, 0, 0, 0 Die             #
 * #  Kontonummer ist einschließlich der Prüfziffer 10- stellig,        #
 * #  ggf. ist die Kontonummer für die Prüfzifferberechnung durch       #
 * #  linksbündige Auffüllung mit Nullen 10-stellig darzustellen.       #
 * #  Die Stelle 10 ist die Prüfziffer. Die einzelnen Stellen der       #
 * #  Kontonummer (ohne Prüfziffer) sind von rechts nach links mit      #
 * #  dem zugehörigen Gewicht (2, 4, 8, 5, 10, 0, 0, 0, 0) zu           #
 * #  multiplizieren. Die Produkte werden addiert. Das Ergebnis ist     #
 * #  durch 11 zu dividieren. Ergibt sich nach der Division ein         #
 * #  Rest von 0 oder 1, so ist die Prüfziffer 0. Ansonsten ist der     #
 * #  Rest vom Divisor (11) zu subtrahieren. Das Ergebnis ist die       #
 * #  Prüfziffer.                                                       #
 * #  Ausnahme: 3-stellige Kontonummern bzw. Kontonummern, deren        #
 * #  Stellen 1 bis 7 = 0 sind, enthalten keine Prüfziffer und sind     #
 * #  als richtig anzusehen.                                            #
 * ######################################################################
 */
      CASE1(100)
         if(!strncmp(kto,"0000000",7))return OK_NO_CHK;
         pz = (kto[4]-'0') * 10
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 8
            + (kto[7]-'0') * 4
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode A1 +§§§4 */
/*
 * ######################################################################
 * #         Berechnung nach der Methode A1 (neu, ab 9. Juni 2003)     #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 0, 0. Die Kontonummern #
 * # sind 8- oder 10-stellig. Kontonummern (ohne führende Nullen        #
 * # gezählt) mit 9 oder weniger als 8 Stellen sind falsch. 8-stellige  #
 * # Kontonummern sind für die Prüfzifferberechnung durch linksbündige  #
 * # Auffüllung mit Nullen 10-stellig darzustellen. Die Berechnung      #
 * # erfolgt wie beim Verfahren 00.                                     #
 * ######################################################################
 */
      CASE1(101)
         if(*kto=='0' && *(kto+1)!='0'
               || *kto=='0' && *(kto+1)=='0' && *(kto+2)=='0')
            return INVALID_KTO;
#ifdef __ALPHA
         pz = ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz =(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif

         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode A2 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode A2                       #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2                   #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 2, 3, 4                   #
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * #                                                                    #
 * # Variante 1: Gewichtung und Berechnung erfolgen nach der Methode    #
 * # 00. Führt die Berechnung nach Variante 1 zu einem                  #
 * # Prüfzifferfehler, so ist nach Variante 2 zu prüfen.                #
 * #                                                                    #
 * # Testkontonummern (richtig): 3456789019, 5678901231, 6789012348     #
 * # Testkontonummer (falsch): 3456789012, 1234567890                   #
 * #                                                                    #
 * # Variante 2: Gewichtung und Berechnung erfolgen nach der Methode    #
 * # 04.                                                                #
 * # Testkontonummer (richtig): 3456789012                              #
 * # Testkontonummern (falsch): 1234567890, 0123456789                  #
 * ######################################################################
 */
      CASE1(102)
      CASE_U1(102,1)

            /* Variante 1: Berechnung nach Methode 00 */
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz =(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif

         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

            /* Variante 2: Berechnung nach Methode 04 */
      CASE_U1(102,2)
         pz = (kto[0]-'0') * 4
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 2
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz)pz=11-pz;
         if(pz==10)return INVALID_KTO;
         CHECK_PZ10;

/*  Berechnung nach der Methode A3 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode A3                       #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2                   #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 10                  #
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * #                                                                    #
 * # Variante 1: Gewichtung und Berechnung erfolgen nach der Methode    #
 * # 00. Führt die Berechnung nach Variante 1 zu einem                  #
 * # Prüfzifferfehler, so ist nach Variante 2 zu prüfen.                #
 * #                                                                    #
 * # Testkontonummern (richtig): 1234567897, 0123456782                 #
 * # Testkontonummern (falsch): 9876543210, 1234567890,                 #
 * #                            6543217890, 0543216789                  #
 * #                                                                    #
 * # Variante 2: Gewichtung und Berechnung erfolgen nach der Methode    #
 * # 10.                                                                #
 * #                                                                    #
 * # Testkontonummern (richtig): 9876543210, 1234567890, 0123456789     #
 * # Testkontonummern (falsch): 6543217890, 0543216789                  #
 * ######################################################################
 */
      CASE1(103)

            /* Variante 1: Berechnung nach Methode 00 */
      CASE_U1(103,1)
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz =(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif

         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

            /* Variante 2: Berechnung nach Methode 10 */
      CASE_U1(103,2)
         pz = (kto[0]-'0') * 10
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode A4 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode A4                       #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 0, 0, 0                   #
 * # Modulus 7,  Gewichtung 2, 3, 4, 5, 6, 7, 0, 0, 0                   #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 0, 0, 0, 0                   #
 * # Modulus 7,  Gewichtung 2, 3, 4, 5, 6, 0, 0, 0, 0                   #
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen. Zur    #
 * # Prüfung einer Kontonummer sind die folgenden Varianten zu          #
 * # rechnen. Dabei ist zu beachten, dass Kontonummern mit der          #
 * # Ziffernfolge 99 an den Stellen 3 und 4 (XX99XXXXXX) nur nach       #
 * # Variante 3 und ggf. 4 zu prüfen sind. Alle anderen Kontonummern    #
 * # sind nacheinander nach den Varianten 1, ggf. 2 und ggf. 4 zu       #
 * # prüfen.                                                            #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 0, 0, 0                   #
 * #                                                                    #
 * # In die Prüfzifferberechnung werden nur die Stellen 4 bis 9         #
 * # einbezogen. Die Stelle 10 ist die Prüfziffer. Die weitere          #
 * # Berechnung erfolgt nach dem Verfahren 06.                          #
 * #                                                                    #
 * # Führt die Berechnung zu einem Fehler, ist nach Variante 2 zu       #
 * # prüfen.                                                            #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 7, Gewichtung 2, 3, 4, 5, 6, 7, 0, 0, 0                    #
 * #                                                                    #
 * # Die Stellen 4 bis 9 der Kontonummer werden von rechts nach links   #
 * # mit den Gewichten multipliziert. Die jeweiligen Produkte werden    #
 * # addiert. Die Summe ist durch 7 zu dividieren. Der verbleibende     #
 * # Rest wird vom Divisor (7) subtrahiert. Das Ergebnis ist die        #
 * # Prüfziffer (Stelle 10). Verbleibt nach der Division kein Rest,     #
 * # ist die Prüfziffer 0.                                              #
 * #                                                                    #
 * # Führt die Berechnung zu einem Fehler, ist nach Variante 4 zu       #
 * # prüfen.                                                            #
 * #                                                                    #
 * # Variante 3:                                                        #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 0, 0, 0, 0                   #
 * #                                                                    #
 * # In die Prüfzifferberechnung werden nur die Stellen 5 bis 9         #
 * # einbezogen. Die Stelle 10 ist die Prüfziffer. Die weitere          #
 * # Berechnung erfolgt nach dem Verfahren 06.                          #
 * #                                                                    #
 * # Führt die Berechnung zu einem Fehler, ist nach Variante 4 zu       #
 * # prüfen.                                                            #
 * #                                                                    #
 * # Variante 4:                                                        #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 0, 0, 0, 0                   #
 * # Modulus 7,  Gewichtung 2, 3, 4, 5, 6, 0, 0, 0, 0                   #
 * # Die Berechnung erfolgt nach der Methode 93.                        #
 * ######################################################################
 */

      CASE1(104)
         if(*(kto+2)!='9' || *(kto+3)!='9'){

                /* Variante 1 */
      CASE_U1(104,1)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZX10;

                /* Variante 2 */
      CASE_U1(104,2)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_7_224;   /* pz%=7 */
            if(pz)pz=7-pz;
            CHECK_PZX10;
         }
         else{ /* 3. und 4. Stelle sind 9 */

                /* Variante 3 */
      CASE_U1(104,3)
         pz = (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZX10;
         }

            /* Variante 4: Methode 93, Variante 1 */
      CASE_U1(104,4)
         if(*kto=='0' && *(kto+1)=='0' && *(kto+2)=='0' && *(kto+3)=='0'){   /* Fall b) */
            pz = (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            p1=pz;
            MOD_11_176;   /* pz%=11 */
         }
         else{
            pz = (kto[0]-'0') * 6
               + (kto[1]-'0') * 5
               + (kto[2]-'0') * 4
               + (kto[3]-'0') * 3
               + (kto[4]-'0') * 2;

            kto[9]=kto[5];  /* Prüfziffer nach Stelle 10 */
            p1=pz;
            MOD_11_176;   /* pz%=11 */
         }
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

            /* Methode 93, Variante 2 */
#if DEBUG
      CASE_U1(104,5)
         if(untermethode){ /* pz wurde noch nicht berechnet */
            if(*kto=='0' && *(kto+1)=='0' && *(kto+2)=='0' && *(kto+3)=='0'){   /* Fall b) */
               for(p1=0,ptr=kto+8,i=0;i<5;ptr--,i++)
                  p1+=(*ptr-'0')*w93[i];
            }
            else{
               for(p1=0,ptr=kto+4,i=0;i<5;ptr--,i++)
                  p1+=(*ptr-'0')*w93[i];
                  *(kto+9)= *(kto+5);  /* Prüfziffer nach Stelle 10 */
            }
         }
#endif
         pz=p1%7;
         if(pz)pz=7-pz;
         CHECK_PZ10;
         break;

/*  Berechnung nach der Methode A5 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode A5                       #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2                   #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 10                  #
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * #                                                                    #
 * # Variante 1: Gewichtung und Berechnung erfolgen nach der Methode    #
 * # 00. Führt die Berechnung nach Variante 1 zu einem                  #
 * # Prüfzifferfehler, so sind 10-stellige Konten mit einer 9 an        #
 * # Stelle 1 falsch, alle anderen Konten sind nach Variante 2 zu       #
 * # prüfen.                                                            #
 * #                                                                    #
 * # Variante 2: Gewichtung und Berechnung erfolgen nach der Methode 10.#
 * ######################################################################
 */
      CASE1(105)
      CASE_U1(105,1)
            /* Variante 1: Berechnung nach Methode 00 */
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz =(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif

         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;
         if(*kto=='9')return INVALID_KTO;

            /* Variante 2: Berechnung nach Methode 10 */
      CASE_U1(105,2)
         pz = (kto[0]-'0') * 10
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode A6 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode A6                       #
 * ######################################################################
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2                   #
 * # Modulus 10, Gewichtung 3, 7, 1, 3, 7, 1, 3, 7, 1                   #
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10- stellig,     #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen. Die    #
 * # Stelle 10 ist die Prüfziffer.                                      #
 * #                                                                    #
 * # Sofern dann an der zweiten Stelle der Kontonummer eine 8 steht,    #
 * # erfolgen Gewichtung und Berechnung wie beim Verfahren 00.          #
 * #                                                                    #
 * # Bei allen Kontonummern, die keine 8 an der zweiten Stelle          #
 * # haben, erfolgen Gewichtung und Berechnung wie beim Verfahren 01.   #
 * ######################################################################
 */

      CASE1(106)
         if(kto[1]=='8'){

                /* Variante 1 */
      CASE_U1(106,1)
#ifdef __ALPHA
            pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
               +  8
               + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
               +  (kto[3]-'0')
               + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
               +  (kto[7]-'0')
               + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
            pz=8+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
            if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
            if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
            if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
            MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }
         else{

                /* Variante 2 */
      CASE_U1(106,2)
            pz = (kto[0]-'0')
               + (kto[1]-'0') * 7
               + (kto[2]-'0') * 3
               + (kto[3]-'0')
               + (kto[4]-'0') * 7
               + (kto[5]-'0') * 3
               + (kto[6]-'0')
               + (kto[7]-'0') * 7
               + (kto[8]-'0') * 3;

            MOD_10_160;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }

/*  Berechnung nach der Methode A7 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode A7                       #
 * ######################################################################
 * #  Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2                  #
 * #                                                                    #
 * #  Die Kontonummer ist einschließlich der Prüfziffer 10- stellig,    #
 * #  ggf. ist die Kontonummer für die Prüfzifferberechnung durch       #
 * #  linksbündige Auffüllung mit Nullen 10-stellig darzustellen.       #
 * #                                                                    #
 * #  Variante 1:                                                       #
 * #  Gewichtung und Berechnung erfolgen nach der Methode 00. Führt die #
 * #  Berechnung nach Variante 1 zu einem Prüfzifferfehler, ist nach    #
 * #  Variante 2 zu prüfen.                                             #
 * #                                                                    #
 * #  Variante 2:                                                       #
 * #  Gewichtung und Berechnung erfolgen nach der Methode 03.           #
 * ######################################################################
 */

      CASE1(107)

                /* Variante 1 */
      CASE_U1(107,1)
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

                /* Variante 2 */
      CASE_U1(107,2)
         pz = (kto[0]-'0') * 2
            + (kto[1]-'0')
            + (kto[2]-'0') * 2
            + (kto[3]-'0')
            + (kto[4]-'0') * 2
            + (kto[5]-'0')
            + (kto[6]-'0') * 2
            + (kto[7]-'0')
            + (kto[8]-'0') * 2;

         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;


/*  Berechnung nach der Methode A8 (alt) +§§§4 */
/*
 * ######################################################################
 * #   Berechnung nach der Methode A8 alt (nur für Vergleichszwecke)    #
 * ######################################################################
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7                            #
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1                            #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Gewichtung und Berechnung erfolgen nach der Methode 81. Führt die  #
 * # Berechnung nach Variante 1 zu einem Prüfzifferfehler, so sind      #
 * # Konten mit einer 9 an Stelle 3 (in der 10-stelligen Darstellung)   #
 * # falsch, alle anderen Konten sind nach Variante 2 zu prüfen.        #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Gewichtung und Berechnung erfolgen nach der Methode 73. (Hinweis:  #
 * # Die Ausnahmeregelung der Methode 73 für Kontonummern mit einer 9   #
 * # an Stelle 3 wird nicht wirksam, da diese Kontonummern nur nach     #
 * # Variante 1 geprüft werden.)                                        #
 * #                                                                    #
 * ######################################################################
       Berechnung nach der Methode A8 (neu)  +§§§4
 * #   Berechnung nach der Methode A8 (geändert zum 7.3.05)             #
 * ######################################################################
 * # Die Kontonummer ist durch linksbündige Nullenauffüllung 10-        #
 * # stellig darzustellen. Die 10. Stelle ist per Definition die        #
 * # Prüfziffer.                                                        #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7                            #
 * # Die Stellen 4 bis 9 der Kontonummer werden von rechts nach links   #
 * # mit den Ziffern 2, 3, 4, 5, 6, 7 multipliziert. Die weitere        #
 * # Berechnung und die möglichen Ergebnisse entsprechen dem Verfahren  #
 * # 06. Führt die Berechnung nach Variante 1 zu einem Prüfziffer-      #
 * # fehler, so sind die Konten nach Variante 2 zu prüfen.              #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1                            #
 * # Die Stellen 4 bis 9 der Kontonummer werden von rechts nach links   #
 * # mit den Ziffern 2, 1, 2, 1, 2, 1 multipliziert. Die weiter         #
 * # Berechnung und die möglichen Ergebnisse entsprechen dem Verfahren  #
 * # 00.                                                                #
 * #                                                                    #
 * # Ausnahme:                                                          #
 * # Ist nach linksbündiger Auffüllung mit Nullen auf 10 Stellen die    #
 * # 3. Stelle der Kontonummer = 9 (Sachkonten), so erfolgt die         #
 * # Berechnung gemäß der Ausnahme in Methode 51 mit den gleichen       #
 * # Ergebnissen und Testkontonummern.                                  #
 * ######################################################################
 */

      CASE1(108)

                /* Ausnahme, Variante 1 */
         if(*(kto+2)=='9'){   /* Berechnung wie in Verfahren 51 */
      CASE_U1(108,3)
            pz =         9 * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZX10;

            /* Ausnahme, Variante 2 */
      CASE_U1(108,4)
               pz = (kto[0]-'0') * 10
               + (kto[1]-'0') * 9
               + (kto[2]-'0') * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

               MOD_11_352;   /* pz%=11 */
               if(pz<=1)
                  pz=0;
               else
                  pz=11-pz;
               CHECK_PZ10;
            }

      CASE_U1(108,1)
         pz = (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

                /* Variante 2 */
      CASE_U1(108,2)
#ifdef __ALPHA
         pz =  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else

         pz=(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode A9 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode A9                       #
 * ######################################################################
 * # Modulus 10, Gewichtung 3, 7, 1, 3, 7, 1, 3, 7, 1                   #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 2, 3, 4                   #
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Gewichtung und Berechnung erfolgen nach der Methode 01. Führt die  #
 * # Berechnung nach Variante 1 zu einem Prüfzifferfehler, so ist nach  #
 * # Variante 2 zu prüfen.                                              #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Gewichtung und Berechnung erfolgen nach der Methode 06.            #
 * ######################################################################
 */

      CASE1(109)

            /* Variante 1: Berechnung nach Methode 01 */
      CASE_U1(109,1)
         pz = (kto[0]-'0')
            + (kto[1]-'0') * 7
            + (kto[2]-'0') * 3
            + (kto[3]-'0')
            + (kto[4]-'0') * 7
            + (kto[5]-'0') * 3
            + (kto[6]-'0')
            + (kto[7]-'0') * 7
            + (kto[8]-'0') * 3;

         MOD_10_160;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

            /* Variante 2: Berechnung nach Methode 06 */
      CASE_U1(109,2)
         pz = (kto[0]-'0') * 4
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 2
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/* Berechnungsmethoden B0 bis B9 +§§§3
   Berechnung nach der Methode B0 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode B0                       #
 * ######################################################################
 * #                                                                    #
 * # Die Kontonummern sind immer 10-stellig. Kontonummern (ohne         #
 * # führende Nullen gezählt) mit 9 oder weniger Stellen sind falsch.   #
 * # Kontonummern mit 8 an der ersten Stelle sind ebenfalls falsch.     #
 * # Die weitere Verfahrensweise richtet sich nach der 8. Stelle der    #
 * # Kontonummer:                                                       #
 * #                                                                    #
 * # Variante 1                                                         #
 * #                                                                    #
 * # Für Kontonummern mit einer 1, 2, 3, oder 6 an der 8. Stelle gilt   #
 * # das Verfahren 09 (Keine Prüfzifferberechnung, alle Kontonummern    #
 * # sind richtig).                                                     #
 * #                                                                    #
 * # Variante 2                                                         #
 * #                                                                    #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 2, 3, 4                   #
 * #             (von rechts beginnend)                                 #
 * #                                                                    #
 * # Für Kontonummern mit einer 0, 4, 5, 7, 8 oder 9 an der 8. Stelle   #
 * # erfolgen Gewichtung und Berechnung wie beim Verfahren 06.          #
 * ######################################################################
 */

      CASE1(110)
         if(kto[0]=='0' || kto[0]=='8')return INVALID_KTO;

            /* Variante 1 */
         if(kto[7]=='1' || kto[7]=='2' || kto[7]=='3' || kto[7]=='6'){
      CASE_U1(110,1)
            return OK_NO_CHK;
         }

            /* Variante 2 */
      CASE_U1(110,2)
         pz = (kto[0]-'0') * 4
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 2
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_176;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode B1 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode B1                       #
 * ######################################################################
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 10, Gewichtung 7, 3, 1, 7, 3, 1, 7, 3, 1                   #
 * # Gewichtung und Berechnung erfolgen nach der Methode 05. Führt die  #
 * # Berechnung nach Variante 1 zu einem Prüfzifferfehler, so ist nach  #
 * # Variante 2 zu prüfen.                                              #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 10, Gewichtung 3, 7, 1, 3, 7, 1, 3, 7, 1                   #
 * # Gewichtung und Berechnung erfolgen nach der Methode 01.            #
 * ######################################################################
 */

      CASE1(111)

            /* Variante 1: Berechnung nach Methode 05 */
      CASE_U1(111,1)
         pz = (kto[0]-'0')
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 7
            + (kto[3]-'0')
            + (kto[4]-'0') * 3
            + (kto[5]-'0') * 7
            + (kto[6]-'0')
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 7;

         MOD_10_160;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

            /* Variante 2: Berechnung nach Methode 01 */
      CASE_U1(111,2)
         pz = (kto[0]-'0')
            + (kto[1]-'0') * 7
            + (kto[2]-'0') * 3
            + (kto[3]-'0')
            + (kto[4]-'0') * 7
            + (kto[5]-'0') * 3
            + (kto[6]-'0')
            + (kto[7]-'0') * 7
            + (kto[8]-'0') * 3;

         MOD_10_160;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode B2 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode B2                        #
 * ######################################################################
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 2                   #
 * # Kontonummern, die an der 1. Stelle von links der 10- stelligen     #
 * # Kontonummer den Wert 0 bis 7 beinhalten, sind nach der Methode 02  #
 * # zu rechnen.                                                        #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2                   #
 * # Kontonummern, die an der 1. Stelle von links der 10- stelligen     #
 * # Kontonummer den Wert 8 oder 9 beinhalten, sind nach der Methode    #
 * # 00 zu rechnen.                                                     #
 * ######################################################################
 */

      CASE1(112)

            /* Variante 1: Methode 02 */
         if(*kto<'8'){
      CASE_U1(112,1)
            pz = (kto[0]-'0') * 2
               + (kto[1]-'0') * 9
               + (kto[2]-'0') * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            MOD_11_352;   /* pz%=11 */
            if(pz)pz=11-pz;
            if(pz==10)return INVALID_KTO;
            CHECK_PZ10;
         }
         else{
      CASE_U1(112,2)
               /* Variante 2: Methode 00 */
#ifdef __ALPHA
            pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
               +  (kto[1]-'0')
               + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
               +  (kto[3]-'0')
               + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
               +  (kto[7]-'0')
               + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
            pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
            if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
            if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
            if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
            MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }

/*  Berechnung nach der Methode B3 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode B3                        #
 * ######################################################################
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig, ggf. #
 * # ist die Kontonummer für die Prüfzifferberechnung durch             #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * #                                                                    #
 * # Variante 1:                                                        #
 * #                                                                    #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7                            #
 * #                                                                    #
 * # Die Kontonummer ist 10-stellig. Kontonummern, die an der 1. Stelle #
 * # von links der 10-stelligen Kontonummer den Wert bis 8 beinhalten   #
 * # sind nach der Methode 32 zu rechen.                                #
 * #                                                                    #
 * #                                                                    #
 * # Variante 2:                                                        #
 * #                                                                    #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 2, 3, 4                   #
 * #                                                                    #
 * # Kontonummern, die an der 1. Stelle von links der 10- stelligen     #
 * # Kontonummer den Wert 9 beinhalten sind nach der Methode 06 zu      #
 * # rechen.                                                            #
 * ######################################################################
 */

      CASE1(113)

            /* Variante 1: Methode 32 */
         if(*kto<'9'){
            CASE_U1(113,1)
            pz = (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZ10;
         }
         else{

            /* Variante 2: Methode 06 */
            CASE_U1(113,2)
            pz = (kto[0]-'0') * 4
               + (kto[1]-'0') * 3
               + (kto[2]-'0') * 2
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZ10;
         }

/*  Berechnung nach der Methode B4 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode B4                        #
 * ######################################################################
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * #                                                                    #
 * # Variante 1:                                                        #
 * #                                                                    #
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2                   #
 * # Kontonummern, die an der 1. Stelle von links der 10-stelligen      #
 * # Kontonummer den Wert 9 beinhalten, sind nach der Methode 00 zu     #
 * # rechnen.                                                           #
 * #                                                                    #
 * # Variante 2:                                                        #
 * #                                                                    #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 10                  #
 * # Kontonummern, die an der 1. Stelle von links der 10-stelligen      #
 * # Kontonummer den Wert 0 bis 8 beinhalten, sind nach der Methode     #
 * # 02 zu rechnen.                                                     #
 * ######################################################################
 */

      CASE1(114)

         if(*kto=='9'){
            /* Variante 1: Methode 00 */
            CASE_U1(114,1)

               /* Alpha: etwas andere Berechnung, wesentlich schneller
                * (benötigt nur 75 statt 123 Takte, falls Berechnung wie Intel.
                * Die Intel-Methode wäre somit sogar langsamer als die alte Version
                * mit 105 Takten)
                */
#ifdef __ALPHA
            pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
               +  (kto[1]-'0')
               + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
               +  (kto[3]-'0')
               + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
               +  (kto[7]-'0')
               + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
            pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
            if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
            if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
            if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
            MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }
         else{

            /* Variante 2: Methode 02 */
            CASE_U1(114,2)
            pz = (kto[0]-'0') * 10
               + (kto[1]-'0') * 9
               + (kto[2]-'0') * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            MOD_11_352;   /* pz%=11 */
            if(pz)pz=11-pz;
            if(pz==10)return INVALID_KTO;
            CHECK_PZ10;
         }

/*  Berechnung nach der Methode B5 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode B5                        #
 * ######################################################################
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 10, Gewichtung 7, 3, 1 ,7, 3, 1, 7, 3, 1                   #
 * # Die Gewichtung entspricht der Methode 05. Die Berechnung           #
 * # entspricht der Methode 01. Führt die Berechnung nach der Variante  #
 * # 1 zu einem Prüfzifferfehler, so sind Kontonummern, die an der 1.   #
 * # Stelle von links der 10-stelligen Kontonummer den Wert 8 oder 9    #
 * # beinhalten, falsch. Alle anderen Kontonummern sind nach der        #
 * # Variante 2 zu prüfen.                                              #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2                   #
 * # Gewichtung und Berechnung erfolgen nach der Methode  00.           #
 * ######################################################################
 */

      CASE1(115)
      CASE_U1(115,1)
         pz = (kto[0]-'0')
            + (kto[1]-'0') * 3
            + (kto[2]-'0') * 7
            + (kto[3]-'0')
            + (kto[4]-'0') * 3
            + (kto[5]-'0') * 7
            + (kto[6]-'0')
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 7;

         MOD_10_160;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;
         if(*kto>'7')return FALSE;

      CASE_U1(115,2)
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;


/*  Berechnung nach der Methode B6 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode B6                        #
 * ######################################################################
 * # Variante 1:                                                        #
 * # Modulus 11, Gewichtung 2,3,4,5,6,7,8,9,3                           #
 * # Kontonummern, die an der 1. Stelle der 10-stelligen Kontonummer    #
 * # den Wert 1-9 beinhalten, sind nach der Methode 20 zu prüfen.       #
 * # Alle anderen Kontonummern sind nach der Variante 2 zu prüfen.      #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 11, Gewichtung 2,4,8,5,10,9,7,3,6,1,2,4                    #
 * # Die Berechnung erfolgt nach der Methode 53.                        #
 * ######################################################################
 */

      CASE1(116)
         if(kto[0]>'0'){
      CASE_U1(116,1)
            pz = (kto[0]-'0') * 3
               + (kto[1]-'0') * 9
               + (kto[2]-'0') * 8
               + (kto[3]-'0') * 7
               + (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            MOD_11_352;   /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZ10;
         }
         else{
      CASE_U1(116,2)
            if(strlen(pz_or_blz)!=8){
               fprintf(stderr,"Warnung: für Methode B6b wird die BLZ benötigt!!; benutze zum Test 80053762\n");
               pz_or_blz="80053762";
            }

               /* Generieren der Konto-Nr. des ESER-Altsystems */
            for(ptr=kto;*ptr=='0';ptr++);
            if(*kto!='0' || *(kto+1)=='0'){  /* Kto-Nr. muß neunstellig sein */
               pz= -2;
               return INVALID_KTO;
            }
            kto_alt[0]=pz_or_blz[4];
            kto_alt[1]=pz_or_blz[5];
            kto_alt[2]=kto[2];         /* T-Ziffer */
            kto_alt[3]=pz_or_blz[7];
            kto_alt[4]=kto[1];
            kto_alt[5]=kto[3];
            for(ptr=kto+4;*ptr=='0' && *ptr;ptr++);
            for(dptr=kto_alt+6;*dptr= *ptr++;dptr++);
            kto=kto_alt;
            p1=kto_alt[5];   /* Prüfziffer merken */
            kto_alt[5]='0';
            for(pz=0,ptr=kto_alt+strlen(kto_alt)-1,i=0;ptr>=kto_alt;ptr--,i++)
               pz+=(*ptr-'0')*w52[i];
            kto_alt[5]=p1;   /* Prüfziffer zurückschreiben */
            pz=pz%11;
            p1=w52[i-6];

               /* passenden Faktor suchen */
            tmp=pz;
            for(i=0;i<10;i++){
               pz=tmp+p1*i;
               MOD_11_88;
               if(pz==10)break;
            }
            pz=i;
            if(i==10)return INVALID_KTO;
            CHECK_PZ6;
         }

/*  Berechnung nach der Methode B7 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode B7                        #
 * ######################################################################
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggff. ist die Kontonummer für die Prüfzifferberechnung durch       #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen. Die    #
 * # 10. Stelle der Kontonummer ist die Prüfziffer.                     #
 * #                                                                    #
 * # Variante 1: Modulus 10, Gewichtung 3, 7, 1, 3, 7, 1, 3, 7, 1       #
 * # Kontonummern der Kontenkreise 0001000000 bis 0005999999 sowie      #
 * # 0700000000 bis 0899999999 sind nach der Methode (Kennziffer) 01    #
 * # zu prüfen. Führt die Berechnung nach der Variante 1 zu einem       #
 * # Prüfzifferfehler, so ist die Kontonummer falsch.                   #
 * #                                                                    #
 * # Variante 2: Für alle anderen Kontonummern gilt die Methode 09      #
 * # (keine Prüfzifferberechnung).                                      #
 * ######################################################################
 */

      CASE1(117)
      CASE_U1(117,1)
         if(kto[0]=='0' && ((kto[1]=='7' || kto[1]=='8')
                 || (kto[1]=='0' && kto[2]=='0' && kto[3]>='1' && kto[3]<'6'))){
            pz = (kto[0]-'0')
               + (kto[1]-'0') * 7
               + (kto[2]-'0') * 3
               + (kto[3]-'0')
               + (kto[4]-'0') * 7
               + (kto[5]-'0') * 3
               + (kto[6]-'0')
               + (kto[7]-'0') * 7
               + (kto[8]-'0') * 3;

            MOD_10_160;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }
         else{
      CASE_U1(117,2)
            return OK_NO_CHK;
         }

/*  Berechnung nach der Methode B8 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode B8                        #
 * ######################################################################
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen. Die    #
 * # 10. Stelle der Kontonummer ist die Prüfziffer.                     #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 3 (modifiziert)     #
 * #                                                                    #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der Methode     #
 * # 20. Führt die Berechnung nach Variante 1 zu einem                  #
 * # Prüfzifferfehler, so ist nach Variante 2 zu prüfen.                #
 * #                                                                    #
 * # Variante 2: Modulus 10, iterierte Transformation.                  #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der Methode 29. #
 * ######################################################################
 */

      CASE1(118)
      CASE_U1(118,1)
         pz = (kto[0]-'0') * 3
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZX10;

      CASE_U1(118,2)
         pz = m10h_digits[0][(unsigned int)(kto[0]-'0')]
            + m10h_digits[3][(unsigned int)(kto[1]-'0')]
            + m10h_digits[2][(unsigned int)(kto[2]-'0')]
            + m10h_digits[1][(unsigned int)(kto[3]-'0')]
            + m10h_digits[0][(unsigned int)(kto[4]-'0')]
            + m10h_digits[3][(unsigned int)(kto[5]-'0')]
            + m10h_digits[2][(unsigned int)(kto[6]-'0')]
            + m10h_digits[1][(unsigned int)(kto[7]-'0')]
            + m10h_digits[0][(unsigned int)(kto[8]-'0')];
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/*  Berechnung nach der Methode B9 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode B9                        #
 * ######################################################################
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * #                                                                    #
 * # Kontonummern mit weniger als zwei oder mehr als drei führenden     #
 * # Nullen sind falsch. Die Kontonummern mit zwei führenden Nullen     #
 * # sind nach Variante 1, mit drei führenden Nullen nach Variante 2    #
 * # zu prüfen.                                                         #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus (11,10), Gewichtung 1, 3, 2, 1, 3, 2, 1                    #
 * # Die für die Berechnung relevanten Stellen der Kontonummer befinden #
 * # sich - von links nach rechts gelesen ­ in den Stellen 3-9 (die     #
 * # Prüfziffer ist in Stelle 10). Sie sind von rechts nach links       #
 * # mit den zugehörigen Gewichtungsfaktoren zu multiplizieren.         #
 * #                                                                    #
 * # Zum jeweiligen Produkt ist der zugehörige Gewichtungsfaktor zu     #
 * # addieren. Das jeweilige Ergebnis ist durch 11 zu dividieren. Die   #
 * # sich aus der Division ergebenden Reste sind zu summieren. Diese    #
 * # Summe ist durch 10 zu dividieren. Der Rest ist die berechnete      #
 * # Prüfziffer.                                                        #
 * #                                                                    #
 * # Führt die Berechnung zu einem Prüfzifferfehler, so ist die         #
 * # berechnete Prüfziffer um 5 zu erhöhen und erneut zu prüfen. Ist    #
 * # die Prüfziffer größer oder gleich 10, ist 10 abzuziehen und das    #
 * # Ergebnis ist dann die Prüfziffer.                                  #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 11, Gewichtung 1, 2, 3, 4, 5, 6                            #
 * # Die für die Berechnung relevanten Stellen der Kontonummer          #
 * # befinden sich - von links nach rechts gelesen - in den Stellen     #
 * # 4-9 (die Prüfziffer ist in Stelle 10). Sie sind von rechts nach    #
 * # links mit den zugehörigen Gewichtungsfaktoren zu multiplizieren.   #
 * # Die Summe dieser Produkte ist zu bilden, und das erzielte          #
 * # Ergebnis ist durch 11 zu dividieren. Der Rest ist die berechnete   #
 * # Prüfziffer.                                                        #
 * #                                                                    #
 * # Führt die Berechnung zu einem Prüfzifferfehler, so ist die         #
 * # berechnete Prüfziffer um 5 zu erhöhen und erneut zu prüfen. Ist    #
 * # die Prüfziffer größer oder gleich 10, ist 10 abzuziehen und das    #
 * # Ergebnis ist dann die Prüfziffer.                                  #
 * ######################################################################
 */

      CASE1(119)
         if(*kto!='0' || kto[1]!='0')return INVALID_KTO;
         if(*kto=='0' && kto[1]=='0' && kto[2]=='0' && kto[3]=='0')return INVALID_KTO;
         if(kto[3]!='0'){
      CASE_U1(119,1)
            pz  = (kto[2]-'0') * 1 + 1;   /* Maximum von pz1 ist 9*1+1=10 -> kann direkt genommen werden */

            pz1 = (kto[3]-'0') * 2 + 2;   /* Maximum von pz1 ist 9*2+2=20 -> nur ein Test auf >=11 */
            if(pz1>=11)
               pz+=pz1-11;
            else
               pz+=pz1;

            pz1 = (kto[4]-'0') * 3 + 3;   /* Maximum von pz1 ist 9*3+3=30 -> zwei Tests auf >=22 und >=11 nötig */
            if(pz1>=22)
               pz+=pz1-22;
            else if(pz1>=11)
               pz+=pz1-11;
            else
               pz+=pz1;

            pz += (kto[5]-'0') * 1 + 1;   /* Maximum von pz1 ist 9*1+1=10 -> kann direkt genommen werden */

            pz1 = (kto[6]-'0') * 2 + 2;   /* Maximum von pz1 ist 9*2+2=20 -> nur ein Test auf >=11 */
            if(pz1>=11)
               pz+=pz1-11;
            else
               pz+=pz1;

            pz1 = (kto[7]-'0') * 3 + 3;   /* Maximum von pz1 ist 9*3+3=30 -> zwei Tests auf >=22 und >=11 nötig */
            if(pz1>=22)
               pz+=pz1-22;
            else if(pz1>=11)
               pz+=pz1-11;
            else
               pz+=pz1;

            pz += (kto[8]-'0') * 1 + 1;   /* Maximum von pz1 ist 9*1+1=10 -> kann direkt genommen werden */

            MOD_10_80;   /* pz%=10 */
            if(kto[9]-'0'==pz)return OK;
            pz+=5;
            MOD_10_10;
            CHECK_PZ10;
         }
         else{
      CASE_U1(119,2)
            pz = (kto[3]-'0') * 6
               + (kto[4]-'0') * 5
               + (kto[5]-'0') * 4
               + (kto[6]-'0') * 3
               + (kto[7]-'0') * 2
               + (kto[8]-'0') * 1;

            MOD_11_176;   /* pz%=11 */

            if(kto[9]-'0'==pz)return OK;
            pz+=5;
            MOD_10_10;
            CHECK_PZ10;
         }



/* Berechnungsmethoden C0 bis C6 +§§§3
   Berechnung nach der Methode C0 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode C0                        #
 * ######################################################################
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * #                                                                    #
 * # Kontonummern mit zwei führenden Nullen sind nach Variante 1 zu     #
 * # prüfen. Führt die Berechnung nach der Variante 1 zu einem          #
 * # Prüfzifferfehler, ist die Berechnung nach Variante 2               #
 * # vorzunehmen.                                                       #
 * #                                                                    #
 * # Kontonummern mit weniger oder mehr als zwei führenden Nullen       #
 * # sind ausschließlich nach der Variante 2 zu                         #
 * # prüfen.                                                            #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 11, Gewichtung 2, 4, 8, 5, 10, 9, 7, 3, 6, 1, 2, 4         #
 * # Die Berechnung und mögliche Ergebnisse entsprechen                 #
 * # der Methode 52.                                                    #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 7, 8, 9, 3                   #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der Methode 20  #
 * ######################################################################
 */
      CASE1(120)
         if(*kto=='0' && kto[1]=='0'){
      CASE_U1(120,1)
            if(strlen(pz_or_blz)!=8){
               fprintf(stderr,"Warnung: für Methode C0a wird die BLZ benötigt!!; benutze zum Test 13051172\n");
               pz_or_blz="13051172";
            }

               /* Generieren der Konto-Nr. des ESER-Altsystems */
            for(ptr=kto;*ptr=='0';ptr++);
            if(ptr>kto+2)return INVALID_KTO;
            kto_alt[0]=pz_or_blz[4];
            kto_alt[1]=pz_or_blz[5];
            kto_alt[2]=pz_or_blz[6];
            kto_alt[3]=pz_or_blz[7];
            kto_alt[4]= *ptr++;
            kto_alt[5]= *ptr++;
            while(*ptr=='0' && *ptr)ptr++;
            for(dptr=kto_alt+6;*dptr= *ptr++;dptr++);
            p1=kto_alt[5];   /* Prüfziffer */
            kto_alt[5]='0';
            for(pz=0,ptr=dptr-1,i=0;ptr>=kto_alt;ptr--,i++)
               pz+=(*ptr-'0')*w52[i];
            kto_alt[5]=p1;
            pz=pz%11;
            p1=w52[i-6];

               /* passenden Faktor suchen */
            tmp=pz;
            for(i=0;i<10;i++){
               pz=tmp+p1*i;
               MOD_11_88;
               if(pz==10)break;
            }
            pz=i;
            if(i==10)return INVALID_KTO;
            if(*(kto_alt+5)-'0'==pz)return OK;
#if DEBUG
            if(untermethode)return FALSE;
#endif
         }
      CASE_U1(120,2)
         pz = (kto[0]-'0') * 3
            + (kto[1]-'0') * 9
            + (kto[2]-'0') * 8
            + (kto[3]-'0') * 7
            + (kto[4]-'0') * 6
            + (kto[5]-'0') * 5
            + (kto[6]-'0') * 4
            + (kto[7]-'0') * 3
            + (kto[8]-'0') * 2;

         MOD_11_352;   /* pz%=11 */
         if(pz<=1)
            pz=0;
         else
            pz=11-pz;
         CHECK_PZ10;

/* Berechnung nach der Methode C1 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode C1                        #
 * ######################################################################
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * #                                                                    #
 * # Kontonummern, die an der 1. Stelle der 10-stelligen                #
 * # Kontonummer einen Wert ungleich  5  beinhalten, sind nach der      #
 * # Variante 1 zu prüfen. Kontonummern, die an der 1. Stelle der       #
 * # 10-stelligen Kontonummer den Wert  5  beinhalten, sind nach        #
 * # der Variante 2 zu prüfen.                                          #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 11, Gewichtung 1, 2, 1, 2, 1, 2                            #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der             #
 * # Methode 17. Führt die Berechnung nach der Variante 1 zu einem      #
 * # Prüfzifferfehler, so ist die Kontonummer falsch.                   #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 11, Gewichtung 1, 2, 1, 2, 1, 2                            #
 * # Die Kontonummer ist 10-stellig mit folgendem Aufbau:               #
 * #                                                                    #
 * # KNNNNNNNNP                                                         #
 * # K = Kontoartziffer                                                 #
 * # N = laufende Nummer                                                #
 * # P = Prüfziffer                                                     #
 * #                                                                    #
 * # Für die Berechnung fließen die Stellen 1 bis 9 ein. Stelle 10      #
 * # ist die ermittelte Prüfziffer. Die Stellen 1 bis 9 sind von        #
 * # links nach rechts mit den Ziffern 1, 2, 1, 2, 1, 2, 1, 2, 1        #
 * # zu multiplizieren. Die jeweiligen Produkte sind zu addieren,       #
 * # nachdem aus eventuell zweistelligen Produkten der 2., 4., 6.       #
 * # und 8. Stelle die Quersumme gebildet wurde. Von der Summe ist      #
 * # der Wert  1  zu subtrahieren. Das Ergebnis ist dann durch 11       #
 * # zu dividieren. Der verbleibende Rest wird von 10 subtrahiert.      #
 * # Das Ergebnis ist die Prüfziffer. Verbleibt nach der Division       #
 * # durch 11 kein Rest, ist die Prüfziffer 0.                          #
 * #                                                                    #
 * # Beispiel:                                                          #
 * #                                                                    #
 * # Stellen-Nr.: K   N   N   N   N   N   N   N   N   P                 #
 * # Konto-Nr.:   5   4   3   2   1   1   2   3   4   9                 #
 * # Gewichtung:  1   2   1   2   1   2   1   2   1                     #
 * #              5 + 8 + 3 + 4 + 1 + 2 + 2 + 6 + 4 = 35                #
 * # 35 - 1 = 34                                                        #
 * # 34 : 11 = 3, Rest 1                                                #
 * # 10 - 1 = 9 (Prüfziffer)                                            #
 * ######################################################################
 */

      CASE1(121)
         if(*kto!='5'){ /* Prüfung nach Methode 17 */
      CASE_U1(121,1)
#ifdef __ALPHA
            pz =  (kto[1]-'0')
               + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
               +  (kto[3]-'0')
               + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9);
#else
            pz =(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0');
            if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
#endif

            pz-=1;
            MOD_11_44;   /* pz%=11 */
            if(pz)pz=10-pz;
            CHECK_PZ8;
         }
         else{
      CASE_U1(121,2)
#ifdef __ALPHA
            pz =  (kto[0]-'0')
               + ((kto[1]<'5') ? (kto[1]-'0')*2 : (kto[1]-'0')*2-9)
               +  (kto[2]-'0')
               + ((kto[3]<'5') ? (kto[3]-'0')*2 : (kto[3]-'0')*2-9)
               +  (kto[4]-'0')
               + ((kto[5]<'5') ? (kto[5]-'0')*2 : (kto[5]-'0')*2-9)
               +  (kto[6]-'0')
               + ((kto[7]<'5') ? (kto[7]-'0')*2 : (kto[7]-'0')*2-9)
               +  (kto[8]-'0');
#else
            pz =(kto[0]-'0')+(kto[2]-'0')+(kto[4]-'0')+(kto[6]-'0')+(kto[8]-'0');
            if(kto[1]<'5')pz+=(kto[1]-'0')*2; else pz+=(kto[1]-'0')*2-9;
            if(kto[3]<'5')pz+=(kto[3]-'0')*2; else pz+=(kto[3]-'0')*2-9;
            if(kto[5]<'5')pz+=(kto[5]-'0')*2; else pz+=(kto[5]-'0')*2-9;
            if(kto[7]<'5')pz+=(kto[7]-'0')*2; else pz+=(kto[7]-'0')*2-9;
#endif

            pz-=1;
            MOD_11_44;   /* pz%=11 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }


/* Berechnung nach der Methode C2 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode C2                        #
 * ######################################################################
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * # Die 10. Stelle der Kontonummer ist die Prüfziffer.                 #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 10, Gewichtung 3, 1, 3, 1, 3, 1, 3, 1, 3                   #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der             #
 * # Methode 22. Führt die Berechnung nach Variante 1 zu einem          #
 * # Prüfzifferfehler, so ist nach Variante 2 zu prüfen.                #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2                   #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der             #
 * # Methode 00.                                                        #
 * ######################################################################
 */

      CASE1(122)
      CASE_U1(122,1)
         pz = (kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');

            if(kto[0]<'4')
               pz+=(kto[0]-'0')*3;
            else if(kto[0]<'7')
               pz+=(kto[0]-'0')*3-10;
            else
               pz+=(kto[0]-'0')*3-20;

            if(kto[2]<'4')
               pz+=(kto[2]-'0')*3;
            else if(kto[2]<'7')
               pz+=(kto[2]-'0')*3-10;
            else
               pz+=(kto[2]-'0')*3-20;

            if(kto[4]<'4')
               pz+=(kto[4]-'0')*3;
            else if(kto[4]<'7')
               pz+=(kto[4]-'0')*3-10;
            else
               pz+=(kto[4]-'0')*3-20;

            if(kto[6]<'4')
               pz+=(kto[6]-'0')*3;
            else if(kto[6]<'7')
               pz+=(kto[6]-'0')*3-10;
            else
               pz+=(kto[6]-'0')*3-20;

            if(kto[8]<'4')
               pz+=(kto[8]-'0')*3;
            else if(kto[8]<'7')

               pz+=(kto[8]-'0')*3-10;
            else
               pz+=(kto[8]-'0')*3-20;

         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZX10;

      CASE_U1(122,2)
#ifdef __ALPHA
         pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
            +  (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;


/* Berechnung nach der Methode C3 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode C3                        #
 * ######################################################################
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen.        #
 * # Die 10. Stelle der Kontonummer ist die Prüfziffer. Kontonummern,   #
 * # die an der 1. Stelle der 10-stelligen Kontonummer einen Wert       #
 * # ungleich 9 beinhalten, sind nach der Variante 1 zu prüfen.         #
 * # Kontonummern, die an der 1. Stelle der 10-stelligen Kontonummer    #
 * # den Wert 9 beinhalten, sind nach der Variante 2 zu prüfen.         #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2                   #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der             #
 * # Methode 00.                                                        #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 0, 0, 0, 0                   #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der             #
 * # Methode 58.                                                        #
 * ######################################################################
 */

      CASE1(123)
         if(kto[0]!='9'){
      CASE_U1(123,1)
#ifdef __ALPHA
            pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
               +  (kto[1]-'0')
               + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
               +  (kto[3]-'0')
               + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
               +  (kto[7]-'0')
               + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
            pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
            if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
            if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
            if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
            MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }
         else{
      CASE_U1(123,2)
            pz = (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz)pz=11-pz;
            if(pz==10)return INVALID_KTO;
            CHECK_PZ10;
         }

/* Berechnung nach der Methode C4 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode C4                        #
 * ######################################################################
 * #                                                                    #
 * # Die Kontonummer ist einschließlich der Prüfziffer 10-stellig,      #
 * # ggf. ist die Kontonummer für die Prüfzifferberechnung durch        #
 * # linksbündige Auffüllung mit Nullen 10-stellig darzustellen. Die    #
 * # 10. Stelle der Kontonummer ist die Prüfziffer.                     #
 * # Kontonummern, die an der 1. Stelle der 10-stelligen Kontonummer    #
 * # einen Wert ungleich 9 beinhalten, sind nach der Variante 1 zu      #
 * # prüfen.                                                            #
 * # Kontonummern, die an der 1. Stelle der 10-stelligen Kontonummer    #
 * # den Wert 9 beinhalten, sind nach der Variante 2 zu prüfen.         #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 11, Gewichtung 2, 3, 4, 5                                  #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der Methode 15. #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 11, Gewichtung 2, 3, 4, 5, 6, 0, 0, 0, 0                   #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der Methode 58. #
 * ######################################################################
 */

      CASE1(124)
         if(kto[0]!='9'){
      CASE_U1(124,1)
            pz = (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            MOD_11_88;    /* pz%=11 */
            if(pz<=1)
               pz=0;
            else
               pz=11-pz;
            CHECK_PZ10;
         }
         else{
      CASE_U1(124,2)
            pz = (kto[4]-'0') * 6
               + (kto[5]-'0') * 5
               + (kto[6]-'0') * 4
               + (kto[7]-'0') * 3
               + (kto[8]-'0') * 2;

            MOD_11_176;   /* pz%=11 */
            if(pz)pz=11-pz;
            if(pz==10)return INVALID_KTO;
            CHECK_PZ10;
         }

/* Berechnung nach der Methode C5 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode C5                        #
 * ######################################################################
 * #                                                                    #
 * # Die Kontonummern sind einschließlich der Prüfziffer 6- oder 8-     #
 * # bis 10-stellig, ggf. ist die Kontonummer für die Prüfziffer-       #
 * # berechnung durch linksbündige Auffüllung mit Nullen 10-stellig     #
 * # darzustellen.                                                      #
 * #                                                                    #
 * # Die Berechnung der Prüfziffer und die möglichen Ergebnisse         #
 * # richten sich nach dem jeweils bei der entsprechenden Variante      #
 * # angegebenen Kontonummernkreis. Entspricht eine Kontonummer         #
 * # keinem der vorgegebenen Kontonummernkreise oder führt die          #
 * # Berechnung der Prüfziffer nach der vorgegebenen Variante zu        #
 * # einem Prüfzifferfehler, so ist die Kontonummer ungültig.           #
 * #                                                                    #
 * # Variante 1:                                                        #
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2                               #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der             #
 * # Methode 75.                                                        #
 * #                                                                    #
 * # 6-stellige Kontonummern; 5. Stelle = 1-8                           #
 * # Kontonummernkreis 0000100000 bis 0000899999                        #
 * #                                                                    #
 * # 9-stellige Kontonummern; 2. Stelle = 1-8                           #
 * # Kontonummernkreis 0100000000 bis 0899999999                        #
 * #                                                                    #
 * # Variante 2:                                                        #
 * # Modulus 10, iterierte Transformation                               #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der             #
 * # Methode 29.                                                        #
 * #                                                                    #
 * # 10-stellige Kontonummern, 1. Stelle = 1, 4, 5, 6 oder 9            #
 * # Kontonummernkreis 1000000000 bis 1999999999                        #
 * # Kontonummernkreis 4000000000 bis 6999999999                        #
 * # Kontonummernkreis 9000000000 bis 9999999999                        #
 * #                                                                    #
 * # Variante 3:                                                        #
 * # Modulus 10, Gewichtung 2, 1, 2, 1, 2, 1, 2, 1, 2                   #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der             #
 * # Methode 00.                                                        #
 * # 10-stellige Kontonummern, 1. Stelle = 3                            #
 * # Kontonummernkreis 3000000000 bis 3999999999                        #
 * #                                                                    #
 * # Variante 4:                                                        #
 * # Für die folgenden Kontonummernkreise gilt die Methode 09           #
 * # (keine Prüfzifferberechnung).                                      #
 * #                                                                    #
 * # 8-stellige Kontonummern; 3. Stelle = 3, 4 oder 5                   #
 * # Kontonummernkreis 0030000000 bis 0059999999                        #
 * #                                                                    #
 * # 10-stellige Kontonummern; 1.+ 2. Stelle = 70 oder 85               #
 * # Kontonummernkreis 7000000000 bis 7099999999                        #
 * # Kontonummernkreis 8500000000 bis 8599999999                        #
 * ######################################################################
 */

      CASE1(125)

      CASE_U1(125,1)

            /* Variante 1a:
             *  6-stellige Kontonummern; 5. Stelle = 1-8, Prüfziffer an Stelle 10
             */
         if(kto[0]=='0' && kto[1]=='0' && kto[2]=='0' && kto[3]=='0' && kto[4]>='1' && kto[4]<='8'){
#ifdef __ALPHA
            pz = ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
               +  (kto[7]-'0')
               + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
            pz=(kto[5]-'0')+(kto[7]-'0');
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
            if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }

            /* Variante 1b:
             *    9-stellige Kontonummern; 2. Stelle = 1-8, Prüfziffer an Stelle 7
             */
         else if(kto[0]=='0' && kto[1]>='1' && kto[1]<='8'){
#ifdef __ALPHA
            pz = ((kto[1]<'5') ? (kto[1]-'0')*2 : (kto[1]-'0')*2-9)
               +  (kto[2]-'0')
               + ((kto[3]<'5') ? (kto[3]-'0')*2 : (kto[3]-'0')*2-9)
               +  (kto[4]-'0')
               + ((kto[5]<'5') ? (kto[5]-'0')*2 : (kto[5]-'0')*2-9);
#else
            pz=(kto[2]-'0')+(kto[4]-'0');
            if(kto[1]<'5')pz+=(kto[1]-'0')*2; else pz+=(kto[1]-'0')*2-9;
            if(kto[3]<'5')pz+=(kto[3]-'0')*2; else pz+=(kto[3]-'0')*2-9;
            if(kto[5]<'5')pz+=(kto[5]-'0')*2; else pz+=(kto[5]-'0')*2-9;
#endif
         MOD_10_40;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ7;
         }

            /* Variante 2: 10-stellige Kontonummern, 1. Stelle = 1, 4, 5, 6 oder 9 */
         else if(kto[0]=='1' || kto[0]>='4' && kto[0]<='6' || kto[0]=='9'){
      CASE_U1(125,2)
            pz = m10h_digits[0][(unsigned int)(kto[0]-'0')]
               + m10h_digits[3][(unsigned int)(kto[1]-'0')]
               + m10h_digits[2][(unsigned int)(kto[2]-'0')]
               + m10h_digits[1][(unsigned int)(kto[3]-'0')]
               + m10h_digits[0][(unsigned int)(kto[4]-'0')]
               + m10h_digits[3][(unsigned int)(kto[5]-'0')]
               + m10h_digits[2][(unsigned int)(kto[6]-'0')]
               + m10h_digits[1][(unsigned int)(kto[7]-'0')]
               + m10h_digits[0][(unsigned int)(kto[8]-'0')];
            MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }

            /* Variante 3: 10-stellige Kontonummern, 1. Stelle = 3 */
         else if(kto[0]=='3'){
      CASE_U1(125,3)
#ifdef __ALPHA
            pz = ((kto[0]<'5') ? (kto[0]-'0')*2 : (kto[0]-'0')*2-9)
               +  (kto[1]-'0')
               + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
               +  (kto[3]-'0')
               + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
               +  (kto[5]-'0')
               + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
               +  (kto[7]-'0')
               + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
            pz=(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
            if(kto[0]<'5')pz+=(kto[0]-'0')*2; else pz+=(kto[0]-'0')*2-9;
            if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
            if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
            if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
            if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
            MOD_10_80;   /* pz%=10 */
            if(pz)pz=10-pz;
            CHECK_PZ10;
         }

            /* Variante 4:
             *    8-stellige KOntonummern mit 3. Stelle = 3,4, oder 5
             *    10-stellige Kontonummern, 1. und 2. Stelle = 70 oder 85:
             */
         else if( kto[0]=='0' && kto[1]=='0' && kto[2]>='3' && kto[2]<='5'
               || kto[0]=='7' && kto[1]=='0'
               || kto[0]=='8' && kto[1]=='5'){
      CASE_U1(125,4)
         return OK_NO_CHK;
      }
      else  /* Kontonummer entspricht keinem vorgegebenen Kontenkreis */
         return INVALID_KTO;

/* Berechnung nach der Methode C6 +§§§4 */
/*
 * ######################################################################
 * #              Berechnung nach der Methode C6                        #
 * ######################################################################
 * # Modulus 10, Gewichtung 1, 2, 1, 2, 1, 2, 1, 2                      #
 * # Die Kontonummer ist 10-stellig, ggf. ist die Kontonummer für die   #
 * # Prüfzifferberechnung durch linksbündige Auffüllung mit Nullen      #
 * # 10-stellig darzustellen. Die 10. Stelle der Kontonummer ist die    #
 * # Prüfziffer.                                                        #
 * #                                                                    #
 * # Für die Berechnung der Prüfziffer werden die Stellen 2 bis 9 der   #
 * # Kontonummer verwendet. Diese Stellen sind links um die Zahl        #
 * # (Konstante) 5499570 zu ergänzen.                                   #
 * #                                                                    #
 * # Die Berechnung und mögliche Ergebnisse entsprechen der Methode 00. #
 * ######################################################################
 * #                                                                    #
 * # Anmerkung zur Berechnung (MP): Da die Konstante immer nur einen    #
 * # festen Wert zur Berechnung beiträgt (31), wird diese Berechnung    #
 * # nicht gemacht, sondern gleich der Wert als Initialwert für die     #
 * # Quersumme verwendet und die Berechnung erst mit der zweiten Stelle #
 * # der Kontonummer begonnen.                                          #
 * #                                                                    #
 * ######################################################################
 */

      CASE1(126)
#ifdef __ALPHA
         pz = 31  /* Quersumme der Berechnung für die Konstante */
            + (kto[1]-'0')
            + ((kto[2]<'5') ? (kto[2]-'0')*2 : (kto[2]-'0')*2-9)
            +  (kto[3]-'0')
            + ((kto[4]<'5') ? (kto[4]-'0')*2 : (kto[4]-'0')*2-9)
            +  (kto[5]-'0')
            + ((kto[6]<'5') ? (kto[6]-'0')*2 : (kto[6]-'0')*2-9)
            +  (kto[7]-'0')
            + ((kto[8]<'5') ? (kto[8]-'0')*2 : (kto[8]-'0')*2-9);
#else
         pz=31+(kto[1]-'0')+(kto[3]-'0')+(kto[5]-'0')+(kto[7]-'0');
         if(kto[2]<'5')pz+=(kto[2]-'0')*2; else pz+=(kto[2]-'0')*2-9;
         if(kto[4]<'5')pz+=(kto[4]-'0')*2; else pz+=(kto[4]-'0')*2-9;
         if(kto[6]<'5')pz+=(kto[6]-'0')*2; else pz+=(kto[6]-'0')*2-9;
         if(kto[8]<'5')pz+=(kto[8]-'0')*2; else pz+=(kto[8]-'0')*2-9;
#endif
         MOD_10_160;   /* pz%=10 */
         if(pz)pz=10-pz;
         CHECK_PZ10;

/* nicht abgedeckte Fälle +§§§3 */
/*
 * ######################################################################
 * #               nicht abgedeckte Fälle                               #
 * ######################################################################
 */
      default:
#if DEBUG
         if(untermethode)return UNDEFINED_SUBMETHOD; 
#endif
         return NOT_IMPLEMENTED;
   }
}

/*
 * ######################################################################
 * #               Ende der Berechnungsmethoden                         #
 * ######################################################################
 */


/* Funktion set_msg() +§§§1 */
/* ###########################################################################
 * # Die Funktion set_retval() setzt die globale Variable kto_check_msg auf  #
 * # die entsprechende Klartext-Fehlermeldung.                               #
 * #                                                                         #
 * # Copyright (C) 2002-2005 Michael Plugge <m.plugge@hs-mannheim.de>        #
 * ###########################################################################
 */

#if THREAD_SAFE
static void set_msg(int retval){
   set_msg_t(retval,&global_ctx);
}

static void set_msg_t(int retval,KTO_CHK_CTX *ctx)
#else
static void set_msg(int retval)
#endif
{
   switch(retval){
      case UNDEFINED_SUBMETHOD:
         kto_check_msg="##### Die Untermethode ist nicht definiert #####";
         break;

      case INVALID_LUT_VERSION:
#if HTML_OUTPUT
         kto_check_msg="##### Die Versionsnummer f&uuml;r die LUT-Datei ist ung&uuml;ltig #####";
#else
         kto_check_msg="##### Die Versionsnummer für die LUT-Datei ist ungültig #####";
#endif
         break;

      case INVALID_BLZ_FILE:
#if HTML_OUTPUT
         kto_check_msg="##### Fehler in der blz.txt Datei (falsche Zeilenl&auml;nge) #####";
#else
         kto_check_msg="##### Fehler in der blz.txt Datei (falsche Zeilenlänge) #####";
#endif
         break;

      case LIBRARY_IS_NOT_THREAD_SAFE:
         kto_check_msg="##### undefinierte Funktion; die library wurde mit THREAD_SAFE=0 kompiliert #####";
         break;

      case FATAL_ERROR:
         kto_check_msg="##### schwerer Fehler im Konto-Modul #####";
         break;

      case INVALID_KTO_LENGTH:
#if HTML_OUTPUT
         kto_check_msg="##### ein Konto mu&szlig; zwischen 1 und 10 Stellen haben #####";
#else
         kto_check_msg="##### ein Konto muß zwischen 1 und 10 Stellen haben #####";
#endif
         break;

      case FILE_WRITE_ERROR:
         kto_check_msg="##### kann Datei nicht schreiben #####";
         break;

      case FILE_READ_ERROR:
         kto_check_msg="##### kann Datei nicht lesen #####";
         break;

      case ERROR_MALLOC:
         kto_check_msg="##### kann keinen Speicher allokieren #####";
         break;

      case NO_BLZ_FILE:
         kto_check_msg="##### die Datei mit den Bankleitzahlen wurde nicht gefunden #####";
         break;

      case INVALID_LUT_FILE:
#if HTML_OUTPUT
         kto_check_msg="##### die blz.lut Datei ist inkosistent/ung&uuml;ltig #####";
#else
         kto_check_msg="##### die blz.lut Datei ist inkosistent/ungültig #####";
#endif
         break;

      case NO_LUT_FILE:
         kto_check_msg="##### die blz.lut Datei wurde nicht gefunden #####";
         break;

      case INVALID_BLZ_LENGTH:
         kto_check_msg="##### die Bankleitzahl ist nicht achtstellig #####";
         break;

      case INVALID_BLZ:
#if HTML_OUTPUT
         kto_check_msg="##### die Bankleitzahl ist ung&uuml;ltig #####";
#else
         kto_check_msg="##### die Bankleitzahl ist ungültig #####";
#endif
         break;

      case INVALID_KTO:
#if HTML_OUTPUT
         kto_check_msg="##### das Konto ist ung&uuml;ltig #####";
#else
         kto_check_msg="##### das Konto ist ungültig #####";
#endif
         break;

      case NOT_IMPLEMENTED:
         kto_check_msg="##### die Methode wurde noch nicht implementiert #####";
         break;

      case NOT_DEFINED:
         kto_check_msg="##### die Methode ist nicht definiert #####";
         break;

      case FALSE:
         kto_check_msg="##### falsch #####";
         break;

      case OK:
         kto_check_msg="ok";
         break;

      case OK_NO_CHK:
#if HTML_OUTPUT
         kto_check_msg="ok, ohne Pr&uuml;fung";
#else
         kto_check_msg="ok, ohne Prüfung";
#endif
         break;

      default:
#if HTML_OUTPUT
         kto_check_msg="ung&uuml;ltiger R&uuml;ckgabewert";
#else
         kto_check_msg="ungültiger Rückgabewert";
#endif
         break;

   }
}

/* Funktion kto_check() +§§§1 */
/* ###########################################################################
 * # Die Funktion kto_check() ist die externe Schnittstelle zur Überprüfung  #
 * # einer Kontonummer. Nach Aufruf von kto_check_int() wird noch die        #
 * # (globale) Variable kto_check_msg auf einen Ergebnistext bzw. eine       #
 * # Fehlermeldung gesetzt.                                                  #
 * #                                                                         #
 * # Parameter:                                                              #
 * #    pz_or_blz:  Prüfziffer (2-stellig) oder BLZ (8-stellig)              #
 * #    kto:        Kontonummer                                              #
 * #    lut_name:   Name der Lookup-Datei oder NULL (für DEFAULT_LUT_NAME)   #
 * #                                                                         #
 * # Copyright (C) 2002-2005 Michael Plugge <m.plugge@hs-mannheim.de>        #
 * ###########################################################################
 */

DLL_EXPORT int kto_check(char *pz_or_blz,char *kto,char *lut_name)
{
   int retval;

#if THREAD_SAFE
   retval=kto_check_int_t(pz_or_blz,kto,lut_name,&global_ctx);
   set_msg_t(retval,&global_ctx);
   set_globals(&global_ctx);
#else
   retval=kto_check_int(pz_or_blz,kto,lut_name);
   set_msg(retval);
#endif
   return retval;
}

/* Funktion kto_check_str() +§§§1 */
/* ###########################################################################
 * # Die Funktion kto_check_str() entspricht der Funktion konto_check();     #
 * # die Rückgabe ist allerdings ein String mit einer kurzen Fehlermeldung   #
 * # statt eines numerischen Wertes. Die Funktion wurde zunächst für die     #
 * # Perl-Variante eingeführt.                                               #
 * #                                                                         #
 * # Copyright (C) 2007 Michael Plugge <m.plugge@hs-mannheim.de>             #
 * ###########################################################################
 */

DLL_EXPORT char *kto_check_str(char *pz_or_blz,char *kto,char *lut_name)
{
   int retval;

#if THREAD_SAFE
   retval=kto_check_int_t(pz_or_blz,kto,lut_name,&global_ctx);
   set_msg_t(retval,&global_ctx);
   set_globals(&global_ctx);
#else
   retval=kto_check_int(pz_or_blz,kto,lut_name);
   set_msg(retval);
#endif
   switch(retval){
      case UNDEFINED_SUBMETHOD: return "UNDEFINED_SUBMETHOD";
      case INVALID_LUT_VERSION: return "INVALID_LUT_VERSION";
      case FALSE_GELOESCHT: return "FALSE_GELOESCHT";
      case OK_NO_CHK_GELOESCHT: return "OK_NO_CHK_GELOESCHT";
      case OK_GELOESCHT: return "OK_GELOESCHT";
      case BLZ_GELOESCHT: return "BLZ_GELOESCHT";
      case INVALID_BLZ_FILE: return "INVALID_BLZ_FILE";
      case LIBRARY_IS_NOT_THREAD_SAFE: return "LIBRARY_IS_NOT_THREAD_SAFE";
      case FATAL_ERROR: return "FATAL_ERROR";
      case INVALID_KTO_LENGTH: return "INVALID_KTO_LENGTH";
      case FILE_WRITE_ERROR: return "FILE_WRITE_ERROR";
      case FILE_READ_ERROR: return "FILE_READ_ERROR";
      case ERROR_MALLOC: return "ERROR_MALLOC";
      case NO_BLZ_FILE: return "NO_BLZ_FILE";
      case INVALID_LUT_FILE: return "INVALID_LUT_FILE";
      case NO_LUT_FILE: return "NO_LUT_FILE";
      case INVALID_BLZ_LENGTH: return "INVALID_BLZ_LENGTH";
      case INVALID_BLZ: return "INVALID_BLZ";
      case INVALID_KTO: return "INVALID_KTO";
      case NOT_IMPLEMENTED: return "NOT_IMPLEMENTED";
      case NOT_DEFINED: return "NOT_DEFINED";
      case FALSE: return "FALSE";
      case OK: return "OK";
      case OK_NO_CHK: return "OK_NO_CHK";
      default: return "???";
   }
}

/* Funktion get_lut_info() +§§§1 */
#if THREAD_SAFE
DLL_EXPORT int get_lut_info(char **info,char *lut_name)
{
   return get_lut_info_t(info,lut_name,&global_ctx);
}

DLL_EXPORT int get_lut_info_t(char **info,char *lut_name,KTO_CHK_CTX *ctx)
#else
DLL_EXPORT int get_lut_info_t(char **info,char *lut_name,KTO_CHK_CTX *ctx)
{
   ctx->kto_check_msg="##### undefinierte Funktion; die library wurde mit THREAD_SAFE=0 kompiliert #####";
   return LIBRARY_IS_NOT_THREAD_SAFE;
}

DLL_EXPORT int get_lut_info(char **info,char *lut_name)
#endif
{
#if THREAD_SAFE
   if(cnt_blz<=0 && (cnt_blz=read_lut_t(lut_name,ctx))<=0){
#else
   if(cnt_blz<=0 && (cnt_blz=read_lut(lut_name))<=0){
#endif
      *info="Fehler beim Lesen der LUT-Datei";
      return cnt_blz;  /* Fehler */
   }
   *info=lut_info;
   return OK;
}

/* Funktion cleanup_kto() +§§§1 */
/* ###########################################################################
 * # cleanup_kto_t(): Speicher freigeben, cnt_blz=0 setzen (als flag)        #
 * # Beim Rückgabewert 1 wurde aufgeräumt, bei Rückgabe 0 war nichts         #
 * # zu tun (die library war nicht initialisiert worden) (threadfest)        #
 * ###########################################################################
 */

#if THREAD_SAFE
DLL_EXPORT int cleanup_kto(void)
{
   return cleanup_kto_t(&global_ctx);
}

DLL_EXPORT int cleanup_kto_t(KTO_CHK_CTX *ctx)
#else
DLL_EXPORT int cleanup_kto_t(KTO_CHK_CTX *ctx)
{
   ctx->kto_check_msg="##### undefinierte Funktion; die library wurde mit THREAD_SAFE=0 kompiliert #####";
   return LIBRARY_IS_NOT_THREAD_SAFE;
}

DLL_EXPORT int cleanup_kto(void)
#endif
{
   if(cnt_blz){
      cnt_blz=0;
      free(blz_array);
      free(pz_array);
      free(invalid);
      free(blz_hash_low);
      free(blz_hash_high);
      return 1;
   }
   return 0;
}

/* Funktion get_kto_check_version() +§§§1 */
DLL_EXPORT char *get_kto_check_version(void)
{
   return "konto_check Version " VERSION " vom " VERSION_DATE " (kompiliert " __DATE__ ", " __TIME__ ")";
}

/* Funktion kto_check_t() +§§§1 */
/* ###########################################################################
 * # Die Funktion kto_check_t() entspricht weitgehend der Funktion           #
 * # kto_check(); nur gibt es noch einen zusätzlichen Parameter ctx, der     #
 * # einen Satz von lokalen Variablen enthält, die anstelle der globalen     #
 * # bzw. static Variablen benutzt werden; damit dürfte das Programm         #
 * # threadfest werden (notwendig für mySQL).                                #
 * #                                                                         #
 * # Parameter:                                                              #
 * #    pz_or_blz:  Prüfziffer (2-stellig) oder BLZ (8-stellig)              #
 * #    kto:        Kontonummer                                              #
 * #    lut_name:   Name der Lookup-Datei oder NULL (für DEFAULT_LUT_NAME)   #
 * #    ctx:        Block mit Speicherplatz für static Variablen             #
 * #                                                                         #
 * # Copyright (C) 2002-2005 Michael Plugge <m.plugge@hs-mannheim.de>        #
 * ###########################################################################
 */

#if THREAD_SAFE
DLL_EXPORT int kto_check_t(char *pz_or_blz,char *kto,char *lut_name,KTO_CHK_CTX *ctx)
{
   int retval;

   retval=kto_check_int_t(pz_or_blz,kto,lut_name,ctx);
   set_msg_t(retval,ctx);
   set_globals(ctx);
   return retval;
}
#else
DLL_EXPORT int kto_check_t(char *pz_or_blz,char *kto,char *lut_name,KTO_CHK_CTX *ctx)
{
   ctx->kto_check_msg="##### undefinierte Funktion; die library wurde mit THREAD_SAFE=0 kompiliert #####";
   return LIBRARY_IS_NOT_THREAD_SAFE;
}
#endif

/* Funktion kto_check_str_t() +§§§1 */
/* ###########################################################################
 * # Die Funktion kto_check_str_t() entspricht weitgehend der Funktion       #
 * # kto_check_str(); nur gibt es noch einen zusätzlichen Parameter ctx, der #
 * # einen Satz von lokalen Variablen enthält, die anstelle der globalen     #
 * # bzw. static Variablen benutzt werden; damit dürfte das Programm         #
 * # threadfest werden (notwendig für mySQL).                                #
 * #                                                                         #
 * # Parameter:                                                              #
 * #    pz_or_blz:  Prüfziffer (2-stellig) oder BLZ (8-stellig)              #
 * #    kto:        Kontonummer                                              #
 * #    lut_name:   Name der Lookup-Datei oder NULL (für DEFAULT_LUT_NAME)   #
 * #    ctx:        Block mit Speicherplatz für static Variablen             #
 * #                                                                         #
 * # Copyright (C) 2007 Michael Plugge <m.plugge@hs-mannheim.de>             #
 * ###########################################################################
 */

#if THREAD_SAFE
DLL_EXPORT char *kto_check_str_t(char *pz_or_blz,char *kto,char *lut_name,KTO_CHK_CTX *ctx)
{
   int retval;

   retval=kto_check_int_t(pz_or_blz,kto,lut_name,ctx);
   set_msg_t(retval,ctx);
   set_globals(ctx);
   switch(retval){
      case UNDEFINED_SUBMETHOD: return "UNDEFINED_SUBMETHOD";
      case INVALID_LUT_VERSION: return "INVALID_LUT_VERSION";
      case FALSE_GELOESCHT: return "FALSE_GELOESCHT";
      case OK_NO_CHK_GELOESCHT: return "OK_NO_CHK_GELOESCHT";
      case OK_GELOESCHT: return "OK_GELOESCHT";
      case BLZ_GELOESCHT: return "BLZ_GELOESCHT";
      case INVALID_BLZ_FILE: return "INVALID_BLZ_FILE";
      case LIBRARY_IS_NOT_THREAD_SAFE: return "LIBRARY_IS_NOT_THREAD_SAFE";
      case FATAL_ERROR: return "FATAL_ERROR";
      case INVALID_KTO_LENGTH: return "INVALID_KTO_LENGTH";
      case FILE_WRITE_ERROR: return "FILE_WRITE_ERROR";
      case FILE_READ_ERROR: return "FILE_READ_ERROR";
      case ERROR_MALLOC: return "ERROR_MALLOC";
      case NO_BLZ_FILE: return "NO_BLZ_FILE";
      case INVALID_LUT_FILE: return "INVALID_LUT_FILE";
      case NO_LUT_FILE: return "NO_LUT_FILE";
      case INVALID_BLZ_LENGTH: return "INVALID_BLZ_LENGTH";
      case INVALID_BLZ: return "INVALID_BLZ";
      case INVALID_KTO: return "INVALID_KTO";
      case NOT_IMPLEMENTED: return "NOT_IMPLEMENTED";
      case NOT_DEFINED: return "NOT_DEFINED";
      case FALSE: return "FALSE";
      case OK: return "OK";
      case OK_NO_CHK: return "OK_NO_CHK";
      default: return "???";
   }
}
#else
DLL_EXPORT char *kto_check_str_t(char *pz_or_blz,char *kto,char *lut_name,KTO_CHK_CTX *ctx)
{
   ctx->kto_check_msg="##### undefinierte Funktion; die library wurde mit THREAD_SAFE=0 kompiliert #####";
   return "LIBRARY_IS_NOT_THREAD_SAFE";
}
#endif

/* Funktion kto_check_test_vars() +§§§1 */
/* ###########################################################################
 * # Die Funktion kto_check_test_vars() macht nichts anderes, als die beiden #
 * # übergebenen Variablen txt und i auszugeben. Sie kann benutzt werden,    #
 * # wenn Probleme mit Variablen in der DLL auftreten; ansonsten ist sie     #
 * # nicht allzu nützlich.                                                   #
 * #                                                                         #
 * # Parameter:                                                              #
 * #    txt:        Textvariable                                             #
 * #    i:          Integervariable (4 Byte)                                 #
 * #                                                                         #
 * # Copyright (C) 2006 Michael Plugge <m.plugge@hs-mannheim.de>             #
 * ###########################################################################
 */

DLL_EXPORT void kto_check_test_vars(char *txt,int i)
{
   fprintf(stderr,"Test von Variablen:\nTextvariable: %s\nIntegervariable: %d (Hexwert: 0x%08X)\n",txt,i,i);
}
#else /* !INCLUDE_KONTO_CHECK_DE */
/* Leerdefinitionen für !INCLUDE_KONTO_CHECK_DE +§§§1 */
#include "konto_check.h"
DLL_EXPORT int kto_check(char *pz_or_blz,char *kto,char *lut_name){return EXCLUDED_AT_COMPILETIME;};
DLL_EXPORT int kto_check_t(char *pz_or_blz,char *kto,char *lut_name,KTO_CHK_CTX *ctx){return EXCLUDED_AT_COMPILETIME;};
DLL_EXPORT char *kto_check_str(char *pz_or_blz,char *kto,char *lut_name){return "EXCLUDED_AT_COMPILETIME";};
DLL_EXPORT char *kto_check_str_t(char *pz_or_blz,char *kto,char *lut_name,KTO_CHK_CTX *ctx){return "EXCLUDED_AT_COMPILETIME";};
DLL_EXPORT int cleanup_kto(void){return EXCLUDED_AT_COMPILETIME;};
DLL_EXPORT int cleanup_kto_t(KTO_CHK_CTX *ctx){return EXCLUDED_AT_COMPILETIME;};
DLL_EXPORT int generate_lut(char *inputname,char *outputname,char *user_info,int lut_version){return EXCLUDED_AT_COMPILETIME;};
DLL_EXPORT int get_lut_info(char **info,char *lut_name){return EXCLUDED_AT_COMPILETIME;};
DLL_EXPORT int get_lut_info_t(char **info,char *lut_name,KTO_CHK_CTX *ctx){return EXCLUDED_AT_COMPILETIME;};
DLL_EXPORT char *get_kto_check_version(void){return "EXCLUDED_AT_COMPILETIME";};
#endif
