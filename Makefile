C_SRC = $(wildcard src/*.c)
C_OBJ = $(C_SRC:.c=.o)
ASM = $(wildcard src/arch/$(shell uname -m)/*.S)
LDFLAGS = -luring -lpthread
CFLAGS = -g

rocket_io: $(C_OBJ) $(ASM)
	clang $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(C_OBJ): src/%.o : src/%.c
	clang $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(C_OBJ) rocket_io
