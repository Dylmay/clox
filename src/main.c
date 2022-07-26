#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util/common.h"
#include "vm/virt.h"
#include "util/string/string_util.h"

static void __run_repl(vm_t *vm);
static void __run_file(vm_t *vm, const char *path);
static char *__read_file(const char *path);
static void __proc_cmd(const char *cmd, const size_t cmd_sz, vm_t *vm);

int main(int argc, const char *argv[])
{
	vm_t virt = vm_init();

	if (argc == 1) {
		__run_repl(&virt);
	} else if (argc == 2) {
		__run_file(&virt, argv[1]);
	} else {
		fprintf(stderr, "Usage: clox[path]\n");
		exit(64);
	}

	vm_free(&virt);
	return 0;
}

static void __run_repl(vm_t *vm)
{
	char line_buf[1024];
	struct string *source = NULL;
	list_t scope_stack = list_of_type(char);

	while (true) {
		printf("> ");

		if (!fgets(line_buf, sizeof(line_buf), stdin)) {
			puts("");
			break;
		}

		if (line_buf[0] == '.') {
			__proc_cmd(line_buf + 1, sizeof(line_buf), vm);
			continue;
		}

		for (size_t i = 0; i < sizeof(line_buf); i++) {
			if (line_buf[i] == '\0') {
				break;
			}

			switch (line_buf[i]) {
			case '{':
				list_push(&scope_stack, &line_buf[i]);
				break;
			case '}':
				list_pop(&scope_stack);
				break;
			default:
				break;
			}
		}

		struct string *old_src = source;
		source = string_c_append(source, line_buf, sizeof(line_buf));
		string_free(old_src);

		if (list_size(&scope_stack) != 0) {
			for (size_t i = 0; i < list_size(&scope_stack); i++) {
				putchar('.');
			}
		} else {
			vm_interpret(vm, string_get_cstring(source));

			string_free(source);
			source = NULL;
		}
	}

	string_free(source);
	list_free(&scope_stack);
}

static void __proc_cmd(const char *cmd, const size_t cmd_sz, vm_t *vm)
{
	assert(("No VM passed", vm));

	if (strncmp(cmd, "exit\n", cmd_sz) == 0) {
		puts("Exiting");
		exit(0);
	} else if (strncmp(cmd, "vars\n", cmd_sz) == 0) {
		vm_print_vars(vm);
	} else {
		fprintf(stdout, "Unknown command: .%s\n", cmd);
		return;
	}
}

static void __run_file(vm_t *vm, const char *path)
{
	char *src = __read_file(path);

	enum vm_res result = vm_interpret(vm, src);

	if (result == INTERPRET_COMPILE_ERROR) {
		exit(65);
	}

	if (result == INTERPRET_RUNTIME_ERROR) {
		exit(70);
	}

	free(src);
}

static char *__read_file(const char *path)
{
	FILE *file = fopen(path, "rb");

	if (file == NULL) {
		fprintf(stderr, "Could not open file \"%s\".\n", path);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	size_t file_sz = ftell(file);
	rewind(file);

	char *file_buf = (char *)malloc(file_sz + 1);
	size_t bytes_rd = fread(file_buf, sizeof(char), file_sz, file);

	if (bytes_rd < file_sz) {
		fprintf(stderr, "Could not read file \"%s\".\n", path);
	}

	file_buf[bytes_rd] = '\0';

	fclose(file);

	return file_buf;
}
