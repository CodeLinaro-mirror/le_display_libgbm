/*
* Copyright (c) 2018, 2021 The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
* Changes from Qualcomm Innovation Center are provided under the following license:
* Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
* SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <gbm_priv.h>
#include <msmgbm.h>
#include <msmgbm_common.h>

bool IsImplDefinedFormat(uint32_t format)
{
    if((format == GBM_FORMAT_YCbCr_420_888) ||
           (format == GBM_FORMAT_IMPLEMENTATION_DEFINED))
        return true;
    else
        return false;
}

uint32_t GetDefaultImplDefinedFormat(uint32_t usage_flags, uint32_t format)
{
    uint32_t pixel_format = format;

    if(usage_flags & GBM_BO_USAGE_UBWC_ALIGNED_QTI){
        pixel_format = GBM_FORMAT_YCbCr_420_SP_VENUS_UBWC;
        if (usage_flags & GBM_BO_USAGE_10BIT_QTI) {
          pixel_format = GBM_FORMAT_YCbCr_420_P010_UBWC;
        } else if (usage_flags & GBM_BO_USAGE_10BIT_TP_QTI) {
          pixel_format = GBM_FORMAT_YCbCr_420_TP10_UBWC;
        }
    } else if (usage_flags & GBM_BO_USAGE_10BIT_QTI) {
      pixel_format = GBM_FORMAT_YCbCr_420_P010_VENUS;
    }

    return pixel_format;
}

uint32_t GetImplDefinedFormat(uint32_t usage_flags, uint32_t format)
{
    uint32_t pixel_format = format;


    pixel_format = GetDefaultImplDefinedFormat(usage_flags, pixel_format);
    pixel_format = GetCameraImplDefinedFormat(usage_flags, pixel_format);
    pixel_format = GetVideoImplDefinedFormat(usage_flags, pixel_format);

    /*default if no valid format is set by camera/video*/
    if(IsImplDefinedFormat(pixel_format))
        pixel_format = GBM_FORMAT_NV21_ZSL;

    LOG(LOG_DBG,"%s: format 0x%x\n", __func__, pixel_format);

    return pixel_format;
}
