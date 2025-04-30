# Компилятор и флаги
CC      := gcc
CFLAGS  := -Wall -Wextra -pedantic

# Исходники и объекты
SRCDIR  := src
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(SOURCES:.c=.o)

# Итоговый бинарник
TARGET  := main

.PHONY: run clean

# Первая цель – run (дефолт при вызове make)
run: $(TARGET)
	@./$(TARGET); rm -f $(OBJECTS)

# Линкуем бинарные из объектов
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Компиляция каждого .c в .o
$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Чистка (не удаляет сам бинарь по умолчанию)
clean:
	rm -f $(OBJECTS)
