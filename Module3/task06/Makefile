# Компилятор и опции
CC      := gcc
CFLAGS  := -Wall -Wextra -pedantic

# Директория с исходниками
SRCDIR := src

# Правила по умолчанию: собрать server и client, а потом убрать .o
.PHONY: all clean

all: server client
	@rm -f $(SRCDIR)/*.o

# Линкуем сервер из server.o и tools.o
server: $(SRCDIR)/server.o $(SRCDIR)/tools.o
	$(CC) $(CFLAGS) -o $@ $^

# Аналогично для клиента
client: $(SRCDIR)/client.o $(SRCDIR)/tools.o
	$(CC) $(CFLAGS) -o $@ $^

# Компиляция общих и отдельных модулей
$(SRCDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/tools.h
	$(CC) $(CFLAGS) -c $< -o $@

# Полная очистка (и бинарей, и .o)
clean:
	rm -f $(SRCDIR)/*.o server client
