

#include "main.h"
#include "opcodes6502.h"



static const Instruction6502 Instructions[] = {
	{ 0x00,	L"BRK",	1,	0,	7,	0,	0,	FUNC_BRK,	true	},
	{ 0x01,	L"ORA",	2,	12,	6,	0,	2,	FUNC_ORA,	true	},
	{ 0x02,	L"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x03,	L"ASO",	2,	12,	8,	0,	3,	FUNC_ASO,	false	},
	{ 0x04,	L"SKB",	2,	5,	3,	0,	0,	FUNC_SKB,	false	},
	{ 0x05,	L"ORA",	2,	5,	3,	0,	2,	FUNC_ORA,	true	},
	{ 0x06,	L"ASL",	2,	5,	5,	0,	3,	FUNC_ASL,	true	},
	{ 0x07,	L"ASO",	2,	5,	5,	0,	3,	FUNC_ASO,	false	},
	{ 0x08,	L"PHP",	1,	0,	3,	0,	0,	FUNC_PHP,	true	},
	{ 0x09,	L"ORA",	2,	3,	2,	0,	6,	FUNC_ORA,	true	},
	{ 0x0A,	L"ASL",	1,	2,	2,	0,	1,	FUNC_ASL,	true	},
	{ 0x0B,	L"ANC",	2,	3,	2,	0,	0,	FUNC_ANC,	false	},
	{ 0x0C,	L"SKW",	3,	8,	4,	0,	2,	FUNC_SKW,	false	},
	{ 0x0D,	L"ORA",	3,	8,	4,	0,	2,	FUNC_ORA,	true	},
	{ 0x0E,	L"ASL",	3,	8,	6,	0,	3,	FUNC_ASL,	true	},
	{ 0x0F,	L"ASO",	3,	8,	6,	0,	3,	FUNC_ASO,	false	},
	{ 0x10,	L"BPL",	2,	1,	2,	1,	5,	FUNC_BPL,	true	},
	{ 0x11,	L"ORA",	2,	13,	5,	1,	2,	FUNC_ORA,	true	},
	{ 0x12,	L"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x13,	L"ASO",	2,	13,	8,	0,	3,	FUNC_ASO,	false	},
	{ 0x14,	L"SKB",	2,	6,	4,	0,	0,	FUNC_SKB,	false	},
	{ 0x15,	L"ORA",	2,	6,	4,	0,	2,	FUNC_ORA,	true	},
	{ 0x16,	L"ASL",	2,	6,	6,	0,	3,	FUNC_ASL,	true	},
	{ 0x17,	L"ASO",	2,	6,	6,	0,	3,	FUNC_ASO,	false	},
	{ 0x18,	L"CLC",	1,	0,	2,	0,	0,	FUNC_CLC,	true	},
	{ 0x19,	L"ORA",	3,	10,	4,	1,	2,	FUNC_ORA,	true	},
	{ 0x1A,	L"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	false	},
	{ 0x1B,	L"ASO",	3,	10,	7,	0,	3,	FUNC_ASO,	false	},
	{ 0x1C,	L"SKW",	3,	9,	4,	1,	2,	FUNC_SKW,	false	},
	{ 0x1D,	L"ORA",	3,	9,	4,	1,	2,	FUNC_ORA,	true	},
	{ 0x1E,	L"ASL",	3,	9,	7,	0,	3,	FUNC_ASL,	true	},
	{ 0x1F,	L"ASO",	3,	9,	7,	0,	3,	FUNC_ASO,	false	},
	{ 0x20,	L"JSR",	3,	4,	6,	0,	0,	FUNC_JSR,	true	},
	{ 0x21,	L"AND",	2,	12,	6,	0,	2,	FUNC_AND,	true	},
	{ 0x22,	L"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x23,	L"RLA",	2,	12,	8,	0,	3,	FUNC_RLA,	false	},
	{ 0x24,	L"BIT",	2,	5,	3,	0,	2,	FUNC_BIT,	true	},
	{ 0x25,	L"AND",	2,	5,	3,	0,	2,	FUNC_AND,	true	},
	{ 0x26,	L"ROL",	2,	5,	5,	0,	3,	FUNC_ROL,	true	},
	{ 0x27,	L"RLA",	2,	5,	5,	0,	3,	FUNC_RLA,	false	},
	{ 0x28,	L"PLP",	1,	0,	4,	0,	0,	FUNC_PLP,	true	},
	{ 0x29,	L"AND",	2,	3,	2,	0,	6,	FUNC_AND,	true	},
	{ 0x2A,	L"ROL",	1,	2,	2,	0,	1,	FUNC_ROL,	true	},
	{ 0x2B,	L"ANC",	2,	3,	2,	0,	0,	FUNC_ANC,	false	},
	{ 0x2C,	L"BIT",	3,	8,	4,	0,	2,	FUNC_BIT,	true	},
	{ 0x2D,	L"AND",	3,	8,	4,	0,	2,	FUNC_AND,	true	},
	{ 0x2E,	L"ROL",	3,	8,	6,	0,	3,	FUNC_ROL,	true	},
	{ 0x2F,	L"RLA",	3,	8,	6,	0,	3,	FUNC_RLA,	false	},
	{ 0x30,	L"BMI",	2,	1,	2,	1,	5,	FUNC_BMI,	true	},
	{ 0x31,	L"AND",	2,	13,	5,	1,	2,	FUNC_AND,	true	},
	{ 0x32,	L"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x33,	L"RLA",	2,	13,	8,	0,	3,	FUNC_RLA,	false	},
	{ 0x34,	L"SKB",	2,	6,	4,	0,	0,	FUNC_SKB,	false	},
	{ 0x35,	L"AND",	2,	6,	4,	0,	2,	FUNC_AND,	true	},
	{ 0x36,	L"ROL",	2,	6,	6,	0,	3,	FUNC_ROL,	true	},
	{ 0x37,	L"RLA",	2,	6,	6,	0,	3,	FUNC_RLA,	false	},
	{ 0x38,	L"SEC",	1,	0,	2,	0,	0,	FUNC_SEC,	true	},
	{ 0x39,	L"AND",	3,	10,	4,	1,	2,	FUNC_AND,	true	},
	{ 0x3A,	L"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	false	},
	{ 0x3B,	L"RLA",	3,	10,	7,	0,	3,	FUNC_RLA,	false	},
	{ 0x3C,	L"SKW",	3,	9,	4,	1,	2,	FUNC_SKW,	false	},
	{ 0x3D,	L"AND",	3,	9,	4,	1,	2,	FUNC_AND,	true	},
	{ 0x3E,	L"ROL",	3,	9,	7,	0,	3,	FUNC_ROL,	true	},
	{ 0x3F,	L"RLA",	3,	9,	7,	0,	3,	FUNC_RLA,	false	},
	{ 0x40,	L"RTI",	1,	0,	6,	0,	0,	FUNC_RTI,	true	},
	{ 0x41,	L"EOR",	2,	12,	6,	0,	2,	FUNC_EOR,	true	},
	{ 0x42,	L"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x43,	L"LSE",	2,	12,	8,	0,	3,	FUNC_LSE,	false	},
	{ 0x44,	L"SKB",	2,	5,	3,	0,	0,	FUNC_SKB,	false	},
	{ 0x45,	L"EOR",	2,	5,	3,	0,	2,	FUNC_EOR,	true	},
	{ 0x46,	L"LSR",	2,	5,	5,	0,	3,	FUNC_LSR,	true	},
	{ 0x47,	L"LSE",	2,	5,	5,	0,	3,	FUNC_LSE,	false	},
	{ 0x48,	L"PHA",	1,	0,	3,	0,	0,	FUNC_PHA,	true	},
	{ 0x49,	L"EOR",	2,	3,	2,	0,	6,	FUNC_EOR,	true	},
	{ 0x4A,	L"LSR",	1,	2,	2,	0,	1,	FUNC_LSR,	true	},
	{ 0x4B,	L"ALR",	2,	3,	2,	0,	1,	FUNC_ALR,	false	},
	{ 0x4C,	L"JMP",	3,	4,	3,	0,	0,	FUNC_JMP,	true	},
	{ 0x4D,	L"EOR",	3,	8,	4,	0,	2,	FUNC_EOR,	true	},
	{ 0x4E,	L"LSR",	3,	8,	6,	0,	3,	FUNC_LSR,	true	},
	{ 0x4F,	L"LSE",	3,	8,	6,	0,	3,	FUNC_LSE,	false	},
	{ 0x50,	L"BVC",	2,	1,	2,	1,	5,	FUNC_BVC,	true	},
	{ 0x51,	L"EOR",	2,	13,	5,	1,	2,	FUNC_EOR,	true	},
	{ 0x52,	L"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x53,	L"LSE",	2,	13,	8,	0,	3,	FUNC_LSE,	false	},
	{ 0x54,	L"SKB",	2,	6,	4,	0,	0,	FUNC_SKB,	false	},
	{ 0x55,	L"EOR",	2,	6,	4,	0,	2,	FUNC_EOR,	true	},
	{ 0x56,	L"LSR",	2,	6,	6,	0,	3,	FUNC_LSR,	true	},
	{ 0x57,	L"LSE",	2,	6,	6,	0,	3,	FUNC_LSE,	false	},
	{ 0x58,	L"CLI",	1,	0,	2,	0,	0,	FUNC_CLI,	true	},
	{ 0x59,	L"EOR",	3,	10,	4,	1,	2,	FUNC_EOR,	true	},
	{ 0x5A,	L"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	false	},
	{ 0x5B,	L"LSE",	3,	10,	7,	0,	3,	FUNC_LSE,	false	},
	{ 0x5C,	L"SKW",	3,	9,	4,	1,	2,	FUNC_SKW,	false	},
	{ 0x5D,	L"EOR",	3,	9,	4,	1,	2,	FUNC_EOR,	true	},
	{ 0x5E,	L"LSR",	3,	9,	7,	0,	3,	FUNC_LSR,	true	},
	{ 0x5F,	L"LSE",	3,	9,	7,	0,	3,	FUNC_LSE,	false	},
	{ 0x60,	L"RTS",	1,	0,	6,	0,	0,	FUNC_RTS,	true	},
	{ 0x61,	L"ADC",	2,	12,	6,	0,	2,	FUNC_ADC,	true	},
	{ 0x62,	L"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x63,	L"RRA",	2,	12,	8,	0,	3,	FUNC_RRA,	false	},
	{ 0x64,	L"SKB",	2,	5,	3,	0,	0,	FUNC_SKB,	false	},
	{ 0x65,	L"ADC",	2,	5,	3,	0,	2,	FUNC_ADC,	true	},
	{ 0x66,	L"ROR",	2,	5,	5,	0,	3,	FUNC_ROR,	true	},
	{ 0x67,	L"RRA",	2,	5,	5,	0,	3,	FUNC_RRA,	false	},
	{ 0x68,	L"PLA",	1,	0,	4,	0,	0,	FUNC_PLA,	true	},
	{ 0x69,	L"ADC",	2,	3,	2,	0,	6,	FUNC_ADC,	true	},
	{ 0x6A,	L"ROR",	1,	2,	2,	0,	1,	FUNC_ROR,	true	},
	{ 0x6B,	L"ARR",	2,	3,	2,	0,	2,	FUNC_ARR,	false	},
	{ 0x6C,	L"JMP",	3,	11,	5,	1,	0,	FUNC_JMP,	true	},
	{ 0x6D,	L"ADC",	3,	8,	4,	0,	2,	FUNC_ADC,	true	},
	{ 0x6E,	L"ROR",	3,	8,	6,	0,	3,	FUNC_ROR,	true	},
	{ 0x6F,	L"RRA",	3,	8,	6,	0,	3,	FUNC_RRA,	false	},
	{ 0x70,	L"BVS",	2,	1,	2,	1,	5,	FUNC_BVS,	true	},
	{ 0x71,	L"ADC",	2,	13,	5,	1,	2,	FUNC_ADC,	true	},
	{ 0x72,	L"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x73,	L"RRA",	2,	13,	8,	0,	3,	FUNC_RRA,	false	},
	{ 0x74,	L"SKB",	2,	6,	4,	0,	0,	FUNC_SKB,	false	},
	{ 0x75,	L"ADC",	2,	6,	4,	0,	2,	FUNC_ADC,	true	},
	{ 0x76,	L"ROR",	2,	6,	6,	0,	3,	FUNC_ROR,	true	},
	{ 0x77,	L"RRA",	2,	6,	6,	0,	3,	FUNC_RRA,	false	},
	{ 0x78,	L"SEI",	1,	0,	2,	0,	0,	FUNC_SEI,	true	},
	{ 0x79,	L"ADC",	3,	10,	4,	1,	2,	FUNC_ADC,	true	},
	{ 0x7A,	L"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	false	},
	{ 0x7B,	L"RRA",	3,	10,	7,	0,	3,	FUNC_RRA,	false	},
	{ 0x7C,	L"SKW",	3,	9,	4,	1,	2,	FUNC_SKW,	false	},
	{ 0x7D,	L"ADC",	3,	9,	4,	1,	2,	FUNC_ADC,	true	},
	{ 0x7E,	L"ROR",	3,	9,	7,	0,	3,	FUNC_ROR,	true	},
	{ 0x7F,	L"RRA",	3,	9,	7,	0,	3,	FUNC_RRA,	false	},
	{ 0x80,	L"SKB",	2,	3,	2,	0,	0,	FUNC_SKB,	false	},
	{ 0x81,	L"STA",	2,	12,	6,	0,	4,	FUNC_STA,	true	},
	{ 0x82,	L"SKB",	2,	3,	2,	0,	0,	FUNC_SKB,	false	},
	{ 0x83,	L"AXS",	2,	12,	6,	0,	4,	FUNC_AXS,	false	},
	{ 0x84,	L"STY",	2,	5,	3,	0,	4,	FUNC_STY,	true	},
	{ 0x85,	L"STA",	2,	5,	3,	0,	4,	FUNC_STA,	true	},
	{ 0x86,	L"STX",	2,	5,	3,	0,	4,	FUNC_STX,	true	},
	{ 0x87,	L"AXS",	2,	5,	3,	0,	4,	FUNC_AXS,	false	},
	{ 0x88,	L"DEY",	1,	0,	2,	0,	0,	FUNC_DEY,	true	},
	{ 0x89,	L"SKB",	2,	3,	2,	0,	0,	FUNC_SKB,	false	},
	{ 0x8A,	L"TXA",	1,	0,	2,	0,	0,	FUNC_TXA,	true	},
	{ 0x8B,	L"XAA",	2,	3,	2,	0,	0,	FUNC_XAA,	false	},
	{ 0x8C,	L"STY",	3,	8,	4,	0,	4,	FUNC_STY,	true	},
	{ 0x8D,	L"STA",	3,	8,	4,	0,	4,	FUNC_STA,	true	},
	{ 0x8E,	L"STX",	3,	8,	4,	0,	4,	FUNC_STX,	true	},
	{ 0x8F,	L"AXS",	3,	8,	4,	0,	4,	FUNC_AXS,	false	},
	{ 0x90,	L"BCC",	2,	1,	2,	1,	5,	FUNC_BCC,	true	},
	{ 0x91,	L"STA",	2,	13,	6,	0,	4,	FUNC_STA,	true	},
	{ 0x92,	L"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x93,	L"AXA",	2,	13,	6,	0,	3,	FUNC_AXA,	false	},
	{ 0x94,	L"STY",	2,	6,	4,	0,	4,	FUNC_STY,	true	},
	{ 0x95,	L"STA",	2,	6,	4,	0,	4,	FUNC_STA,	true	},
	{ 0x96,	L"STX",	2,	7,	4,	0,	4,	FUNC_STX,	true	},
	{ 0x97,	L"AXS",	2,	7,	4,	0,	4,	FUNC_AXS,	false	},
	{ 0x98,	L"TYA",	1,	0,	2,	0,	0,	FUNC_TYA,	true	},
	{ 0x99,	L"STA",	3,	10,	5,	0,	4,	FUNC_STA,	true	},
	{ 0x9A,	L"TXS",	1,	0,	2,	0,	0,	FUNC_TXS,	true	},
	{ 0x9B,	L"TAS",	3,	10,	5,	0,	0,	FUNC_TAS,	false	},
	{ 0x9C,	L"SAY",	3,	14,	5,	0,	3,	FUNC_SAY,	false	},
	{ 0x9D,	L"STA",	3,	9,	5,	0,	4,	FUNC_STA,	true	},
	{ 0x9E,	L"XAS",	3,	10,	5,	0,	4,	FUNC_XAS,	false	},
	{ 0x9F,	L"AXA",	3,	10,	5,	0,	3,	FUNC_AXA,	false	},
	{ 0xA0,	L"LDY",	2,	3,	2,	0,	6,	FUNC_LDY,	true	},
	{ 0xA1,	L"LDA",	2,	12,	6,	0,	2,	FUNC_LDA,	true	},
	{ 0xA2,	L"LDX",	2,	3,	2,	0,	6,	FUNC_LDX,	true	},
	{ 0xA3,	L"LAX",	2,	12,	6,	0,	2,	FUNC_LAX,	false	},
	{ 0xA4,	L"LDY",	2,	5,	3,	0,	2,	FUNC_LDY,	true	},
	{ 0xA5,	L"LDA",	2,	5,	3,	0,	2,	FUNC_LDA,	true	},
	{ 0xA6,	L"LDX",	2,	5,	3,	0,	2,	FUNC_LDX,	true	},
	{ 0xA7,	L"LAX",	2,	5,	3,	0,	2,	FUNC_LAX,	false	},
	{ 0xA8,	L"TAY",	1,	0,	2,	0,	0,	FUNC_TAY,	true	},
	{ 0xA9,	L"LDA",	2,	3,	2,	0,	6,	FUNC_LDA,	true	},
	{ 0xAA,	L"TAX",	1,	0,	2,	0,	0,	FUNC_TAX,	true	},
	{ 0xAB,	L"OAL",	2,	3,	2,	0,	0,	FUNC_OAL,	false	},
	{ 0xAC,	L"LDY",	3,	8,	4,	0,	2,	FUNC_LDY,	true	},
	{ 0xAD,	L"LDA",	3,	8,	4,	0,	2,	FUNC_LDA,	true	},
	{ 0xAE,	L"LDX",	3,	8,	4,	0,	2,	FUNC_LDX,	true	},
	{ 0xAF,	L"LAX",	3,	8,	4,	0,	2,	FUNC_LAX,	false	},
	{ 0xB0,	L"BCS",	2,	1,	2,	1,	5,	FUNC_BCS,	true	},
	{ 0xB1,	L"LDA",	2,	13,	5,	1,	2,	FUNC_LDA,	true	},
	{ 0xB2,	L"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0xB3,	L"LAX",	2,	13,	5,	1,	2,	FUNC_LAX,	false	},
	{ 0xB4,	L"LDY",	2,	6,	4,	0,	2,	FUNC_LDY,	true	},
	{ 0xB5,	L"LDA",	2,	6,	4,	0,	2,	FUNC_LDA,	true	},
	{ 0xB6,	L"LDX",	2,	7,	4,	0,	2,	FUNC_LDX,	true	},
	{ 0xB7,	L"LAX",	2,	7,	4,	0,	2,	FUNC_LAX,	false	},
	{ 0xB8,	L"CLV",	1,	0,	2,	0,	0,	FUNC_CLV,	true	},
	{ 0xB9,	L"LDA",	3,	10,	4,	1,	2,	FUNC_LDA,	true	},
	{ 0xBA,	L"TSX",	1,	0,	2,	0,	0,	FUNC_TSX,	true	},
	{ 0xBB,	L"LAS",	3,	10,	4,	1,	2,	FUNC_LAS,	false	},
	{ 0xBC,	L"LDY",	3,	9,	4,	1,	2,	FUNC_LDY,	true	},
	{ 0xBD,	L"LDA",	3,	9,	4,	1,	2,	FUNC_LDA,	true	},
	{ 0xBE,	L"LDX",	3,	10,	4,	1,	2,	FUNC_LDX,	true	},
	{ 0xBF,	L"LAX",	3,	10,	4,	1,	2,	FUNC_LAX,	false	},
	{ 0xC0,	L"CPY",	2,	3,	2,	0,	6,	FUNC_CPY,	true	},
	{ 0xC1,	L"CMP",	2,	12,	6,	0,	2,	FUNC_CMP,	true	},
	{ 0xC2,	L"SKB",	2,	3,	2,	0,	0,	FUNC_SKB,	false	},
	{ 0xC3,	L"DCM",	2,	12,	8,	0,	3,	FUNC_DCM,	false	},
	{ 0xC4,	L"CPY",	2,	5,	3,	0,	2,	FUNC_CPY,	true	},
	{ 0xC5,	L"CMP",	2,	5,	3,	0,	2,	FUNC_CMP,	true	},
	{ 0xC6,	L"DEC",	2,	5,	5,	0,	3,	FUNC_DEC,	true	},
	{ 0xC7,	L"DCM",	2,	5,	5,	0,	3,	FUNC_DCM,	false	},
	{ 0xC8,	L"INY",	1,	0,	2,	0,	0,	FUNC_INY,	true	},
	{ 0xC9,	L"CMP",	2,	3,	2,	0,	6,	FUNC_CMP,	true	},
	{ 0xCA,	L"DEX",	1,	0,	2,	0,	0,	FUNC_DEX,	true	},
	{ 0xCB,	L"SAX",	2,	3,	2,	0,	0,	FUNC_SAX,	false	},
	{ 0xCC,	L"CPY",	3,	8,	4,	0,	2,	FUNC_CPY,	true	},
	{ 0xCD,	L"CMP",	3,	8,	4,	0,	2,	FUNC_CMP,	true	},
	{ 0xCE,	L"DEC",	3,	8,	6,	0,	3,	FUNC_DEC,	true	},
	{ 0xCF,	L"DCM",	3,	8,	6,	0,	3,	FUNC_DCM,	false	},
	{ 0xD0,	L"BNE",	2,	1,	2,	1,	5,	FUNC_BNE,	true	},
	{ 0xD1,	L"CMP",	2,	13,	5,	1,	2,	FUNC_CMP,	true	},
	{ 0xD2,	L"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0xD3,	L"DCM",	2,	13,	8,	0,	3,	FUNC_DCM,	false	},
	{ 0xD4,	L"SKB",	2,	6,	4,	0,	0,	FUNC_SKB,	false	},
	{ 0xD5,	L"CMP",	2,	6,	4,	0,	2,	FUNC_CMP,	true	},
	{ 0xD6,	L"DEC",	2,	6,	6,	0,	3,	FUNC_DEC,	true	},
	{ 0xD7,	L"DCM",	2,	6,	6,	0,	3,	FUNC_DCM,	false	},
	{ 0xD8,	L"CLD",	1,	0,	2,	0,	0,	FUNC_CLD,	true	},
	{ 0xD9,	L"CMP",	3,	10,	4,	1,	2,	FUNC_CMP,	true	},
	{ 0xDA,	L"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	false	},
	{ 0xDB,	L"DCM",	3,	10,	7,	0,	3,	FUNC_DCM,	false	},
	{ 0xDC,	L"SKW",	3,	9,	4,	1,	2,	FUNC_SKW,	false	},
	{ 0xDD,	L"CMP",	3,	9,	4,	1,	2,	FUNC_CMP,	true	},
	{ 0xDE,	L"DEC",	3,	9,	7,	0,	3,	FUNC_DEC,	true	},
	{ 0xDF,	L"DCM",	3,	9,	7,	0,	3,	FUNC_DCM,	false	},
	{ 0xE0,	L"CPX",	2,	3,	2,	0,	6,	FUNC_CPX,	true	},
	{ 0xE1,	L"SBC",	2,	12,	6,	0,	2,	FUNC_SBC,	true	},
	{ 0xE2,	L"SKB",	2,	3,	2,	0,	0,	FUNC_SKB,	false	},
	{ 0xE3,	L"INS",	2,	12,	8,	0,	3,	FUNC_INS,	false	},
	{ 0xE4,	L"CPX",	2,	5,	3,	0,	2,	FUNC_CPX,	true	},
	{ 0xE5,	L"SBC",	2,	5,	3,	0,	2,	FUNC_SBC,	true	},
	{ 0xE6,	L"INC",	2,	5,	5,	0,	3,	FUNC_INC,	true	},
	{ 0xE7,	L"INS",	2,	5,	5,	0,	3,	FUNC_INS,	false	},
	{ 0xE8,	L"INX",	1,	0,	2,	0,	0,	FUNC_INX,	true	},
	{ 0xE9,	L"SBC",	2,	3,	2,	0,	6,	FUNC_SBC,	true	},
	{ 0xEA,	L"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	true	},
	{ 0xEB,	L"SBC",	2,	3,	2,	0,	2,	FUNC_SBC,	false	},
	{ 0xEC,	L"CPX",	3,	8,	4,	0,	2,	FUNC_CPX,	true	},
	{ 0xED,	L"SBC",	3,	8,	4,	0,	2,	FUNC_SBC,	true	},
	{ 0xEE,	L"INC",	3,	8,	6,	0,	3,	FUNC_INC,	true	},
	{ 0xEF,	L"INS",	3,	8,	6,	0,	3,	FUNC_INS,	false	},
	{ 0xF0,	L"BEQ",	2,	1,	2,	1,	5,	FUNC_BEQ,	true	},
	{ 0xF1,	L"SBC",	2,	13,	5,	1,	2,	FUNC_SBC,	true	},
	{ 0xF2,	L"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0xF3,	L"INS",	2,	13,	8,	0,	3,	FUNC_INS,	false	},
	{ 0xF4,	L"SKB",	2,	6,	4,	0,	0,	FUNC_SKB,	false	},
	{ 0xF5,	L"SBC",	2,	6,	4,	0,	2,	FUNC_SBC,	true	},
	{ 0xF6,	L"INC",	2,	6,	6,	0,	3,	FUNC_INC,	true	},
	{ 0xF7,	L"INS",	2,	6,	6,	0,	3,	FUNC_INS,	false	},
	{ 0xF8,	L"SED",	1,	0,	2,	0,	0,	FUNC_SED,	true	},
	{ 0xF9,	L"SBC",	3,	10,	4,	1,	2,	FUNC_SBC,	true	},
	{ 0xFA,	L"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	false	},
	{ 0xFB,	L"INS",	3,	10,	7,	0,	3,	FUNC_INS,	false	},
	{ 0xFC,	L"SKW",	3,	9,	4,	1,	2,	FUNC_SKW,	false	},
	{ 0xFD,	L"SBC",	3,	9,	4,	1,	2,	FUNC_SBC,	true	},
	{ 0xFE,	L"INC",	3,	9,	7,	0,	3,	FUNC_INC,	true	},
	{ 0xFF,	L"INS",	3,	9,	7,	0,	3,	FUNC_INS,	false	}
};



const Instruction6502& GetInstruction6502( Byte_t opcode )
{
	return Instructions[ opcode ];
}

	//switch ( instruction.mFunctionType )
	//{
	//case FUNC_ADC:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationADC );
	//break;
	//case FUNC_AND:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationAND );
	//break;
	//case FUNC_ASL:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationASL );
	//break;
	//case FUNC_BCC:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationBCC );
	//break;
	//case FUNC_BCS:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationBCS );
	//break;
	//case FUNC_BEQ:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationBEQ );
	//break;
	//case FUNC_BIT:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationBIT );
	//break;
	//case FUNC_BMI:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationBMI );
	//break;
	//case FUNC_BNE:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationBNE );
	//break;
	//case FUNC_BPL:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationBPL );
	//break;
	//case FUNC_BRK:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationBRK );
	//break;
	//case FUNC_BVC:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationBVC );
	//break;
	//case FUNC_BVS:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationBVS );
	//break;
	//case FUNC_CLC:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationCLC );
	//break;
	//case FUNC_CLD:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationCLD );
	//break;
	//case FUNC_CLI:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationCLI );
	//break;
	//case FUNC_CLV:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationCLV );
	//break;
	//case FUNC_CMP:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationCMP );
	//break;
	//case FUNC_CPX:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationCPX );
	//break;
	//case FUNC_CPY:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationCPY );
	//break;
	//case FUNC_DEC:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationDEC );
	//break;
	//case FUNC_DEX:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationDEX );
	//break;
	//case FUNC_DEY:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationDEY );
	//break;
	//case FUNC_EOR:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationEOR );
	//break;
	//case FUNC_INC:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationINC );
	//break;
	//case FUNC_INX:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationINX );
	//break;
	//case FUNC_INY:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationINY );
	//break;
	//case FUNC_JMP:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationJMP );
	//break;
	//case FUNC_JSR:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationJSR );
	//break;
	//case FUNC_LDA:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationLDA );
	//break;
	//case FUNC_LDX:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationLDX );
	//break;
	//case FUNC_LDY:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationLDY );
	//break;
	//case FUNC_LSR:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationLSR );
	//break;
	//case FUNC_ORA:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationORA );
	//break;
	//case FUNC_PHA:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationPHA );
	//break;
	//case FUNC_PHP:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationPHP );
	//break;
	//case FUNC_PLA:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationPLA );
	//break;
	//case FUNC_PLP:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationPLP );
	//break;
	//case FUNC_ROL:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationROL );
	//break;
	//case FUNC_ROR:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationROR );
	//break;
	//case FUNC_RTI:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationRTI );
	//break;
	//case FUNC_RTS:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationRTS );
	//break;
	//case FUNC_SBC:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationSBC );
	//break;
	//case FUNC_SEC:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationSEC );
	//break;
	//case FUNC_SED:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationSED );
	//break;
	//case FUNC_SEI:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationSEI );
	//break;
	//case FUNC_STA:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationSTA );
	//break;
	//case FUNC_STX:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationSTX );
	//break;
	//case FUNC_STY:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationSTY );
	//break;
	//case FUNC_TAX:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationTAX );
	//break;
	//case FUNC_TAY:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationTAY );
	//break;
	//case FUNC_TSX:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationTSX );
	//break;
	//case FUNC_TXA:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationTXA );
	//break;
	//case FUNC_TXS:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationTXS );
	//break;
	//case FUNC_TYA:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationTYA );
	//break;

	//case FUNC_ASO:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationASO );
	//break;
	//case FUNC_RLA:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationRLA );
	//break;
	//case FUNC_LSE:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationLSE );
	//break;
	//case FUNC_RRA:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationRRA );
	//break;
	//case FUNC_AXS:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationAXS );
	//break;
	//case FUNC_LAX:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationLAX );
	//break;
	//case FUNC_DCM:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationDCM );
	//break;
	//case FUNC_INS:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationINS );
	//break;
	//case FUNC_ALR:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationALR );
	//break;
	//case FUNC_ARR:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationARR );
	//break;
	//case FUNC_XAA:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationXAA );
	//break;
	//case FUNC_OAL:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationOAL );
	//break;
	//case FUNC_SAX:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationSAX );
	//break;
	//case FUNC_NOP:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationNOP );
	//break;
	//case FUNC_SKB:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationSKB );
	//break;
	//case FUNC_SKW:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationSKW );
	//break;
	//case FUNC_HLT:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationHLT );
	//break;
	//case FUNC_TAS:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationTAS );
	//break;
	//case FUNC_SAY:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationSAY );
	//break;
	//case FUNC_XAS:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationXAS );
	//break;
	//case FUNC_AXA:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationAXA );
	//break;
	//case FUNC_ANC:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationANC );
	//break;
	//case FUNC_LAS:
	//	PERFORM_INSTRUCTION_OPERATION( instruction.mOperationMode, OperationLAS );
	//break;
	//}
