# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g

# Папка с исходниками
SRCDIR = src

# Поиск всех .c файлов и преобразование их в .o
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c, %.o, $(SRCS))

# Основная цель
target = main

all: $(target)

# Обьектный файлы в финальную программу
$(target): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Преобразоание всех .c в .o
%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка
clean:
	rm -f $(OBJS) $(target)

run: all
	@echo "==> Запускаю $(target)…"
	@./$(target)
	@echo "==> Убираю артефакты…"
	@$(MAKE) clean

.PHONY: all clean run