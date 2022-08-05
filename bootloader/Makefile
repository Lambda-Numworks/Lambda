
bootloader_src += $(addprefix bootloader/,\
  boot.cpp \
  main.cpp \
  kernel_header.cpp \
  keys.cpp \
  userland_header.cpp \
  slot.cpp \
  interface.cpp \
  jump_to_firmware.s \
  trampoline.cpp \
  usb_desc.cpp \
)

bootloader_images = $(addprefix bootloader/, \
  cable.png \
  computer.png \
)

bootloader_src += $(filter-out ion/src/device/shared/drivers/usb_desc.cpp,$(ion_src)) $(kandinsky_src) $(liba_src) $(libaxx_src) $(bootloader_images)

$(eval $(call depends_on_image,bootloader/interface.cpp,$(bootloader_images)))

# Keys management

# The header is refered to as <bootloader/keys.h> so make sure it's findable this way
SFLAGS += -I$(BUILD_DIR)

$(eval $(call rule_for, \
  KEYS2H, \
  bootloader/keys.cpp, \
  bootloader/keys.csv, \
  $$(PYTHON) bootloader/keys2h.py $$^  $$@ $$(subst .cpp,.h,$$@), \
  global \
))

$(BUILD_DIR)/bootloader/keys.h: $(BUILD_DIR)/bootloader/keys.cpp
$(call object_for,$(bootloader_src)): $(BUILD_DIR)/bootloader/keys.h
