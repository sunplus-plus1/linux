// SPDX-License-Identifier: GPL-2.0
/*
 * Sunplus Parallel NAND Cotroller driver
 *
 * Copyright Sunplus Technology Co., Ltd.
 *
 * Derived from:
 *	Copyright (C) 2019-2021 Faraday Technology Corp.
 */

/**********************************************************
*	Platform(user) specific
**********************************************************/
#define NAME_DEFINE_IN_UBOOT		"sp_paranand.0"

/* Frequency Setting (unit: HZ) */
#define CONFIG_SP_CLK_100M		100000000
#define CONFIG_SP_CLK_200M		200000000
#define CONFIG_SP_CLK_400M		400000000

/* Reserved BI byte (bad block information) from spare location, BI_byte = 1~6 */
#define CONFIG_BI_BYTE		0

/* Debug level */
#define CONFIG_SP_DEBUG		1

/**********************************************************
*	general register difinition
**********************************************************/
#define ECC_CONTROL			0x8
#define ECC_ERR_MASK(x)			(x << 0)
#define ECC_EN(x)			(x << 8)
#define ECC_BASE			(1 << 16)
#define ECC_NO_PARITY			(1 << 17)
#define ECC_THRES_BITREG1		0x10
#define ECC_THRES_BITREG2		0x14
#define ECC_CORRECT_BITREG1		0x18
#define ECC_CORRECT_BITREG2		0x1C
#define ECC_INTR_EN			0x20
#define ECC_INTR_THRES_HIT		(1 << 1)
#define ECC_INTR_CORRECT_FAIL		(1 << 0)
#define ECC_INTR_STATUS			0x24
#define ECC_ERR_THRES_HIT_FOR_SPARE(x)	(1 << (24 + x))
#define ECC_ERR_FAIL_FOR_SPARE(x)	(1 << (16+ x))
#define ECC_ERR_THRES_HIT(x)		(1 << (8 + x))
#define ECC_ERR_FAIL(x)			(1 << x)

#define ECC_THRES_BIT_FOR_SPARE_REG1	0x34
#define ECC_THRES_BIT_FOR_SPARE_REG2	0x38
#define ECC_CORRECT_BIT_FOR_SPARE_REG1	0x3C
#define ECC_CORRECT_BIT_FOR_SPARE_REG2	0x40
#define DEV_BUSY			0x100
#define GENERAL_SETTING			0x104
#define CE_NUM(x)			(x << 24)
#define BUSY_RDY_LOC(x)			(x << 12)
#define CMD_STS_LOC(x)			(x << 8)
//#define REPORT_ADDR_EN		(1 << 4)
#define WRITE_PROTECT			(1 << 2)
#define DATA_INVERSE			(1 << 1)
#define DATA_SCRAMBLER			(1 << 0)
#define MEM_ATTR_SET			0x108
#define BI_BYTE_MASK			(0x7 << 19) //from PNANDC v2.3
#define PG_SZ_512			(0 << 16)
#define PG_SZ_2K			(1 << 16)
#define PG_SZ_4K			(2 << 16)
#define PG_SZ_8K			(3 << 16)
#define PG_SZ_16K			(4 << 16)
#define ATTR_COL_CYCLE(x)		((x & 0x1) << 12)
#define ATTR_ROW_CYCLE(x)		((x & 0x3) << 13)
#define ROW_ADDR_1CYCLE			0
#define ROW_ADDR_2CYCLE			1
#define ROW_ADDR_3CYCLE			2
#define ROW_ADDR_4CYCLE			3
#define COL_ADDR_1CYCLE			0
#define COL_ADDR_2CYCLE			1
#define MEM_ATTR_SET2			0x10C
//#define SPARE_PAGE_MODE(x)		(x << 0)
//#define SPARE_PROT_EN(x)		(x << 8)
#define VALID_PAGE(x)			((x & 0x3FF) << 16)

#define FL_AC_TIMING0(x)		(0x110 + (x << 3))
#define FL_AC_TIMING1(x)		(0x114 + (x << 3))
#define FL_AC_TIMING2(x)		(0x190 + (x << 3))
#define FL_AC_TIMING3(x)		(0x194 + (x << 3))
#define INTR_ENABLE			0x150
//#define INTR_ENABLE_CRC_CHECK_EN(x)	(x << 8)
#define INTR_ENABLE_STS_CHECK_EN(x)	(x << 0)
#define INTR_STATUS			0x154
#define STATUS_CMD_COMPLETE(x)		(1 << (16 + x))
//#define STATUS_CRC_FAIL(x)		(1 << (8 + x))
#define STATUS_FAIL(x)			(1 << x)
#define READ_STATUS0			0x178
#define NANDC_SW_RESET			0x184
#define NANDC_EXT_CTRL			0x1E0 //from PNANDC v2.4
#define SEED_SEL(x)			(1 << x)
#define CMDQUEUE_STATUS			0x200
#define CMDQUEUE_STATUS_FULL(x)		(1 << (8 + x))
#define CMDQUEUE_STATUS_EMPTY(x)	(1 << (x))
#define CMDQUEUE_FLUSH			0x204

#define CMDQUEUE1(x)			(0x300 + (x << 5))
#define CMDQUEUE2(x)			(0x304 + (x << 5))
#define CMDQUEUE3(x)			(0x308 + (x << 5))
#define CMD_COUNT(x)			(x << 16)
#define CMDQUEUE4(x)			(0x30C + (x << 5))
#define CMD_COMPLETE_EN			(1 << 0)
#define CMD_SCALE(x)			((x & 0x3) << 2)
#define CMD_DMA_HANDSHAKE_EN		(1 << 4)
#define CMD_FLASH_TYPE(x)		((x & 0x7) << 5)
#define CMD_INDEX(x)			((x & 0x3FF) << 8)
#define CMD_PROM_FLOW			(1 << 18)
#define CMD_SPARE_NUM(x)		(((x - 1) &0x1F) << 19)
//from PNANDC v2.4, the extended spare_num is 0x304 b[25:24]
#define CMD_EX_SPARE_NUM(x)		((((x - 1) >> 5) &0x3) << 24)
#define SCR_SEED_VAL1(x)		((x & 0xff) << 24)
#define SCR_SEED_VAL2(x)		(((x & 0x3fff) >> 8) << 26)

#define CMD_BYTE_MODE			(1 << 28)
#define CMD_START_CE(x)			((x & 0x7) << 29)

#define BMC_REGION_STATUS		0x400
#define BMC_REGION_BUF_EMPTY(x)		(1 << (x + 24))
#define BMC_REGION_HALT(x)		(1 << (x + 16))
#define BMC_REGION_FULL(x)		(1 << (x + 8))
#define BMC_REGION_EMPTY(x)		(1 << (x))
#define BMC_REGION_SW_RESET		0x428  //WO

#define REVISION_NUM			0x500
#define FEATURE_1			0x504
#define MAX_SPARE_DATA_128BYTE		(1 << 15) //from PNANDC v2.4
#define AHB_SLAVE_MODE_ASYNC(x)		(1 << (x + 27))  //Asynchronous mode
#define DDR_IF_EN			(1 << 31)  //Enable DDR interface
#define AHB_SLAVEPORT_SIZE		0x508
#define AHB_SLAVE_SPACE_512B		(1 << 0)
#define AHB_SLAVE_SPACE_1KB		(1 << 1)
#define AHB_SLAVE_SPACE_2KB		(1 << 2)
#define AHB_SLAVE_SPACE_4KB		(1 << 3)
#define AHB_SLAVE_SPACE_8KB		(1 << 4)
#define AHB_SLAVE_SPACE_16KB		(1 << 5)
#define AHB_SLAVE_SPACE_32KB		(1 << 6)
#define AHB_SLAVE_SPACE_64KB		(1 << 7)
#define AHB_RETRY_EN(ch_index)		(1 << (ch_index + 8))
#define AHB_PREFETCH(ch_index)		(1 << (ch_index + 12))
#define AHB_PRERETCH_LEN(x_words)	(x_words << 16)
#define AHB_SPLIT_EN			(1 << 25)
#define GLOBAL_RESET			0x50C
#define AHB_SLAVE_RESET			0x510
#define DQS_DELAY			0x520
#define PROGRAMMABLE_OPCODE		0x700
#define PROGRAMMABLE_FLOW_CONTROL	0x2000
#define SPARE_SRAM			0x1000

#define FIXFLOW_READID			0x5F
#define FIXFLOW_RESET			0x65
#define FIXFLOW_READSTATUS		0x96
/* FIX_FLOW_INDEX for small page */
#define SMALL_FIXFLOW_READOOB		0x249
#define SMALL_FIXFLOW_PAGEREAD		0x23E
#define SMALL_FIXFLOW_PAGEWRITE		0x26C
#define SMALL_FIXFLOW_WRITEOOB		0x278
#define SMALL_FIXFLOW_ERASE		0x2C1

/* FIX_FLOW_INDEX for large page */
#define LARGE_FIXFLOW_BYTEREAD		0x8A
#define LARGE_FIXFLOW_READOOB		0x3E
#define LARGE_FIXFLOW_PAGEREAD		0x1C
#define LARGE_FIXFLOW_PAGEREAD_W_SPARE	0x48
#define LARGE_PAGEWRITE_W_SPARE		0x26
#define LARGE_PAGEWRITE			0x54
#define LARGE_FIXFLOW_WRITEOOB		0x33
#define LARGE_FIXFLOW_ERASE		0x68

/* FIX_FLOW_INDEX for ONFI change mode */
#define ONFI_FIXFLOW_GETFEATURE		0x22B
#define ONFI_FIXFLOW_SETFEATURE		0x232
#define ONFI_FIXFLOW_SYNCRESET		0x21A

#define max_2(a,b)			max(a,b)
#define min_2(a,b)			min(a,b)
#define max_3(a,b,c)			(max_2(a,b) > c ? max_2(a,b): c)
#define min_3(a,b,c)			(min_2(a,b) < c ? min_2(a,b): c)
#define max_4(a,b,c,d)			(max_3(a,b,c) > d ? max_3(a,b,c): d)
#define min_4(a,b,c,d)			(min_3(a,b,c) < d ? min_3(a,b,c): d)

#define MAX_CE				1
#define MAX_CHANNEL			1

/* For recording the command execution status*/
#define CMD_SUCCESS			0
#define CMD_CRC_FAIL			(1 << 1)
#define CMD_STATUS_FAIL			(1 << 2)
#define CMD_ECC_FAIL_ON_DATA		(1 << 3)
#define CMD_ECC_FAIL_ON_SPARE		(1 << 4)

/* Feature Setting */
#define CONFIG_PAGE_MODE
//#define CONFIG_SECTOR_MODE

#if CONFIG_SP_DEBUG > 0
	#define TAG "Parallel Nand: "
	#define sp_pnand_dbg(fmt, ...) printk(KERN_INFO TAG fmt, ##__VA_ARGS__);
#else
	#define sp_pnand_dbg(fmt, ...) do {} while (0)
#endif

#if CONFIG_SP_DEBUG > 0
	#define DBGLEVEL1(x)	x
#else
	#define DBGLEVEL1(x)
#endif

#if CONFIG_SP_DEBUG > 1
	#define DBGLEVEL2(x)	x
#else
	#define DBGLEVEL2(x)
#endif

typedef enum {
	/*LEGACY_SMALL = 0,
	LEGACY_LARGE,*/
	LEGACY_FLASH = 0,
	ONFI2,
	ONFI3,
	TOGGLE1,
	TOGGLE2,
} flashtype;

struct sp_pnandchip_attr {
	char *name;
	int sparesize;
	int ecc;
	int eccbaseshift;
	int ecc_spare;
	int block_boundary;
	int crc;
	flashtype flash_type;
};

struct sp_pnand_chip_timing {
	uint8_t		tWH;
	uint8_t		tCH;
	uint8_t		tCLH;
	uint8_t		tALH;
	uint8_t		tCALH;
	uint8_t		tWP;
	uint8_t		tREH;
	uint8_t		tCR;
	uint8_t		tRSTO;
	uint8_t		tREAID;
	uint8_t		tREA;
	uint8_t		tRP;
	uint8_t		tWB;
	uint8_t		tRB;
	uint8_t		tWHR;
	int		tWHR2;
	uint8_t		tRHW;
	uint8_t		tRR;
	uint8_t		tAR;
	uint8_t		tRC;
	int		tADL;
	uint8_t		tRHZ;
	int		tCCS;
	uint8_t		tCS;
	uint8_t		tCS2;
	uint8_t		tCLS;
	uint8_t		tCLR;
	uint8_t		tALS;
	uint8_t		tCALS;
	uint8_t		tCAL2;
	uint8_t		tCRES;
	uint8_t		tCDQSS;
	uint8_t		tDBS;
	int		tCWAW;
	uint8_t		tWPRE;
	uint8_t		tRPRE;
	uint8_t		tWPST;
	uint8_t		tRPST;
	uint8_t		tWPSTH;
	uint8_t		tRPSTH;
	uint8_t		tDQSHZ;
	uint8_t		tDQSCK;
	uint8_t		tCAD;
	uint8_t		tDSL;
	uint8_t		tDSH;
	uint8_t		tDQSL;
	uint8_t		tDQSH;
	uint8_t		tDQSD;
	uint8_t		tCKWR;
	uint8_t		tWRCK;
	uint8_t		tCK;
	uint8_t		tCALS2;
	uint8_t		tDQSRE;
	uint8_t		tWPRE2;
	uint8_t		tRPRE2;
	uint8_t		tCEH;
};

struct cmd_feature {
	u32 cq1;
	u32 cq2;
	u32 cq3;
	u32 cq4;
	u8 row_cycle;
	u8 col_cycle;
};

struct sp_pnand_data {
	struct nand_chip chip;
	struct nand_controller controller;
	void __iomem *io_base;
	struct clk *clk;
	struct reset_control *rstc;
	int sel_chip;
	int cur_cmd;
	int page_addr;
	int column;
	int byte_ofs;
	struct device *dev;
	int cur_chan;
	int valid_chip[MAX_CHANNEL];
	int scan_state;
	int read_state;	//struct nand_chip chip->state is removed
	int cmd_status;
	char flash_raw_id[5];
	int flash_type;
	int large_page;
	int eccbasft;
	int max_spare; //max spare data bytes (SP v2.4: 32/128 bytes)
	int spare_ch_offset; //register 0x1000 channel offset (SP v2.4: 0x80 = shift 7)
	int spare;
	int protect_spare;
	int useecc;
	int useecc_spare;
	int block_boundary;	//addr space of block (unit: page)
	int seed_val; //Scramble Seed bit [7:0] and [13:8] in cq1 and cq2
	int sector_per_page;
	int inverse;
	int scramble;
	int clkfreq;
	struct dma_chan *dmac;

	unsigned long priv ____cacheline_aligned;
};

void sp_pnand_select_chip(struct nand_chip *chip, int cs);
void sp_pnand_abort(struct nand_chip *chip);
void sp_pnand_regdump(struct nand_chip *chip);

extern int sp_pnand_issue_cmd(struct nand_chip *, struct cmd_feature *);
extern void sp_pnand_set_default_timing(struct nand_chip *);
extern int sp_pnand_wait(struct nand_chip *);
extern void sp_pnand_fill_prog_code(struct nand_chip *, int, int);
extern void sp_pnand_fill_prog_flow(struct nand_chip *, int *, int);
extern int sp_pnand_read_page(struct nand_chip *, uint8_t *, int, int);
extern int sp_pnand_write_page(struct nand_chip *, const uint8_t *, int, int);

extern int sp_pnand_read_page_by_dma(struct nand_chip *nand, uint8_t *buf, int oob_required, int page);
extern int sp_pnand_write_page_by_dma(struct nand_chip *nand, const uint8_t *buf, int oob_required, int page);

extern int sp_pnand_read_oob(struct nand_chip *, int);
extern int sp_pnand_write_oob(struct nand_chip *, int);
