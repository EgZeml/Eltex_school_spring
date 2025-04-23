# Компилятор и флаги
CC       := gcc
CFLAGS   := -std=c99 -Wall -Wextra -I./src

# Директория исходников
SRC_DIR  := src

# Библиотека и объекты для неё
LIB        := libcontacts.a
LIB_OBJS   := contacts_logic.o

# Объектные файлы приложения
APP_OBJS   := main.o contacts_ui.o input_utils.o

.PHONY: all clean

# default target — сборка, запуск и очистка
all: $(LIB) contacts_app
	@echo "=== Запуск программы ==="
	./contacts_app
	@echo "=== Очистка ==="
	$(MAKE) clean

# Сборка статической библиотеки
$(LIB): $(LIB_OBJS)
	ar rcs $@ $^

# Линковка приложения с библиотекой
contacts_app: $(APP_OBJS)
	$(CC) $(APP_OBJS) -L. -lcontacts -o $@

# Компиляция каждого .c из src/ в .o в корне
%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Удаление объекта, библиотеки и бинарника
clean:
	rm -f *.o $(LIB) contacts_app
