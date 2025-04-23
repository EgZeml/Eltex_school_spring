#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "permissions.h"

int main() {
    int choice;
    char input[100];
    FilePermission edit_files[10]; // Массив для файлов с изменёнными правами
    int edit_count = 0; // Текущее количество файлов в массиве
    int current_perm = 0;
    
    do {
        printf("Меню:\n");
        printf("1. Ввод прав доступа и их конвертация\n");
        printf("2. Получение информации о файле\n");
        printf("3. Модификация прав доступа (симуляция)\n");
        printf("0. Выход\n");
        printf("Выберите опцию: ");
        scanf("%d", &choice);
        getchar(); // Очистка символа перевода строки после scanf
        
        switch(choice) {
            case 1:
                printf("Введите права доступа (буквенное, напр. rwxr-xr-- или цифровое, напр. 754): ");
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';
                current_perm = process_permission_input(input);
                if (current_perm != -1) {
                    print_permissions(current_perm);
                }
                break;
            case 2:
                printf("Введите имя файла: ");
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';
                if (print_file_info(input) == -1) {
                    fprintf(stderr, "Ошибка при получении информации о файле.\n");
                }
                break;
            case 3: {
                // Запрос имени файла
                printf("Введите имя файла: ");
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = '\0';
                
                // Поиск файла в массиве edit_files
                int found = 0;
                int index = 0;
                for (int i = 0; i < edit_count; i++) {
                    if (strcmp(edit_files[i].name, input) == 0) {
                        found = 1;
                        index = i;
                        break;
                    }
                }
                
                if (found) {
                    current_perm = edit_files[index].permission;
                    printf("Файл найден в edit_files. Текущие сохранённые права для файла %s:\n", input);
                    print_permissions(current_perm);
                } else {
                    // Если файл не найден, получаем права через stat()
                    struct stat st;
                    if (stat(input, &st) == -1) {
                        printf("Ошибка вызова stat");
                        break;
                    }
                    current_perm = st.st_mode & 0777;
                    printf("Файл не найден в edit_files. Текущие права доступа файла %s (получены через stat):\n", input);
                    print_permissions(current_perm);
                }
                
                // Запрос команды модификации прав
                printf("Введите команду модификации (например, u+x, g-w, o=r или числовой режим): ");
                char cmd[20];
                fgets(cmd, sizeof(cmd), stdin);
                cmd[strcspn(cmd, "\n")] = '\0';
                current_perm = modify_permissions(current_perm, cmd);
                printf("Новые права доступа:\n");
                print_permissions(current_perm);
                
                // Обновление или добавление файла в массив edit_files
                if (found) {
                    edit_files[index].permission = current_perm;
                } else {
                    if (edit_count < 10) {
                        strncpy(edit_files[edit_count].name, input, FILE_NAME_LEN - 1);
                        edit_files[edit_count].name[FILE_NAME_LEN - 1] = '\0';
                        edit_files[edit_count].permission = current_perm;
                        edit_count++;
                    } else {
                        fprintf(stderr, "Массив edit_files заполнен.\n");
                    }
                }
                break;
            }
            default:
                printf("Неверный выбор.\n");
        }
    } while (choice != 0);
    
    return 0;
}