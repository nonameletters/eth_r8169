#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/netdevice.h> // struct net_device
#include <linux/etherdevice.h> // alloc_etherdev
#include <linux/socket.h>
#include <linux/ethtool.h>

#include "eth_r8169.h"

#define R8169_REGS_SIZE     256
#define BAR_MAX             6
#define BAR0_OFFSET         0x10
#define INTR_MASK_REG       0x3C
#define INTR_STAT_REG       0x3E
#define COMMAND_REG         0x37

struct net_device *net_dev = NULL;

static u8 mac[ETH_ALEN] = {0x50, 0xb7, 0xc3, 0x69, 0x57, 0x8c};

static struct pci_device_id r_8169[] = {
        {PCI_DEVICE(0x10EC, 0x8169)},
        {PCI_DEVICE(0x10EC, 0x8168)},
        {0,}
};


typedef struct __rt_net_priv
{
    void __iomem *iomem;
    struct net_device *net_dev;
    struct pci_dev *pdev;
} rt_net_priv;


void pr_bar_info(struct pci_dev *pdev)
{
    int rc = 0;
    int i = 0;
    int offset = 0;
    u32 val = 0;
    u8 byte = 0;

    for(i = 0; i < BAR_MAX; i++)
    {
        offset = BAR0_OFFSET + i * sizeof(val);
        rc = pci_read_config_dword(pdev, offset, &val);
        pr_info("BAR%d offset: %x value: %x\n", i, offset, val);
        
        // TODO: Implement correct size output
        // pci_write_config_dword(pdev, offset, 0xFFFFFFFF);        
        // rc = pci_read_config_dword(pdev, offset, &val);
        // pr_info("BAR%d offset: %x value sz: %x\n", i, offset, val);
    }

    pci_read_config_byte(pdev, 0x3C, &byte);
    pr_info("Interrupt pin: %x-dec %d\n", byte, byte);    
    pci_read_config_byte(pdev, 0x3D, &byte);
    pr_info("Interrupt line: %x-dec %d\n", byte, byte);    
}

static void rt_irq_stat_clean(void)
{
    rt_net_priv *rtp = netdev_priv(net_dev);
    // It seems to clear bit we must write 1
    writew(0xffff, rtp->iomem + INTR_STAT_REG);
}

static void rt_irq_stat(void)
{
    uint16_t valw;
    rt_net_priv *rtp = netdev_priv(net_dev);
    valw = readw(rtp->iomem + INTR_STAT_REG);
    pr_info("Interrupt status %x\n", valw);
}

static void rt_irq_enable(void)
{
    uint16_t valw;
    rt_net_priv *rtp = netdev_priv(net_dev);
    valw = readw(rtp->iomem + INTR_MASK_REG);
    pr_info("InterMask 1: %x\n", valw); 
   
    rt_irq_stat_clean(); 
    rt_irq_stat();

    writew(0x25, rtp->iomem + INTR_MASK_REG); // TX-RX interrupt && LinkChanged 
    // writew(0x05, rtp->iomem + INTR_MASK_REG); // TX-RX interrupt 
    
    valw = readw(rtp->iomem + INTR_MASK_REG);
    pr_info("InterMask 2: %x\n", valw); 
    rt_irq_stat();
}

static void rt_irq_disable(void)
{
    uint16_t valw = 0x0;
    rt_net_priv *rtp = netdev_priv(net_dev);
    writew(0x0, rtp->iomem + INTR_MASK_REG);

    valw = readw(rtp->iomem + INTR_MASK_REG);
    pr_info("Inrerrupt mask reg: %x afrer cleaning\n", valw);
    rt_irq_stat_clean();
}


static struct ethtool_ops rt_ethtool_ops = {
    .get_link = ethtool_op_get_link
};

static int rt_open(struct net_device *dev)
{
    int res = 0;
    uint8_t byte = 0x0;
    void __iomem* mem;
    struct sockaddr lmac;

    rt_net_priv* rtp = netdev_priv(dev);
    mem = rtp->iomem;

    memcpy(lmac.sa_data, mac, ETH_ALEN);

    pr_info("%s DEV:%s\n", __FUNCTION__, dev->name);
    if (!netif_running(dev))
    {
        pr_info("Device is running. Can't change HW addr\n");
    }
    
    res = eth_mac_addr(dev, (void*)&lmac);
    if (res != 0)
    {
        pr_info("Failed to set HW addr %d\n", res);
    }

    byte = readb(mem + COMMAND_REG);
    pr_info("%s: Command reg state 1: %x\n", __FUNCTION__, byte);
    writeb(byte | 0x8 /*Rx - enable*/ | 0x4 /*TX - enable*/, mem + COMMAND_REG);

    byte = readb(mem + COMMAND_REG);
    pr_info("%s: Command reg state 2: %x\n", __FUNCTION__, byte);
    return res;
}

static int rt_stop(struct net_device *dev)
{
    char* handler_name;
    rt_net_priv *rtp = NULL;
    rtp = netdev_priv(dev);

    // pr_info("%s DEV:%s\n", __FUNCTION__, dev->name);
    pr_info("%s call net_ops \"stop\".\n", __FUNCTION__);

    handler_name = (char*) free_irq(pci_irq_vector(rtp->pdev,0), rtp);

    pr_info("%s after free interrupt:%s\n", __FUNCTION__, handler_name);
    return 0;
}

static netdev_tx_t rt_start_xmit(struct sk_buff *buff, struct net_device *dev)
{
    static u32 count = 0;
    // netdev_tx_t tx - NETDEV_TX_OK and others NETDEV_TX_XXX
    u8 nr_frags = skb_shinfo(buff)->nr_frags;
    
    pr_info("%d %s DEV:%s nr_frags %d\n",count, __FUNCTION__, dev->name, nr_frags);

    for(u8 cur = 0; cur < nr_frags; cur++)
    {
    }

    count++;
    return NETDEV_TX_OK;
}

static void rt_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
    // pr_info("%s DEV:%s\n", __FUNCTION__, dev->name);
    return;
}

int my_eth_validate_addr(struct net_device* net_dev)
{
    pr_info("%s\n", __FUNCTION__);
    return 0;
}

static struct net_device_ops r_8169_netdev_ops = {
    .ndo_open          = rt_open,
    .ndo_stop          = rt_stop,
    .ndo_start_xmit    = rt_start_xmit,
    .ndo_get_stats64   = rt_get_stats64,
    .ndo_validate_addr = my_eth_validate_addr
};


irqreturn_t eth_rx_irq(int val, void* dev)
{
    pr_info("%s\n", __FUNCTION__);
    rt_irq_stat();
    rt_irq_disable();
    rt_irq_stat_clean();
    rt_irq_stat();
    return IRQ_HANDLED;
}


int eth_probe(struct pci_dev *pdev, const struct pci_device_id *table)
{
    int res = 0;
    int region = 0;
    rt_net_priv *rtp = NULL;
    u8 val = 0;
    unsigned long irqflags = 0x0;

    pr_info("IRQ from struct 0 %d\n", pdev->irq);
    // Actually enable device;
    printk("Probing VEN:%x DEV:%x\n", pdev->vendor, pdev->device);
    res = pci_enable_device(pdev);
    pr_info("IRQ from struct 1 %d\n", pdev->irq);
    if (res == 0)
    {
        pr_info("Device %x:%x enabled. Header type %x\n", pdev->vendor, pdev->device, pdev->hdr_type);
    }
   
    pr_bar_info(pdev);

    if (pcim_set_mwi(pdev) < 0)
        dev_info(&pdev->dev, "Mem-Wr-Inval unavailable\n");

    /* use first MMIO region */
    res = pci_select_bars(pdev, IORESOURCE_MEM);
    pr_info("BARS: %x\n", res);
    region = ffs(pci_select_bars(pdev, IORESOURCE_MEM)) - 1;
    pr_info("Returned region: %x\n", region);
    if (region < 0) {
        dev_err(&pdev->dev, "no MMIO resource found\n");
        return -ENODEV;
    }

    //pr_info("IRQ from struct %d\n", pci_irq_vector(pdev,0));
    pr_info("IRQ from struct 2 %d\n", pdev->irq);

    /* check for weird/broken PCI region reporting */
    if (pci_resource_len(pdev, region) < R8169_REGS_SIZE) {
        dev_err(&pdev->dev, "Invalid PCI region size(s), aborting\n");
        return -ENODEV;
    }

    res = pcim_iomap_regions(pdev, BIT(region), KBUILD_MODNAME);
    if (res < 0) {
        dev_err(&pdev->dev, "cannot remap MMIO, aborting\n");
        return res;
    }
    
    net_dev = NULL;
    net_dev = alloc_etherdev(sizeof(rt_net_priv));
    if (!net_dev)
    {
        pr_info("Net device allocation FAILED\n");
        return -ENOMEM;
    }

    rtp = netdev_priv(net_dev);
    rtp->pdev = pdev;
    rtp->iomem = pcim_iomap_table(pdev)[region];
    val = readb(rtp->iomem);
    pr_info("8169 m: %x\n", val);
    
    net_dev->priv_flags |= IFF_LIVE_ADDR_CHANGE;
    net_dev->netdev_ops = &r_8169_netdev_ops;
    net_dev->ethtool_ops = &rt_ethtool_ops;

    res = register_netdev(net_dev); // Configure everythin before call
    netif_carrier_on(net_dev);
    if (res != 0)
    {
        pr_info("Net device registration FAILED.\n");
        return -ENODEV;
    }
    
    if (netif_carrier_ok(net_dev))
    {
        pr_info("NetIf carrier ON\n");
    }
    else
    {
        pr_info("NetIf carrier OFF\n");
    }

    // irqflags = pci_dev_msi_enabled(pdev) ? IRQF_NO_THREAD : IRQF_SHARED; 
    // pr_info("MSI enabled: %x\n", pdev->msi_enabled);
    // pr_info("MSI-X enabled: %x\n", pdev->msix_enabled);
    
    // For test
    irqflags = pci_dev_msi_enabled(pdev) ? IRQF_NO_THREAD : IRQF_NO_THREAD; 
    if (irqflags == IRQF_NO_THREAD)
    {
        pr_info("IRQ_FLAG: IRQF_NO_THREAD\n");
    }
    else
    {
        pr_info("IRQ_FLAG: IRQF_SHARED\n");
    }


    // res = pci_alloc_irq_vectors(pdev, 1, 1, PCI_IRQ_MSI);
    // pr_info("IRQ vector allocated: %d\n", res);

    res = request_irq(pci_irq_vector(pdev,0), eth_rx_irq, irqflags, "eth_r8169_irq", rtp);
    if (res < 0)
    {
        pr_info("IRQ request failed.\n");
    }

    rt_irq_enable();

    pr_info("Probing VEN:%x DEV:%x DONE\n", pdev->vendor, pdev->device);
    return res;
}

void eth_remove(struct pci_dev *pdev)
{

    printk("Remove eth 0x8169\n");
    rt_irq_disable();
    // pci_free_irq_vectors(pdev);
    pcim_iounmap_regions(pdev, BIT(0x2));
    pci_disable_device(pdev);
    if (net_dev)
    {
        // TODO: Why it failes?
        // unregister_netdev(net_dev);
        // free_netdev(net_dev);
    }
}

static struct pci_driver eth_drv = {
    .name = "eth_r8169",
    .id_table = r_8169,
    .probe = eth_probe,
    .remove = eth_remove
};

static int enter_m(void)
{
    int res = 0;
    pr_info("// ---------- ---------- ---------- ---------- ---------- ----------\n");
	printk("Enter R8169\n");
    // 1. Register driver in PCI CORE

    res = pci_register_driver(&eth_drv);
    if (res == 0)
    {
        printk("R8169 register driver. DONE\n");
    }

	return res;
}

static void exit_m(void)
{
	printk("Exit R8169\n"); 
    // Just to test
    // rt_irq_disable();

    pci_unregister_driver(&eth_drv);
    unregister_netdev(net_dev);
}

module_init(enter_m);
module_exit(exit_m);
MODULE_LICENSE("Dual BSD/GPL");
