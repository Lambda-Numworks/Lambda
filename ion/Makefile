GIT := $(shell command -v git 2> /dev/null)
PATCH_LEVEL = NONE
ifdef GIT
  PATCH_LEVEL = $(shell git rev-parse HEAD | head -c 7)
endif

SFLAGS += -Iion/include -DKD_CONFIG_H=1

ifndef ION_KEYBOARD_LAYOUT
  $(error platform.mak should define ION_KEYBOARD_LAYOUT)
endif
SFLAGS += -Iion/include/ion/keyboard/$(ION_KEYBOARD_LAYOUT)
SFLAGS += -Iion/include/ion/keyboard/
ion_src += ion/src/shared/keyboard/$(ION_KEYBOARD_LAYOUT)/layout_events.cpp

include ion/src/$(PLATFORM)/Makefile
-include ion/test/$(PLATFORM)/Makefile
include ion/src/shared/tools/Makefile

ion_src += $(addprefix ion/src/shared/, \
  console_line.cpp \
  crc32_eat_byte.cpp \
  decompress.cpp \
  events.cpp \
  events_keyboard.cpp \
  events_modifier.cpp \
  filesystem.cpp \
  stack_position.cpp \
  storage.cpp \
  unicode/utf8_decoder.cpp\
  unicode/utf8_helper.cpp\
)

ion_src += ion/src/external/lz4/lz4.c

ion_src += $(addprefix ion/src/external/spiffs/,\
  spiffs_cache.c \
  spiffs_check.c \
  spiffs_gc.c \
  spiffs_hydrogen.c \
  spiffs_nucleus.c \
)

tests_src += $(addprefix ion/test/,\
  crc32.cpp\
  events.cpp\
  keyboard.cpp\
  storage.cpp\
  utf8_decoder.cpp\
  utf8_helper.cpp\
)

ifdef ION_STORAGE_LOG
SFLAGS += -DION_STORAGE_LOG=1
endif
