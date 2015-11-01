
SRC = $(wildcard deps/*/*.c)
SRC += flag.c

CFLAGS = -std=c99 -I deps -g

check: test
	@./test

test: $(SRC) flag_test.c
	@$(CC) $(CFLAGS) $^ -o $@

example: $(SRC) flag_example.c
	@$(CC) $(CFLAGS) $^ -o $@

clean:
	@rm -f test example *.dSYM
.PHONY: clean
