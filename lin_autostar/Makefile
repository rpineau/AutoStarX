# Makefile for lin_autostar

CC = gcc
CFLAGS = -fPIC -Wall -Wextra -O2
CPPFLAGS = -fPIC -Wall -Wextra -O2
LDFLAGS = -lcurses
RM = rm -f
STRIP = strip
TARGET = lin_autostar

SRCS = lin_autostar.c serial.c
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: ${TARGET}

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ ${LDFLAGS}
	$(STRIP) $@ >/dev/null 2>&1  || true

$(SRCS:.cpp=.d):%.d:%.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $< >$@

.PHONY: clean
clean:
	${RM} ${TARGET} ${OBJS} *~
