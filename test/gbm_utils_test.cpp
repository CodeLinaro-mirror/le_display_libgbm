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
#include <stdlib.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <gbm.h>
#include <gbm_utils.h>
#include <gbm_priv.h>

#define DRM_DEVICE_NAME "/dev/dri/card0"

#define ENODISPLAY -1
#define CHECK(cond) do {\
    if (!(cond)) {\
        printf("CHECK failed in %s() %s:%d\n", __func__, __FILE__, __LINE__);\
        return 0;\
    }\
} while(0)

using namespace msm_gbm;

static int fd;
static struct gbm_device *gbm;

static int check_bo(struct gbm_bo *bo)
{
    CHECK(bo);
    CHECK(gbm_bo_get_width(bo) >= 0);
    CHECK(gbm_bo_get_height(bo) >= 0);
    CHECK(gbm_bo_get_stride(bo) >= gbm_bo_get_width(bo));

    return 1;
}

int open_device() {
  int fd = -1;

  fd = open(DRM_DEVICE_NAME, O_RDWR | O_CLOEXEC);
  if (fd < 0) {
    return ENODISPLAY;
  }

  return fd;
}

int test_init() {
  fd = open_device();
  if (fd < 0)
  if (fd == ENODISPLAY)
      return ENODISPLAY;
  CHECK(fd >= 0);

  gbm = gbm_create_device(fd);

  CHECK(gbm_device_get_fd(gbm) == fd);

  printf("[ test_init(): gbm_create_device(),gbm_device_get_fd] success\n");

  return 1;
}

static int test_destroy() {
    printf("Inside test_destroy\n");
    gbm_device_destroy(gbm);
    close(fd);

    return 1;
}

char *get_format_string(uint32_t format) {
  switch(format)
  {
    case GBM_FORMAT_YCbCr_420_888:
        return "GBM_FORMAT_YCbCr_420_888";
    case GBM_FORMAT_NV12:
        return "GBM_FORMAT_NV12";
    case GBM_FORMAT_XBGR8888:
        return "GBM_FORMAT_XBGR8888";
    case GBM_FORMAT_NV12_ENCODEABLE:
        return "GBM_FORMAT_NV12_ENCODEABLE";
    case GBM_FORMAT_NV21_ZSL:
        return "GBM_FORMAT_NV21_ZSL";
    case GBM_FORMAT_YCrCb_420_SP:
        return "GBM_FORMAT_YCrCb_420_SP";
    case GBM_FORMAT_YCrCb_420_SP_VENUS:
        return "GBM_FORMAT_YCrCb_420_SP_VENUS";
    case GBM_FORMAT_YCbCr_420_SP_VENUS_UBWC:
        return "GBM_FORMAT_YCbCr_420_SP_VENUS_UBWC";
    case GBM_FORMAT_IMPLEMENTATION_DEFINED:
        return "GBM_FORMAT_IMPLEMENTATION_DEFINED";
    case GBM_FORMAT_RGBA8888:
        return "GBM_FORMAT_RGBA8888";
    case GBM_FORMAT_NV12_HEIF:
        return "GBM_FORMAT_NV12_HEIF";
    case GBM_FORMAT_YCbCr_420_P010_VENUS:
        return "GBM_FORMAT_YCbCr_420_P010_VENUS";
    case GBM_FORMAT_YCbCr_420_P010_UBWC:
        return "GBM_FORMAT_YCbCr_420_P010_UBWC";
    case GBM_FORMAT_YCbCr_420_TP10_UBWC:
        return "GBM_FORMAT_YCbCr_420_TP10_UBWC";
    case GBM_FORMAT_C8:
        return "GBM_FORMAT_C8";
    default:
        return "NA";
  }
}

int test_get_format_layout() {
  GbmUtils gbm_utils;
  generic_buf_layout_t buf_lyt;
  uint64_t size;
  struct gbm_buf_desc bufdesc = {1024, 1024, GBM_FORMAT_NV12, GBM_BO_USE_RENDERING, 0};
  if(gbm_utils.GetFormatLayout(bufdesc, &buf_lyt, &size) != GBM_ERROR_NONE) {
    printf("failed to get format layout\n");
    return 0;
  }

  printf("BO size=%d pixel_format %d num_planes %d\n", size, buf_lyt.pixel_format,
                                                      buf_lyt.num_planes);
  for(int j = 0; j < (buf_lyt.num_planes); j++){
      printf("plane[%d].h_increment=%d\n",j,buf_lyt.planes[j].h_increment);
      printf("plane[%d].v_increment=%d\n",j,buf_lyt.planes[j].v_increment);
      printf("plane[%d].offset=%p\n",j,buf_lyt.planes[j].offset);
  }

  return 1;
}

int test_get_aligned_width() {
  uint64_t result;
  GbmUtils gbm_utils;
  struct gbm_buf_desc bufdesc = {1024, 2160, GBM_FORMAT_NV12, GBM_BO_USE_RENDERING, 0};
  if(gbm_utils.GetWidth(bufdesc, &result) != GBM_ERROR_NONE) {
    printf("failed to get aligned Width\n");
    return 0;
  }

  printf("aligned width %d\n", result);

  return 1;
}

int test_get_aligned_height() {
  uint64_t result;
  GbmUtils gbm_utils;
  struct gbm_buf_desc bufdesc = {1024, 2160, GBM_FORMAT_NV12, GBM_BO_USE_RENDERING, 0};
  if(gbm_utils.GetHeight(bufdesc, &result) != GBM_ERROR_NONE) {
    printf("failed to get aligned height\n");
    return 0;
  }

  printf("aligned height %d\n", result);

  return 1;
}

int test_get_size() {
  uint64_t result;
  GbmUtils gbm_utils;
  struct gbm_buf_desc bufdesc = {1024, 1024, GBM_FORMAT_NV12, GBM_BO_USE_RENDERING, 0};
  if(gbm_utils.GetSize(bufdesc, &result) != GBM_ERROR_NONE) {
    printf("failed to get size\n");
    return 0;
  }

  printf("size %d\n", result);

  return 1;
}

int test_allocate_free_native_handle() {
  struct gbm_bo *bo = gbm_bo_create(gbm, 1024, 1024, GBM_FORMAT_NV12, GBM_BO_USE_RENDERING);
  CHECK(check_bo(bo));

  GbmUtils gbm_utils;
  native_handle_t *native_handle = gbm_utils.AllocateNativeHandle(bo);
  if (!native_handle) {
    fprintf(stderr, "Allocated native_handle is null \n");
    return 0;
  }


  struct gbm_bo *bo_from_native_handle = gbm_utils.GetGbmBo(native_handle);
  if (!bo_from_native_handle) {
    fprintf(stderr, "Retrived bo is null \n");
    return 0;
  }

  // Do any operation from bo using gbm apis
  CHECK(gbm_bo_get_width(bo_from_native_handle) == gbm_bo_get_width(bo));
  CHECK(gbm_bo_get_height(bo_from_native_handle) == gbm_bo_get_height(bo));
  CHECK(gbm_bo_get_stride(bo_from_native_handle) == gbm_bo_get_stride(bo));
  CHECK(gbm_bo_get_bpp(bo_from_native_handle) == gbm_bo_get_bpp(bo));
  CHECK(gbm_bo_get_fd(bo_from_native_handle) == gbm_bo_get_fd(bo));

  // First destroy bo then destroy native handle
  gbm_bo_destroy(bo);
  gbm_utils.FreeNativeHandle(native_handle);

  return 1;
}

int gbm_utils_test_help() {
  printf("Please Enter Test No:\n");
  printf("1 for Get format layout\n");
  printf("2 for Get aligned width\n");
  printf("3 for Get aligned height\n");
  printf("4 for Get size\n");
  printf("5 for Allocate/Free native handle\n");
  return 0;
}

int main(int argc, char *argv[])
{
  int result=1;
  int param=0;

  if(argc > 1)
      param=atoi(argv[argc-1]);
  else {
      return gbm_utils_test_help();
  }

  switch(param) {
    case 1:
      result &= test_get_format_layout();
    break;
    case 2:
      result &= test_get_aligned_width();
    break;
    case 3:
      result &= test_get_aligned_height();
    break;
    case 4:
      result &= test_get_size();
    break;
    case 5:
      result &= test_init();
      result &= test_allocate_free_native_handle();
      result &= test_destroy();
    break;
    default:
      gbm_utils_test_help();
      return 0;
  }

  if (!result) {
    printf("[  FAILED  ] gbm_utils test failed\n");
    return EXIT_FAILURE;
  } else {
    printf("[  PASSED  ] gbm_utils test success\n");
    return EXIT_SUCCESS;
  }
}
