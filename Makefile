CC = cc
CFLAGS = -Wall -Wextra -O2
SRC_DIR = src
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/backup.c $(SRC_DIR)/list.c $(SRC_DIR)/hash.c $(SRC_DIR)/fsutil.c $(SRC_DIR)/restore.c $(SRC_DIR)/compress.c $(SRC_DIR)/encrypt.c
OBJS = $(SRCS:.c=.o)
TARGET = safestore

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)