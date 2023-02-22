#include "eth_tool.h"

static void pr_skb_main_info(const struct sk_buff *skb)
{
    pr_info("Len: %d Data_len: %d True_size: %d Hdr_len: %d\n", 
            skb->len, skb->data_len, skb->truesize, skb->hdr_len);
    pr_info("Mac_hdr_len: %d\n", skb->mac_len);

    // unsigned char* mac_h = skb_mac_header(skb);
    // unsigned char* data = skb->data;


    unsigned char* h = skb->head;
    unsigned char* d = skb->data;
    // unsigned char* t = skb->tail;
    // unsigned char* e = skb->end;

    // pr_info("SKB total %ld", e - h);
    pr_info("SKB headroom %ld, %d", d - h, skb_headroom(skb));
    pr_info("SKB tailroom %d", skb_tailroom(skb));
    // pr_info("Head_p %p", (skb->head));
    // pr_info("Data_p %p", skb->data);
    // pr_info("Head_p2 %lx", (long unsigned int) &(skb->head));
    pr_info("Head %lx", (long unsigned int) (skb->head));
    pr_info("Data %lx", (long unsigned int) skb->data);
    pr_info("Tail %x, dec: %d", skb->tail, skb->tail);
    pr_info("End  %x, dec: %d", skb->end, skb->end);
    
    pr_info("skb->mac_header: %d", skb->mac_header);
    pr_info("skb->network_header: %d", skb->network_header);
    pr_info("skb->transport_header: %d\n", skb->transport_header);

    u8 count = 1;
    for(u16 i = 0; i < skb->len; i++)
    {
        pr_cont("%02x", skb->data[i]);
        if (count % 4 == 0)
        {
            pr_cont(" ");
        }

        if (count == 16)
        {
            pr_info("");
            count = 0;
        }
        count++;
    }
    pr_info("\n");
}

void pr_sk_buff_info(const struct sk_buff *skb)
{
    u8 nr_frags = skb_shinfo(skb)->nr_frags;

    pr_info("// ---------- ---------- ----------\n");
    pr_info("%s nr_frags %d\n", __FUNCTION__,  nr_frags);

    pr_skb_main_info(skb);

    for(u8 frag = 0; frag < nr_frags; frag++)
    {
    }
}
