#ifndef ETH_TOOL_H
#define ETH_TOOL_H

#include <linux/ethtool.h>

void pr_sk_buff_info(const struct sk_buff *skb);

#endif // ETH_TOOL_H
