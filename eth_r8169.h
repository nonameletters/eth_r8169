#ifndef ETH_R8169_H
#define ETH_R8169_H

// Transmit Configuration Register
#define TCR0 0x0040U
#define TCR1 0x0041U
#define TCR2 0x0042U
#define TCR3 0x0043U

// Transmit Normal Priority Registrt 0x0020-0x0027 
// As I understood 64 bit register. We have 4 pair of registers
#define TNPR0 0x0020U
#define TNPR_LOW  0x0020U
#define TNPR_HIGH 0x0024U

// Transmit High Priority Registrt 0x0028-0x002F 
// As I understood 64 bit register
#define THPR0 0x0028U

// Transmit Priority Pooling (size is byte)
#define TPPOOL 0x0038U 

#define CONFIG_2   0x0053U
#define CONFIG_3   0x0054U
#define PHY_STATUS 0x006CU

#define INTR_MASK_REG       0x3C
#define INTR_STAT_REG       0x3E
#define COMMAND_REG         0x37


typedef struct __desc
{
	u32 opts0;
	u32 opts1;
	u64 addr;
} desc;


#endif
