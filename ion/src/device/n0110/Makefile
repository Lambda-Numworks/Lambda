ion_device_src += $(addprefix ion/src/device/n0110/drivers/, \
  board.cpp \
  cache.cpp \
  external_flash.cpp \
  led.cpp \
  power.cpp \
  reset.cpp \
  usb.cpp \
)

LDSCRIPT ?= ion/src/device/n0110/flash.ld

# We need to work around a GCC bug (concerning versions < 5.1). It is valid in
# C++11 to initialize a character array by providing a string litteral (e.g.
# char test[4]= "ab"; is valid and should initialize test to 'a','b',0,0).
# Older versions of GCC are not conformant so we resort to an initializer list.
initializer_list = $(shell echo $(1) | sed  "s/\(.\)/'\1',/g")0
$(call object_for,ion/src/device/n0110/platform_info.cpp): SFLAGS += -DPATCH_LEVEL="$(call initializer_list,$(PATCH_LEVEL))" -DEPSILON_VERSION="$(call initializer_list,$(EPSILON_VERSION))"

ion_device_src += $(addprefix ion/src/device/n0110/, \
  platform_info.cpp \
)