#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "netlink_common.h"

int open_netlink(void) {
    int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid    = getpid(); // PID процесса
    addr.nl_groups = 0;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        return -1;
    }
    return sock;
}

void usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s listen      # подписаться и ждать сообщений\n", prog);
    printf("  %s send <msg>  # отправить <msg> всем подписчикам\n", prog);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Режим слушателя
    if (strcmp(argv[1], "listen") == 0) {
        int sock = open_netlink();
        if (sock < 0) return EXIT_FAILURE;

        // Подписка на группу
        int grp = NETLINK_GROUP;
        if (setsockopt(sock, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, &grp, sizeof(grp)) < 0) {
            perror("setsockopt");
            close(sock);
            return EXIT_FAILURE;
        }

        printf("Listening for broadcast messages (group %d)…\n", grp);
        while (1) {
            struct sockaddr_nl nladdr;
            struct iovec iov;
            struct msghdr msg = { 0 };
            char buffer[MAX_PAYLOAD];

            struct nlmsghdr *nlh = (struct nlmsghdr*)buffer;
            iov.iov_base    = buffer;
            iov.iov_len     = sizeof(buffer);
            msg.msg_name    = &nladdr;
            msg.msg_namelen = sizeof(nladdr);
            msg.msg_iov     = &iov;
            msg.msg_iovlen  = 1;

            // Получение сообщения
            ssize_t ret = recvmsg(sock, &msg, 0);
            if (ret < 0) {
                perror("recvmsg");
                break;
            }

            // Получение данных
            char *data = (char *)NLMSG_DATA((struct nlmsghdr *)buffer);

            // Специальный сигнал выхода
            if (strcmp(data, EXIT_MESSAGE) == 0) {
                printf(">> broadcast: received shutdown, exiting.\n");
                break;
            }
            // Вывод только самого payload
            printf(">> broadcast: %.*s\n", (int)nlh->nlmsg_len - NLMSG_HDRLEN, (char*)NLMSG_DATA(nlh));
        }

        close(sock);
    } // Режим отправки сообщения ядру 
    else if (strcmp(argv[1], "send") == 0 && argc == 3) {
        const char *payload = argv[2];
        int sock = open_netlink();
        if (sock < 0) return EXIT_FAILURE;

        struct sockaddr_nl dest;
        memset(&dest, 0, sizeof(dest));
        dest.nl_family = AF_NETLINK;
        dest.nl_pid    = 0;    // Ядро
        dest.nl_groups = 0;    // Не групповое сообщение

        // формируем nlmsg
        struct nlmsghdr *nlh;
        int msg_size = strlen(payload) + 1;
        nlh = malloc(NLMSG_SPACE(msg_size));
        if (!nlh) {
            perror("malloc");
            close(sock);
            return EXIT_FAILURE;
        }

        nlh->nlmsg_len   = NLMSG_LENGTH(msg_size);
        nlh->nlmsg_pid   = getpid();
        nlh->nlmsg_flags = 0;
        memcpy(NLMSG_DATA(nlh), payload, msg_size);

        struct iovec iov = { nlh, nlh->nlmsg_len };
        struct msghdr msg = {
            .msg_name    = &dest,
            .msg_namelen = sizeof(dest),
            .msg_iov     = &iov,
            .msg_iovlen  = 1,
        };

        printf("Sending to kernel: \"%s\"\n", payload);
        if (sendmsg(sock, &msg, 0) < 0) {
            perror("sendmsg");
        }

        free(nlh);
        close(sock);
    }
    else {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}