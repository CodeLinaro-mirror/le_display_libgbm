
gbm_h_sources = ${srcdir}/inc/gbm*.h
gbm_includedir = $(includedir)
gbm_include_HEADERS = $(gbm_h_sources)

AM_CFLAGS = -Wundef -Wstrict-prototypes -Wno-trigraphs
AM_CFLAGS+= -I./inc
AM_CFLAGS+= -DGBM_DEBUG -DALLOCATE_SURFACE_BO_AT_CREATION -DVENUS_COLOR_FORMAT

AM_CPPFLAGS = -Wundef -Wstrict-prototypes -Wno-trigraphs
AM_CPPFLAGS+= -I./inc
AM_CPPFLAGS+= -DGBM_DEBUG -DALLOCATE_SURFACE_BO_AT_CREATION -DVENUS_COLOR_FORMAT

if ENABLE_COMPILEWITHDRM
AM_CFLAGS += -DENABLE_DRM
AM_CPPFLAGS += -DENABLE_DRM
else
AM_CFLAGS += -DENABLE_FB
AM_CPPFLAGS += -DENABLE_FB
endif

lib_LTLIBRARIES  = libgbm.la libgbmutils.la
libgbm_la_CFLAGS = $(AM_CFLAGS)
libgbm_la_CPPFLAGS = $(AM_CPPFLAGS)
libgbm_la_CPPFLAGS +=  @LIBDMABUFHEAP_CFLAGS@
libgbm_la_CXXFLAGS =  -std=c++17
libgbm_la_CPPFLAGS +=  @LIBVMMEM_CFLAGS@
libgbm_la_LDFLAGS = -shared -avoid-version -lm -ldl -ldmabufheap
libgbm_la_LDFLAGS += @LIBVMMEM_LIBS@
libgbm_la_SOURCES = src/gbm.c \
                    src/msmgbm_camera.c \
                    src/msmgbm_video.c \
                    src/msmgbm_common.c \
                    src/msmgbm_platform_wrapper.cpp \
                    src/msmgbm_adreno_utils.cpp \
                    src/msmgbm_camera_utils.cpp \
                    src/msmgbm_mapper.cpp

if ENABLE_COMPILEWITHDRM
libgbm_la_SOURCES += src/msmgbm.c
libgbm_la_SOURCES += src/msmgbm_dma.cpp
else
libgbm_la_SOURCES += src/msmgbm_fb.c
endif

libgbmutils_la_CFLAGS = $(AM_CFLAGS)
libgbmutils_la_CPPFLAGS = $(AM_CFLAGS)
libgbmutils_la_CPPFLAGS += -I$(srcdir)/inc/
libgbmutils_la_LDFLAGS = -shared -avoid-version -lm -ldl -lcutils
libgbmutils_la_LIBADD = libgbm.la
libgbmutils_la_SOURCES = src/gbm_utils.cpp

bin_PROGRAMS = gbmtest
gbmtest_SOURCES = test/gbmtest.c
gbmtest_LDADD = libgbm.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gbm.pc
EXTRA_DIST = $(pkgconfig_DATA)
