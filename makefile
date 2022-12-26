CC ?= gcc
CFLAGS ?= -O2 -g

SRC := $(shell echo src/*.c)
OBJ := $(SRC:.c=.o)


default: all

all: CC

clean:
	rm -f CC ${OBJ}

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

CC: ccc.c ${OBJ}
	${CC} ${CFLAGS} $^ -o $@

.PHONY: all clean
