/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.
Part of 6.3.0

FILE NAME
    anc_licence_check.c

DESCRIPTION

*/

#include <csrtypes.h>
#include <feature.h>
#include "anc_licence_check.h"

bool ancLicenceCheckIsAncLicenced(void)
{
    return FeatureVerifyLicense(FEATURE_ANC) || FeatureVerifyLicense(FEATURE_HALF_STEREO_ANC);
}

