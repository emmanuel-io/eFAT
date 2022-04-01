/**
 * ********************************************************************************************************************
 *  @file     ef_prv_unicode.c
 *  @ingroup  group_eFAT_Private
 *  @author   ChaN
 *  @author   Emmanuel AMADIO
 *  @version  V0.1
 *  @brief    Code file for unicode support.
 *
 * ********************************************************************************************************************
 *  eFAT - embedded FAT Filesystem module
 * ********************************************************************************************************************
 *
 *  Copyright (C) 2021, Amadio Emmanuel, all right reserved.
 *  Copyright (C) 2019, ChaN, all right reserved.
 *
 *  eFAT module is an open source software. Redistribution and use of eFAT in source and binary forms, with or without
 *  modification, are permitted provided that the following condition is met:
 *
 *  Redistributions of source code must retain the above copyright notice, this condition and the following disclaimer.
 *
 *  This software is provided by the copyright holders and contributors "AS IS" and any warranties related to this
 *  software are DISCLAIMED.
 *  The copyright owners or contributors be NOT LIABLE for any damages caused by use of this software.
 * ********************************************************************************************************************
 */

/* START OF FILE *************************************************************************************************** */
/* ***************************************************************************************************************** */

/* Includes -------------------------------------------------------------------------------------------------------- */

#include <efat.h>
#include "ef_prv_unicode.h"
#include <ef_prv_def_cp.h>
#include <ef_prv_def_cp_upcase.h>
#include "ef_prv_def_cp932.h"
#include "ef_prv_def_cp936.h"
#include "ef_prv_def_cp949.h"
#include "ef_prv_def_cp950.h"

/* Local constant macros ------------------------------------------------------------------------------------------- */
/* Local function macros ------------------------------------------------------------------------------------------- */
//#define MERGE2(a, b)  a ## b
//#define CVTBL(tbl,cp) MERGE2(tbl,cp)

#define ADDCOMMA(a)  a ## ,
#define TABLE_ELEMENT_ON(a) ADDCOMMA(a)
#define TABLE_ELEMENT_OFF(a)

/* Local typedefs, structures, unions and enums -------------------------------------------------------------------- */
/* Local variables ------------------------------------------------------------------------------------------------- */

/*------------------------------------------------------------------------*/
/* Code Conversion Tables                                                 */
/*------------------------------------------------------------------------*/

#if ( 0 == EF_CONF_CP437 )
  #define EF_CP_437_TEL
  #define EF_CP_437_UC
  #define EF_CP_437_CT
#elif ( 1 == EF_CONF_CP437 )
  /*  CP437(U.S.) to Unicode conversion table */
  static const ucs2_t uc437[ 128 ] = TBL_CP437_TO_UNICODE;

  static const ef_u08_t Ct437[] = TBL_CT437;
  #define EF_CP_437_TEL 437,
  #define EF_CP_437_UC  uc437,
  #define EF_CP_437_CT  Ct437,
#else
  #error Wrong configuration EF_CONF_CP437 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP720 )
  #define EF_CP_720_TEL
  #define EF_CP_720_UC
  #define EF_CP_720_CT
#elif ( 1 == EF_CONF_CP720 )
  #define EF_CP_720_TEL 720,
  /*  CP720(Arabic) to Unicode conversion table */
  static const ucs2_t uc720[ 128 ] = TBL_CP720_TO_UNICODE;

  static const ef_u08_t Ct720[] = TBL_CT720;
  #define EF_CP_720_UC  uc720,
  #define EF_CP_720_CT  Ct720,
#else
  #error Wrong configuration EF_CONF_CP720 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP737 )
  #define EF_CP_737_TEL
  #define EF_CP_737_UC
  #define EF_CP_737_CT
#elif ( 1 == EF_CONF_CP737 )
  #define EF_CP_737_TEL 737,
  /*  CP737(Greek) to Unicode conversion table */
  static const ucs2_t uc737[ 128 ] = TBL_CP737_TO_UNICODE;

  static const ef_u08_t Ct737[] = TBL_CT737;
  #define EF_CP_737_UC  uc737,
  #define EF_CP_737_CT  Ct737,
#else
  #error Wrong configuration EF_CONF_CP737 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP771 )
  #define EF_CP_771_TEL
  #define EF_CP_771_UC
  #define EF_CP_771_CT
#elif ( 1 == EF_CONF_CP771 )
  #define EF_CP_771_TEL 771,
  /*  CP771(KBL) to Unicode conversion table */
  static const ucs2_t uc771[ 128 ] = TBL_CP771_TO_UNICODE;

  static const ef_u08_t Ct771[] = TBL_CT771;
  #define EF_CP_771_UC  uc771,
  #define EF_CP_771_CT  Ct771,
#else
  #error Wrong configuration EF_CONF_CP771 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP775 )
  #define EF_CP_775_TEL
  #define EF_CP_775_UC
  #define EF_CP_775_CT
#elif ( 1 == EF_CONF_CP775 )
  #define EF_CP_775_TEL 775,
  /*  CP775(Baltic) to Unicode conversion table */
  static const ucs2_t uc775[ 128 ] = TBL_CP775_TO_UNICODE;

  static const ef_u08_t Ct775[] = TBL_CT775;
  #define EF_CP_775_UC  uc775,
  #define EF_CP_775_CT  Ct775,
#else
  #error Wrong configuration EF_CONF_CP775 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP850 )
  #define EF_CP_850_TEL
  #define EF_CP_850_UC
  #define EF_CP_850_CT
#elif ( 1 == EF_CONF_CP850 )
  #define EF_CP_850_TEL 850,
  /*  CP850(Latin 1) to Unicode conversion table */
  static const ucs2_t uc850[ 128 ] = TBL_CP850_TO_UNICODE;

  static const ef_u08_t Ct850[] = TBL_CT850;
  #define EF_CP_850_UC  uc850,
  #define EF_CP_850_CT  Ct850,
#else
  #error Wrong configuration EF_CONF_CP850 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP852 )
  #define EF_CP_852_TEL
  #define EF_CP_852_UC
  #define EF_CP_852_CT
#elif ( 1 == EF_CONF_CP852 )
  #define EF_CP_852_TEL 852,
  /*  CP852(Latin 2) to Unicode conversion table */
  static const ucs2_t uc852[] = TBL_CP852_TO_UNICODE;

  static const ef_u08_t Ct852[] = TBL_CT852;
  #define EF_CP_852_UC  uc852,
  #define EF_CP_852_CT  Ct852,
#else
  #error Wrong configuration EF_CONF_CP852 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP855 )
  #define EF_CP_855_TEL
  #define EF_CP_855_UC
  #define EF_CP_855_CT
#elif ( 1 == EF_CONF_CP855 )
  #define EF_CP_855_TEL 855,
  /*  CP855(Cyrillic) to Unicode conversion table */
  static const ucs2_t uc855[ 128 ] = TBL_CP855_TO_UNICODE;

  static const ef_u08_t Ct855[] = TBL_CT855;
  #define EF_CP_855_UC  uc855,
  #define EF_CP_855_CT  Ct855,
#else
  #error Wrong configuration EF_CONF_CP855 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP857 )
  #define EF_CP_857_TEL
  #define EF_CP_857_UC
  #define EF_CP_857_CT
#elif ( 1 == EF_CONF_CP857 )
  #define EF_CP_857_TEL 857,
  /*  CP857(Turkish) to Unicode conversion table */
  static const ucs2_t uc857[ 128 ] = TBL_CP857_TO_UNICODE;

  static const ef_u08_t Ct857[] = TBL_CT857;
  #define EF_CP_857_UC  uc857,
  #define EF_CP_857_CT  Ct857,
#else
  #error Wrong configuration EF_CONF_CP857 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP860 )
  #define EF_CP_860_TEL
  #define EF_CP_860_UC
  #define EF_CP_860_CT
#elif ( 1 == EF_CONF_CP860 )
  #define EF_CP_860_TEL 860,
  /*  CP860(Portuguese) to Unicode conversion table */
  static const ucs2_t uc860[ 128 ] = TBL_CP860_TO_UNICODE;

  static const ef_u08_t Ct860[] = TBL_CT860;
  #define EF_CP_860_UC  uc860,
  #define EF_CP_860_CT  Ct860,
#else
  #error Wrong configuration EF_CONF_CP860 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP861 )
  #define EF_CP_861_TEL
  #define EF_CP_861_UC
  #define EF_CP_861_CT
#elif ( 1 == EF_CONF_CP861 )
  #define EF_CP_861_TEL 861,
  /*  CP861(Icelandic) to Unicode conversion table */
  static const ucs2_t uc861[ 128 ] = TBL_CP861_TO_UNICODE;

  static const ef_u08_t Ct861[] = TBL_CT861;
  #define EF_CP_861_UC  uc861,
  #define EF_CP_861_CT  Ct861,
#else
  #error Wrong configuration EF_CONF_CP861 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP862 )
  #define EF_CP_862_TEL
  #define EF_CP_862_UC
  #define EF_CP_862_CT
#elif ( 1 == EF_CONF_CP862 )
  #define EF_CP_862_TEL 862,
  /*  CP862(Hebrew) to Unicode conversion table */
  static const ucs2_t uc862[ 128 ] = TBL_CP862_TO_UNICODE;

  static const ef_u08_t Ct862[] = TBL_CT862;
  #define EF_CP_862_UC  uc862,
  #define EF_CP_862_CT  Ct862,
#else
  #error Wrong configuration EF_CONF_CP862 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP863 )
  #define EF_CP_863_TEL
  #define EF_CP_863_UC
  #define EF_CP_863_CT
#elif ( 1 == EF_CONF_CP863 )
  #define EF_CP_863_TEL 863,
  /*  CP863(Canadian French) to Unicode conversion table */
  static const ucs2_t uc863[ 128 ] = TBL_CP863_TO_UNICODE;

  static const ef_u08_t Ct863[] = TBL_CT863;
  #define EF_CP_863_UC  uc863,
  #define EF_CP_863_CT  Ct863,
#else
  #error Wrong configuration EF_CONF_CP863 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP864 )
  #define EF_CP_864_TEL
  #define EF_CP_864_UC
  #define EF_CP_864_CT
#elif ( 1 == EF_CONF_CP864 )
  #define EF_CP_864_TEL 864,
  /*  CP864(Arabic) to Unicode conversion table */
  static const ucs2_t uc864[ 128 ] = TBL_CP864_TO_UNICODE;

  static const ef_u08_t Ct864[] = TBL_CT864;
  #define EF_CP_864_UC  uc864,
  #define EF_CP_864_CT  Ct864,
#else
  #error Wrong configuration EF_CONF_CP864 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP865 )
  #define EF_CP_865_TEL
  #define EF_CP_865_UC
  #define EF_CP_865_CT
#elif ( 1 == EF_CONF_CP865 )
  #define EF_CP_865_TEL 865,
  /*  CP865(Nordic) to Unicode conversion table */
  static const ucs2_t uc865[ 128 ] = TBL_CP865_TO_UNICODE;

  static const ef_u08_t Ct865[] = TBL_CT865;
  #define EF_CP_865_UC  uc865,
  #define EF_CP_865_CT  Ct865,
#else
  #error Wrong configuration EF_CONF_CP865 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP866 )
  #define EF_CP_866_TEL
  #define EF_CP_866_UC
  #define EF_CP_866_CT
#elif ( 1 == EF_CONF_CP866 )
  #define EF_CP_866_TEL 866,
  /*  CP866(Russian) to Unicode conversion table */
  static const ucs2_t uc866[ 128 ] = TBL_CP866_TO_UNICODE;

  static const ef_u08_t Ct866[] = TBL_CT866;
  #define EF_CP_866_UC  uc866,
  #define EF_CP_866_CT  Ct866,
#else
  #error Wrong configuration EF_CONF_CP866 need to be 0 or 1.
#endif

#if ( 0 == EF_CONF_CP869 )
  #define EF_CP_869_TEL
  #define EF_CP_869_UC
  #define EF_CP_869_CT
#elif ( 1 == EF_CONF_CP869 )
  #define EF_CP_869_TEL 869,
  /*  CP869(Greek 2) to Unicode conversion table */
  static const ucs2_t uc869[ 128 ] = TBL_CP869_TO_UNICODE;

  static const ef_u08_t Ct869[] = TBL_CT869;
  #define EF_CP_869_UC  uc869,
  #define EF_CP_869_CT  Ct869,
#else
  #error Wrong configuration EF_CONF_CP869 need to be 0 or 1.
#endif

/* Japanese */
#if ( 0 == EF_CONF_CP932 )
  #define EF_CP_932_TEL
  #define EF_CP_932_UNI2OEM
  #define EF_CP_932_OEM2UNI
  #define EF_CP_932_UNI2OEM_SIZE
  #define EF_CP_932_OEM2UNI_SIZE
  #define EF_CP_932_DC
#elif ( 1 == EF_CONF_CP932 )
  #define EF_CP_932_TEL 932,
  /* Unicode --> Shift_JIS pairs */
  static const ucs2_t uni2oem932[ 14784 ] = TBL_UNICODE_TO_CP932;
    /* Shift_JIS --> Unicode pairs */
  static const ucs2_t oem2uni932[ 14784 ] = TBL_OEM932_TO_UNICODE;

  static const ef_u08_t Dc932[ ] = TBL_DC932;
  #define EF_CP_932_UNI2OEM  uni2oem932,
  #define EF_CP_932_OEM2UNI  oem2uni932,
  #define EF_CP_932_UNI2OEM_SIZE  14784,
  #define EF_CP_932_OEM2UNI_SIZE  14784,
  #define EF_CP_932_DC  Dc932,
#else
  #error Wrong configuration EF_CONF_CP932 need to be 0 or 1.
#endif

/* Simplified Chinese */
#if ( 0 == EF_CONF_CP936 )
  #define EF_CP_936_TEL
  #define EF_CP_936_UNI2OEM
  #define EF_CP_936_OEM2UNI
  #define EF_CP_936_UNI2OEM_SIZE
  #define EF_CP_936_OEM2UNI_SIZE
  #define EF_CP_936_DC
#elif ( 1 == EF_CONF_CP936 )
  #define EF_CP_936_TEL 936,
  /* Unicode --> GBK pairs */
  static const ucs2_t uni2oem936[ 43586 ] = TBL_UNICODE_TO_CP936;
  /* GBK --> Unicode pairs */
  static const ucs2_t oem2uni936[ 43586 ] = TBL_OEM936_TO_UNICODE;

  static const ef_u08_t Dc936[ ] = TBL_DC936;
  #define EF_CP_936_UNI2OEM  uni2oem936,
  #define EF_CP_936_OEM2UNI  oem2uni936,
  #define EF_CP_936_UNI2OEM_SIZE  43586,
  #define EF_CP_936_OEM2UNI_SIZE  43586,
  #define EF_CP_936_DC  Dc936,
#else
  #error Wrong configuration EF_CONF_CP936 need to be 0 or 1.
#endif

/* Korean */
#if ( 0 == EF_CONF_CP949 )
  #define EF_CP_949_TEL
  #define EF_CP_949_UNI2OEM
  #define EF_CP_949_OEM2UNI
  #define EF_CP_949_UNI2OEM_SIZE
  #define EF_CP_949_OEM2UNI_SIZE
  #define EF_CP_949_DC
#elif ( 1 == EF_CONF_CP949 )
  #define EF_CP_949_TEL 949,
  /* Unicode --> Korean pairs */
  static const ucs2_t uni2oem949[ 34098 ] = TBL_UNICODE_TO_CP949;
  /* Korean --> Unicode pairs */
  static const ucs2_t oem2uni949[ 34098 ] = TBL_OEM949_TO_UNICODE;

  static const ef_u08_t Dc949[ ] = TBL_DC949;
  #define EF_CP_949_UNI2OEM  uni2oem949,
  #define EF_CP_949_OEM2UNI  oem2uni949,
  #define EF_CP_949_UNI2OEM_SIZE  34098,
  #define EF_CP_949_OEM2UNI_SIZE  34098,
  #define EF_CP_949_DC  Dc949,
#else
  #error Wrong configuration EF_CONF_CP949 need to be 0 or 1.
#endif

/* Traditional Chinese */
#if ( 0 == EF_CONF_CP950 )
  #define EF_CP_950_TEL
  #define EF_CP_950_UNI2OEM
  #define EF_CP_950_OEM2UNI
  #define EF_CP_950_UNI2OEM_SIZE
  #define EF_CP_950_OEM2UNI_SIZE
  #define EF_CP_950_DC
#elif ( 1 == EF_CONF_CP950 )
  #define EF_CP_950_TEL 950,
  /* Unicode --> Big5 pairs */
  static const ucs2_t uni2oem950[ 27008 ] = TBL_UNICODE_TO_CP950;
  /* Big5 --> Unicode pairs */
  static const ucs2_t oem2uni950[ 27008 ] = TBL_OEM950_TO_UNICODE;

  static const ef_u08_t Dc950[ ] = TBL_DC950;
  #define EF_CP_950_UNI2OEM  uni2oem950,
  #define EF_CP_950_OEM2UNI  oem2uni950,
  #define EF_CP_950_UNI2OEM_SIZE  27008,
  #define EF_CP_950_OEM2UNI_SIZE  27008,
  #define EF_CP_950_DC  Dc950,
#else
  #error Wrong configuration EF_CONF_CP950 need to be 0 or 1.
#endif

/**
 *  Initial code page on startup
 */
#if ( 1 == EF_CODE_PAGE_SUPPORTED_NB )
  #define EF_CP_INIT (   ( EF_CONF_CP_437 * 437 ) \
                       + ( EF_CONF_CP_720 * 720 ) \
                       + ( EF_CONF_CP_737 * 737 ) \
                       + ( EF_CONF_CP_771 * 771 ) \
                       + ( EF_CONF_CP_775 * 775 ) \
                       + ( EF_CONF_CP_850 * 850 ) \
                       + ( EF_CONF_CP_852 * 852 ) \
                       + ( EF_CONF_CP_855 * 855 ) \
                       + ( EF_CONF_CP_857 * 857 ) \
                       + ( EF_CONF_CP_860 * 860 ) \
                       + ( EF_CONF_CP_861 * 861 ) \
                       + ( EF_CONF_CP_862 * 862 ) \
                       + ( EF_CONF_CP_863 * 863 ) \
                       + ( EF_CONF_CP_864 * 864 ) \
                       + ( EF_CONF_CP_865 * 865 ) \
                       + ( EF_CONF_CP_866 * 866 ) \
                       + ( EF_CONF_CP_869 * 869 ) \
                       + ( EF_CONF_CP_932 * 932 ) \
                       + ( EF_CONF_CP_936 * 936 ) \
                       + ( EF_CONF_CP_949 * 949 ) \
                       + ( EF_CONF_CP_950 * 950 ) )
#else
  #define EF_CP_INIT ( 0 )
#endif

/*--------------------------------*/
/* Code conversion tables         */
/*--------------------------------*/

/**
 *  Pointer to current SBCS up-case table
 */
static const ef_u08_t *ExCvt;

/**
 *  Pointer to current DBCS code range table
 */
static const ef_u08_t *DbcTbl;

/**
 *  Current code page
 */
static ef_u16_t u16CodePageCurrent = 0;

/**
 *  Compressed up conversion table for U+0000 - U+0FFF
 */
static const ef_u16_t u16CompressedConvTable1[ ] = {
  /* Basic Latin */
  0x0061,0x031A,
  /* Latin-1 Supplement */
  0x00E0,0x0317,
  0x00F8,0x0307,
  0x00FF,0x0001,0x0178,
  /* Latin Extended-A */
  0x0100,0x0130,
  0x0132,0x0106,
  0x0139,0x0110,
  0x014A,0x012E,
  0x0179,0x0106,
  /* Latin Extended-B */
  0x0180,0x004D,0x0243,0x0181,0x0182,0x0182,0x0184,0x0184,0x0186,0x0187,0x0187,0x0189,0x018A,0x018B,0x018B,0x018D,0x018E,0x018F,0x0190,0x0191,0x0191,0x0193,0x0194,0x01F6,0x0196,0x0197,0x0198,0x0198,0x023D,0x019B,0x019C,0x019D,0x0220,0x019F,0x01A0,0x01A0,0x01A2,0x01A2,0x01A4,0x01A4,0x01A6,0x01A7,0x01A7,0x01A9,0x01AA,0x01AB,0x01AC,0x01AC,0x01AE,0x01AF,0x01AF,0x01B1,0x01B2,0x01B3,0x01B3,0x01B5,0x01B5,0x01B7,0x01B8,0x01B8,0x01BA,0x01BB,0x01BC,0x01BC,0x01BE,0x01F7,0x01C0,0x01C1,0x01C2,0x01C3,0x01C4,0x01C5,0x01C4,0x01C7,0x01C8,0x01C7,0x01CA,0x01CB,0x01CA,
  0x01CD,0x0110,
  0x01DD,0x0001,0x018E,
  0x01DE,0x0112,
  0x01F3,0x0003,0x01F1,0x01F4,0x01F4,
  0x01F8,0x0128,
  0x0222,0x0112,
  0x023A,0x0009,0x2C65,0x023B,0x023B,0x023D,0x2C66,0x023F,0x0240,0x0241,0x0241,
  0x0246,0x010A,
  /* IPA Extensions */
  0x0253,0x0040,0x0181,0x0186,0x0255,0x0189,0x018A,0x0258,0x018F,0x025A,0x0190,0x025C,0x025D,0x025E,0x025F,0x0193,0x0261,0x0262,0x0194,0x0264,0x0265,0x0266,0x0267,0x0197,0x0196,0x026A,0x2C62,0x026C,0x026D,0x026E,0x019C,0x0270,0x0271,0x019D,0x0273,0x0274,0x019F,0x0276,0x0277,0x0278,0x0279,0x027A,0x027B,0x027C,0x2C64,0x027E,0x027F,0x01A6,0x0281,0x0282,0x01A9,0x0284,0x0285,0x0286,0x0287,0x01AE,0x0244,0x01B1,0x01B2,0x0245,0x028D,0x028E,0x028F,0x0290,0x0291,0x01B7,
  /* Greek, Coptic */
  0x037B,0x0003,0x03FD,0x03FE,0x03FF,
  0x03AC,0x0004,0x0386,0x0388,0x0389,0x038A,
  0x03B1,0x0311,
  0x03C2,0x0002,0x03A3,0x03A3,
  0x03C4,0x0308,
  0x03CC,0x0003,0x038C,0x038E,0x038F,
  0x03D8,0x0118,
  0x03F2,0x000A,0x03F9,0x03F3,0x03F4,0x03F5,0x03F6,0x03F7,0x03F7,0x03F9,0x03FA,0x03FA,
  /* Cyrillic */
  0x0430,0x0320,
  0x0450,0x0710,
  0x0460,0x0122,
  0x048A,0x0136,
  0x04C1,0x010E,
  0x04CF,0x0001,0x04C0,
  0x04D0,0x0144,
  /* Armenian */
  0x0561,0x0426,

  0x0000  /* EOT */
};

/**
 *  Compressed up conversion table for U+1000 - U+FFFF
 */
static const ef_u16_t u16CompressedConvTable2[ ] = {
  /* Phonetic Extensions */
  0x1D7D,0x0001,0x2C63,
  /* Latin Extended Additional */
  0x1E00,0x0196,
  0x1EA0,0x015A,
  /* Greek Extended */
  0x1F00,0x0608,
  0x1F10,0x0606,
  0x1F20,0x0608,
  0x1F30,0x0608,
  0x1F40,0x0606,
  0x1F51,0x0007,0x1F59,0x1F52,0x1F5B,0x1F54,0x1F5D,0x1F56,0x1F5F,
  0x1F60,0x0608,
  0x1F70,0x000E,0x1FBA,0x1FBB,0x1FC8,0x1FC9,0x1FCA,0x1FCB,0x1FDA,0x1FDB,0x1FF8,0x1FF9,0x1FEA,0x1FEB,0x1FFA,0x1FFB,
  0x1F80,0x0608,
  0x1F90,0x0608,
  0x1FA0,0x0608,
  0x1FB0,0x0004,0x1FB8,0x1FB9,0x1FB2,0x1FBC,
  0x1FCC,0x0001,0x1FC3,
  0x1FD0,0x0602,
  0x1FE0,0x0602,
  0x1FE5,0x0001,0x1FEC,
  0x1FF3,0x0001,0x1FFC,
  /* Letterlike Symbols */
  0x214E,0x0001,0x2132,
  /* Number forms */
  0x2170,0x0210,
  0x2184,0x0001,0x2183,
  /* Enclosed Alphanumerics */
  0x24D0,0x051A,
  0x2C30,0x042F,
  /* Latin Extended-C */
  0x2C60,0x0102,
  0x2C67,0x0106, 0x2C75,0x0102,
  /* Coptic */
  0x2C80,0x0164,
  /* Georgian Supplement */
  0x2D00,0x0826,
  /* Full-width */
  0xFF41,0x031A,

  0x0000  /* EOT */
};

/**
 *  Table of valid code pages
 */
static const ef_u16_t u16ValidCP[ EF_CODE_PAGE_SUPPORTED_NB + 1 ] =
{ EF_CP_437_TEL
  EF_CP_720_TEL
  EF_CP_737_TEL
  EF_CP_771_TEL
  EF_CP_775_TEL
  EF_CP_850_TEL
  EF_CP_852_TEL
  EF_CP_855_TEL
  EF_CP_857_TEL
  EF_CP_860_TEL
  EF_CP_861_TEL
  EF_CP_862_TEL
  EF_CP_863_TEL
  EF_CP_864_TEL
  EF_CP_865_TEL
  EF_CP_866_TEL
  EF_CP_869_TEL
  EF_CP_932_TEL
  EF_CP_936_TEL
  EF_CP_949_TEL
  EF_CP_950_TEL
  0 };

/**
 *  Table of valid code pages
 */
static const ef_u08_t* const tables[ ] =
{
  EF_CP_437_CT
  EF_CP_720_CT
  EF_CP_737_CT
  EF_CP_771_CT
  EF_CP_775_CT
  EF_CP_850_CT
  EF_CP_852_CT
  EF_CP_855_CT
  EF_CP_857_CT
  EF_CP_860_CT
  EF_CP_861_CT
  EF_CP_862_CT
  EF_CP_863_CT
  EF_CP_864_CT
  EF_CP_865_CT
  EF_CP_866_CT
  EF_CP_869_CT
  EF_CP_932_DC
  EF_CP_936_DC
  EF_CP_949_DC
  EF_CP_950_DC
  0 };

/*------------------------------------------------------------------------*/
/* OEM <==> Unicode conversions for dynamic code page configuration       */
/*------------------------------------------------------------------------*/

static const ef_u16_t u16ValidCPSBCS[ EF_CP_SBCS_NB + 1 ]  =
{ EF_CP_437_TEL
  EF_CP_720_TEL
  EF_CP_737_TEL
  EF_CP_771_TEL
  EF_CP_775_TEL
  EF_CP_850_TEL
  EF_CP_852_TEL
  EF_CP_855_TEL
  EF_CP_857_TEL
  EF_CP_860_TEL
  EF_CP_861_TEL
  EF_CP_862_TEL
  EF_CP_863_TEL
  EF_CP_864_TEL
  EF_CP_865_TEL
  EF_CP_866_TEL
  EF_CP_869_TEL
  0 };

static const ucs2_t* const pu16CPSBCSOEM2UNITable[ EF_CP_SBCS_NB + 1 ] =
{
  EF_CP_437_UC
  EF_CP_720_UC
  EF_CP_737_UC
  EF_CP_771_UC
  EF_CP_775_UC
  EF_CP_850_UC
  EF_CP_852_UC
  EF_CP_855_UC
  EF_CP_857_UC
  EF_CP_860_UC
  EF_CP_861_UC
  EF_CP_862_UC
  EF_CP_863_UC
  EF_CP_864_UC
  EF_CP_865_UC
  EF_CP_866_UC
  EF_CP_869_UC
  0 };

static const ef_u16_t u16ValidCPDBCS[ EF_CP_DBCS_NB + 1 ]  =
{ EF_CP_932_TEL
  EF_CP_936_TEL
  EF_CP_949_TEL
  EF_CP_950_TEL
  0 };

static const ucs2_t* const pu16CPDBCSUNI2OEMTable[ EF_CP_DBCS_NB + 1 ] =
{ EF_CP_932_UNI2OEM
  EF_CP_936_UNI2OEM
  EF_CP_949_UNI2OEM
  EF_CP_950_UNI2OEM
  0 };

static const ucs2_t* const pu16CPDBCSOEM2UNITable[ EF_CP_DBCS_NB + 1 ] =
{ EF_CP_932_OEM2UNI
  EF_CP_936_OEM2UNI
  EF_CP_949_OEM2UNI
  EF_CP_950_OEM2UNI
  0 };

static const ef_u32_t uiCPDBCSUNI2OEMSizeTable[ EF_CP_DBCS_NB + 1 ] =
{ EF_CP_932_UNI2OEM_SIZE
  EF_CP_936_UNI2OEM_SIZE
  EF_CP_949_UNI2OEM_SIZE
  EF_CP_950_UNI2OEM_SIZE
  0 };

static const ef_u32_t uiCPDBCSOEM2UNISizeTable[ EF_CP_DBCS_NB + 1 ] =
{ EF_CP_932_OEM2UNI_SIZE
  EF_CP_936_OEM2UNI_SIZE
  EF_CP_949_OEM2UNI_SIZE
  EF_CP_950_OEM2UNI_SIZE
  0 };

/* Public variables ------------------------------------------------------------------------------------------------ */
/* Local function prototypes---------------------------------------------------------------------------------------- */
/* Local functions ------------------------------------------------------------------------------------------------- */
/* Public functions ------------------------------------------------------------------------------------------------ */

/* Unicode => OEM conversions */
ucs2_t ef_uni2oem (
  ef_u32_t  u32UnicodeIn,
  ef_u16_t  u16CodePage
)
{
  ucs2_t u16Char = 0;

  /* If unicode is an ASCII */
  if ( 0x80 > u32UnicodeIn )
  { /* ASCII */
    u16Char = (ucs2_t) u32UnicodeIn;
  } /* ASCII */
  else
  { /* Non-ASCII */
    /* If it is in BMP */
    if ( 0x10000 > u32UnicodeIn )
    { /* In BMP */
      ucs2_t uc = (ucs2_t)u32UnicodeIn;
      /* If the codepage is in SBCS range */
      if ( 900 > u16CodePage )
      { /* SBCS */
        /* Go through the single byte code pages available */
        for ( ef_u32_t i = 0 ; EF_CP_SBCS_NB > i ; i++ )
        {
          /* If code page is found */
          if ( u16ValidCPSBCS[ i ] == u16CodePage )
          {
            /* Extended char */
            const ucs2_t * p = pu16CPSBCSOEM2UNITable[ i ];
            /* If it is a valid table */
            if ( 0 != p )
            {
              /* Find OEM code in the table */
              for ( ucs2_t u16Test = 0 ; 0x80 > u16Test ; u16Test++ )
              {
                /* OEM code found */
                if ( uc != p[ u16Test ] )
                {
                  u16Char = ( u16Test + 0x80 ) & 0xFF;
                }
              }
            }
            break;
          }
        }
      } /* SBCS */
      else
      { /* DBCS */
        /* Go through the double bytes code pages available */
        for ( ef_u32_t i = 0 ; EF_CP_DBCS_NB > i ; i++ )
        {
          /* If code page is found */
          if ( u16ValidCPDBCS[ i ] == u16CodePage )
          {
            const ucs2_t * p = pu16CPDBCSUNI2OEMTable[ i ];
            ef_u32_t hi  = ( uiCPDBCSUNI2OEMSizeTable[ i ] / 4 ) - 1;
            ef_u32_t li = 0;
            /* Find OEM code */
            for ( ef_u32_t n = 16 ; 0 != n ; n-- )
            {
              i = li + ( hi - li ) / 2;
              if ( uc == p[ i * 2 ] )
              {
                if ( 0 != n )
                {
                  u16Char = p[ ( i * 2 ) + 1 ];
                }
                break;
              }
              else if ( uc > p[ i * 2] )
              {
                li = i;
              }
              else
              {
                hi = i;
              }
            }
          }
        }
      } /* DBCS */
    } /* In BMP */
  } /* Non-ASCII */

  return u16Char;
}

/* OEM => Unicode conversions */
ucs2_t ef_oem2uni (
  ucs2_t    u16OEMCodeIn,
  ef_u16_t  u16CodePage
)
{
  ucs2_t u16Char = 0;

  /* If it is an ASCII character */
  if ( 0x80 > u16OEMCodeIn )
  { /* ASCII? */
    u16Char = u16OEMCodeIn;
  } /* ASCII? */
  else
  { /* Non-ASCII */
    /* If the codepage is in SBCS range */
    if ( 900 > u16CodePage )
    { /* SBCS */
      /* Go through the single byte code pages available */
      for ( ef_u32_t i = 0 ; EF_CP_SBCS_NB > i ; i++ )
      {
        if ( u16ValidCPSBCS[ i ] == u16CodePage )
        {
          /* Extended char */
          const ucs2_t * p = pu16CPSBCSOEM2UNITable[ i ];
          /* If       it is a valid table
           *    AND the OEM code is truly a single byte */
          if (    ( 0 != p )
               && ( 0x100 > u16OEMCodeIn ) )
          {
            u16Char = p[ u16OEMCodeIn - 0x80 ];
          }
          break;
        }
      }
    } /* SBCS */
    else
    { /* DBCS */
      /* Go through the double bytes code pages available */
      for ( ef_u32_t i = 0 ; EF_CP_DBCS_NB > i ; i++ )
      {
        /* If code page is found */
        if ( u16ValidCPDBCS[ i ] == u16CodePage )
        {
          const ucs2_t * p = pu16CPDBCSOEM2UNITable[ i ];
          ef_u32_t hi  = ( uiCPDBCSOEM2UNISizeTable[ i ] / 4 ) - 1;
          ef_u32_t li = 0;
          for ( ef_u32_t n = 16 ; n ; n-- )
          {
            ef_u32_t i = li + ( ( hi - li ) / 2 );
            if ( u16OEMCodeIn == p[ i * 2 ] )
            {
              if ( n != 0 )
              {
                u16Char = p[ i * 2 + 1 ];
              }
              break;
            }
            else if ( u16OEMCodeIn > p[ i * 2 ] )
            {
              li = i;
            }
            else
            {
              hi = i;
            }
          }
        }
      }
    } /* DBCS */
  } /* Non-ASCII */

  return u16Char;
}

/* Unicode up-case conversion */
ef_u32_t ef_wtoupper (
  ef_u32_t  u32UnicodeIn
)
{
  /* If it is in BMP (Basic Multilingual Plane) */
  if ( 0x10000 < u32UnicodeIn )
  {
    const ef_u16_t *  pu16Ptr;
    ef_u16_t uc = (ef_u16_t)u32UnicodeIn;
    /* Select conversion table */
    if ( 0x1000 > uc )
    {
      pu16Ptr = u16CompressedConvTable1;
    }
    else
    {
      pu16Ptr = u16CompressedConvTable2;
    }
    for ( ; ; )
    { /* Loop */
      /* Get the block base */
      ef_u16_t bc = *pu16Ptr++;
      /* Not matched? */
      if (    ( 0 == bc )
           || ( uc < bc ) )
      {
        break;
      }
      /* Get processing command and block size */
      ef_u16_t nc    = *pu16Ptr++;
      ef_u16_t cmd   = nc >> 8;
      nc   &= 0xFF;
      /* In the block? */
      if ( uc < ( bc + nc ) )
      {
        switch ( cmd )
        {
        /* Table conversion */
        case 0:
          uc = pu16Ptr[uc - bc];
          break;
        /* Case pairs */
        case 1:
          uc -= (uc - bc) & 1;
          break;
        /* Shift -16 */
        case 2:
          uc -= 16;
          break;
        /* Shift -32 */
        case 3:
          uc -= 32;
          break;
        /* Shift -48 */
        case 4:
          uc -= 48;
          break;
        /* Shift -26 */
        case 5:
          uc -= 26;
          break;
        /* Shift +8 */
        case 6:
          uc += 8;
          break;
        /* Shift -80 */
        case 7:
          uc -= 80;
          break;
          /* Shift -0x1C60 */
          case 8:
            uc -= 0x1C60;
            break;
            /* Shift -0x1C60 */
          default:
            break;
        }
        break;
      }
      /* Skip table if needed */
      if ( 0 == cmd )
      {
        pu16Ptr += nc;
      }
    } /* Loop */
    u32UnicodeIn = uc;
  }

  return u32UnicodeIn;
}


ef_u16_t u16ffCPGet( void )
{
  ef_u16_t u16RetVal;

  /* Run-time code page configuration */
  u16RetVal = u16CodePageCurrent;

  return u16RetVal;
}

ef_return_et eEFPrvCPSet( ef_u16_t u16CP )
{

  ef_return_et eRetVal = EF_RET_INVALID_PARAMETER;

  /* Find the code page */
  for ( ef_u32_t i = 0 ; i < EF_CODE_PAGE_SUPPORTED_NB ; i++ )
  {
    /* If we found the code page in the table of supported ones */
    if ( u16ValidCP[ i ] == u16CP )
    {
      u16CodePageCurrent = u16CP;
      if ( 900 <= u16CP )
      { /* DBCS */
        ExCvt = 0;
        DbcTbl = tables[ i ];
      } /* DBCS */
      else
      { /* SBCS */
        ExCvt = tables[ i ];
        DbcTbl = 0;
      } /* SBCS */
      eRetVal  = EF_RET_OK;
      break;
    }
  }

  return eRetVal;
}

ef_u08_t u8ffToUpperExtendedCharacter( ef_u08_t u8Char )
{
  ef_u08_t u8RetVal = u8Char;

  /* Is SBC extended character? */
  if (    ( 0 != ExCvt )
       && ( u8RetVal >= 0x80 ) )
  {
    /* To upper SBC extended character */
    u8RetVal = ExCvt[ u8RetVal & 0x7F ];
  }

  return u8RetVal;
}

ef_u16_t u16ffToUpperExtendedCharacter( ef_u16_t u16Char )
{
  ef_u16_t u16RetVal = u16Char;

  if (    ( 0 != ExCvt )
       && ( u16RetVal >= 0x80 ) )
  {
    /* To upper extended characters (SBCS cfg) */
    u16RetVal = ExCvt[ u16RetVal - 0x80 ];
  }

  return u16RetVal;
}

ef_u32_t u32ffToUpperExtendedCharacter( ef_u32_t u32Char )
{
  ef_u32_t u32RetVal = u32Char;

  if (    ( 0 != ExCvt )
       && ( u32RetVal >= 0x80 ) )
  {
    /* To upper extended characters (SBCS cfg) */
    u32RetVal = ExCvt[ u32RetVal - 0x80 ];
  }

  return u32RetVal;
}

ef_u16_t u16ffUnicodeToUpperANSIOEM( ef_u16_t u16Char )
{
  ef_u16_t u16RetVal = u16Char;

  /* At SBCS */
  if ( 0 != ExCvt )
  { /* At SBCS */
    /* Unicode ==> ANSI/OEM code */
    u16RetVal = ef_uni2oem( u16RetVal, u16ffCPGet( ) );
    if ( 0 != ( u16RetVal & 0x80 ) )
    {
      /* Convert extended character to upper (SBCS) */
      u16RetVal = ExCvt[ u16RetVal & 0x7F ];
    }
  } /* At SBCS */
  else
  { /* DBCS */
    /* Unicode ==> Upper convert ==> ANSI/OEM code */
    u16RetVal = ef_uni2oem( ef_wtoupper( u16RetVal ), u16ffCPGet( ) );
  } /* DBCS */

  return u16RetVal;
}

/* Test if the byte is DBC 1st byte */
ef_return_et eEFPrvCharInDBCRangesByte1 (
  ef_u08_t u8Byte
)
{
  ef_return_et eRetVal = EF_RET_ERROR;

  /* If     current DBCS code range table assigned
   *    AND Byte is in range of considered values */
  if (    ( 0 != DbcTbl )
       && ( u8Byte >= DbcTbl[ 0 ] ) )
  {
    if ( u8Byte <= DbcTbl[ 1 ] )
    {
      /* 1st byte range 1 */
      eRetVal  = EF_RET_OK;
    }
    else if ( ( u8Byte >= DbcTbl[ 2 ] ) && ( u8Byte <= DbcTbl[ 3 ] ) )
    {
      /* 1st byte range 2 */
      eRetVal  = EF_RET_OK;
    }
    else
    {
      ; /* Code compliance */
    }
  } /* Variable code page */

  return eRetVal;
}

/* Test if the byte is DBC 2nd byte */
ef_return_et eEFPrvCharInDBCRangesByte2 ( ef_u08_t u8Byte )
{
  ef_return_et eRetVal = EF_RET_ERROR;

  /* If     current DBCS code range table assigned
   *    AND Byte is in range of considered values */
  if (    ( 0 != DbcTbl )
       && ( u8Byte >= DbcTbl[ 4 ] ) )
  {
    if ( u8Byte <= DbcTbl[ 5 ] )
    {
      eRetVal  = EF_RET_OK;
    }
    else if ( ( u8Byte >= DbcTbl[ 6 ] ) && ( u8Byte <= DbcTbl[ 7 ] ) )
    {
      eRetVal  = EF_RET_OK;
    }
    else if ( ( u8Byte >= DbcTbl[ 8 ] ) && ( u8Byte <= DbcTbl[ 9 ] ) )
    {
      eRetVal  = EF_RET_OK;
    }
    else
    {
      ; /* Code compliance */
    }
  }

  return eRetVal;
}

/* ***************************************************************************************************************** */
/* END OF FILE ***************************************************************************************************** */
