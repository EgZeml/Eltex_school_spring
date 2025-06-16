#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>
#include <linux/string.h> 

#include "netlink_common.h"

struct sock *nl_sk = NULL; // Сокет ядра

static void nl_recv_msg(struct sk_buff *skb) {
    // Разборка полученного пакета
    struct nlmsghdr *nlh = nlmsg_hdr(skb);
    char *payload = (char *)nlmsg_data(nlh);
    int len = nlmsg_len(nlh);
    printk(KERN_INFO "netlink_broadcast: got msg: \"%s\"\n", payload);

    // Сборка нового skb для рассылки
    struct sk_buff *skb_out;
    struct nlmsghdr *nlh_out;
    int msg_size = len;
    
    // Выделение буфера под сообщение
    skb_out = nlmsg_new(msg_size, GFP_KERNEL);
    if (!skb_out) {
        printk(KERN_ERR "netlink_broadcast: failed to alloc skb_out\n");
        return;
    }
    // Инициализация Netlink-заголовка внутри skb
    nlh_out = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    if (!nlh_out) {
        printk(KERN_ERR "netlink_broadcast: nlmsg_put failed\n");
        nlmsg_free(skb_out);
        return;
    }

    memcpy(nlmsg_data(nlh_out), payload, msg_size);

    // Указание группы для multicast рассылки
    NETLINK_CB(skb_out).dst_group = NETLINK_GROUP;

    // Multicast всем подписчикам группы
    nlmsg_multicast(nl_sk, skb_out, 0, NETLINK_GROUP, GFP_KERNEL);

}

struct netlink_kernel_cfg cfg = {
    .input  = nl_recv_msg,
    .groups = 1 << (NETLINK_GROUP - 1),  // Битовая маска подписки: группа N -> 1 << N - 1
};

static int __init broadcast_init(void) {
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        pr_err("netlink_broadcast: failed to create netlink socket\n");
        return -ENOMEM;
    }
    pr_info("netlink_broadcast: module loaded\n");
    return 0;
}

static void __exit broadcast_exit(void) {
    struct sk_buff *skb_out;
    struct nlmsghdr *nlh_out;
    int msg_size = strlen(EXIT_MESSAGE) + 1;

    skb_out = nlmsg_new(msg_size, GFP_KERNEL);
    if (skb_out) {
        nlh_out = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
        if (nlh_out) {
            memcpy(nlmsg_data(nlh_out), EXIT_MESSAGE, msg_size);
            NETLINK_CB(skb_out).dst_group = NETLINK_GROUP;
            nlmsg_multicast(nl_sk, skb_out, 0, NETLINK_GROUP, GFP_KERNEL);
            pr_info("netlink_broadcast: sent shutdown broadcast\n");
        } else {
            nlmsg_free(skb_out);
            pr_err("netlink_broadcast: nlmsg_put failed in exit\n");
        }
    } else {
        pr_err("netlink_broadcast: alloc skb_out failed in exit\n");
    }

    netlink_kernel_release(nl_sk);
    pr_info("netlink_broadcast: module unloaded\n");
}


module_init(broadcast_init);
module_exit(broadcast_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Egor Zemlyanoi");
MODULE_DESCRIPTION("Netlink broadcast kernel module");
