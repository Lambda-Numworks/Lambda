epsilon_flavors_bootloader = $(foreach flavor,$(epsilon_flavors),$(flavor).A $(flavor).B $(flavor).A.signed $(flavor).B.signed)

define rule_for_epsilon_flavor_bootloader
$$(BUILD_DIR)/epsilon.$(1).A.$$(EXE): $$(call flavored_object_for,$$(epsilon_src),bootloader $(1))
$$(BUILD_DIR)/epsilon.$(1).A.$$(EXE): LDSCRIPT = ion/src/device/n0110/bootloader.A.ld
$$(BUILD_DIR)/epsilon.$(1).A.signed.bin: $(EPSILON_KEY) $$(BUILD_DIR)/epsilon.$(1).A.bin
	@echo "SIGN    $$@"
	$$(Q) $$(PYTHON) build/device/sign.py $$^ $$@

$$(BUILD_DIR)/epsilon.$(1).B.$$(EXE): $$(call flavored_object_for,$$(epsilon_src),bootloader $(1))
$$(BUILD_DIR)/epsilon.$(1).B.$$(EXE): LDSCRIPT = ion/src/device/n0110/bootloader.B.ld
$$(BUILD_DIR)/epsilon.$(1).B.signed.bin: $(EPSILON_KEY) $$(BUILD_DIR)/epsilon.$(1).B.bin
	@echo "SIGN    $$@"
	$$(Q) $$(PYTHON) build/device/sign.py $$^ $$@

$$(BUILD_DIR)/epsilon.$(1).bin: $$(BUILD_DIR)/epsilon.$(1).A.bin $$(BUILD_DIR)/epsilon.$(1).B.bin
	@echo "COMBINE $$@"
	$(Q) cat $$(BUILD_DIR)/epsilon.$(1).A.bin >> $$(BUILD_DIR)/epsilon.$(1).bin
	$(Q) truncate -s 4MiB $$(BUILD_DIR)/epsilon.$(1).bin
	$(Q) cat $$(BUILD_DIR)/epsilon.$(1).B.bin >> $$(BUILD_DIR)/epsilon.$(1).bin
	$(Q) truncate -s 8MiB $$(BUILD_DIR)/epsilon.$(1).bin

$$(BUILD_DIR)/epsilon.$(1).signed.bin: $$(BUILD_DIR)/epsilon.$(1).A.signed.bin $$(BUILD_DIR)/epsilon.$(1).B.signed.bin
	@echo "COMBINE $$@"
	$(Q) cat $$(BUILD_DIR)/epsilon.$(1).A.signed.bin >> $$(BUILD_DIR)/epsilon.$(1).signed.bin
	$(Q) truncate -s 4MiB $$(BUILD_DIR)/epsilon.$(1).signed.bin
	$(Q) cat $$(BUILD_DIR)/epsilon.$(1).B.signed.bin >> $$(BUILD_DIR)/epsilon.$(1).signed.bin
	$(Q) truncate -s 8MiB $$(BUILD_DIR)/epsilon.$(1).signed.bin
endef

$(BUILD_DIR)/epsilon.A.$(EXE): $(call flavored_object_for,$(epsilon_src),bootloader)
$(BUILD_DIR)/epsilon.A.$(EXE): LDSCRIPT = ion/src/device/n0110/bootloader.A.ld
$(BUILD_DIR)/epsilon.A.signed.bin: $(EPSILON_KEY) $$(BUILD_DIR)/epsilon.A.bin
	@echo "SIGN    $@"
	$(Q) $(PYTHON) build/device/sign.py $^ $@

$(BUILD_DIR)/epsilon.B.$(EXE): $(call flavored_object_for,$(epsilon_src),bootloader)
$(BUILD_DIR)/epsilon.B.$(EXE): LDSCRIPT = ion/src/device/n0110/bootloader.B.ld
$(BUILD_DIR)/epsilon.B.signed.bin: $(EPSILON_KEY) $$(BUILD_DIR)/epsilon.B.bin
	@echo "SIGN    $@"
	$(Q) $(PYTHON) build/device/sign.py $^ $@

$(BUILD_DIR)/epsilon.bin: $(BUILD_DIR)/epsilon.A.bin $(BUILD_DIR)/epsilon.B.bin
	@echo "COMBINE $@"
	$(Q) cat $(BUILD_DIR)/epsilon.A.bin >> $(BUILD_DIR)/epsilon.bin
	$(Q) truncate -s 4MiB $(BUILD_DIR)/epsilon.bin
	$(Q) cat $(BUILD_DIR)/epsilon.B.bin >> $(BUILD_DIR)/epsilon.bin
	$(Q) truncate -s 8MiB $(BUILD_DIR)/epsilon.bin

$(BUILD_DIR)/epsilon.signed.bin: $(BUILD_DIR)/epsilon.A.signed.bin $(BUILD_DIR)/epsilon.B.signed.bin
	@echo "COMBINE $@"
	$(Q) cat $(BUILD_DIR)/epsilon.A.signed.bin >> $(BUILD_DIR)/epsilon.signed.bin
	$(Q) truncate -s 4MiB $(BUILD_DIR)/epsilon.signed.bin
	$(Q) cat $(BUILD_DIR)/epsilon.B.signed.bin >> $(BUILD_DIR)/epsilon.signed.bin
	$(Q) truncate -s 8MiB $(BUILD_DIR)/epsilon.signed.bin

$(foreach flavor,$(epsilon_flavors),$(eval $(call rule_for_epsilon_flavor_bootloader,$(flavor))))

HANDY_TARGETS += $(foreach flavor,$(epsilon_flavors_bootloader),epsilon.$(flavor))
HANDY_TARGETS += epsilon.A epsilon.B epsilon.A.signed epsilon.B.signed epsilon.signed

HANDY_TARGETS += test.external_flash.write test.external_flash.read bootloader

$(BUILD_DIR)/test.external_flash.%.$(EXE): LDSCRIPT = ion/test/device/n0110/external_flash_tests.ld
test_external_flash_src = $(ion_src) $(liba_src) $(libaxx_src) $(kandinsky_src) $(poincare_src) $(ion_device_dfu_relogated_src) $(runner_src)
$(BUILD_DIR)/test.external_flash.read.$(EXE): $(BUILD_DIR)/quiz/src/test_ion_external_flash_read_symbols.o $(call object_for,$(test_external_flash_src) $(test_ion_external_flash_read_src))
$(BUILD_DIR)/test.external_flash.write.$(EXE): $(BUILD_DIR)/quiz/src/test_ion_external_flash_write_symbols.o $(call object_for,$(test_external_flash_src) $(test_ion_external_flash_write_src))

.PHONY: bootloader
bootloader: $(BUILD_DIR)/bootloader.bin
$(BUILD_DIR)/bootloader.$(EXE): $(call flavored_object_for,$(bootloader_src),usbxip nofilesystem)
$(BUILD_DIR)/bootloader.$(EXE): LDSCRIPT = ion/src/device/n0110/bootloader.ld

.PHONY: %_flash
%_flash: $(BUILD_DIR)/%.dfu $(BUILD_DIR)/flasher.light.dfu
	@echo "DFU     $@"
	@echo "INFO    About to flash your device. Please plug your device to your computer"
	@echo "        using an USB cable and press at the same time the 6 key and the RESET"
	@echo "        button on the back of your device."
	$(Q) until $(PYTHON) build/device/dfu.py -l | grep -E "0483:a291|0483:df11" > /dev/null 2>&1; do sleep 2;done
	$(eval DFU_SLAVE := $(shell $(PYTHON) build/device/dfu.py -l | grep -E "0483:a291|0483:df11"))
	$(Q) if [[ "$(DFU_SLAVE)" == *"0483:df11"* ]]; \
	  then \
	    $(PYTHON) build/device/dfu.py -u $(word 2,$^); \
	    sleep 2; \
	fi
	$(Q) $(PYTHON) build/device/dfu.py -u $(word 1,$^)

.PHONY: %.two_binaries
%.two_binaries: %.elf
	@echo "Building an internal and an external binary for     $<"
	$(Q) $(OBJCOPY) -O binary -j .text.external -j .rodata.external -j .exam_mode_buffer $(BUILD_DIR)/$< $(BUILD_DIR)/$(basename $<).external.bin
	$(Q) $(OBJCOPY) -O binary -R .text.external -R .rodata.external -R .exam_mode_buffer $(BUILD_DIR)/$< $(BUILD_DIR)/$(basename $<).internal.bin
	@echo "Padding $(basename $<).external.bin and $(basename $<).internal.bin"
	$(Q) printf "\xFF\xFF\xFF\xFF" >> $(BUILD_DIR)/$(basename $<).external.bin
	$(Q) printf "\xFF\xFF\xFF\xFF" >> $(BUILD_DIR)/$(basename $<).internal.bin

.PHONY: binpack
binpack:
ifndef USE_IN_FACTORY
	@echo "CAUTION: You are building a binpack."
	@echo "You must specify where this binpack will be used."
	@echo "Please set the USE_IN_FACTORY environment variable to either:"
	@echo "  - 0 for use in diagnostic"
	@echo "  - 1 for use in production"
	@exit -1
endif
	rm -rf output/binpack
	mkdir -p output/binpack
	$(MAKE) clean
	$(MAKE) IN_FACTORY=$(USE_IN_FACTORY) $(BUILD_DIR)/flasher.light.bin
	cp $(BUILD_DIR)/flasher.light.bin output/binpack
	$(MAKE) IN_FACTORY=$(USE_IN_FACTORY) $(BUILD_DIR)/bench.flash.bin
	$(MAKE) IN_FACTORY=$(USE_IN_FACTORY) $(BUILD_DIR)/bench.ram.bin
	cp $(BUILD_DIR)/bench.ram.bin $(BUILD_DIR)/bench.flash.bin output/binpack
	$(MAKE) IN_FACTORY=$(USE_IN_FACTORY) epsilon.official.onboarding.update.two_binaries
	cp $(BUILD_DIR)/epsilon.official.onboarding.update.internal.bin $(BUILD_DIR)/epsilon.official.onboarding.update.external.bin output/binpack
	$(MAKE) clean
	cd output && for binary in flasher.light.bin bench.flash.bin bench.ram.bin epsilon.official.onboarding.update.internal.bin epsilon.official.onboarding.update.external.bin; do shasum -a 256 -b binpack/$${binary} > binpack/$${binary}.sha256;done
	cd output && tar cvfz binpack-`git rev-parse HEAD | head -c 7`.tgz binpack
	rm -rf output/binpack
