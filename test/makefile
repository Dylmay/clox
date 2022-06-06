TARGET_EXEC ?= clox_test

PROG_SRC_DIRS ?= ../src
PROG_BUILD_DIR ?= ../build/src

BUILD_DIR ?= ../build/test_src
EXEC_DIR ?= ../build
SRC_DIRS ?= .

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

PROG_SRCS := $(shell find $(PROG_SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
PROG_OBJS := $(shell find $(PROG_BUILD_DIR) -mindepth 2 -type f -name *.o)
PROG_DEPS := $(PROG_OBJS:.o=.d)
PROG_INC_DIRS := $(shell find $(PROG_SRC_DIRS) -type d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS)) $(addprefix -I,$(PROG_INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP
CFLAGS = -Wall -Wno-unused-value -g


$(EXEC_DIR)/$(TARGET_EXEC): $(OBJS) $(PROG_OBJS)
	$(MKDIR_P) $(BUILD_DIR)
	$(CC) $(OBJS) $(PROG_OBJS) -o $@ $(LDFLAGS)
# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)
-include $(PROG_DEPS)

MKDIR_P ?= mkdir -p
