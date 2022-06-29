/*
* Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted (subject to the limitations in the
* disclaimer below) provided that the following conditions are met:
*
*    * Redistributions of source code must retain the above copyright
*      notice, this list of conditions and the following disclaimer.
*
*    * Redistributions in binary form must reproduce the above
*      copyright notice, this list of conditions and the following
*      disclaimer in the documentation and/or other materials provided
*      with the distribution.
*
*    * Neither the name of Qualcomm Innovation Center, Inc. nor the names of its
*      contributors may be used to endorse or promote products derived
*      from this software without specific prior written permission.
*
* NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
* GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
* HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
* IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _GBM_UTILS_H_
#define _GBM_UTILS_H_

#include <stdint.h>
#include <string>

#include <errno.h>
#include <sys/mman.h>
#include <cutils/native_handle.h>
#include <gbm.h>
#include <gbm_priv.h>

namespace msm_gbm {

struct gbm_buf_desc {
  uint32_t width;
  uint32_t height;
  uint32_t format;
  uint32_t usage;
  uint32_t flags;
};

struct gbm_handle : native_handle_t {
  gbm_bo *bo;
};

class GbmUtils {
 public:
  int GetFormatLayout(gbm_buf_desc descriptor, generic_buf_layout_t *buf_lyt, uint64_t *size);
  int GetWidth(gbm_buf_desc descriptor, uint64_t *aligned_width);
  int GetHeight(gbm_buf_desc descriptor, uint64_t *aligned_height);
  int GetSize(gbm_buf_desc descriptor, uint64_t *size);
  native_handle_t* AllocateNativeHandle(gbm_bo *bo);
  gbm_bo *GetGbmBo(native_handle_t *native_handle);
  void FreeNativeHandle(native_handle_t *native_handle);
  int LendBufferToSecureVM(struct gbm_bo *bo, std::string vm_name, int64_t *lenddma_handle);
  int ReclaimBufferFromSecureVM(struct gbm_bo *bo, int64_t lenddma_handle);
};

}  // namespace msm_gbm

#endif
