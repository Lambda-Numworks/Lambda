epsilon_flavors_bootloader = $(foreach floavor,$(epsilon_flavors),$(floavor).A $(floavor).B)

define rule_for_epsilon_flavor_bootloader
$$(BUILD_DIR)/epsilon.$(1).A.$$(EXE): $$(call flavored_object_for,$$(epsilon_src),$(1))
$$(BUILD_DIR)/epsilon.$(1).A.$$(EXE): LDSCRIPT = ion/src/device/bootloader/bootloader.A.ld
$$(BUILD_DIR)/epsilon.$(1).B.$$(EXE): $$(call flavored_object_for,$$(epsilon_src),$(1))
$$(BUILD_DIR)/epsilon.$(1).B.$$(EXE): LDSCRIPT = ion/src/device/bootloader/bootloader.B.ld
$$(BUILD_DIR)/epsilon.$(1).bin: $$(BUILD_DIR)/epsilon.$(1).A.bin $$(BUILD_DIR)/epsilon.$(1).B.bin
	@echo "COMBINE $$@"
	$(Q) cat $$(BUILD_DIR)/epsilon.$(1).A.bin >> $$(BUILD_DIR)/epsilon.$(1).bin
	$(Q) truncate -s 4MiB $$(BUILD_DIR)/epsilon.$(1).bin
	$(Q) cat $$(BUILD_DIR)/epsilon.$(1).B.bin >> $$(BUILD_DIR)/epsilon.$(1).bin
	$(Q) truncate -s 8MiB $$(BUILD_DIR)/epsilon.$(1).bin
endef

$(BUILD_DIR)/epsilon.A.$(EXE): $(call flavored_object_for,$(epsilon_src))
$(BUILD_DIR)/epsilon.A.$(EXE): LDSCRIPT = ion/src/device/bootloader/bootloader.A.ld

$(BUILD_DIR)/epsilon.B.$(EXE): $(call flavored_object_for,$(epsilon_src))
$(BUILD_DIR)/epsilon.B.$(EXE): LDSCRIPT = ion/src/device/bootloader/bootloader.B.ld

$(BUILD_DIR)/epsilon.bin: $(BUILD_DIR)/epsilon.A.bin $(BUILD_DIR)/epsilon.B.bin
	@echo "COMBINE $@"
	$(Q) cat $(BUILD_DIR)/epsilon.A.bin >> $(BUILD_DIR)/epsilon.bin
	$(Q) truncate -s 4MiB $(BUILD_DIR)/epsilon.bin
	$(Q) cat $(BUILD_DIR)/epsilon.B.bin >> $(BUILD_DIR)/epsilon.bin
	$(Q) truncate -s 8MiB $(BUILD_DIR)/epsilon.bin

$(foreach flavor,$(epsilon_flavors),$(eval $(call rule_for_epsilon_flavor_bootloader,$(flavor))))


HANDY_TARGETS = $(foreach flavor,$(epsilon_flavors_bootloader),epsilon.$(flavor))
HANDY_TARGETS += epsilon.A epsilon.B

.PHONY: epsilon
epsilon: $(BUILD_DIR)/epsilon.onboarding.bin
.DEFAULT_GOAL := epsilon
 
.PHONY: %_flash
%_flash: $(BUILD_DIR)/%.dfu
	@echo "DFU     $@"
	@echo "INFO    About to flash your device. Please plug your device to your computer"
	@echo "        using an USB cable and press at the same time the 6 key and the RESET"
	@echo "        button on the back of your device."
	$(Q) until $(PYTHON) build/device/dfu.py -l | grep -E "0483:a291|0483:df11" > /dev/null 2>&1; do sleep 2;done
	$(Q) $(PYTHON) build/device/dfu.py -u $(word 1,$^)

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
