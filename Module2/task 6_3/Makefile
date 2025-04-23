# Makefile для проекта калькулятора с плагинами
# Расположить в корне рядом с папками src/ и plugins/

CC         := gcc
CFLAGS     := -Wall -Wextra -fPIC
LDFLAGS    := -ldl

SRC_DIR    := src
PLUGIN_DIR := plugins

APP        := calculator

# Файлы, которые не должны собираться как плагины
EXCLUDE_SRCS := $(SRC_DIR)/main.c $(SRC_DIR)/calculator.c

# В приложение идёт только main.c
CORE_SRCS   := $(SRC_DIR)/main.c

# Плагин‑исходники: все .c, кроме тех, что в EXCLUDE_SRCS
PLUGIN_SRCS := $(filter-out $(EXCLUDE_SRCS), $(wildcard $(SRC_DIR)/*.c))
PLUGIN_LIBS := $(patsubst $(SRC_DIR)/%.c, $(PLUGIN_DIR)/lib%.so, $(PLUGIN_SRCS))

.PHONY: all clean

# По умолчанию: собрать, запустить, и после выхода очистить
all: $(APP) plugins
	@echo "\nЗапуск программы... (по завершении будет выполнена очистка)"
	@./$(APP)
	@echo "\nОчистка артефактов..."
	@$(MAKE) clean

# Собираем калькулятор (main.c)
$(APP): $(CORE_SRCS) $(PLUGIN_LIBS)
	$(CC) $(CFLAGS) -o $@ $(CORE_SRCS) $(LDFLAGS)

# Мета‑цель для сборки плагинов
plugins: $(PLUGIN_LIBS)

# Правило сборки каждого плагина; при необходимости создаёт каталог plugins/
$(PLUGIN_DIR)/lib%.so: $(SRC_DIR)/%.c
	mkdir -p $(PLUGIN_DIR)
	$(CC) $(CFLAGS) -shared $< -o $@

# Удаляем бинарник и папку с плагинами
clean:
	rm -f $(APP)
	rm -rf $(PLUGIN_DIR)
