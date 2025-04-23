#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

// Считываем 32-битное значение в виде строки и возвращаем число
unsigned int parse_ip(char* ip_str) {
    struct in_addr addr;
    if (inet_pton(AF_INET, ip_str, &addr) != 1) {
        printf("Неверный формат ввода!\n");
        return -1;
    }
    return ntohl(addr.s_addr);
}

int main(int argc, char *argv[]) {
    printf("Для работы введите: <gateway_IP> <subnet_mask> <packet_count>\n");

    // Проверка аргументов
    if (argc != 4) {
        printf("Для работы введите: <gateway_IP> <subnet_mask> <packet_count>\n");
        return EXIT_FAILURE;
    }

    // Считываем аргументы пользователья
    unsigned int gateway_ip = parse_ip(argv[1]);
    unsigned int subnet_mask = parse_ip(argv[2]);
    int packet_count = atoi(argv[3]);

    if (packet_count <= 0) {
        printf(stderr, "Ошибка: количество пакетов должно быть положительным числом\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    // Считаем пакеты для нашей и чужой сети
    int own_count = 0, other_count = 0;
    for (int i = 0; i < packet_count; i++) {
        unsigned int dest_ip = ((unsigned int)rand() << 1) | (rand() & 1);
        if ((dest_ip & subnet_mask) == (gateway_ip & subnet_mask)) {
            own_count++;
        } else {
            other_count++;
        }
    }

    double percent_own = (own_count * 100.0) / packet_count;
    double percent_other = (other_count * 100.0) / packet_count;

    // Вывод статистики
    printf("Пакетов в своей подсети: %d (%.2f%%)\n", own_count, percent_own);
    printf("Пакетов в других сетях: %d (%.2f%%)\n", other_count, percent_other);
    
    return EXIT_SUCCESS;
}