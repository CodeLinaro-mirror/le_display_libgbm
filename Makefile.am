
gbm_h_sources = ${srcdir}/inc/gbm*.h
gbm_includedir = $(includedir)
gbm_include_HEADERS = $(gbm_h_sources)

AM_CFLAGS = -Wundef -Wstrict-prototypes -Wno-trigraphs
AM_CFLAGS+= -I./inc -I$(WORKSPACE)/vendor/qcom/opensource/commonsys-intf/display/include
AM_CFLAGS+= -DGBM_DEBUG -DALLOCATE_SURFACE_BO_AT_CREATION -DBUILD_HAS_WAYLAND_SUPPORT -DVENUS_COLOR_FORMAT -DGET_FD_WITH_NEW

AM_CPPFLAGS = -Wundef -Wstrict-prototypes -Wno-trigraphs
AM_CPPFLAGS+= -I./inc -I$(WORKSPACE)/vendor/qcom/opensource/commonsys-intf/display/include
AM_CPPFLAGS+= -DGBM_DEBUG -DALLOCATE_SURFACE_BO_AT_CREATION -DBUILD_HAS_WAYLAND_SUPPORT -DVENUS_COLOR_FORMAT -DGET_FD_WITH_NEW


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
                    src/msmgbm_mapper.cpp \
                    src/msmgbm.c \
                    src/msmgbm_dma.cpp

libgbmutils_la_CFLAGS = $(AM_CFLAGS)
libgbmutils_la_CPPFLAGS = $(AM_CFLAGS)
libgbmutils_la_CPPFLAGS += -I$(srcdir)/inc/
libgbmutils_la_LDFLAGS = -shared -avoid-version -lm -ldl -lcutils
libgbmutils_la_LIBADD = libgbm.la
libgbmutils_la_SOURCES = src/gbm_utils.cpp

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gbm.pc
EXTRA_DIST = $(pkgconfig_DATA)
