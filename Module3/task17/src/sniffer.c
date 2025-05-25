#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>
#include <fcntl.h>

#include "utils.h"
#include "raw_utils.h"

#define BUFSIZE 1024

int main(void) {
    struct sockaddr_in server_addr = {0};
    int sockfd;
    char packet[BUFSIZE];

    // Открываем raw-сокет для UDP
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        fatal("socket");
    }

    // Получаем данные сервера
    create_server_data(&server_addr);

    printf("Sniffer запущен, ожидание UDP-пакетов...\n");

    while (1) {
        // Получение пакета 
        ssize_t packet_len = recvfrom(sockfd, packet, BUFSIZE, 0, NULL, NULL);
        if (packet_len < 0) {
            perror("recvfrom");
            break;
        }

        // Разборка полученного пакета
        struct iphdr  *ip;
        struct udphdr *udp;
        char msg[BUFSIZE];
        int data_len = extract_ip_udp_data(packet, &ip, &udp, msg, packet_len);
        if (data_len < 0) {
            close(sockfd);
            fatal("Ошибка разбора пакета\n");
        }
        msg[data_len] = '\0';

        // Фильтр (перехват пакетов, предназначавшихся серверу)
        if (ip->protocol != IPPROTO_UDP) continue;   
        if (ip->daddr != server_addr.sin_addr.s_addr) continue;
        if (udp->dest  != server_addr.sin_port) continue;

        // Сохраняем данные в дамп-файл
        int fd = open("dump.bin", O_CREAT | O_WRONLY | O_APPEND, 0644);
        if (fd < 0) {
            perror("open");
            break;
        }
        if (write(fd, msg, packet_len) < 0) {
            perror("write");
            close(fd);
            break;
        }
        close(fd);

        // Выводим информацию в консоль
        char src_ip[INET_ADDRSTRLEN];
        char dst_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip->saddr, src_ip, sizeof(src_ip));
        inet_ntop(AF_INET, &ip->daddr, dst_ip, sizeof(dst_ip));

        printf("Пакет %d байт: %s:%d -> %s:%d, данные: %.*s\n",
               packet_len,
               src_ip, ntohs(udp->source),
               dst_ip, ntohs(udp->dest),
               packet_len, msg);

        // Обработка выхода
        if (strcmp(msg, EXIT_MESSAGE) == 0) {
            break;
        }
    }

    close(sockfd);
    return EXIT_SUCCESS;
}
