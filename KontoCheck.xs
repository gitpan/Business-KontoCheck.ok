#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"
#include "konto_check.h"
#include "konto_check-at.h"

/* Global Data */

#define MY_CXT_KEY "Business::KontoCheck::_guts" XS_VERSION

typedef struct {
    /* Put Global Data in here */
    int dummy;		/* you can access this elsewhere as MY_CXT.dummy */
} my_cxt_t;

START_MY_CXT


MODULE = Business::KontoCheck		PACKAGE = Business::KontoCheck		


BOOT:
{
    MY_CXT_INIT;
    /* If any of the fields in the my_cxt_t struct need
       to be initialised, do it here.
     */
}


PROTOTYPES: ENABLE

# Aufruf der konto_check-Bibliothek

int
kto_check(pz_or_blz,kto,lut_name)
   char *pz_or_blz;
   char *kto;
   char *lut_name;
   CODE:
   RETVAL=kto_check(pz_or_blz,kto,lut_name);
   OUTPUT: RETVAL

const char *
kto_check_str(pz_or_blz,kto,lut_name)
   char *pz_or_blz;
   char *kto;
   char *lut_name;
   CODE:
   RETVAL=kto_check_str(pz_or_blz,kto,lut_name);
   OUTPUT: RETVAL

int
generate_lut(inputname,outputname,user_info,lut_version)
   char *inputname;
   char *outputname;
   char *user_info;
   int lut_version;
   CODE:
   RETVAL=generate_lut(inputname,outputname,user_info,lut_version);
   OUTPUT: RETVAL

int
kto_check_at(blz,kto,lut_name)
   char *blz;
   char *kto;
   char *lut_name;
   CODE:
   RETVAL=kto_check_at(blz,kto,lut_name);
   OUTPUT: RETVAL

const char *
kto_check_at_str(blz,kto,lut_name)
   char *blz;
   char *kto;
   char *lut_name;
   CODE:
   RETVAL=kto_check_at_str(blz,kto,lut_name);
   OUTPUT: RETVAL

int
generate_lut_at(inputname,outputname,plain_name,plain_format)
   char *inputname;
   char *outputname;
   char *plain_name;
   char *plain_format;
   CODE:
   RETVAL=generate_lut_at(inputname,outputname,plain_name,plain_format);
   OUTPUT: RETVAL

