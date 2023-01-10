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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <gbm_utils.h>
#include <msmgbm.h>
#include <msmgbm_platform_wrapper.h>
#include <msmgbm_dma.h>
#include <msmgbm_camera_utils.h>

namespace msm_gbm {

int GbmUtils::GetFormatLayout(gbm_buf_desc descriptor, generic_buf_layout_t *buf_lyt,
                              uint64_t *size) {
  if (!buf_lyt || !size) {
    return GBM_ERROR_BAD_VALUE;
  }

  unsigned int alignedw , alignedh;
  struct gbm_bufdesc bufdesc = {descriptor.width, descriptor.height,
                                descriptor.format, descriptor.usage};
  qry_aligned_wdth_hght(&bufdesc, &alignedw, &alignedh);
  *size = qry_size(&bufdesc, alignedw, alignedh);

  struct gbm_bo bo;
  // Fill required info in the bo
  bo.format = descriptor.format;
  bo.width = descriptor.width;
  bo.height = descriptor.height;
  bo.usage_flags = descriptor.usage;
  bo.aligned_width = alignedw;
  bo.aligned_height = alignedh;

  gbm_perform(GBM_PERFORM_GET_YUV_PLANE_INFO, &bo, buf_lyt);

  return GBM_ERROR_NONE;
}



int GbmUtils::GetWidth(gbm_buf_desc descriptor, uint64_t *aligned_width) {
  if (!aligned_width) {
    return GBM_ERROR_BAD_VALUE;
  }

  unsigned int alignedw , alignedh;
  struct gbm_bufdesc bufdesc = {descriptor.width, descriptor.height,
                                descriptor.format, descriptor.usage};
  qry_aligned_wdth_hght(&bufdesc, &alignedw, &alignedh);
  *aligned_width = alignedw;

  return GBM_ERROR_NONE;
}

int GbmUtils::GetHeight(gbm_buf_desc descriptor, uint64_t *aligned_height) {
  if (!aligned_height) {
    return GBM_ERROR_BAD_VALUE;
  }

  unsigned int alignedw , alignedh;
  struct gbm_bufdesc bufdesc = {descriptor.width, descriptor.height,
                                descriptor.format, descriptor.usage};
  qry_aligned_wdth_hght(&bufdesc, &alignedw, &alignedh);
  *aligned_height = alignedh;

  return GBM_ERROR_NONE;
}

int GbmUtils::GetSize(gbm_buf_desc descriptor, uint64_t *size) {
  if (!size) {
    return GBM_ERROR_BAD_VALUE;
  }

  unsigned int alignedw , alignedh;
  struct gbm_bufdesc bufdesc = {descriptor.width, descriptor.height,
                                descriptor.format, descriptor.usage};
  qry_aligned_wdth_hght(&bufdesc, &alignedw, &alignedh);
  *size = qry_size(&bufdesc, alignedw, alignedh);

  return GBM_ERROR_NONE;
}


int GbmUtils::GetAlignedWidthAndHeight(gbm_buf_desc descriptor, uint64_t *aligned_width,
                                       uint64_t *aligned_height) {
  if (!aligned_width || !aligned_height) {
    return GBM_ERROR_BAD_VALUE;
  }

  unsigned int alignedw , alignedh;
  struct gbm_bufdesc bufdesc = {descriptor.width, descriptor.height,
                                descriptor.format, descriptor.usage};
  qry_aligned_wdth_hght(&bufdesc, &alignedw, &alignedh);
  *aligned_width = alignedw;
  *aligned_height = alignedh;

  return GBM_ERROR_NONE;
}

native_handle_t* GbmUtils::AllocateNativeHandle(gbm_bo *bo) {
  if (!bo) {
    LOG(LOG_ERR,"Failed to allocate native handle bo is null\n");
    return NULL;
  }

  struct gbm_handle *handle = new gbm_handle();
  if (!handle) {
    LOG(LOG_ERR,"gbm handle creation failed\n");
    return NULL;
  }

  handle->bo = bo;
  // NOTE: Client should not close this fd as this FD is created by gbm
  // during gbm_bo_destroy FD will be closed.
  handle->data[0] = bo->ion_fd;

  return reinterpret_cast< native_handle_t *>(handle);
}

gbm_bo * GbmUtils::GetGbmBo(native_handle_t *native_handle) {
  if (!native_handle) {
    LOG(LOG_ERR,"native handle is null\n");
    return NULL;
  }

  struct gbm_handle *handle = reinterpret_cast<struct gbm_handle *>(native_handle);
  return handle->bo;
}

void GbmUtils::FreeNativeHandle(native_handle_t *native_handle) {
  delete reinterpret_cast<struct gbm_handle *>(native_handle);
}

int GbmUtils::LendBufferToSecureVM(struct gbm_bo *bo, std::string vm_name,
                                   int64_t *lenddma_handle) {
  int ret = GBM_ERROR_NONE;
  struct meta_data_t *mt_data = nullptr;
  if (bo == nullptr) {
    LOG(LOG_ERR,"Bo is null\n");
    return GBM_ERROR_BAD_HANDLE;
  }

  if(msmgbm_bo_cpu_unmap(bo) != GBM_ERROR_NONE) {
    LOG(LOG_ERR," munmap failed for cpuaddr=0x%x\n", msmgbm_bo_cpu_map(bo));
    return GBM_ERROR_BAD_HANDLE;
  }

  ret = LendBufferToSecure(bo->ion_fd, vm_name, lenddma_handle);
  if (ret) {
    LOG(LOG_ERR,"Failed to LendBufferToSecure\n");
    return ret;
  }

  mt_data = (struct meta_data_t *)msmgbm_bo_meta_map(bo);
  if (mt_data) {
    mt_data->is_buffer_secure = true;
  } else {
    LOG(LOG_ERR,"mt_data is null\n");
    return GBM_ERROR_NO_RESOURCES;
  }

  return ret;
}

int GbmUtils::ReclaimBufferFromSecureVM(struct gbm_bo *bo, int64_t lenddma_handle) {
  int ret = GBM_ERROR_NONE;
  struct meta_data_t *mt_data = nullptr;
  if (bo == nullptr) {
    LOG(LOG_ERR,"Bo is null\n");
    return GBM_ERROR_BAD_HANDLE;
  }

  ret = ReclaimBufferFromSecure(bo->ion_fd, lenddma_handle);
  if (ret) {
    LOG(LOG_ERR,"Failed to LendBufferToSecure\n");
    return ret;
  }

  mt_data = (struct meta_data_t *)msmgbm_bo_meta_map(bo);
  if (mt_data) {
    mt_data->is_buffer_secure = false;
  } else {
    LOG(LOG_ERR,"mt_data is NULL\n");
    return GBM_ERROR_NO_RESOURCES;
  }

  if (msmgbm_bo_cpu_map(bo) == nullptr) {
    LOG(LOG_ERR,"Failed to map the buffer\n");
    return GBM_ERROR_NO_RESOURCES;
  }

  return ret;
}

}  // namespace msm_gbm
