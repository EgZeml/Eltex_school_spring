CC      := gcc
CFLAGS  := -Wall -Wextra -pedantic

SRCDIR  := src

define make_srcs
$(SRCDIR)/$1.c $(SRCDIR)/utils.c $(SRCDIR)/math.c
endef

CLIENT_SRCS := $(call make_srcs,client)
SERVER_SRCS := $(call make_srcs,server)

CLIENT_OBJS := $(CLIENT_SRCS:.c=.o)
SERVER_OBJS := $(SERVER_SRCS:.c=.o)

.PHONY: all client server clean
all: client server

client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRCDIR)/*.o client server
