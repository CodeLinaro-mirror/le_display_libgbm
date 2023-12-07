/*
* Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
*
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
* Copyright (c) 2021 - 2023 Qualcomm Innovation Center, Inc. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted (subject to the limitations in the
* disclaimer below) provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*
*     * Neither the name of Qualcomm Innovation Center, Inc. nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
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

#include <unordered_map>
#include <mutex>
#include <msmgbm_dma.h>
#include <unistd.h>
#include <errno.h>

#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>


std::unordered_map<std::string, int32_t> dmabuf_heap_fds_;
static constexpr char kDmaHeapRoot[] = "/dev/dma_heap/";
std::mutex heap_lock_;

int CloseDmabufFds() {
  std::lock_guard<std::mutex> lock(heap_lock_);
  for (auto it : dmabuf_heap_fds_) {
    close(it.second);
  }

  dmabuf_heap_fds_.clear();
  return 0;
}

int OpenDmabufHeap(const std::string& heap_name) {
  std::lock_guard<std::mutex> lock(heap_lock_);

  /* Check if heap has already been opened. */
  auto it = dmabuf_heap_fds_.find(heap_name);
  if (it != dmabuf_heap_fds_.end()) {
    return it->second;
  }

  std::string heap_path = kDmaHeapRoot + heap_name;
  int fd = TEMP_FAILURE_RETRY(open(heap_path.c_str(), O_RDONLY | O_CLOEXEC));
  if (fd < 0) {
    LOG(LOG_ERR,"%s: failed to open dmabuffheap path %s\n", __FUNCTION__, heap_path.c_str());
    return -errno;
  }

  auto ret = dmabuf_heap_fds_.insert({heap_name, fd});
  if (!ret.second) {
    LOG(LOG_WARN,"%s: map insert failed\n", __FUNCTION__);
  }
  return fd;
}

int DmabufSetName(uint64_t dmabuf_fd, const std::string& name) {
  /* dma_buf_set_name truncates instead of returning an error */
  if (name.length() > DMA_BUF_NAME_LEN) {
      errno = ENAMETOOLONG;
      return -1;
  }

  return TEMP_FAILURE_RETRY(ioctl(dmabuf_fd, _IOW(DMA_BUF_BASE, 1, uint64_t), name.c_str()));
}

bool IsCarveoutHeap(uint64_t usage) {
  uint64_t carveout_flags = GBM_BO_ALLOC_CARVEOUT_HEAP_LEFT_QTI |
                            GBM_BO_ALLOC_CARVEOUT_HEAP_RIGHT_QTI |
                            GBM_BO_ALLOC_CARVEOUT_HEAP_DEPTH_QTI |
                            GBM_BO_ALLOC_CARVEOUT_HEAP_MISC_QTI;
  if (usage & carveout_flags) {
    return true;
  }

  return false;
}

void GetHeapInfo(uint64_t usage, std::string *vm_name, std::string *dma_heap_name) {
  bool secure = false;
  bool secure_carveout = false;

  if (usage & GBM_BO_USAGE_PROTECTED_QTI) {
    secure = true;
    *vm_name = IsCarveoutHeap(usage) ? "qcom,cp_pixel" : "";
  }

  std::string heap_name = secure ? "qcom,display" : "system";

  if (usage & GBM_BO_ALLOC_CARVEOUT_HEAP_LEFT_QTI) {
    heap_name = "qcom,lsr_lefteye";
  } else if (usage & GBM_BO_ALLOC_CARVEOUT_HEAP_RIGHT_QTI) {
    heap_name = "qcom,lsr_righteye";
  } else if (usage & GBM_BO_ALLOC_CARVEOUT_HEAP_DEPTH_QTI) {
    heap_name = "qcom,lsr_depth";
  } else if (usage & GBM_BO_ALLOC_CARVEOUT_HEAP_MISC_QTI) {
    heap_name = "qcom,lsr_misc";
  } else if (usage & GBM_BO_USE_RENDERING) {
    heap_name = "system";
    *vm_name = secure ? "qcom,cp_pixel" : "";
  } else if (usage & GBM_BO_ALLOC_SECURE_HEAP_QTI) {
    heap_name = "qcom,secure-pixel";
  } else if ((usage & GBM_BO_ALLOC_SECURE_DISPLAY_HEAP_QTI) ||
             ((usage & GBM_BO_USAGE_PROTECTED_QTI) && (usage & GBM_BO_USAGE_CAMERA_WRITE_QTI))) {
    heap_name = "qcom,display";
  } else if (usage & GBM_BO_ALLOC_ADSP_HEAP_QTI) {
    heap_name = "qcom,adsp";
  }

  *dma_heap_name = heap_name;
  LOG(LOG_DBG,"using dma_heap_name %s\n", heap_name.c_str());

  return;
}

int AllocBuffer(uint64_t usage, uint32_t size, uint32_t align) {
  std::string dma_heap_name;
  std::string vm_name = "";
  int64_t lenddma_handle = 0;
  GetHeapInfo(usage, &vm_name, &dma_heap_name);

  int fd = OpenDmabufHeap(dma_heap_name);
  if (fd < 0) {
    LOG(LOG_ERR,"%s: failed to open dmabuffheap\n", __FUNCTION__);
    return fd;
  }

  struct dma_heap_allocation_data heap_data{
      .len = size,                      // length of data to be allocated in bytes
      .fd_flags = O_RDWR | O_CLOEXEC,   // permissions for the memory to be allocated
  };

  auto ret = TEMP_FAILURE_RETRY(ioctl(fd, DMA_HEAP_IOCTL_ALLOC, &heap_data));
  if (ret < 0) {
      LOG(LOG_ERR,"%s: failed to allocate from heap %s\n", __FUNCTION__, dma_heap_name);
      return ret;
  }

  if (heap_data.fd >= 0) {
      if (DmabufSetName(heap_data.fd, dma_heap_name))
          LOG(LOG_WARN,"%s: Unable to name DMA buffer for: %s\n", __FUNCTION__, dma_heap_name);
  }

  return heap_data.fd;
}
