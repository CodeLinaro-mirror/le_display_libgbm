
gbm_h_sources = ${srcdir}/inc/gbm*.h
gbm_includedir = $(includedir)
gbm_include_HEADERS = $(gbm_h_sources)

AM_CFLAGS = -Wundef -Wstrict-prototypes -Wno-trigraphs
AM_CFLAGS+= -I./inc
AM_CFLAGS+= -DGBM_DEBUG -DALLOCATE_SURFACE_BO_AT_CREATION -DVENUS_COLOR_FORMAT -DGET_FD_WITH_NEW

AM_CPPFLAGS = -Wundef -Wstrict-prototypes -Wno-trigraphs
AM_CPPFLAGS+= -I./inc
AM_CPPFLAGS+= -DGBM_DEBUG -DALLOCATE_SURFACE_BO_AT_CREATION -DVENUS_COLOR_FORMAT -DGET_FD_WITH_NEW

if ENABLE_COMPILEWITHDRM
AM_CFLAGS += -DENABLE_DRM
AM_CPPFLAGS += -DENABLE_DRM
else
AM_CFLAGS += -DENABLE_FB
AM_CPPFLAGS += -DENABLE_FB
endif

lib_LTLIBRARIES  = libqbm.la
libqbm_la_CFLAGS = $(AM_CFLAGS)
libqbm_la_CPPFLAGS = $(AM_CPPFLAGS)
libqbm_la_CXXFLAGS =  -std=c++17
libqbm_la_LDFLAGS = -shared -avoid-version -lm -ldl
libqbm_la_SOURCES = src/gbm.c \
                    src/msmgbm_camera.c \
                    src/msmgbm_video.c \
                    src/msmgbm_common.c \
                    src/msmgbm_platform_wrapper.cpp \
                    src/msmgbm_adreno_utils.cpp \
                    src/msmgbm_camera_utils.cpp \
                    src/msmgbm_mapper.cpp

if ENABLE_COMPILEWITHDRM
libqbm_la_SOURCES += src/msmgbm.c
libqbm_la_SOURCES += src/msmgbm_dma.cpp
else
libqbm_la_SOURCES += src/msmgbm_fb.c
endif

bin_PROGRAMS = qbmtest
qbmtest_SOURCES = test/gbmtest.c
qbmtest_LDADD = libqbm.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = qbm.pc
EXTRA_DIST = $(pkgconfig_DATA)
