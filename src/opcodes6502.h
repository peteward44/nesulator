
#ifndef OPCODES6502_H
#define OPCODES6502_H


// Defines all opcodes for the 6502 processor and its addressing modes



// Defines the operation to perform
enum FUNCTION_TYPE
{
	FUNC_ADC,
	FUNC_AND,
	FUNC_ASL,
	FUNC_BCC,
	FUNC_BCS,
	FUNC_BEQ,
	FUNC_BIT,
	FUNC_BMI,
	FUNC_BNE,
	FUNC_BPL,
	FUNC_BRK,
	FUNC_BVC,
	FUNC_BVS,
	FUNC_CLC,
	FUNC_CLD,
	FUNC_CLI,
	FUNC_CLV,
	FUNC_CMP,
	FUNC_CPX,
	FUNC_CPY,
	FUNC_DEC,
	FUNC_DEX,
	FUNC_DEY,
	FUNC_EOR,
	FUNC_INC,
	FUNC_INX,
	FUNC_INY,
	FUNC_JMP,
	FUNC_JSR,
	FUNC_LDA,
	FUNC_LDX,
	FUNC_LDY,
	FUNC_LSR,
	FUNC_ORA,
	FUNC_PHA,
	FUNC_PHP,
	FUNC_PLA,
	FUNC_PLP,
	FUNC_ROL,
	FUNC_ROR,
	FUNC_RTI,
	FUNC_RTS,
	FUNC_SBC,
	FUNC_SEC,
	FUNC_SED,
	FUNC_SEI,
	FUNC_STA,
	FUNC_STX,
	FUNC_STY,
	FUNC_TAX,
	FUNC_TAY,
	FUNC_TSX,
	FUNC_TXA,
	FUNC_TXS,
	FUNC_TYA,

	FUNC_ASO,
	FUNC_RLA,
	FUNC_LSE,
	FUNC_RRA,
	FUNC_AXS,
	FUNC_LAX,
	FUNC_DCM,
	FUNC_INS,
	FUNC_ALR,
	FUNC_ARR,
	FUNC_XAA,
	FUNC_OAL,
	FUNC_SAX,
	FUNC_NOP,
	FUNC_SKB,
	FUNC_SKW,
	FUNC_HLT,
	FUNC_TAS,
	FUNC_SAY,
	FUNC_XAS,
	FUNC_AXA,
	FUNC_ANC,
	FUNC_LAS,
};




struct Instruction6502
{
	Byte_t mOpcode;
	std::wstring mName;
	Byte_t mSize;
	Byte_t mAddressingMode;
	Byte_t mBaseCycles;
	Byte_t mPageBoundaryCycles;
	Byte_t mOperationMode;
	FUNCTION_TYPE mFunctionType;
	bool mIsLegal;
};


const Instruction6502& GetInstruction6502( Byte_t opcode );

// Operation mode: Different opcodes store their result in a different manner,
// either in the accumulator, read-only, read-then-write, or write-only
enum OPERATION_MODE
{
	OPERATION_NONE,
	OPERATION_ACCUMULATOR,
	OPERATION_READ,
	OPERATION_READ_MODIFY_WRITE,
	OPERATION_WRITE,
	OPERATION_BRANCH,
	OPERATION_IMMEDIATE,
};

// Addressing mode: How each operation either gets the value to operate on
// or calculates the address to operate on in a different manner
enum ADDRESSING_MODE
{
	ADDRESS_NONE,
	ADDRESS_RELATIVE,
	ADDRESS_ACCUMULATOR,
	ADDRESS_IMMEDIATE,
	ADDRESS_IMMEDIATE16,
	ADDRESS_ZEROPAGE,
	ADDRESS_ZEROPAGEX,
	ADDRESS_ZEROPAGEY,
	ADDRESS_ABSOLUTE,
	ADDRESS_ABSOLUTEX,
	ADDRESS_ABSOLUTEY,
	ADDRESS_INDIRECT,
	ADDRESS_INDIRECTX,
	ADDRESS_INDIRECTY,
	ADDRESS_SAY // requires it's own addressing method
};




enum OPCODE_6502
{
	// Add memory to accumulator with carry
	OP_ADC_IMMEDIATE = 0x69,
	OP_ADC_ZEROPAGE = 0x65,
	OP_ADC_ZEROPAGEX = 0x75,
	OP_ADC_ABSOLUTE = 0x6D,
	OP_ADC_ABSOLUTEX = 0x7D,
	OP_ADC_ABSOLUTEY = 0x79,
	OP_ADC_INDIRECTX = 0x61,
	OP_ADC_INDIRECTY = 0x71,
	
	// "AND" memory with accumulator
	OP_AND_IMMEDIATE = 0x29,
	OP_AND_ZEROPAGE = 0x25,
	OP_AND_ZEROPAGEX = 0x35,
	OP_AND_ABSOLUTE = 0x2D,
	OP_AND_ABSOLUTEX = 0x3D,
	OP_AND_ABSOLUTEY = 0x39,
	OP_AND_INDIRECTX = 0x21,
	OP_AND_INDIRECTY = 0x31,
	
	// ASL Shift Left One Bit (Memory or Accumulator)
	OP_ASL_ACCUMULATOR = 0x0A,
	OP_ASL_ZEROPAGE = 0x06,
	OP_ASL_ZEROPAGEX = 0x16,
	OP_ASL_ABSOLUTE = 0x0E,
	OP_ASL_ABSOLUTEX = 0x1E,
	
	// BCC Branch on Carry Clear
	OP_BCC = 0x90,
	
	// BCS Branch on carry set
	OP_BCS = 0xB0,
	
	// BEQ Branch on result zero
	OP_BEQ = 0xF0,
	
	// BIT Test bits in memory with accumulator
	OP_BIT_ZEROPAGE = 0x24,
	OP_BIT_ABSOLUTE = 0x2C,
	
	// BMI Branch on result minus
	OP_BMI = 0x30,
	
	// BNE Branch on result not zero
	OP_BNE = 0xD0,
	
	// BPL Branch on result plus
	OP_BPL = 0x10,
	
	// BRK Force Break
	OP_BRK = 0x00,
	
	// BVC Branch on overflow clear
	OP_BVC = 0x50,
	
	// BVS Branch on overflow set
	OP_BVS = 0x70,
	
	// CLC Clear carry flag
	OP_CLC = 0x18,
	
	// CLD Clear decimal mode
	OP_CLD = 0xD8,
	
	// CLI Clear interrupt disable bit
	OP_CLI = 0x58,
	
	// CLV Clear overflow flag
	OP_CLV = 0xB8,
	
	// CMP Compare memory and accumulator
	OP_CMP_IMMEDIATE = 0xC9,
	OP_CMP_ZEROPAGE = 0xC5,
	OP_CMP_ZEROPAGEX = 0xD5,
	OP_CMP_ABSOLUTE = 0xCD,
	OP_CMP_ABSOLUTEX = 0xDD,
	OP_CMP_ABSOLUTEY = 0xD9,
	OP_CMP_INDIRECTX = 0xC1,
	OP_CMP_INDIRECTY = 0xD1,
	
	// CPX Compare Memory and Index X
	OP_CPX_IMMEDIATE = 0xE0,
	OP_CPX_ZEROPAGE = 0xE4,
	OP_CPX_ABSOLUTE = 0xEC,
	
	// CPY Compare memory and index Y
	OP_CPY_IMMEDIATE = 0xC0,
	OP_CPY_ZEROPAGE = 0xC4,
	OP_CPY_ABSOLUTE = 0xCC,
	
	// DEC Decrement memory by one
	OP_DEC_ZEROPAGE = 0xC6,
	OP_DEC_ZEROPAGEX = 0xD6,
	OP_DEC_ABSOLUTE = 0xCE,
	OP_DEC_ABSOLUTEX = 0xDE,
	
	// DEX Decrement index X by one
	OP_DEX = 0xCA,
	
	// DEY Decrement index Y by one
	OP_DEY = 0x88,
	
	// EOR "Exclusive-Or" memory with accumulator
	OP_EOR_IMMEDIATE = 0x49,
	OP_EOR_ZEROPAGE = 0x45,
	OP_EOR_ZEROPAGEX = 0x55,
	OP_EOR_ABSOLUTE = 0x4D,
	OP_EOR_ABSOLUTEX = 0x5D,
	OP_EOR_ABSOLUTEY = 0x59,
	OP_EOR_INDIRECTX = 0x41,
	OP_EOR_INDIRECTY = 0x51,
	
	// INC Increment memory by one
	OP_INC_ZEROPAGE = 0xE6,
	OP_INC_ZEROPAGEX = 0xF6,
	OP_INC_ABSOLUTE = 0xEE,
	OP_INC_ABSOLUTEX = 0xFE,
	
	// INX Increment Index X by one
	OP_INX = 0xE8,
	
	// INY Increment Index Y by one
	OP_INY = 0xC8,

	// JMP Jump to new location
	OP_JMP_ABSOLUTE = 0x4C, // TESTED
	OP_JMP_INDIRECT = 0x6C, // TESTED
	
	// JSR Jump to new location saving return address
	OP_JSR = 0x20,
	
	// LDA Load accumulator with memory
	OP_LDA_IMMEDIATE = 0xA9,
	OP_LDA_ZEROPAGE = 0xA5,
	OP_LDA_ZEROPAGEX = 0xB5,
	OP_LDA_ABSOLUTE = 0xAD,
	OP_LDA_ABSOLUTEX = 0xBD,
	OP_LDA_ABSOLUTEY = 0xB9,
	OP_LDA_INDIRECTX = 0xA1,
	OP_LDA_INDIRECTY = 0xB1,
	
	// LDX Load index X with memory
	OP_LDX_IMMEDIATE = 0xA2,
	OP_LDX_ZEROPAGE = 0xA6,
	OP_LDX_ZEROPAGEY = 0xB6,
	OP_LDX_ABSOLUTE = 0xAE,
	OP_LDX_ABSOLUTEY = 0xBE,
	
	// LDY Load index Y with memory
	OP_LDY_IMMEDIATE = 0xA0,
	OP_LDY_ZEROPAGE = 0xA4,
	OP_LDY_ZEROPAGEX = 0xB4,
	OP_LDY_ABSOLUTE = 0xAC,
	OP_LDY_ABSOLUTEX = 0xBC,
	
	// LSR Shift right one bit (memory or accumulator)
	OP_LSR_ACCUMULATOR = 0x4A,
	OP_LSR_ZEROPAGE = 0x46,
	OP_LSR_ZEROPAGEX = 0x56,
	OP_LSR_ABSOLUTE = 0x4E,
	OP_LSR_ABSOLUTEX = 0x5E,
	
	// NOP No operation
	OP_NOP = 0xEA,
	
	// ORA "OR" memory with accumulator
	OP_ORA_IMMEDIATE = 0x09,
	OP_ORA_ZEROPAGE = 0x05,
	OP_ORA_ZEROPAGEX = 0x15,
	OP_ORA_ABSOLUTE = 0x0D,
	OP_ORA_ABSOLUTEX = 0x1D,
	OP_ORA_ABSOLUTEY = 0x19,
	OP_ORA_INDIRECTX = 0x01,
	OP_ORA_INDIRECTY = 0x11,
	
	// PHA Push accumulator on stack
	OP_PHA = 0x48,
	
	// PHP Push processor status on stack
	OP_PHP = 0x08,
	
	// PLA Pull accumulator from stack
	OP_PLA = 0x68,
	
	// PLP Pull processor status from stack
	OP_PLP = 0x28,
	
	// ROL Rotate one bit left (memory or accumulator)
	OP_ROL_ACCUMULATOR = 0x2A,
	OP_ROL_ZEROPAGE = 0x26,
	OP_ROL_ZEROPAGEX = 0x36,
	OP_ROL_ABSOLUTE = 0x2E,
	OP_ROL_ABSOLUTEX = 0x3E,
	
	// ROR Rotate one bit right (memory or accumulator)
	OP_ROR_ACCUMULATOR = 0x6A,
	OP_ROR_ZEROPAGE = 0x66,
	OP_ROR_ZEROPAGEX = 0x76,
	OP_ROR_ABSOLUTE = 0x6E,
	OP_ROR_ABSOLUTEX = 0x7E,
	
	// RTI Return from interrupt
	OP_RTI = 0x40,
	
	// RTS Return from subroutine
	OP_RTS = 0x60,
	
	// SBC Subtract memory from accumulator with borrow
	OP_SBC_IMMEDIATE = 0xE9,
	OP_SBC_ZEROPAGE = 0xE5,
	OP_SBC_ZEROPAGEX = 0xF5,
	OP_SBC_ABSOLUTE = 0xED,
	OP_SBC_ABSOLUTEX = 0xFD,
	OP_SBC_ABSOLUTEY = 0xF9,
	OP_SBC_INDIRECTX = 0xE1,
	OP_SBC_INDIRECTY = 0xF1,
	
	// SEC Set carry flag
	OP_SEC = 0x38,
	
	// SED Set decimal mode
	OP_SED = 0xF8,
	
	// SEI Set interrupt disable status
	OP_SEI = 0x78,
	
	// STA Store accumulator in memory
	OP_STA_ZEROPAGE = 0x85,
	OP_STA_ZEROPAGEX = 0x95,
	OP_STA_ABSOLUTE = 0x8D,
	OP_STA_ABSOLUTEX = 0x9D,
	OP_STA_ABSOLUTEY = 0x99,
	OP_STA_INDIRECTX = 0x81,
	OP_STA_INDIRECTY = 0x91,
	
	// STX Store index X in memory
	OP_STX_ZEROPAGE = 0x86,
	OP_STX_ZEROPAGEY = 0x96,
	OP_STX_ABSOLUTE = 0x8E,
	
	// STY Store index Y in memory
	OP_STY_ZEROPAGE = 0x84,
	OP_STY_ZEROPAGEX = 0x94,
	OP_STY_ABSOLUTE = 0x8C,
	
	// TAX Transfer accumulator to index X
	OP_TAX = 0xAA,
	
	// TAY Transfer accumulator to index Y
	OP_TAY = 0xA8,
	
	// TSX Transfer stack pointer to index X
	OP_TSX = 0xBA,
	
	// TXA Transfer index X to accumulator
	OP_TXA = 0x8A,
	
	// TXS Transfer index X to stack pointer
	OP_TXS = 0x9A,
	
	// TYA Transfer index Y to accumulator
	OP_TYA = 0x98,

	////////////////////////////////////////////
	// UNOFFICIAL OPCODES

	// This opcode ASLs the contents of a memory location and then ORs the result 
	// with the accumulator.  
	OP_ASO_ABSOLUTE = 0x0F,
	OP_ASO_ABSOLUTEX = 0x1F,
	OP_ASO_ABSOLUTEY = 0x1B,
	OP_ASO_ZEROPAGE = 0x07,
	OP_ASO_ZEROPAGEX = 0x17,
	OP_ASO_INDIRECTX = 0x03,
	OP_ASO_INDIRECTY = 0x13,

	// RLA ROLs the contents of a memory location and then ANDs the result with 
	// the accumulator.
	OP_RLA_ABSOLUTE = 0x2F,
	OP_RLA_ABSOLUTEX = 0x3F,
	OP_RLA_ABSOLUTEY = 0x3B,
	OP_RLA_ZEROPAGE = 0x27,
	OP_RLA_ZEROPAGEX = 0x37,
	OP_RLA_INDIRECTX = 0x23,
	OP_RLA_INDIRECTY = 0x33,

	// LSE LSRs the contents of a memory location and then EORs the result with 
	// the accumulator.
	OP_LSE_ABSOLUTE = 0x4F,
	OP_LSE_ABSOLUTEX = 0x5F,
	OP_LSE_ABSOLUTEY = 0x5B,
	OP_LSE_ZEROPAGE = 0x47,
	OP_LSE_ZEROPAGEX = 0x57,
	OP_LSE_INDIRECTX = 0x43,
	OP_LSE_INDIRECTY = 0x53,

	// RRA RORs the contents of a memory location and then ADCs the result with 
	// the accumulator.
	OP_RRA_ABSOLUTE = 0x6F,
	OP_RRA_ABSOLUTEX = 0x7F,
	OP_RRA_ABSOLUTEY = 0x7B,
	OP_RRA_ZEROPAGE = 0x67,
	OP_RRA_ZEROPAGEX = 0x77,
	OP_RRA_INDIRECTX = 0x63,
	OP_RRA_INDIRECTY = 0x73,

	// AXS ANDs the contents of the A and X registers (without changing the 
	// contents of either register) and stores the result in memory.
	// AXS does not affect any flags in the processor status register.
	OP_AXS_ABSOLUTE = 0x8F,
	OP_AXS_ZEROPAGE = 0x87,
	OP_AXS_ZEROPAGEY = 0x97,
	OP_AXS_INDIRECTX = 0x83,

	// This opcode loads both the accumulator and the X register with the contents 
	// of a memory location.
	OP_LAX_ABSOLUTE = 0xAF,
	OP_LAX_ABSOLUTEY = 0xBF,
	OP_LAX_ZEROPAGE = 0xA7,
	OP_LAX_ZEROPAGEY = 0xB7,
	OP_LAX_INDIRECTX = 0xA3,
	OP_LAX_INDIRECTY = 0xB3,

	// This opcode DECs the contents of a memory location and then CMPs the result 
	// with the A register.
	OP_DCM_ABSOLUTE = 0xCF,
	OP_DCM_ABSOLUTEX = 0xDF,
	OP_DCM_ABSOLUTEY = 0xDB,
	OP_DCM_ZEROPAGE = 0xC7,
	OP_DCM_ZEROPAGEX = 0xD7,
	OP_DCM_INDIRECTX = 0xC3,
	OP_DCM_INDIRECTY = 0xD3,

	// This opcode INCs the contents of a memory location and then SBCs the result 
	// from the A register.
	OP_INS_ABSOLUTE = 0xEF,
	OP_INS_ABSOLUTEX = 0xFF,
	OP_INS_ABSOLUTEY = 0xFB,
	OP_INS_ZEROPAGE = 0xE7,
	OP_INS_ZEROPAGEX = 0xF7,
	OP_INS_INDIRECTX = 0xE3,
	OP_INS_INDIRECTY = 0xF3,

	// This opcode ANDs the contents of the A register with an immediate value and 
	// then LSRs the result.
	OP_ALR_IMMEDIATE = 0x4B,

	// This opcode ANDs the contents of the A register with an immediate value and 
	// then RORs the result.
	OP_ARR_IMMEDIATE = 0x6B,

	// XAA transfers the contents of the X register to the A register and then 
	// ANDs the A register with an immediate value.
	OP_XAA_IMMEDIATE = 0x8B,

	// This opcode ORs the A register with #$EE, ANDs the result with an immediate 
	// value, and then stores the result in both A and X.
	OP_OAL_IMMEDIATE = 0xAB,

	// SAX ANDs the contents of the A and X registers (leaving the contents of A 
	// intact), subtracts an immediate value, and then stores the result in X.
	// ... A few points might be made about the action of subtracting an immediate 
	// value.  It actually works just like the CMP instruction, except that CMP 
	// does not store the result of the subtraction it performs in any register.  
	// This subtract operation is not affected by the state of the Carry flag, 
	// though it does affect the Carry flag.  It does not affect the Overflow flag.
	OP_SAX_IMMEDIATE = 0xCB,

	OP_NOP_1 = 0x1A,
	OP_NOP_2 = 0x3A,
	OP_NOP_3 = 0x5A,
	OP_NOP_4 = 0x7A,
	OP_NOP_5 = 0xDA,
	OP_NOP_6 = 0xFA,

	// SKB stands for skip next byte.
	OP_SKB_1 = 0x80, // two bytes

	OP_SKB_2 = 0x82,
	OP_SKB_3 = 0xC2,
	OP_SKB_4 = 0xE2,

	OP_SKB_5 = 0x04, // all two bytes
	OP_SKB_6 = 0x14,
	OP_SKB_7 = 0x34,
	OP_SKB_8 = 0x44,
	OP_SKB_9 = 0x54,
	OP_SKB_A = 0x64,
	OP_SKB_B = 0x74,
	OP_SKB_C = 0xD4,
	OP_SKB_D = 0xF4,

	OP_SKB_E = 0x89, // Under "Opcode 89" in the docs - It requires 2 cycles to execute.

	// SKW skips next word (two bytes).
	OP_SKW_1 = 0x0C,

	// SKW 2 - 7 use indirect X addressing
	OP_SKW_2 = 0x1C,
	OP_SKW_3 = 0x3C,
	OP_SKW_4 = 0x5C,
	OP_SKW_5 = 0x7C,
	OP_SKW_6 = 0xDC,
	OP_SKW_7 = 0xFC,


	// HLT crashes the microprocessor.  When this opcode is executed, program 
	// execution ceases.  No hardware interrupts will execute either.  The author 
	// has characterized this instruction as a halt instruction since this is the 
	// most straightforward explanation for this opcode's behaviour.  Only a reset 
	// will restart execution.  This opcode leaves no trace of any operation 
	// performed!  No registers affected.
	OP_HLT_1 = 0x02,
	OP_HLT_2 = 0x12,
	OP_HLT_3 = 0x22,
	OP_HLT_4 = 0x32,
	OP_HLT_5 = 0x42,
	OP_HLT_6 = 0x52,
	OP_HLT_7 = 0x62,
	OP_HLT_8 = 0x72,
	OP_HLT_9 = 0x92,
	OP_HLT_A = 0xB2,
	OP_HLT_B = 0xD2,
	OP_HLT_C = 0xF2,

	// This opcode ANDs the contents of the A and X registers (without changing 
	// the contents of either register) and transfers the result to the stack 
	// pointer.  It then ANDs that result with the contents of the high byte of 
	// the target address of the operand +1 and stores that final result in memory.  
	OP_TAS_ABSOLUTEY = 0x9B,

	// This opcode ANDs the contents of the Y register with  and stores the 
	// result in memory.
	OP_SAY_ABSOLUTEX = 0x9C,

	// This opcode ANDs the contents of the X register with  and stores the 
	// result in memory.
	OP_XAS_ABSOLUTEY = 0x9E,

	// This opcode stores the result of A AND X AND the high byte of the target 
	// address of the operand +1 in memory.
	OP_AXA_ABSOLUTEY = 0x9F,
	OP_AXA_INDIRECTY = 0x93,

	// ANC ANDs the contents of the A register with an immediate value and then 
	// moves bit 7 of A into the Carry flag.  This opcode works basically 
	// identically to AND #immed. except that the Carry flag is set to the same 
	// state that the Negative flag is set to.
	OP_ANC_IMMEDIATE_1 = 0x2B,
	OP_ANC_IMMEDIATE_2 = 0x0B,

	// This opcode ANDs the contents of a memory location with the contents of the 
	// stack pointer register and stores the result in the accumulator, the X 
	// register, and the stack pointer.  Affected flags: N Z.
	OP_LAS_ABSOLUTEY = 0xBB,

	// Opcode EB - Opcode EB seems to work exactly like SBC #immediate.  Takes 2 cycles.
	OP_SBC_IMMEDIATE_1 = 0xEB

	////////////////////////////////////////////
};

#endif // #ifndef OPCODES6502_H

