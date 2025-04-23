# Компилятор и флаги
CC       := gcc
CFLAGS   := -std=c99 -Wall -Wextra -I./src
PICFLAGS := -fPIC

# Директория с исходниками
SRC_DIR  := src

# Динамическая библиотека и объекты для неё
LIB       := libcontacts.so
LIB_OBJS  := contacts_logic.o

# Объектные файлы приложения
APP_OBJS  := main.o contacts_ui.o input_utils.o

# Флаги линковки для приложения (включая rpath, чтобы .so искалось в ./)
LDFLAGS   := -L. -lcontacts -Wl,-rpath=.

.PHONY: all clean

# при вызове make — собрать, запустить и потом очистить
all: $(LIB) contacts_app
	@echo "=== Запуск программы ==="
	./contacts_app
	@echo "=== Очистка ==="
	$(MAKE) clean

# сборка динамической библиотеки из одного объекта
$(LIB): $(LIB_OBJS)
	$(CC) -shared -o $@ $^

# contacts_logic.o компилируем с -fPIC
contacts_logic.o: $(SRC_DIR)/contacts_logic.c
	$(CC) $(CFLAGS) $(PICFLAGS) -c $< -o $@

# все остальные .c из src/ — в обычные .o
%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# линковка приложения с динамической библиотекой
contacts_app: $(APP_OBJS)
	$(CC) $(APP_OBJS) $(LDFLAGS) -o $@

# убрать все .o, .so и бинарник
clean:
	rm -f *.o $(LIB) contacts_app
