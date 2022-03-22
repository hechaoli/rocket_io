LIB_ROCKET_IO_SRC = $(filter-out src/main.c, $(wildcard src/*.c))
LIB_ROCKET_IO_OBJ = $(LIB_ROCKET_IO_SRC:.c=.o)
MAIN_SRC = src/main.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)
ASM_SRC = $(wildcard src/arch/$(shell uname -m)/*.S)
ASM_OBJ = $(ASM_SRC:.S=.o)
LIBS = librocket_io.a
BINS = rocket_io benchmark_file_io echo_server
LDFLAGS = -luring -lpthread -lrocket_io
INTERNAL_INCLUDES = -Iinclude_internal
PUBLIC_INCLUDES = -Iinclude
CFLAGS = -g

rocket_io: $(MAIN_OBJ) $(LIBS)
	clang $(CFLAGS) -o $@ $^ -L. $(LDFLAGS)

$(MAIN_OBJ): src/%.o : src/%.c
	clang $(CFLAGS) $(PUBLIC_INCLUDES) -c -o $@ $<

$(LIB_ROCKET_IO_OBJ): src/%.o : src/%.c
	clang $(CFLAGS) $(INTERNAL_INCLUDES) $(PUBLIC_INCLUDES) -c -o $@ $<

$(ASM_OBJ): src/%.o : src/%.S
	clang $(CFLAGS) -c -o $@ $<

librocket_io.a: $(LIB_ROCKET_IO_OBJ) $(ASM_OBJ)
	ar -rcs librocket_io.a $^

BENCHMARK_SRC = $(wildcard src/benchmark/benchmark*.c)
BENCHMARK_OBJ = $(BENCHMARK_SRC:.c=.o)

$(BENCHMARK_OBJ): src/benchmark/%.o : src/benchmark/%.c
	clang $(CFLAGS) $(PUBLIC_INCLUDES) -c -o $@ $<

benchmark_file_io: $(BENCHMARK_OBJ) $(LIBS)
	clang $(CFLAGS) -o $@ $^ -L. $(LDFLAGS)

echo_server: src/benchmark/echo_server.c $(LIBS)
	clang $(CFLAGS) $(PUBLIC_INCLUDES) $^ -o $@ -L. $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(LIB_ROCKET_IO_OBJ) $(MAIN_OBJ) $(BENCHMARK_OBJ) $(BINS) $(LIBS)
