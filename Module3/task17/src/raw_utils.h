#ifndef RAW_UTILS_H
#define RAW_UTILS_H

#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>

/**
 * @brief Построение IP-заголовка.
 * @param ip        Указатель на структуру iphdr для заполнения.
 * @param payload_len Длина полезной нагрузки.
 * @param src_ip    Строка с исходным IP-адресом (например, "192.168.1.2").
 * @param dst_ip    Строка с целевым IP-адресом.
 */
void build_ip_header(struct iphdr *ip, int payload_len,
                     const char *src_ip, const char *dst_ip);

/**
 * @brief Построение UDP-заголовка.
 * @param udp       Указатель на структуру udphdr для заполнения.
 * @param payload_len Длина полезной нагрузки.
 * @param src_port  Исходный порт.
 * @param dst_port  Целевой порт.
 */
void build_udp_header(struct udphdr *udp, int payload_len,
                      unsigned short src_port, unsigned short dst_port);

/**
 * @brief Отправка "сырых" UDP-пакетов.
 * @param sockfd    Файловый дескриптор сокета.
 * @param payload   Указатель на данные для отправки.
 * @param payload_len Размер данных.
 * @param src_ip    Строка исходного IP-адреса.
 * @param src_port  Исходный порт.
 * @param dst_ip    Строка целевого IP-адреса.
 * @param dst_port  Целевой порт.
 * @param dst_addr  Указатель на структуру sockaddr_in с адресом получателя.
 * @return Количество отправленных байт или -1 при ошибке.
 */
int raw_sendto(int sockfd,
               const char *payload, int payload_len,
               const char *src_ip, unsigned short src_port,
               const char *dst_ip, unsigned short dst_port,
               struct sockaddr_in *dst_addr);

/**
 * @brief Получение эфемерного порта, выделенного ОС.
 * @param sock  Дескриптор сокета.
 * @return Эфемерный порт.
 */
static unsigned short get_ephemeral_port(int sock);

/**
 * @brief Проверка, адресован ли пакет данному получателю.
 * @param pkt   Указатель на данные пакета.
 * @param pkt_len Длина пакета.
 * @param sip   Source IP (host byte order).
 * @param sport Source port (host byte order).
 * @param dip   Dest IP (host byte order).
 * @param dport Dest port (host byte order).
 * @return 1, если пакет адресован нам; 0 иначе.
 */
static int packet_is_for_me(const unsigned char *pkt, ssize_t pkt_len,
                            uint32_t sip,  unsigned short sport,
                            uint32_t dip,  unsigned short dport);

/**
 * @brief Извлечение IP/UDP-заголовков и данных из пакета.
 * @param packet    Указатель на начало пакета.
 * @param pip       Адрес для указателя на iphdr.
 * @param pudp      Адрес для указателя на udphdr.
 * @param data      Буфер для полезных данных.
 * @param packet_len Длина пакета.
 * @return Количество извлеченных байт данных.
 */
int extract_ip_udp_data(char *packet,
                        struct iphdr **pip,
                        struct udphdr **pudp,
                        char *data,
                        int packet_len);

/**
 * @brief Создание RAW-сокета для отправки/приема IP-пакетов.
 * @return Дескриптор сокета или -1 при ошибке.
 */
int create_raw_socket(void);

#endif // RAW_UTILS_H
