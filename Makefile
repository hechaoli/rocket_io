LIB_ROCKET_IO_SRC = $(filter-out src/main.c, $(wildcard src/*.c))
LIB_ROCKET_IO_OBJ = $(LIB_ROCKET_IO_SRC:.c=.o)
MAIN_SRC = src/main.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)
ASM = $(wildcard src/arch/$(shell uname -m)/*.S)
LIBS = librocket_io.a
BINS = rocket_io benchmark_file_io
LDFLAGS = -luring -lpthread -lrocket_io
CFLAGS = -g

rocket_io: $(MAIN_OBJ) $(ASM) $(LIBS)
	clang $(CFLAGS) -o $@ $^ -L. $(LDFLAGS)

$(MAIN_OBJ): src/%.o : src/%.c
	clang $(CFLAGS) -c -o $@ $<

$(LIB_ROCKET_IO_OBJ): src/%.o : src/%.c
	clang $(CFLAGS) -c -o $@ $<

librocket_io.a: $(LIB_ROCKET_IO_OBJ) $(ASM)
	ar -rcs librocket_io.a $^

BENCHMARK_SRC = $(wildcard src/benchmark/*.c)
BENCHMARK_OBJ = $(BENCHMARK_SRC:.c=.o)

$(BENCHMARK_OBJ): src/benchmark/%.o : src/benchmark/%.c
	clang $(CFLAGS) -c -o $@ $<

benchmark_file_io: $(BENCHMARK_OBJ) $(ASM) $(LIBS)
	clang $(CFLAGS) -o $@ $^ -L. $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(LIB_ROCKET_IO_OBJ) $(MAIN_OBJ) $(BINS) $(LIBS)
