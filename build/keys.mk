
$(EPSILON_KEY):
	@echo "GENKEY  $@"
	$(Q) $(PYTHON) build/device/genkeys.py $@
