/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include "schema_parser.h"
#include "buffer_layout.h"
#include "drm_fourcc.h"

#define ALIGN(x, align) (((x) + ((align)-1)) & ~((align)-1))

int get_num_planes(struct gbm_bufdesc *descriptor, uint32_t *num_planes) {
   if (!descriptor)
      return false;

   const char *format_name = find_format_name(descriptor->format);
   if (!format_name) {
      fprintf(stderr,"Unsupported format\n");
      return -1;
   }
   struct format_info *info = get_format_info(format_name);
   if (!info)
      return -1;

   *num_planes = info->plane_count;
   return 0;
}

bool ubwc_enabled(struct gbm_bufdesc *descriptor) {
   if (!descriptor)
      return false;

   if (descriptor->modifiers & DRM_FORMAT_MOD_QCOM_COMPRESSED)
      return true;

   return false;
}

uint32_t get_plane_height(uint32_t buffer_height, uint32_t format, int plane) {
   uint32_t plane_height = buffer_height;

   if (plane == 0)
      return plane_height;

   switch(format) {
   case GBM_FORMAT_NV12:
   case GBM_FORMAT_NV21:
   case GBM_FORMAT_NV16:
   case GBM_FORMAT_NV61:
      plane_height = (plane_height+1)>>1;
      break;
   default:
      break;
   }

   return plane_height;
}

int get_aligned_width_and_height(struct gbm_bufdesc *descriptor, int plane,
                                 uint32_t *alignedw, uint32_t *alignedh)
{
   if (!descriptor || !alignedw || !alignedh)
      return -1;

   uint32_t height = get_plane_height(descriptor->height, descriptor->format, plane);

   const char *format_name = find_format_name(descriptor->format);
   if (!format_name) {
      fprintf(stderr,"Unsupported format\n");
      return -1;
   }

   struct format_info *info = get_format_info(format_name);
   if (!info)
      return -1;

   uint32_t pitch_align_factor = info->planes[plane].pitch_align;
   uint32_t height_align_factor = info->planes[plane].height_align;

   if (ubwc_enabled(descriptor)) {
      pitch_align_factor = info->planes[plane].meta_info->pitch_align;
      height_align_factor = info->planes[plane].meta_info->height_align;
   }

   *alignedw = ALIGN(descriptor->width, pitch_align_factor);
   *alignedh = ALIGN(height, height_align_factor);

   return 0;
}

int get_stride(struct gbm_bufdesc *descriptor, int plane, uint32_t *stride)
{
   if (!descriptor || !stride)
      return -1;

   const char *format_name = find_format_name(descriptor->format);
   if (!format_name) {
      fprintf(stderr,"Unsupported format\n");
      return -1;
   }

   struct format_info *info = get_format_info(format_name);
   if (!info)
      return -1;

   uint32_t alignedw = 0, alignedh = 0;

   if (get_aligned_width_and_height(descriptor, plane, &alignedw, &alignedh) != 0)
      return -1;

   uint32_t alignment = info->planes[plane].pitch_align;
   if (ubwc_enabled(descriptor)) {
      alignment = info->planes[plane].meta_info->pitch_align;
   }

   uint32_t bpp = info->bpp;
   *stride = ALIGN(alignedw * bpp, alignment);

   return 0;
}

int get_data_plane_size(struct gbm_bufdesc *descriptor, int plane, uint32_t *plane_size) {
   *plane_size = 0;

   const char *format_name = find_format_name(descriptor->format);
   if (!format_name) {
      fprintf(stderr,"Unsupported format\n");
      return -1;
   }

   struct format_info *info = get_format_info(format_name);
   if (!info)
      return -1;
    
   uint32_t alignedw = 0, alignedh = 0;
   if (get_aligned_width_and_height(descriptor, plane, &alignedw, &alignedh) != 0)
      return -1;

   uint32_t stride = alignedw;
   if (get_stride(descriptor, plane, &stride) != 0)
      return -1;

   uint32_t alignment = (ubwc_enabled(descriptor)) ? info->planes[plane].meta_info->size_align : info->size_align;

   *plane_size = ALIGN(stride * alignedh, alignment);
   return 0;
}

int get_meta_buffer_size(struct gbm_bufdesc *descriptor, int plane, uint32_t *metabuffer_size)
{
   if (!descriptor || !metabuffer_size)
      return -1;

   uint32_t height = get_plane_height(descriptor->height, descriptor->format, plane);

   const char *format_name = find_format_name(descriptor->format);
   if (!format_name) {
      fprintf(stderr,"Unsupported format\n");
      return -1;
   }

   struct format_info *info = get_format_info(format_name);
   if (!info)
      return -1;

   if (!ubwc_enabled(descriptor)) {
      *metabuffer_size = 0;
      return 0;
   }

   uint32_t block_width = info->planes[plane].meta_info->block_width;
   uint32_t block_height = info->planes[plane].meta_info->block_height;

   int meta_height = ALIGN(((height + block_height - 1) / block_height), 16);
   int meta_width = ALIGN(((descriptor->width + block_width - 1) / block_width), 64);
   *metabuffer_size = (unsigned int)ALIGN((meta_width * meta_height), 4096);

   return 0;
}

int get_size(struct gbm_bufdesc *descriptor, uint32_t *buffer_size)
{
   if (!descriptor || !buffer_size)
      return -1;

   const char *format_name = find_format_name(descriptor->format);
   if (!format_name) {
      fprintf(stderr,"Unsupported format\n");
      return -1;
   }

   struct format_info *info = get_format_info(format_name);
   if (!info)
      return -1;

   uint32_t plane_count = info->plane_count;

   uint32_t size = 0;
   for (uint32_t i = 0; i < plane_count; i++) {
      uint32_t data_size = 0, ubwc_size = 0;
      if (get_data_plane_size(descriptor, i, &data_size) != 0)
         return -1;
      if (get_meta_buffer_size(descriptor, i, &ubwc_size) != 0)
         return -1;

      size += data_size + ubwc_size;
   }

   *buffer_size = size;
   return 0;
}

int get_plane_offset(struct gbm_bufdesc *descriptor, int plane, uint32_t *plane_offset)
{
   if (!descriptor || !plane_offset)
      return -1;

   const char *format_name = find_format_name(descriptor->format);
   if (!format_name) {
      fprintf(stderr,"Unsupported format\n");
      return -1;
   }

   struct format_info *info = get_format_info(format_name);
   if (!info || (plane > info->plane_count - 1))
      return -1;

   uint32_t offset = 0;
   for (uint32_t i = 0; i < plane; i++) {
      uint32_t data_size = 0, ubwc_size = 0;
      if (get_data_plane_size(descriptor, i, &data_size) != 0)
         return -1;
      if (get_meta_buffer_size(descriptor, i, &ubwc_size) != 0)
         return -1;

      offset += data_size + ubwc_size;
   }

   *plane_offset = offset;
   return 0;
}
