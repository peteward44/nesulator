
#ifndef OPCODES_65C816_H
#define OPCODES_65C816_H


enum ADDRESSING_MODE_65C816
{
	AMS_IMPLIED = 0,
	AMS_IMMEDIATE8, // Immediate Addressing -- # always 8 bit
	AMS_IMMEDIATE16, // Immediate Addressing -- # always 16 bit
	AMS_IMMEDIATE_EMU, // Immediate Addressing -- # Depends on memory/accumulator status flag - if 1 it is 8-bit 16 otherwise
	AMS_IMMEDIATE_INDEX, // Immediate Addressing -- # Depends on index 8bit status flag - if 1 it is 8-bit 16 otherwise
	AMS_ACCUMULATOR, // Accumulator -- A
	AMS_ZEROPAGE, // Direct -- d
	AMS_ZEROPAGEX, // Direct Indexed With X -- d,x
	AMS_ZEROPAGEY, // Direct Indexed With Y -- d,y
	AMS_INDIRECTX, // Direct Indexed Indirect -- (d,x)
	AMS_INDIRECTY, // Direct Indirect Indexed -- (d),y
	AMS_ZEROPAGE_INDIRECT, // Direct Indirect -- (d)
	AMS_ZEROPAGE_INDIRECT_LONG, // Direct Indirect Long -- [d]
	AMS_ZEROPAGE_INDIRECT_LONGY, // Direct Indirect Indexed Long -- [d],y
	AMS_ABSOLUTE, // Absolute -- a
	AMS_ABSOLUTELONG, // Absolute Long -- al
	AMS_ABSOLUTELONGX, // Absolute Long Indexed With X -- al,x
	AMS_ABSOLUTEX, // Absolute Indexed With X -- a,x
	AMS_ABSOLUTEY, // Absolute Indexed With Y -- a,y
	AMS_ABSOLUTE_INDIRECT, // Absolute indirect -- (a)
	AMS_ABSOLUTE_INDIRECT_DIRECT, // Used by JML and JMP, uses direct bank instead of program bank
	AMS_ABSOLUTE_INDIRECTX, // Absolute Indexed Indirect -- (a,x)
	AMS_STACK, // Stack -- s
	AMS_STACKRELATIVE, // Stack Relative -- d,s
	AMS_STACKRELATIVEINDEXED, // Stack Relative Indirect Indexed -- (d,s),y
	AMS_STACK_DIRECTPAGEINDIRECT, // Only used by PEI command
	AMS_RELATIVE, // Program counter relative -- r -- used by branch instructions
	AMS_RELATIVELONG, // Program counter relative long -- rl -- used by branch instructions
	AMS_BLOCKMOVE, // Block move -- xya -- used by MVP and MVN instructions
};


// 0x42 - unknown

enum OPCODE_65C816
{
	// Add memory to accumulator with carry
	OPS_ADC_IMMEDIATE = 0x69,
	OPS_ADC_ZEROPAGE = 0x65,
	OPS_ADC_ZEROPAGEX = 0x75,
	OPS_ADC_ABSOLUTE = 0x6D,
	OPS_ADC_ABSOLUTEX = 0x7D,
	OPS_ADC_ABSOLUTEY = 0x79,
	OPS_ADC_INDIRECTX = 0x61, // Direct Indexed Indirect -- (d,x)
	OPS_ADC_INDIRECTY = 0x71, // Direct Indirect Indexed -- (d),y
	OPS_ADC_STACKRELATIVE = 0x63, // NEW: stack relative
	OPS_ADC_ZEROPAGE_INDIRECT_LONG = 0x67, // NEW: direct indirect long
	OPS_ADC_ABSOLUTELONG = 0x6F, // NEW: absolute long
	OPS_ADC_ZEROPAGE_INDIRECT = 0x72, // NEW: direct Indirect
	OPS_ADC_STACKRELATIVEINDEXED = 0x73, // NEW: (d,s),y	stack relative indirect Indexed
	OPS_ADC_ZEROPAGE_INDIRECT_LONGY = 0x77, // NEW: [d],y		direct indirect indexed long
	OPS_ADC_ABSOLUTELONGX = 0x7F, // NEW: al,x		absolute indexed long

	// "AND" memory with accumulator
	OPS_AND_IMMEDIATE = 0x29, // NEW: check
	OPS_AND_ZEROPAGE = 0x25,
	OPS_AND_ZEROPAGEX = 0x35,
	OPS_AND_ABSOLUTE = 0x2D,
	OPS_AND_ABSOLUTEX = 0x3D,
	OPS_AND_ABSOLUTEY = 0x39,
	OPS_AND_INDIRECTX = 0x21,
	OPS_AND_INDIRECTY = 0x31,
	OPS_AND_STACKRELATIVE = 0x23, // NEW: check d,s		stack relative
	OPS_AND_ZEROPAGE_INDIRECT_LONG = 0x27, // NEW: check direct indirect long
	OPS_AND_ABSOLUTELONG = 0x2F, // NEW: check absolute long
	OPS_AND_ABSOLUTELONGX = 0x3F, // NEW: check absolute indexed long
	OPS_AND_ZEROPAGE_INDIRECT = 0x32, // NEW: (d)		direct Indirect
	OPS_AND_STACKRELATIVEINDEXED = 0x33, // NEW: (d,s),y	stack relative indirect Indexed
	OPS_AND_ZEROPAGE_INDIRECT_LONGY = 0x37, // NEW: [d],y		direct indirect indexed long

	// ASL Shift Left One Bit (Memory or Accumulator)
	OPS_ASL_ACCUMULATOR = 0x0A,
	OPS_ASL_ZEROPAGE = 0x06,
	OPS_ASL_ZEROPAGEX = 0x16,
	OPS_ASL_ABSOLUTE = 0x0E,
	OPS_ASL_ABSOLUTEX = 0x1E,

	// BCC Branch on Carry Clear
	OPS_BCC = 0x90,

	// BCS Branch on carry set
	OPS_BCS = 0xB0,

	// BEQ Branch on result zero
	OPS_BEQ = 0xF0,

	// BIT Test bits in memory with accumulator
	OPS_BIT_IMMEDIATE = 0x89, // NEW: check
	OPS_BIT_ZEROPAGE = 0x24,
	OPS_BIT_ABSOLUTE = 0x2C,
	OPS_BIT_ZEROPAGEX = 0x34, // NEW: direct indexed (with x)
	OPS_BIT_ABSOLUTEX = 0x3C, // NEW: absolute indexed (with x)

	// BMI Branch on result minus
	OPS_BMI = 0x30,
	
	// BNE Branch on result not zero
	OPS_BNE = 0xD0,
	
	// BPL Branch on result plus
	OPS_BPL = 0x10,

	// NEW: Branch always
	OPS_BRA = 0x80,

	// BRK Force Break
	OPS_BRK = 0x00,

	// NEW: Branch always long
	OPS_BRL = 0x82,

	// BVC Branch on overflow clear
	OPS_BVC = 0x50,
	
	// BVS Branch on overflow set
	OPS_BVS = 0x70,

	// CLC Clear carry flag
	OPS_CLC = 0x18,
	
	// CLD Clear decimal mode
	OPS_CLD = 0xD8,
	
	// CLI Clear interrupt disable bit
	OPS_CLI = 0x58,
	
	// CLV Clear overflow flag
	OPS_CLV = 0xB8,

	// CMP Compare memory and accumulator
	OPS_CMP_IMMEDIATE = 0xC9,
	OPS_CMP_ZEROPAGE = 0xC5,
	OPS_CMP_ZEROPAGEX = 0xD5,
	OPS_CMP_ABSOLUTE = 0xCD,
	OPS_CMP_ABSOLUTEX = 0xDD,
	OPS_CMP_ABSOLUTEY = 0xD9,
	OPS_CMP_INDIRECTX = 0xC1,
	OPS_CMP_INDIRECTY = 0xD1,
	OPS_CMP_STACKRELATIVE = 0xC3, // NEW: d,s		stack relative
	OPS_CMP_ZEROPAGE_INDIRECT_LONG = 0xC7, // NEW:  [d]		direct indirect long
	OPS_CMP_ABSOLUTELONG = 0xCF, // NEW: CMP al
	OPS_CMP_ZEROPAGE_INDIRECT = 0xD2, // NEW: CMP (d)
	OPS_CMP_STACKRELATIVEINDEXED = 0xD3, // NEW: CMP(d,s),y
	OPS_CMP_ZEROPAGE_INDIRECT_LONGY = 0xD7, // NEW: CMP [d],y
	OPS_CMP_ABSOLUTELONGX = 0xDF, // NEW: CMP al,x

	// NEW: COP co-processor
	OPS_COP = 0x02,

	// CPX Compare Memory and Index X
	OPS_CPX_IMMEDIATE = 0xE0,
	OPS_CPX_ZEROPAGE = 0xE4,
	OPS_CPX_ABSOLUTE = 0xEC,
	
	// CPY Compare memory and index Y
	OPS_CPY_IMMEDIATE = 0xC0,
	OPS_CPY_ZEROPAGE = 0xC4,
	OPS_CPY_ABSOLUTE = 0xCC,

	// DEC Decrement memory by one
	OPS_DEC_ACCUMULATOR = 0x3A, // NEW: check
	OPS_DEC_ZEROPAGE = 0xC6,
	OPS_DEC_ZEROPAGEX = 0xD6,
	OPS_DEC_ABSOLUTE = 0xCE,
	OPS_DEC_ABSOLUTEX = 0xDE,
	
	// DEX Decrement index X by one
	OPS_DEX = 0xCA,
	
	// DEY Decrement index Y by one
	OPS_DEY = 0x88,

	// EOR "Exclusive-Or" memory with accumulator
	OPS_EOR_IMMEDIATE = 0x49,
	OPS_EOR_ZEROPAGE = 0x45,
	OPS_EOR_ZEROPAGEX = 0x55,
	OPS_EOR_ABSOLUTE = 0x4D,
	OPS_EOR_ABSOLUTEX = 0x5D,
	OPS_EOR_ABSOLUTEY = 0x59,
	OPS_EOR_INDIRECTX = 0x41,
	OPS_EOR_INDIRECTY = 0x51,
	OPS_EOR_STACKRELATIVE = 0x43, // NEW: stack relative
	OPS_EOR_ZEROPAGE_INDIRECT_LONG = 0x47, // NEW: direct indirect long
	OPS_EOR_ABSOLUTELONG = 0x4F, // NEW: absolute long
	OPS_EOR_ZEROPAGE_INDIRECT = 0x52, // NEW: direct indirect
	OPS_EOR_STACKRELATIVEINDEXED = 0x53, // NEW: (d,s),y	stack relative indirect Indexed
	OPS_EOR_ZEROPAGE_INDIRECT_LONGY = 0x57, // NEW: [d],y		direct indirect indexed long
	OPS_EOR_ABSOLUTELONGX = 0x5F, // NEW: absolute indexed long

	// INC Increment memory by one
	OPS_INC_ACCUMULATOR = 0x1A, // NEW: check
	OPS_INC_ZEROPAGE = 0xE6,
	OPS_INC_ZEROPAGEX = 0xF6,
	OPS_INC_ABSOLUTE = 0xEE,
	OPS_INC_ABSOLUTEX = 0xFE,
	
	// INX Increment Index X by one
	OPS_INX = 0xE8,
	
	// INY Increment Index Y by one
	OPS_INY = 0xC8,

	// NEW: JML Jump long
	OPS_JML = 0xDC,

	// JMP Jump to new location
	OPS_JMP_ABSOLUTE = 0x4C,
	OPS_JMP_ABSOLUTELONG = 0x5C, // NEW: check this
	OPS_JMP_INDIRECT = 0x6C,
	OPS_JMP_ABSOLUTE_INDIRECTX = 0x7C, // NEW: check this

	// NEW: JSL Jump Subroutine Long
	OPS_JSL = 0x22,

	// JSR Jump to new location saving return address
	OPS_JSR = 0x20,
	OPS_JSR_ABSOLUTE_INDIRECTX = 0xFC, // NEW: check this

	// LDA Load accumulator with memory
	OPS_LDA_IMMEDIATE = 0xA9,
	OPS_LDA_ZEROPAGE = 0xA5,
	OPS_LDA_ZEROPAGEX = 0xB5,
	OPS_LDA_ABSOLUTE = 0xAD,
	OPS_LDA_ABSOLUTEX = 0xBD,
	OPS_LDA_ABSOLUTEY = 0xB9,
	OPS_LDA_INDIRECTX = 0xA1,
	OPS_LDA_INDIRECTY = 0xB1,
	OPS_LDA_STACKRELATIVE = 0xA3, // NEW: d,s
	OPS_LDA_ZEROPAGE_INDIRECT_LONG = 0xA7, // NEW: [d]
	OPS_LDA_ABSOLUTELONG = 0xAF, // NEW: absolute long
	OPS_LDA_ZEROPAGE_INDIRECT = 0xB2, // NEW: (d)
	OPS_LDA_STACKRELATIVEINDEXED = 0xB3, // NEW: (d,s),y	stack relative indirect Indexed
	OPS_LDA_ZEROPAGE_INDIRECT_LONGY = 0xB7, // NEW: [d],y		direct indirect indexed long
	OPS_LDA_ABSOLUTELONGX = 0xBF, // NEW: al,x		absolute indexed long
	
	// LDX Load index X with memory
	OPS_LDX_IMMEDIATE = 0xA2,
	OPS_LDX_ZEROPAGE = 0xA6,
	OPS_LDX_ZEROPAGEY = 0xB6,
	OPS_LDX_ABSOLUTE = 0xAE,
	OPS_LDX_ABSOLUTEY = 0xBE,
	
	// LDY Load index Y with memory
	OPS_LDY_IMMEDIATE = 0xA0,
	OPS_LDY_ZEROPAGE = 0xA4,
	OPS_LDY_ZEROPAGEX = 0xB4,
	OPS_LDY_ABSOLUTE = 0xAC,
	OPS_LDY_ABSOLUTEX = 0xBC,

	// LSR Shift right one bit (memory or accumulator)
	OPS_LSR_ACCUMULATOR = 0x4A,
	OPS_LSR_ZEROPAGE = 0x46,
	OPS_LSR_ZEROPAGEX = 0x56,
	OPS_LSR_ABSOLUTE = 0x4E,
	OPS_LSR_ABSOLUTEX = 0x5E,

	// NEW: MVN Block Move Negative
	OPS_MVN = 0x54,

	// NEW: MVP	Block Move Positive
	OPS_MVP = 0x44,

	// NOP No operation
	OPS_NOP = 0xEA,

	// ORA "OR" memory with accumulator
	OPS_ORA_IMMEDIATE = 0x09,
	OPS_ORA_ZEROPAGE = 0x05,
	OPS_ORA_ZEROPAGEX = 0x15,
	OPS_ORA_ABSOLUTE = 0x0D,
	OPS_ORA_ABSOLUTEX = 0x1D,
	OPS_ORA_ABSOLUTEY = 0x19,
	OPS_ORA_INDIRECTX = 0x01,
	OPS_ORA_INDIRECTY = 0x11,
	OPS_ORA_STACKRELATIVE = 0x03, // NEW: check this??
	OPS_ORA_ZEROPAGE_INDIRECT_LONG = 0x07, // NEW: def. check this!
	OPS_ORA_ABSOLUTELONG = 0x0F, // NEW: check
	OPS_ORA_ZEROPAGE_INDIRECT = 0x12, // NEW: (d) ( direct Indirect )
	OPS_ORA_STACKRELATIVEINDEXED = 0x13, // NEW: (d,s),y	stack relative indirect Indexed
	OPS_ORA_ZEROPAGE_INDIRECT_LONGY = 0x17, // NEW: [d],y		direct indirect indexed long
	OPS_ORA_ABSOLUTELONGX = 0x1F, // NEW: al, x absolute indexed long

	// NEW: PEA	Push Effective Absolute Address on Stack (or Push Immediate Data on Stack)
	OPS_PEA = 0xF4,

	// NEW: PEI	Push Effective Indirect Address on Stack (add one cycle if DL f 0)
	OPS_PEI = 0xD4,

	// NEW: PER	Push Effective Program Counter Relative Address on Stack
	OPS_PER = 0x62,

	// PHA Push accumulator on stack
	OPS_PHA = 0x48,

	// NEW: PHB	Push Data Bank Register on Stack
	OPS_PHB = 0x8B,

	// NEW: PHD	Push Direct Register on Stack
	OPS_PHD = 0x0B,

	// NEW: PHK	Push Program Bank Register on Stack
	OPS_PHK = 0x4B,

	// PHP Push processor status on stack
	OPS_PHP = 0x08,

	// NEW: PHX	Push Index X on Stack
	OPS_PHX = 0xDA,

	// NEW: PHY	Push index Y on Stack
	OPS_PHY = 0x5A,

	// PLA Pull accumulator from stack
	OPS_PLA = 0x68,

	// NEW: PLB	Pull Data Bank Register from Stack
	OPS_PLB = 0xAB,

	// NEW: PLD	Pull Direct Register from Stack
	OPS_PLD = 0x2B,

	// PLP Pull processor status from stack
	OPS_PLP = 0x28,

	// NEW: PLX	Pull Index X from Stack
	OPS_PLX = 0xFA,

	// NEW: PLY	Pull Index Y form Stack
	OPS_PLY = 0x7A,

	// NEW: REP	Reset Status Bits
	OPS_REP = 0xC2,

	// ROL Rotate one bit left (memory or accumulator)
	OPS_ROL_ACCUMULATOR = 0x2A,
	OPS_ROL_ZEROPAGE = 0x26,
	OPS_ROL_ZEROPAGEX = 0x36,
	OPS_ROL_ABSOLUTE = 0x2E,
	OPS_ROL_ABSOLUTEX = 0x3E,
	
	// ROR Rotate one bit right (memory or accumulator)
	OPS_ROR_ACCUMULATOR = 0x6A,
	OPS_ROR_ZEROPAGE = 0x66,
	OPS_ROR_ZEROPAGEX = 0x76,
	OPS_ROR_ABSOLUTE = 0x6E,
	OPS_ROR_ABSOLUTEX = 0x7E,

	// RTI Return from interrupt
	OPS_RTI = 0x40,

	// NEW: RTL Return from Subroutine Long
	OPS_RTL = 0x6B,

	// RTS Return from subroutine
	OPS_RTS = 0x60,

	// SBC Subtract memory from accumulator with borrow
	OPS_SBC_IMMEDIATE = 0xE9,
	OPS_SBC_ZEROPAGE = 0xE5,
	OPS_SBC_ZEROPAGEX = 0xF5,
	OPS_SBC_ABSOLUTE = 0xED,
	OPS_SBC_ABSOLUTEX = 0xFD,
	OPS_SBC_ABSOLUTEY = 0xF9,
	OPS_SBC_INDIRECTX = 0xE1,
	OPS_SBC_INDIRECTY = 0xF1,
	OPS_SBC_STACKRELATIVE = 0xE3, // NEW: SBC d,s
	OPS_SBC_ZEROPAGE_INDIRECT_LONG = 0xE7, // NEW: SBC [d]
	OPS_SBC_ABSOLUTELONG = 0xEF, // NEW: SBC al
	OPS_SBC_ZEROPAGE_INDIRECT = 0xF2, // NEW SBC (d)
	OPS_SBC_STACKRELATIVEINDEXED = 0xF3, // NEW: SBC(d,s),y
	OPS_SBC_ZEROPAGE_INDIRECT_LONGY = 0xF7, // NEW: SBC [d],y
	OPS_SBC_ABSOLUTELONGX = 0xFF, // NEW: SBC al,x

	// SEC Set carry flag
	OPS_SEC = 0x38,
	
	// SED Set decimal mode
	OPS_SED = 0xF8,
	
	// SEI Set interrupt disable status
	OPS_SEI = 0x78,

	// NEW: SEP	Set Processor Status Bits
	OPS_SEP = 0xE2,

	// STA Store accumulator in memory
	OPS_STA_ZEROPAGE = 0x85,
	OPS_STA_ZEROPAGEX = 0x95,
	OPS_STA_ABSOLUTE = 0x8D,
	OPS_STA_ABSOLUTEX = 0x9D,
	OPS_STA_ABSOLUTEY = 0x99,
	OPS_STA_INDIRECTX = 0x81,
	OPS_STA_INDIRECTY = 0x91,
	OPS_STA_STACKRELATIVE = 0x83, // NEW: d,s		stack relative
	OPS_STA_ZEROPAGE_INDIRECT_LONG = 0x87, // NEW: [d]		direct indirect long
	OPS_STA_ABSOLUTELONG = 0x8F, // NEW: al		absolute long
	OPS_STA_ZEROPAGE_INDIRECT = 0x92, // NEW: direct indirect
	OPS_STA_STACKRELATIVEINDEXED = 0x93, // (d,s),y	stack relative indirect Indexed
	OPS_STA_ZEROPAGE_INDIRECT_LONGY = 0x97, // [d],y		direct indirect indexed long
	OPS_STA_ABSOLUTELONGX = 0x9F, //  al,x		absolute indexed long

	// NEW: STP	Stop the Clock
	OPS_STP = 0xDB,

	// STX Store index X in memory
	OPS_STX_ZEROPAGE = 0x86,
	OPS_STX_ZEROPAGEY = 0x96,
	OPS_STX_ABSOLUTE = 0x8E,
	
	// STY Store index Y in memory
	OPS_STY_ZEROPAGE = 0x84,
	OPS_STY_ZEROPAGEX = 0x94,
	OPS_STY_ABSOLUTE = 0x8C,

	// NEW: STZ	Store Zero in Memory
	OPS_STZ_ZEROPAGE = 0x64,
	OPS_STZ_ZEROPAGEX = 0x74,
	OPS_STZ_ABSOLUTE = 0x9C,
	OPS_STZ_ABSOLUTEX = 0x9E,

	// TAX Transfer accumulator to index X
	OPS_TAX = 0xAA,
	
	// TAY Transfer accumulator to index Y
	OPS_TAY = 0xA8,

	// NEW: TCD*	Transfer Accumulator to Direct Register
	OPS_TCD = 0x5B,

	// NEW: TCS*	Transfer Accumulator to Stack Pointer Register
	OPS_TCS = 0x1B,

	// NEW: TDC*	Transfer Direct Register to Accumulator
	OPS_TDC = 0x7B,

	// NEW: TRB	Test and Reset Bit
	OPS_TRB_ZEROPAGE = 0x14,
	OPS_TRB_ABSOLUTE = 0x1C,

	// NEW: TSB	Test and Set Bit
	OPS_TSB_ZEROPAGE = 0x04,
	OPS_TSB_ABSOLUTE = 0x0C,

	// NEW: TSC*	Transfer Stack Pointer Register to Accumulator
	OPS_TSC = 0x3B,

	// TSX Transfer stack pointer to index X
	OPS_TSX = 0xBA,

	// TXA Transfer index X to accumulator
	OPS_TXA = 0x8A,

	// TXS Transfer index X to stack pointer
	OPS_TXS = 0x9A,

	// NEW: TXY	Transfer Index X to Index Y
	OPS_TXY = 0x9B,

	// TYA Transfer index Y to accumulator
	OPS_TYA = 0x98,

	// NEW: TYX 	Transfer Index Y to Index X
	OPS_TYX = 0xBB,

	// NEW: WAI	Wait for Interrupt
	OPS_WAI = 0xCB,

	// NEW: XBA*	Exchange AH and AL
	OPS_XBA = 0xEB,

	// NEW: XCE	Exchange Carry and Emulation Bits
	OPS_XCE = 0xFB,
};


#endif

