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
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef __MSMGBM_CAMERA_UTILS_H__
#define __MSMGBM_CAMERA_UTILS_H__

#include <cstdio>
#include <cstddef>
#include <dlfcn.h>
#include <mutex>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include "gbm_priv.h"
#include "gbm.h"
#include "msmgbm.h"

namespace msm_gbm {

class CameraInfo {
 public:

  int GetBufferSize(int format, int width, int height, unsigned int *size);
  int GetUBWCInfo(int format, bool *is_Supported, bool *is_PI, int *version);

  int GetPlaneAlignment(int format, int plane_type, unsigned int *alignment);

  int GetBpp(int format, int *bpp);

  int GetPerPlaneBpp(int format, int plane_type, int *bpp);

  int GetStrideInBytes(int format, int plane_type, int width, int *stride_bytes);

  int GetPixelIncrement(int format, int plane_type, int *pixel_increment);

  int GetPlaneOffset(int format, int plane_type, int width, int height, int *offset);

  int GetSubsamplingFactor(int format, int plane_type, bool isHorizontal, int *subsampling_factor);

  int GetPlaneTypes(int format, PlaneComponent *plane_component_array, int *plane_count);

  int GetScanline(int format, int plane_type, int height, int *scanlines);

  int GetPlaneSize(int format, PlaneComponent plane_type, int width, int height, unsigned int *size);

  int GetCameraFormatPlaneInfo(struct msmgbm_bo *msm_gbm_bo, generic_buf_layout_t *buf_lyt);

  static CameraInfo *GetInstance();

  bool IsCameraCustomFormat(uint32_t format, uint64_t usage);

 private:
  CameraInfo() {}
  ~CameraInfo() {}

  static CameraInfo *s_instance;
  static const int kDefaultStrideAlign = 64;
  static const int kDefaultScanlineAlign = 64;
  static const int kDefaultPlaneAlign = 4096;
};

}  // namespace msm_gbm

#endif  // __MSMGBM_CAMERA_UTILS_H__
