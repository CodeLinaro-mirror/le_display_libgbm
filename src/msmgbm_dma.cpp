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
*
* ​​​​​Changes from Qualcomm Technologies, Inc. are provided under the following license:
* Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
* SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#include <BufferAllocator/BufferAllocator.h>
#include <msmgbm_dma.h>

BufferAllocator buffer_allocator_;
std::unique_ptr<VmMem> vm_mem_ = nullptr;

int Init() {
  if (vm_mem_) {
    return 0;
  }

  vm_mem_ = VmMem::CreateVmMem();
  if (!vm_mem_) {
    LOG(LOG_ERR,"Failed to create VmMem\n");
    return -1;
  }

  return 0;
}

int LendBufferToSecure(int buffer_fd, std::string vm_name, int64_t *lenddma_handle) {
  if (Init()) {
    return -1;
  }

  VmPerm vm_perms;

  VmHandle handle = vm_mem_->FindVmByName(vm_name);
  if (handle < 0)  {
    LOG(LOG_ERR,"Failed to find the %s VM!\n", vm_name.c_str());
    return -1;
  }

  if (vm_name == "qcom,cp_pixel") {
    vm_perms.push_back(std::make_pair(handle, VMMEM_READ | VMMEM_WRITE));
  } else {
    vm_perms.push_back(std::make_pair(handle, VMMEM_READ | VMMEM_WRITE | VMMEM_EXEC));
  }

  int ret = vm_mem_->LendDmabuf(buffer_fd, vm_perms, lenddma_handle);
  if (ret) {
    LOG(LOG_ERR,"lenddmabuf failed with ret %d\n", ret);
    return ret;
  }
  LOG(LOG_DBG,"lenddmabuf successful lenddma_handle %d\n", *lenddma_handle);

  return ret;
}

int ReclaimBufferFromSecure(int buffer_fd, int64_t lenddma_handle) {
  if (Init()) {
    return -1;
  }
  int ret = vm_mem_->ReclaimDmabuf(buffer_fd, lenddma_handle);
  if (ret) {
    LOG(LOG_ERR,"Failed to reclaim buffer from secure %d\n", ret);
    return ret;
  }

  return ret;
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

void GetHeapInfo(uint64_t usage, std::string *vm_name, std::string *dma_heap_name,
                 uint32_t flags) {
  bool secure = false;
  bool secure_carveout = false;

  if (usage & GBM_BO_USAGE_PROTECTED_QTI) {
    secure = true;
    *vm_name = IsCarveoutHeap(usage) ? "qcom,cp_pixel" : "";
  }

  std::string heap_name = secure ? "qcom,display" : "qcom,system";
  std::string ion_heap_name = secure ? "secure_display" : "system";

  if (usage & GBM_BO_ALLOC_CARVEOUT_HEAP_LEFT_QTI) {
    heap_name = "qcom,lsr_lefteye";
  } else if (usage & GBM_BO_ALLOC_CARVEOUT_HEAP_RIGHT_QTI) {
    heap_name = "qcom,lsr_righteye";
  } else if (usage & GBM_BO_ALLOC_CARVEOUT_HEAP_DEPTH_QTI) {
    heap_name = "qcom,lsr_depth";
  } else if (usage & GBM_BO_ALLOC_CARVEOUT_HEAP_MISC_QTI) {
    heap_name = "qcom,lsr_misc";
  } else if (usage & GBM_BO_ALLOC_SECURE_HEAP_QTI) {
    heap_name = "qcom,secure-pixel";
    ion_heap_name = "secure_heap";
    buffer_allocator_.MapNameToIonHeap(heap_name, ion_heap_name, flags,
                                       ION_HEAP(ION_SECURE_HEAP_ID), flags);
  } else if (usage & GBM_BO_USE_RENDERING) {
    heap_name = "qcom,system";
    *vm_name = secure ? "qcom,cp_pixel" : "";
  } else if ((usage & GBM_BO_ALLOC_SECURE_DISPLAY_HEAP_QTI) ||
             ((usage & GBM_BO_USAGE_PROTECTED_QTI) && (usage & GBM_BO_USAGE_CAMERA_WRITE_QTI))) {
    heap_name = "qcom,display";
    ion_heap_name = "secure_display";
    buffer_allocator_.MapNameToIonHeap(heap_name, ion_heap_name, flags,
                                       ION_HEAP(ION_SECURE_DISPLAY_HEAP_ID), flags);
  } else if (usage & GBM_BO_ALLOC_CAMERA_HEAP_QTI) {
    buffer_allocator_.MapNameToIonHeap(heap_name, ion_heap_name, flags,
                                       ION_HEAP(ION_CAMERA_HEAP_ID), flags);
  } else if (usage & GBM_BO_ALLOC_IOMMU_HEAP_QTI) {
    /*IOMMU_HEAP is deprecated, use ION_SYSTEM_HEAP_ID*/
    buffer_allocator_.MapNameToIonHeap(heap_name, ion_heap_name, flags,
                                       ION_HEAP(ION_SYSTEM_HEAP_ID), flags);
  } else if (usage & GBM_BO_ALLOC_MM_HEAP_QTI) {
    buffer_allocator_.MapNameToIonHeap(heap_name, ion_heap_name, flags,
                                       ION_HEAP(ION_CP_MM_HEAP_ID), flags);
  } else if (usage & GBM_BO_ALLOC_ADSP_HEAP_QTI) {
    heap_name = "qcom,adsp";
    ion_heap_name = "adsp";
    buffer_allocator_.MapNameToIonHeap(heap_name, ion_heap_name, flags,
                                       ION_HEAP(ION_ADSP_HEAP_ID), flags);
  } else {
    buffer_allocator_.MapNameToIonHeap(heap_name, ion_heap_name, flags,
                                       ION_HEAP(ION_SYSTEM_HEAP_ID), flags);
  }

  *dma_heap_name = heap_name;
  LOG(LOG_DBG,"using dma_heap_name %s\n", heap_name.c_str());

  return;
}

int AllocBuffer(uint64_t usage, uint32_t size, uint32_t align) {
  std::string dma_heap_name;
  uint32_t ionflags = GetIonAllocFlags(usage);
  std::string vm_name = "";
  int64_t lenddma_handle = 0;
  GetHeapInfo(usage, &vm_name, &dma_heap_name, ionflags);

  int buffer_fd = buffer_allocator_.Alloc(dma_heap_name, size, ionflags, align);
  if (vm_name.size() != 0) {
    int ret = LendBufferToSecure(buffer_fd, vm_name, &lenddma_handle);
    if (ret) {
      LOG(LOG_ERR,"%s: LendBufferToSecure Failed error %d\n", __FUNCTION__, ret);
      close(buffer_fd);
      return -1;
    }
  }

  return buffer_fd;
}
