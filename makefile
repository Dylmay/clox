CC = clang
BUILD_ROOT = build/

build/clox:
	make -C src/

.PHONY: clean
.PHONY: test
.PHONY: dbg
.PHONY: rel

clean:
	$(RM) -r $(BUILD_ROOT)

test: $(BUILD_DIR)/$(TARGET_EXEC)
	make dbg
	make -C test/ CC=$(CC)
	./build/clox_test

dbg:
	make -C src/ CC=$(CC)

rel:
	make -C src/ clean
	make -C src/ CFLAGS="-Wall -Wno-unused-value -O3" CC=$(CC)

MKDIR_P ?= mkdir -p
