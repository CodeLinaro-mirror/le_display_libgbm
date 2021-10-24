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
* Changes from Qualcomm Innovation Center are provided under the following license:
*
* Copyright (c) 2021, 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <msmgbm_dma.h>
#include <gbm_priv.h>
#include <msmgbm_common.h>
#include <BufferAllocator/BufferAllocator.h>

#include <linux/ion.h>
#include <linux/msm_ion.h>

#include <string>



void GetHeapInfo(uint64_t usage, std::string *dma_heap_name, uint32_t flags) {
  bool secure = false;

  if (usage & GBM_BO_USAGE_PROTECTED_QTI) {
    secure = true;
  }

  std::string heap_name = secure ? "qcom,display" : "qcom,system";
  std::string ion_heap_name = secure ? "secure_display" : "system";

  if (usage & GBM_BO_ALLOC_SECURE_HEAP_QTI) {
    heap_name = "qcom,secure-pixel";
    ion_heap_name = "secure_heap";
    buffer_allocator_.MapNameToIonHeap(heap_name, ion_heap_name, flags,
                                       ION_HEAP(ION_SECURE_HEAP_ID), flags);
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

  return;
}

int AllocBuffer(uint64_t usage, uint32_t size, uint32_t align) {
  std::string dma_heap_name;
  uint32_t ionflags = GetIonAllocFlags(usage);
  GetHeapInfo(usage, &dma_heap_name, ionflags);
  return buffer_allocator_.Alloc(dma_heap_name, size, ionflags, align);
}
