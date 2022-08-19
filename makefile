CC = clang
BUILD_ROOT = build/
DBG_DEFINES = "-DDEBUG_TRACE_EXECUTION -DDEBUG_PRINT_CODE -DDEBUG_BENCH"
BENCH_DEFINES = "-DNDEBUG -DDEBUG_BENCH"
REL_DEFINES = "-DNDEBUG"
VALGRIND ?= valgrind "--leak-check=yes"

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
	$(VALGRIND) ./build/clox_test

dbg:
	make -C src/ CC=$(CC) DEFINES=$(DBG_DEFINES)

bench:
	make -C src/ CFLAGS="-Wall -Wno-unused-value -O3" CC=$(CC) DEFINES=$(BENCH_DEFINES)

rel:
	make -C src/ CFLAGS="-Wall -Wno-unused-value -O3" CC=$(CC) DEFINES=$(REL_DEFINES)

MKDIR_P ?= mkdir -p
