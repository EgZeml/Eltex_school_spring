# Компилятор и флаги
CC      := gcc
CFLAGS  := -Wall -Wextra -pedantic

# Исходники и объекты
SRCDIR  := src
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(SOURCES:.c=.o)

# Итоговый бинарник
TARGET  := main

.PHONY: all run clean

# 1) По умолчанию только собираем
all: $(TARGET)

# 2) Для запуска — отдельная цель
run: all
	@./$(TARGET)
	@rm -f $(OBJECTS)

# Линковка
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

# Компиляция каждого .c в .o
$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Чистка
clean:
	rm -f $(OBJECTS) $(TARGET)
