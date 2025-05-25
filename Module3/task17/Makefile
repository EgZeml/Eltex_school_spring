CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11

# Бинарники
SERVER_BIN   = server
CLIENT_BIN   = client_raw
SNIFFER_BIN  = sniffer

# Исходники (если они лежат в папке src/)
SRC_DIR      = src
SERVER_SRCS  = $(SRC_DIR)/server.c $(SRC_DIR)/utils.c
CLIENT_SRCS  = $(SRC_DIR)/client.c $(SRC_DIR)/raw_utils.c $(SRC_DIR)/utils.c
SNIFFER_SRCS = $(SRC_DIR)/sniffer.c $(SRC_DIR)/raw_utils.c $(SRC_DIR)/utils.c

all: $(SERVER_BIN) $(CLIENT_BIN) $(SNIFFER_BIN)

$(SERVER_BIN): $(SERVER_SRCS) $(SRC_DIR)/utils.h
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRCS)

$(CLIENT_BIN): $(CLIENT_SRCS) $(SRC_DIR)/utils.h $(SRC_DIR)/raw_utils.h
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRCS)

$(SNIFFER_BIN): $(SNIFFER_SRCS)
	$(CC) $(CFLAGS) -o $@ $(SNIFFER_SRCS) $(SRC_DIR)/utils.h $(SRC_DIR)/raw_utils.h

clean:
	rm -f *.o $(SERVER_BIN) $(CLIENT_BIN) $(SNIFFER_BIN) dump.bin

.PHONY: all clean
