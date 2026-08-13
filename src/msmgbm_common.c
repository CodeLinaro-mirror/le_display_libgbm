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
* Changes from Qualcomm Technologies, Inc. are provided under the following license:
* Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
* SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <linux/msm_ion.h>
#include <linux/ion.h>
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

    LOG(LOG_DBG,"%s: format %s(0x%x)\n", __func__, get_format_string(pixel_format), pixel_format);

    return pixel_format;
}

static uint32_t GetDefaultIonAllocFlags(uint32_t alloc_flags)
{
    uint32_t ion_flags = 0;

    /*set heap specific flags*/
    if(alloc_flags & GBM_BO_ALLOC_SECURE_HEAP_QTI){
            ion_flags |= ION_FLAG_CP_PIXEL;
    }else if((alloc_flags & GBM_BO_ALLOC_SECURE_DISPLAY_HEAP_QTI) &&
		    !(alloc_flags & GBM_BO_USAGE_CAMERA_WRITE_QTI)){
        /*check for secure display*/
        ion_flags |= ION_FLAG_CP_SEC_DISPLAY;
    }

    /*check if it is secure allocation*/
    if(alloc_flags & GBM_BO_USAGE_PROTECTED_QTI){
        ion_flags |= ION_FLAG_SECURE;
    }

    /*check if uncached buffer is requested*/
    if(!(alloc_flags & GBM_BO_USAGE_UNCACHED_QTI)){
        ion_flags |= ION_FLAG_CACHED;
    }

    return ion_flags;
}

uint32_t GetIonAllocFlags(uint32_t alloc_flags)
{
    FUNCTION_ENTRY();
    uint32_t ion_flags = 0;

    ion_flags |= GetDefaultIonAllocFlags(alloc_flags);
    ion_flags |= GetCameraIonAllocFlags(alloc_flags);
    ion_flags |= GetVideoIonAllocFlags(alloc_flags);

    LOG(LOG_DBG,"%s: ion_flags 0x%x\n", __func__, ion_flags);

    FUNCTION_EXIT();
    return ion_flags;
}

static uint32_t GetDefaultIonHeapId(uint32_t alloc_flags)
{
    uint32_t ion_heap_id = 0;

    if(alloc_flags & GBM_BO_ALLOC_SECURE_HEAP_QTI){
        ion_heap_id = ION_HEAP(ION_SECURE_HEAP_ID);
    }
    if(alloc_flags & GBM_BO_ALLOC_SECURE_DISPLAY_HEAP_QTI){
        ion_heap_id |= ION_HEAP(ION_SECURE_DISPLAY_HEAP_ID);
    }
    if(alloc_flags & GBM_BO_ALLOC_ADSP_HEAP_QTI){
        ion_heap_id |= ION_HEAP(ION_ADSP_HEAP_ID);
    }
    if(alloc_flags & GBM_BO_ALLOC_CAMERA_HEAP_QTI){
        ion_heap_id |= ION_HEAP(ION_CAMERA_HEAP_ID);
    }
    if(alloc_flags & GBM_BO_ALLOC_IOMMU_HEAP_QTI){
        /*IOMMU_HEAP is deprecated, use ION_SYSTEM_HEAP_ID*/
        ion_heap_id |= ION_HEAP(ION_SYSTEM_HEAP_ID);
    }
    if(alloc_flags & GBM_BO_ALLOC_MM_HEAP_QTI){
        ion_heap_id |= ION_HEAP(ION_CP_MM_HEAP_ID);
    }
    if (!ion_heap_id) {
        ion_heap_id = ION_HEAP(ION_SYSTEM_HEAP_ID);
    }

    return ion_heap_id;
}

uint32_t GetIonHeapId(uint32_t alloc_flags)
{
    FUNCTION_ENTRY();
    uint32_t ion_heap_id = 0;

    ion_heap_id |= GetDefaultIonHeapId(alloc_flags);
    ion_heap_id |= GetCameraIonHeapId(alloc_flags);
    ion_heap_id |= GetVideoIonHeapId(alloc_flags);

    LOG(LOG_DBG,"%s: ion_heap_id 0x%x\n", __func__, ion_heap_id);

    FUNCTION_EXIT();
    return ion_heap_id;
}

const char *get_format_string(uint32_t format)
{
    switch(format)
    {
        case GBM_FORMAT_RAW16:
            return "GBM_FORMAT_RAW16";
        case GBM_FORMAT_RAW10:
            return "GBM_FORMAT_RAW10";
        case GBM_FORMAT_RAW12:
            return "GBM_FORMAT_RAW12";
        case GBM_FORMAT_RAW8:
            return "GBM_FORMAT_RAW8";
        case GBM_FORMAT_YV12:
            return "GBM_FORMAT_YV12";
        case GBM_FORMAT_YCbCr_420_SP:
            return "GBM_FORMAT_YCbCr_420_SP";
        case GBM_FORMAT_YCrCb_420_SP:
            return "GBM_FORMAT_YCrCb_420_SP";
        case GBM_FORMAT_YCbCr_422_SP:
            return "GBM_FORMAT_YCbCr_422_SP";
        case GBM_FORMAT_YCrCb_422_SP:
            return "GBM_FORMAT_YCrCb_422_SP";
        case GBM_FORMAT_YCbCr_422_I:
            return "GBM_FORMAT_YCbCr_422_I";
        case GBM_FORMAT_YCrCb_422_I:
            return "GBM_FORMAT_YCrCb_422_I";
        case GBM_FORMAT_YCbCr_420_SP_VENUS:
            return "GBM_FORMAT_YCbCr_420_SP_VENUS";
        case GBM_FORMAT_NV12_ENCODEABLE:
            return "GBM_FORMAT_NV12_ENCODEABLE";
        case GBM_FORMAT_YCrCb_420_SP_VENUS:
            return "GBM_FORMAT_YCrCb_420_SP_VENUS";
        case GBM_FORMAT_NV21_ZSL:
            return "GBM_FORMAT_NV21_ZSL";
        case GBM_FORMAT_BLOB:
            return "GBM_FORMAT_BLOB";
        case GBM_FORMAT_RAW_OPAQUE:
            return "GBM_FORMAT_RAW_OPAQUE";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_4x4_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_4x4_KHR";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_5x4_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_5x4_KHR";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_5x5_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_5x5_KHR";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_6x5_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_6x5_KHR";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_6x6_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_6x6_KHR";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_8x5_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_8x5_KHR";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_8x6_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_8x6_KHR";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_8x8_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_8x8_KHR";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_10x5_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_10x5_KHR";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_10x6_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_10x6_KHR";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_10x8_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_10x8_KHR";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_10x10_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_10x10_KHR";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_12x10_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_12x10_KHR";
        case GBM_FORMAT_COMPRESSED_RGBA_ASTC_12x12_KHR:
            return "GBM_FORMAT_COMPRESSED_RGBA_ASTC_12x12_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR";
        case GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
            return "GBM_FORMAT_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR";
        case GBM_FORMAT_YCbCr_420_SP_VENUS_UBWC:
            return "GBM_FORMAT_YCbCr_420_SP_VENUS_UBWC";
        case GBM_FORMAT_YCbCr_420_888:
            return "GBM_FORMAT_YCbCr_420_888";
        case GBM_FORMAT_IMPLEMENTATION_DEFINED:
            return "GBM_FORMAT_IMPLEMENTATION_DEFINED";
        case GBM_FORMAT_NV12_HEIF:
            return "GBM_FORMAT_NV12_HEIF";
        case GBM_FORMAT_YCbCr_420_P010_VENUS:
            return "GBM_FORMAT_YCbCr_420_P010_VENUS";
        case GBM_FORMAT_NV12_LINEAR_FLEX:
            return "GBM_FORMAT_NV12_LINEAR_FLEX";
        case GBM_FORMAT_NV12_UBWC_FLEX:
            return "GBM_FORMAT_NV12_UBWC_FLEX";
        case GBM_FORMAT_MULTIPLANAR_FLEX:
            return "GBM_FORMAT_MULTIPLANAR_FLEX";
        case GBM_FORMAT_RGBA16161616F:
            return "GBM_FORMAT_RGBA16161616F";
        case GBM_FORMAT_RGB161616F:
            return "GBM_FORMAT_RGB161616F";
        case GBM_FORMAT_RGBA32323232F:
            return "GBM_FORMAT_RGBA32323232F";
        case GBM_FORMAT_RGB323232F:
            return "GBM_FORMAT_RGB323232F";
        case GBM_FORMAT_NV12_UBWC_FLEX_2_BATCH:
            return "GBM_FORMAT_NV12_UBWC_FLEX_2_BATCH";
        case GBM_FORMAT_NV12_UBWC_FLEX_4_BATCH:
            return "GBM_FORMAT_NV12_UBWC_FLEX_4_BATCH";
        case GBM_FORMAT_NV12_UBWC_FLEX_8_BATCH:
            return "GBM_FORMAT_NV12_UBWC_FLEX_8_BATCH";
        case GBM_FORMAT_NV12_FLEX:
            return "GBM_FORMAT_NV12_FLEX";
        case GBM_FORMAT_NV12_FLEX_2_BATCH:
            return "GBM_FORMAT_NV12_FLEX_2_BATCH";
        case GBM_FORMAT_NV12_FLEX_4_BATCH:
            return "GBM_FORMAT_NV12_FLEX_4_BATCH";
        case GBM_FORMAT_NV12_FLEX_8_BATCH:
            return "GBM_FORMAT_NV12_FLEX_8_BATCH";
        case GBM_FORMAT_YCbCr_420_P010_FLEX:
            return "GBM_FORMAT_YCbCr_420_P010_FLEX";
        case GBM_FORMAT_YCbCr_420_P010_FLEX_2_BATCH:
            return "GBM_FORMAT_YCbCr_420_P010_FLEX_2_BATCH";
        case GBM_FORMAT_YCbCr_420_P010_FLEX_4_BATCH:
            return "GBM_FORMAT_YCbCr_420_P010_FLEX_4_BATCH";
        case GBM_FORMAT_YCbCr_420_P010_FLEX_8_BATCH:
            return "GBM_FORMAT_YCbCr_420_P010_FLEX_8_BATCH";
        case GBM_FORMAT_YCbCr_420_TP10_UBWC_FLEX:
            return "GBM_FORMAT_YCbCr_420_TP10_UBWC_FLEX";
        case GBM_FORMAT_YCbCr_420_TP10_UBWC_FLEX_2_BATCH:
            return "GBM_FORMAT_YCbCr_420_TP10_UBWC_FLEX_2_BATCH";
        case GBM_FORMAT_YCbCr_420_TP10_UBWC_FLEX_4_BATCH:
            return "GBM_FORMAT_YCbCr_420_TP10_UBWC_FLEX_4_BATCH";
        case GBM_FORMAT_YCbCr_420_TP10_UBWC_FLEX_8_BATCH:
            return "GBM_FORMAT_YCbCr_420_TP10_UBWC_FLEX_8_BATCH";
        case GBM_FORMAT_YCbCr_420_P010_512:
            return "GBM_FORMAT_YCbCr_420_P010_512";
        case GBM_FORMAT_YCbCr_420_TP10_UBWC:
            return "GBM_FORMAT_YCbCr_420_TP10_UBWC";
        case GBM_FORMAT_YCbCr_420_P010_UBWC:
            return "GBM_FORMAT_YCbCr_420_P010_UBWC";
        case GBM_FORMAT_P010:
            return "GBM_FORMAT_P010";
        case GBM_FORMAT_R8:
            return "GBM_FORMAT_R8";
        case GBM_FORMAT_RG88:
            return "GBM_FORMAT_RG88";
        case GBM_FORMAT_R16:
            return "GBM_FORMAT_R16";
        case GBM_FORMAT_RG1616:
            return "GBM_FORMAT_RG1616";
        case GBM_FORMAT_RGB332:
            return "GBM_FORMAT_RGB332";
        case GBM_FORMAT_BGR233:
            return "GBM_FORMAT_BGR233";
        case GBM_FORMAT_XRGB4444:
            return "GBM_FORMAT_XRGB4444";
        case GBM_FORMAT_XBGR4444:
            return "GBM_FORMAT_XBGR4444";
        case GBM_FORMAT_RGBX4444:
            return "GBM_FORMAT_RGBX4444";
        case GBM_FORMAT_BGRX4444:
            return "GBM_FORMAT_BGRX4444";
        case GBM_FORMAT_ARGB4444:
            return "GBM_FORMAT_ARGB4444";
        case GBM_FORMAT_ABGR4444:
            return "GBM_FORMAT_ABGR4444";
        case GBM_FORMAT_RGBA4444:
            return "GBM_FORMAT_RGBA4444";
        case GBM_FORMAT_BGRA4444:
            return "GBM_FORMAT_BGRA4444";
        case GBM_FORMAT_XRGB1555:
            return "GBM_FORMAT_XRGB1555";
        case GBM_FORMAT_XBGR1555:
            return "GBM_FORMAT_XBGR1555";
        case GBM_FORMAT_ARGB1555:
            return "GBM_FORMAT_ARGB1555";
        case GBM_FORMAT_ABGR1555:
            return "GBM_FORMAT_ABGR1555";
        case GBM_FORMAT_RGBX5551:
            return "GBM_FORMAT_RGBX5551";
        case GBM_FORMAT_BGRX5551:
            return "GBM_FORMAT_BGRX5551";
        case GBM_FORMAT_RGBA5551:
            return "GBM_FORMAT_RGBA5551";
        case GBM_FORMAT_BGRA5551:
            return "GBM_FORMAT_BGRA5551";
        case GBM_FORMAT_RGB565:
            return "GBM_FORMAT_RGB565";
        case GBM_FORMAT_BGR565:
            return "GBM_FORMAT_BGR565";
        case GBM_FORMAT_RGB888:
            return "GBM_FORMAT_RGB888";
        case GBM_FORMAT_BGR888:
            return "GBM_FORMAT_BGR888";
        case GBM_FORMAT_XRGB8888:
            return "GBM_FORMAT_XRGB8888";
        case GBM_FORMAT_XBGR8888:
            return "GBM_FORMAT_XBGR8888";
        case GBM_FORMAT_RGBX8888:
            return "GBM_FORMAT_RGBX8888";
        case GBM_FORMAT_BGRX8888:
            return "GBM_FORMAT_BGRX8888";
        case GBM_FORMAT_ARGB8888:
            return "GBM_FORMAT_ARGB8888";
        case GBM_FORMAT_ABGR8888:
            return "GBM_FORMAT_ABGR8888";
        case GBM_FORMAT_BGRA8888:
            return "GBM_FORMAT_BGRA8888";
        case GBM_FORMAT_RGBA8888:
            return "GBM_FORMAT_RGBA8888";
        case GBM_FORMAT_XRGB2101010:
            return "GBM_FORMAT_XRGB2101010";
        case GBM_FORMAT_XBGR2101010:
            return "GBM_FORMAT_XBGR2101010";
        case GBM_FORMAT_RGBX1010102:
            return "GBM_FORMAT_RGBX1010102";
        case GBM_FORMAT_BGRX1010102:
            return "GBM_FORMAT_BGRX1010102";
        case GBM_FORMAT_ARGB2101010:
            return "GBM_FORMAT_ARGB2101010";
        case GBM_FORMAT_ABGR2101010:
            return "GBM_FORMAT_ABGR2101010";
        case GBM_FORMAT_RGBA1010102:
            return "GBM_FORMAT_RGBA1010102";
        case GBM_FORMAT_BGRA1010102:
            return "GBM_FORMAT_BGRA1010102";
        case GBM_FORMAT_YUYV:
            return "GBM_FORMAT_YUYV";
        case GBM_FORMAT_YVYU:
            return "GBM_FORMAT_YVYU";
        case GBM_FORMAT_UYVY:
            return "GBM_FORMAT_UYVY";
        case GBM_FORMAT_VYUY:
            return "GBM_FORMAT_VYUY";
        case GBM_FORMAT_AYUV:
            return "GBM_FORMAT_AYUV";
        case GBM_FORMAT_NV12:
            return "GBM_FORMAT_NV12";
        case GBM_FORMAT_NV21:
            return "GBM_FORMAT_NV21";
        case GBM_FORMAT_NV16:
            return "GBM_FORMAT_NV16";
        case GBM_FORMAT_NV61:
            return "GBM_FORMAT_NV61";
        case GBM_FORMAT_YUV410:
            return "GBM_FORMAT_YUV410";
        case GBM_FORMAT_YVU410:
            return "GBM_FORMAT_YVU410";
        case GBM_FORMAT_YUV411:
            return "GBM_FORMAT_YUV411";
        case GBM_FORMAT_YVU411:
            return "GBM_FORMAT_YVU411";
        case GBM_FORMAT_YUV420:
            return "GBM_FORMAT_YUV420";
        case GBM_FORMAT_YVU420:
            return "GBM_FORMAT_YVU420";
        case GBM_FORMAT_YUV422:
            return "GBM_FORMAT_YUV422";
        case GBM_FORMAT_YVU422:
            return "GBM_FORMAT_YVU422";
        case GBM_FORMAT_YUV444:
            return "GBM_FORMAT_YUV444";
        case GBM_FORMAT_YVU444:
            return "GBM_FORMAT_YVU444";
        default:
            return "UNKNOWN";
    }
}
