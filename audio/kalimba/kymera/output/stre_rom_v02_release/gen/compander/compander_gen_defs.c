// -----------------------------------------------------------------------------
// Copyright (c) 2018                  Qualcomm Technologies International, Ltd.
//
#include "compander_gen_c.h"

#ifndef __GNUC__ 
_Pragma("datasection CONST")
#endif /* __GNUC__ */

static unsigned defaults_companderCOMPANDER[] = {
   0x00000000u,			// COMPANDER_CONFIG
   0x00000006u,			// NUM_SECTIONS
   0x30000000u,			// GAIN_RATIO_SECTION1
   0x08000000u,			// GAIN_RATIO_SECTION2
   0x04000000u,			// GAIN_RATIO_SECTION3
   0x08000000u,			// GAIN_RATIO_SECTION4
   0x08000000u,			// GAIN_RATIO_SECTION5
   0xF2B65A9Bu,			// GAIN_THRESHOLD_SECTION1
   0xD02A0BA2u,			// GAIN_THRESHOLD_SECTION2
   0xCD81B867u,			// GAIN_THRESHOLD_SECTION3
   0xC2E0663Cu,			// GAIN_THRESHOLD_SECTION4
   0xBAE7674Du,			// GAIN_THRESHOLD_SECTION5
   0x550A6762u,			// GAIN_KNEEWIDTH_SECTION1
   0x15426FE7u,			// GAIN_KNEEWIDTH_SECTION2
   0x15426FE7u,			// GAIN_KNEEWIDTH_SECTION3
   0x00000000u,			// GAIN_KNEEWIDTH_SECTION4
   0x00000000u,			// GAIN_KNEEWIDTH_SECTION5
   0x00147AE1u,			// GAIN_ATTACK_TC
   0x02666666u,			// GAIN_RELEASE_TC
   0x000014F9u,			// LEVEL_ATTACK_TC
   0x0020C49Cu,			// LEVEL_RELEASE_TC
   0x00147AE1u,			// LEVEL_AVERAGE_TC
   0x00000000u,			// MAKEUP_GAIN
   0x00000000u,			// LOOKAHEAD_TIME
   0x00000001u,			// LEVEL_ESTIMATION_FLAG
   0x00000001u,			// GAIN_UPDATE_FLAG
   0x00000001u,			// GAIN_INTERP_FLAG
   0xFFFD7DCDu,			// SOFT_KNEE_1_COEFF_A
   0xFFFA0000u,			// SOFT_KNEE_1_COEFF_B
   0xFFF75CEDu,			// SOFT_KNEE_1_COEFF_C
   0xFFF3F574u,			// SOFT_KNEE_2_COEFF_A
   0xFF760000u,			// SOFT_KNEE_2_COEFF_B
   0xFE5EAECBu,			// SOFT_KNEE_2_COEFF_C
   0x000C0A8Cu,			// SOFT_KNEE_3_COEFF_A
   0x009E0000u,			// SOFT_KNEE_3_COEFF_B
   0x01EC648Eu,			// SOFT_KNEE_3_COEFF_C
   0x00000000u,			// SOFT_KNEE_4_COEFF_A
   0x00040000u,			// SOFT_KNEE_4_COEFF_B
   0x00000000u,			// SOFT_KNEE_4_COEFF_C
   0x00000000u,			// SOFT_KNEE_5_COEFF_A
   0x00040000u,			// SOFT_KNEE_5_COEFF_B
   0x00000000u			// SOFT_KNEE_5_COEFF_C
};

unsigned *COMPANDER_GetDefaults(unsigned capid){
	switch(capid){
		case 0x0092: return defaults_companderCOMPANDER;
		case 0x4057: return defaults_companderCOMPANDER;
	}
	return((unsigned *)0);
}