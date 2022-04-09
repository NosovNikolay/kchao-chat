FW_PATHS=$(wildcard frameworks/*)
# sudo apt install libgtk-3-0 libgtk-3-dev libvte-2.91-dev libmongoc-1.0-0 libmongoc-dev

all: $(FW_PATHS)
	$(MAKE) -C client
	$(MAKE) -C server

clean:
	$(MAKE) -C client clean
	$(MAKE) -C server clean

uninstall: clean
	$(MAKE) -C client uninstall
	$(MAKE) -C server uninstall

deploy: $(FW_PATHS)
	./scripts/deploy.sh

$(FW_PATHS):
	$(MAKE) -C $@

reinstall: uninstall all

.PHONY: $(FW_PATHS) deploy
