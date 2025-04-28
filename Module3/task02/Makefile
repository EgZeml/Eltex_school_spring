# --- добавить в PATH папку ~/ для всех рецептов
export PATH := $(PATH):~/

# Компилятор и флаги
CC      := gcc
CFLAGS  := -Wall -Wextra -pedantic

# Директории
SRCDIR     := src
PROGRAMDIR := program

# Источники и объекты для main
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(SOURCES:.c=.o)
TARGET  := main

# Источники и исполняемые для программ
PROG_SRCS := $(wildcard $(PROGRAMDIR)/*.c)
PROG_EXES := $(PROG_SRCS:.c=)

.PHONY: run programs clean

# --- default: собираем всё и запускаем main
run: programs $(TARGET)
	@./$(TARGET)
	@rm -f $(OBJECTS)

# Линкуем main из .o
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Компилируем каждый src/%.c → src/%.o
$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- собираем все программы в папке program
programs: $(PROG_EXES)

# правило сборки program/foo из program/foo.c
$(PROGRAMDIR)/%: $(PROGRAMDIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

# Явная чистка только .o (main остаётся)
clean:
	rm -f $(OBJECTS)
