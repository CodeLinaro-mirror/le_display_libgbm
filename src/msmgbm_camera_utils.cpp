/*
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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
 *
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "msmgbm_camera_utils.h"

#define ALIGN(x, align) (((x) + ((align)-1)) & ~((align)-1))

using std::lock_guard;
using std::mutex;

namespace msm_gbm {
CameraInfo *CameraInfo::s_instance = nullptr;

CameraInfo *CameraInfo::GetInstance() {
  static mutex s_lock;
  lock_guard<mutex> obj(s_lock);
  if (!s_instance) {
    s_instance = new CameraInfo();
  }

  return s_instance;
}

int CameraInfo::GetBufferSize(int format, int width, int height, unsigned int *size) {
  PlaneComponent plane_types[MAX_NUM_OF_PLANES] = {};
  int num_planes;
  int result = GetPlaneTypes(format, plane_types, &num_planes);
  *size = 0;
  for (int i = 0; i < num_planes; i++) {
    unsigned int plane_size = 0;
    GetPlaneSize(format, plane_types[i], width, height, &plane_size);
    LOG(LOG_DBG, "plane %d size %u \n", i, plane_size);
    *size += plane_size;
  }
  *size = ALIGN(*size, kDefaultPlaneAlign);
  LOG(LOG_DBG, "Camera custom format size %d\n", *size);

  return 0;
}


int CameraInfo::GetStrideInBytes(int format, int plane_type, int width, int *stride_bytes) {
  int stride = 0;
  int bpp = 0;
  GetPerPlaneBpp(format, plane_type, &bpp);
  switch (format) {
    case GBM_FORMAT_NV21_ZSL:
      stride = ALIGN(width, kDefaultStrideAlign);
      *stride_bytes = stride * bpp / 8;
      break;
    default:
      LOG(LOG_ERR, "Unhandled format %s for GetStrideInBytes\n", get_msmgbm_format_name(format));
      break;
  }

  return 0;
}

int CameraInfo::GetPixelIncrement(int format, int plane_type, int *pixel_increment) {
  return 0;
}

int CameraInfo::GetPlaneOffset(int format, int plane_type, int width, int height, int *offset) {
  return 0;
}

int CameraInfo::GetSubsamplingFactor(int format, int plane_type, bool isHorizontal,
                                     int *subsampling_factor) {
  return 0;
}

int CameraInfo::GetPlaneTypes(int format, PlaneComponent *plane_component_array,
                              int *plane_count) {
  switch (format) {
    case GBM_FORMAT_NV21_ZSL:
      plane_component_array[0] = PLANE_COMPONENT_Y;
      plane_component_array[1] = PLANE_COMPONENT_Cb;
      *plane_count = 2;
      break;
    default:
      *plane_count = 0;
      LOG(LOG_ERR, "Unhandled format %s for GetPlaneTypes\n", get_msmgbm_format_name(format));
      break;
  }

  return 0;
}

int CameraInfo::GetScanline(int format, int plane_type, int height, int *scanlines) {
  switch (format) {
  case GBM_FORMAT_NV21_ZSL:
    if (plane_type == PLANE_COMPONENT_Y) {
      *scanlines = ALIGN(height, kDefaultScanlineAlign);
    } else if ((plane_type == PLANE_COMPONENT_Cb) || (plane_type == PLANE_COMPONENT_Cr)) {
      *scanlines = ALIGN((height >> 1), kDefaultScanlineAlign);
    }
    break;
  default:
    LOG(LOG_ERR, "Unhandled format %s for GetScanline\n", get_msmgbm_format_name(format));
    break;
  }

  return 0;
}

int CameraInfo::GetPlaneSize(int format, PlaneComponent plane_type, int width, int height,
                             unsigned int *size) {
  *size = 0;
  int stride_bytes = 0;
  int scanlines = 0;
  uint32_t alignment = 0;
  GetStrideInBytes(format, plane_type, width, &stride_bytes);
  GetScanline(format, plane_type, height, &scanlines);
  GetPlaneAlignment(format, plane_type, &alignment);
  *size = ALIGN(stride_bytes * scanlines, alignment);

  return 0;
}

int CameraInfo::GetPlaneAlignment(int format, int plane_type, unsigned int *alignment) {
  switch (format) {
    case GBM_FORMAT_NV21_ZSL:
      if ((plane_type == PLANE_COMPONENT_Y) ||
          (plane_type == PLANE_COMPONENT_Cb) ||
          (plane_type == PLANE_COMPONENT_Cr)) {
        *alignment  = kDefaultPlaneAlign;
      }
      break;
    default:
      *alignment = 1;
      LOG(LOG_ERR, "Unhandled format %s for GetPlaneAlignment\n", get_msmgbm_format_name(format));
      return -1;
  }

  return 0;
}

int CameraInfo::GetBpp(int format, int *bpp) {
  switch (format) {
    case GBM_FORMAT_NV21_ZSL:
      *bpp = 12;
      break;
    case GBM_FORMAT_NV12_UBWC_FLEX:
    case GBM_FORMAT_NV12_UBWC_FLEX_2_BATCH:
    case GBM_FORMAT_NV12_UBWC_FLEX_4_BATCH:
    case GBM_FORMAT_NV12_UBWC_FLEX_8_BATCH:
      *bpp = 1;
      break;
    default:
      LOG(LOG_ERR, "Unhandled format %s for GetBpp\n", get_msmgbm_format_name(format));
      *bpp = 0;
      return -1;
  }

  return 0;
}

int CameraInfo::GetPerPlaneBpp(int format, int plane_type, int *bpp) {
  switch (format) {
    case GBM_FORMAT_NV21_ZSL:
      if ((plane_type == PLANE_COMPONENT_Y) ||
          (plane_type == PLANE_COMPONENT_Cb) ||
          (plane_type == PLANE_COMPONENT_Cr)) {
        *bpp  = 8;
      }
      break;
    default:
      LOG(LOG_ERR, "Unhandled format %s for GetPerPlaneBpp\n", get_msmgbm_format_name(format));
      *bpp = 0;
      return -1;
  }

  return 0;
}

bool CameraInfo::IsCameraCustomFormat(uint32_t format, uint64_t usage) {
  if (usage & GBM_BO_USAGE_HW_RENDERING_QTI) {
    LOG(LOG_DBG, "GBM_BO_USAGE_HW_RENDERING_QTI flag is set for camera custom format");
    return false;
  }
  switch (format) {
    case GBM_FORMAT_NV21_ZSL:
      return true;
    default:
      break;
  }

  return false;
}

int CameraInfo::GetCameraFormatPlaneInfo(struct msmgbm_bo *msm_gbm_bo,
                                         generic_buf_layout_t *buf_lyt) {
  int result = 0;
  struct gbm_bo &bo = msm_gbm_bo->base;
  buf_lyt->pixel_format = bo.format;

  int bpp = 0;
  result = GetBpp(bo.format, &bpp);
  if (result != 0) {
    LOG(LOG_ERR, "Failed to get bpp. Error code : %d", result);
    return result;
  }
  PlaneComponent plane_type[MAX_NUM_OF_PLANES] = {};
  result = GetPlaneTypes(bo.format, plane_type, (int *)&buf_lyt->num_planes);
  if (result != 0) {
    LOG(LOG_ERR, "Failed to get the plane types. Error code : %d", result);
    return result;
  }

  for (int i = 0; i < buf_lyt->num_planes; i++) {
    buf_lyt->planes[i].bits_per_component = bpp;
    int h_subsampling = 0;
    result = GetSubsamplingFactor(bo.format, plane_type[i], true, &h_subsampling);
    if (result != 0) {
      LOG(LOG_ERR, "Failed to get horizontal subsampling factor. plane_type = %d, Error code : %d",
                    plane_type[i], result);
      break;
    }
    buf_lyt->planes[i].h_subsampling = (int32_t)h_subsampling;

    int v_subsampling = 0;
    result = GetSubsamplingFactor(bo.format, plane_type[i], false, &v_subsampling);
    if (result != 0) {
      LOG(LOG_ERR, "Failed to get vertical subsampling factor. plane_type = %d, Error code : %d",
                    plane_type[i], result);
      break;
    }
    buf_lyt->planes[i].v_subsampling = (int32_t)v_subsampling;

    int offset = 0;
    result = GetPlaneOffset(bo.format, plane_type[i], bo.width, bo.height, &offset);
    if (result != 0) {
      LOG(LOG_ERR, "Failed to get plane offset. plane_type = %d, Error code : %d",
                    plane_type[i], result);
      break;
    }
    buf_lyt->planes[i].offset = (int32_t)offset;

    int step = 0;
    result = GetPixelIncrement(bo.format, plane_type[i], &step);
    if (result != 0) {
      LOG(LOG_ERR, "Failed to get pixel increment. plane_type = %d, Error code : %d",
                    plane_type[i], result);
      break;
    }
    buf_lyt->planes[i].h_increment = (int32_t)step * bpp;

    int stride_bytes = 0;
    result = GetStrideInBytes(bo.format, plane_type[i], bo.width, &stride_bytes);
    if (result != 0) {
      LOG(LOG_ERR, "Failed to get stride in bytes. plane_type = %d, Error code : %d",
                    plane_type[i], result);
      break;
    }
    buf_lyt->planes[i].stride = (int32_t)stride_bytes;

    int scanlines = 0;
    result = GetScanline(bo.format, plane_type[i], bo.height, &scanlines);
    if (result != 0) {
      LOG(LOG_ERR, "Failed to get scanlines. plane_type = %d, Error code : %d",
                    plane_type[i], result);
      break;
    }
    buf_lyt->planes[i].v_increment = (int32_t)scanlines;

    unsigned int plane_size = 0;
    result = GetPlaneSize(bo.format, plane_type[i], bo.width, bo.height, &plane_size);
    if (result != 0) {
      LOG(LOG_ERR, "Failed to get plane size. plane_type = %d, Error code : %d",
                    plane_type[i], result);
      break;
    }
    buf_lyt->planes[i].size = (uint32_t)plane_size;
  }

  return result;
}

}  // namespace msm_gbm
