//---------------------------------------------------------------------------
//
//	X-UNIT
//	Programable General Extention Board for X68K
//
//	Powered by XM6 TypeG Technology.
//	Copyright (C) 2019-2000 GIMONS
//	[ Main ]
//
//---------------------------------------------------------------------------

#include "rpi-SmartStart.h"
#include "emb-stdio.h"
#include "windows.h"

//---------------------------------------------------------------------------
//
//	Constant statements (GIC)
//
//---------------------------------------------------------------------------
#define GICD_BASEADDR		(uint32_t *)(0xFF841000)
#define GICC_BASEADDR		(uint32_t *)(0xFF842000)
#define GICD_CTLR			0x000
#define GICD_IGROUPR0		0x020
#define GICD_ISENABLER0		0x040
#define GICD_ICENABLER0		0x060
#define GICD_ISPENDR0		0x080
#define GICD_ICPENDR0		0x0A0
#define GICD_ISACTIVER0		0x0C0
#define GICD_ICACTIVER0		0x0E0
#define GICD_IPRIORITYR0	0x100
#define GICD_ITARGETSR0		0x200
#define GICD_ICFGR0			0x300
#define GICD_SGIR			0x3C0
#define GICC_CTLR			0x000
#define GICC_PMR			0x001
#define GICC_IAR			0x003
#define GICC_EOIR			0x004

//---------------------------------------------------------------------------
//
//	Constant definitions (MMU)
//
//---------------------------------------------------------------------------
#define NUM_PGTABLE_ENTRIES	4096
#define LEVEL1_BLOCKSIZE	(1 << 20)
#define TLB_ALIGNMENT		16384
#define MT_DEVICE_NS		0x10412
#define MT_DEVICE			0x10416
#define MT_NORMAL			0x1040E
#define MT_NORMAL_NS		0x1040A
#define MT_NORMAL_XN		0x1041E

//---------------------------------------------------------------------------
//
//	MMU conversion table
//
//---------------------------------------------------------------------------
static RegType_t
	__attribute__((aligned(TLB_ALIGNMENT)))
		pagetable[NUM_PGTABLE_ENTRIES] = { 0 };

//---------------------------------------------------------------------------
//
//	MMU conversion table structure
//
//---------------------------------------------------------------------------
int SetupPageTable()
{
	int i;
	uint32_t msg[5] = { 0 };

	// Get the VC start address
	if (mailbox_tag_message(
		&msg[0], 5, MAILBOX_TAG_GET_VC_MEMORY, 8, 8, 0, 0) == 0) {
		return -1;
	}

	// Normal cache settings for the VC start address
	msg[3] /= LEVEL1_BLOCKSIZE;
	for (i = 0; i < msg[3]; i++) 
	{
		pagetable[i] = (LEVEL1_BLOCKSIZE * i) | MT_NORMAL;
	}

	// For the rest, use strong order and no cache
	for (; i < NUM_PGTABLE_ENTRIES; i++) {
		pagetable[i] = (LEVEL1_BLOCKSIZE * i) | MT_DEVICE_NS;
	}

	return 0;
}

//---------------------------------------------------------------------------
//
//	Enable MMU
//
//---------------------------------------------------------------------------
void EnableMMU()
{
	enable_mmu_tables(pagetable);
}

//---------------------------------------------------------------------------
//
//	Initialize GICD
//
//---------------------------------------------------------------------------
void InitGICD()
{
	int i;
	volatile uint32_t *gicd;

	// Only RPI4
	if (RPi_IO_Base_Addr != 0xfe000000) {
		return;
	}

	// GICD's base address
	gicd = GICD_BASEADDR;

	// Disable GIC
	gicd[GICD_CTLR] = 0;

	// Clear interrupt
	for (i = 0; i < 8; i++) {
		// Interrupt enable clear
		gicd[GICD_ICENABLER0 + i] = 0xffffffff;
		// Interrupt pending clear
		gicd[GICD_ICPENDR0 + i] = 0xffffffff;
		// Interrupt active clear
		gicd[GICD_ICACTIVER0 + i] = 0xffffffff;
	}

	// Interrupt priority
	for (i = 0; i < 64; i++) {
		gicd[GICD_IPRIORITYR0 + i] = 0xa0a0a0a0;
	}

	// Set the interrupt target to core 0
	for (i = 0; i < 64; i++) {
		gicd[GICD_ITARGETSR0 + i] = 0x01010101;
	}

	// Set interrupt to level trigger
	for (i = 0; i < 64; i++) {
		gicd[GICD_ICFGR0 + i] = 0;
	}

	// Enable GIC
	gicd[GICD_CTLR] = 1;
}

//---------------------------------------------------------------------------
//
//	Initialize GICC
//
//---------------------------------------------------------------------------
void InitGICC()
{
	volatile uint32_t *gicc;

	// Only RPI4
	if (RPi_IO_Base_Addr != 0xfe000000) {
		return;
	}

	// GICC base address
	gicc = GICC_BASEADDR;

	// Enable core CPU interface
	gicc[GICC_PMR] = 0xf0;
	gicc[GICC_CTLR] = 1;
}

//---------------------------------------------------------------------------
//
//	Stage two of each core initilization
//
//---------------------------------------------------------------------------
static volatile int nSetup = 0;
void CoreSetup()
{
	// Initialize MMU
	EnableMMU();

	// Initialize GICC
	InitGICC();

	// Finish setup
	nSetup++;
}

//---------------------------------------------------------------------------
//
//	RaSCSI Main
//
//---------------------------------------------------------------------------
void startrascsi(void);

//---------------------------------------------------------------------------
//
//	Main
//
//---------------------------------------------------------------------------
void main(void)
{
	// Initialize SmartStart
	Init_EmbStdio(WriteText);
	PiConsole_Init(0, 0, 0, printf);
	displaySmartStart(printf);
	ARM_setmaxspeed(printf);

	// Initialize MMU conversion table
	SetupPageTable();

	// Initialize GICD
	InitGICD();

	// Setup core 0
	CoreSetup();

	printf("\n");

	// Start RaSCSI
	startrascsi();
}
