#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip.h> 
#include <netinet/udp.h>
#include <stdlib.h>
#include <unistd.h>

void build_ip_header(struct iphdr *ip, int payload_len, const char* src_ip, const char* dst_ip) {
    ip->ihl = 5; // Длина заголовка 5*4 = 20 байт
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(*ip) + sizeof(struct udphdr) + payload_len); // Общий размер
    ip->id = htons(0);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_UDP;
    ip->check = 0;
    ip->saddr = inet_addr(src_ip);
    ip->daddr = inet_addr(dst_ip);
}

void build_udp_header(struct udphdr *udp, int payload_len, unsigned short src_port, unsigned short dst_port) {
    udp->source = htons(src_port);
    udp->dest = htons(dst_port);
    udp->len = htons(sizeof(*udp) + payload_len);
    udp->check = 0; 
}

int raw_sendto(int sockfd,
               const char *payload, int payload_len,
               const char *src_ip, unsigned short src_port,
               const char *dst_ip, unsigned short dst_port,
               struct sockaddr_in *dst_addr) {

    // Выделяем память под весь пакет
    int packet_len = sizeof(struct iphdr) + sizeof(struct udphdr) + payload_len;
    char *packet = malloc(packet_len);
    if (!packet) return -1;

    // Создаем указатели в пакете
    struct iphdr *ip  = (struct iphdr*)packet;
    struct udphdr *udp = (struct udphdr*)(packet + sizeof(*ip));
    char *data = packet + sizeof(*ip) + sizeof(*udp);
    
    // Копируем данные в пакет
    memcpy(data, payload, payload_len);
    
    // Заполняем ip структуру
    build_ip_header(ip, payload_len, src_ip, dst_ip);

    // Заполняем udp структуру
    build_udp_header(udp, payload_len, src_port, dst_port);

    int res = sendto(sockfd, packet, packet_len, 0, (struct sockaddr*)dst_addr, sizeof(*dst_addr));

    free(packet);
    return res;
}

int packet_is_for_me(const unsigned char *pkt, ssize_t pkt_len,
                            uint32_t sip,  unsigned short sport,
                            uint32_t dip,  unsigned short dport)
{
    const struct iphdr *ip = (const struct iphdr*)pkt;
    int iph_len = ip->ihl * 4;
    const struct udphdr *udp = (const struct udphdr*)(pkt + iph_len);

    if (ip->saddr != sip) return 0;
    if (udp->source != htons(sport)) return 0;
    if (udp->dest  != htons(dport)) return 0;

    return 1;
}

// Выдача временного порта
unsigned short get_ephemeral_port(int sock) {
    // Создаем порт
    if (sock < 0) {
        perror("socket for ephemeral port");
        exit(EXIT_FAILURE);
    }

    // Заполняем данными
    struct sockaddr_in addr = {0};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // Просим систему выдать нам порт
    addr.sin_port        = htons(0);

    // Связываем сокет с заполненными данным
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind for ephemeral port");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Получаем назначенный порт
    socklen_t len = sizeof(addr);
    if (getsockname(sock, (struct sockaddr *)&addr, &len) < 0) {
        perror("getsockname");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Извлекаем порт
    unsigned short port = ntohs(addr.sin_port);
    return port;
}

int extract_ip_udp_data(char *packet,
                        struct iphdr **pip,
                        struct udphdr **pudp,
                        char *data,
                        int packet_len) {

    // Парсим IP заголовок
    *pip = (struct iphdr *)packet;
    int ip_hlen = (*pip)->ihl * 4;
    if (packet_len <= ip_hlen) {
        return -1;  // некорректный IP заголовок
    }

    // Парсим UDP заголовок
    *pudp = (struct udphdr *)(packet + ip_hlen);
    int udp_hlen = sizeof(struct udphdr);
    if (packet_len <= ip_hlen + udp_hlen) {
        return -1;  // нет места для payload
    }

    // Копируем нагрузку
    int payload_len = packet_len - ip_hlen - udp_hlen;
    memcpy(data, packet + ip_hlen + udp_hlen, payload_len);

    return payload_len;
}

int create_raw_socket() {
    int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

    if (socket_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Включаем ручное формирование ip заголовка
    int one = 1;
    if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt"); 
        exit(EXIT_FAILURE);
    }

    return socket_fd;
}