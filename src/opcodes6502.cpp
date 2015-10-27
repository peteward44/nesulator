
#include "stdafx.h"
#include "main.h"
#include "opcodes6502.h"



static const Instruction6502 Instructions[] = {
	{ 0x00,	"BRK",	1,	0,	7,	0,	0,	FUNC_BRK,	true	},
	{ 0x01,	"ORA",	2,	12,	6,	0,	2,	FUNC_ORA,	true	},
	{ 0x02,	"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x03,	"ASO",	2,	12,	8,	0,	3,	FUNC_ASO,	false	},
	{ 0x04,	"SKB",	2,	5,	3,	0,	0,	FUNC_SKB,	false	},
	{ 0x05,	"ORA",	2,	5,	3,	0,	2,	FUNC_ORA,	true	},
	{ 0x06,	"ASL",	2,	5,	5,	0,	3,	FUNC_ASL,	true	},
	{ 0x07,	"ASO",	2,	5,	5,	0,	3,	FUNC_ASO,	false	},
	{ 0x08,	"PHP",	1,	0,	3,	0,	0,	FUNC_PHP,	true	},
	{ 0x09,	"ORA",	2,	3,	2,	0,	6,	FUNC_ORA,	true	},
	{ 0x0A,	"ASL",	1,	2,	2,	0,	1,	FUNC_ASL,	true	},
	{ 0x0B,	"ANC",	2,	3,	2,	0,	0,	FUNC_ANC,	false	},
	{ 0x0C,	"SKW",	3,	8,	4,	0,	2,	FUNC_SKW,	false	},
	{ 0x0D,	"ORA",	3,	8,	4,	0,	2,	FUNC_ORA,	true	},
	{ 0x0E,	"ASL",	3,	8,	6,	0,	3,	FUNC_ASL,	true	},
	{ 0x0F,	"ASO",	3,	8,	6,	0,	3,	FUNC_ASO,	false	},
	{ 0x10,	"BPL",	2,	1,	2,	1,	5,	FUNC_BPL,	true	},
	{ 0x11,	"ORA",	2,	13,	5,	1,	2,	FUNC_ORA,	true	},
	{ 0x12,	"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x13,	"ASO",	2,	13,	8,	0,	3,	FUNC_ASO,	false	},
	{ 0x14,	"SKB",	2,	6,	4,	0,	0,	FUNC_SKB,	false	},
	{ 0x15,	"ORA",	2,	6,	4,	0,	2,	FUNC_ORA,	true	},
	{ 0x16,	"ASL",	2,	6,	6,	0,	3,	FUNC_ASL,	true	},
	{ 0x17,	"ASO",	2,	6,	6,	0,	3,	FUNC_ASO,	false	},
	{ 0x18,	"CLC",	1,	0,	2,	0,	0,	FUNC_CLC,	true	},
	{ 0x19,	"ORA",	3,	10,	4,	1,	2,	FUNC_ORA,	true	},
	{ 0x1A,	"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	false	},
	{ 0x1B,	"ASO",	3,	10,	7,	0,	3,	FUNC_ASO,	false	},
	{ 0x1C,	"SKW",	3,	9,	4,	1,	2,	FUNC_SKW,	false	},
	{ 0x1D,	"ORA",	3,	9,	4,	1,	2,	FUNC_ORA,	true	},
	{ 0x1E,	"ASL",	3,	9,	7,	0,	3,	FUNC_ASL,	true	},
	{ 0x1F,	"ASO",	3,	9,	7,	0,	3,	FUNC_ASO,	false	},
	{ 0x20,	"JSR",	3,	4,	6,	0,	0,	FUNC_JSR,	true	},
	{ 0x21,	"AND",	2,	12,	6,	0,	2,	FUNC_AND,	true	},
	{ 0x22,	"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x23,	"RLA",	2,	12,	8,	0,	3,	FUNC_RLA,	false	},
	{ 0x24,	"BIT",	2,	5,	3,	0,	2,	FUNC_BIT,	true	},
	{ 0x25,	"AND",	2,	5,	3,	0,	2,	FUNC_AND,	true	},
	{ 0x26,	"ROL",	2,	5,	5,	0,	3,	FUNC_ROL,	true	},
	{ 0x27,	"RLA",	2,	5,	5,	0,	3,	FUNC_RLA,	false	},
	{ 0x28,	"PLP",	1,	0,	4,	0,	0,	FUNC_PLP,	true	},
	{ 0x29,	"AND",	2,	3,	2,	0,	6,	FUNC_AND,	true	},
	{ 0x2A,	"ROL",	1,	2,	2,	0,	1,	FUNC_ROL,	true	},
	{ 0x2B,	"ANC",	2,	3,	2,	0,	0,	FUNC_ANC,	false	},
	{ 0x2C,	"BIT",	3,	8,	4,	0,	2,	FUNC_BIT,	true	},
	{ 0x2D,	"AND",	3,	8,	4,	0,	2,	FUNC_AND,	true	},
	{ 0x2E,	"ROL",	3,	8,	6,	0,	3,	FUNC_ROL,	true	},
	{ 0x2F,	"RLA",	3,	8,	6,	0,	3,	FUNC_RLA,	false	},
	{ 0x30,	"BMI",	2,	1,	2,	1,	5,	FUNC_BMI,	true	},
	{ 0x31,	"AND",	2,	13,	5,	1,	2,	FUNC_AND,	true	},
	{ 0x32,	"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x33,	"RLA",	2,	13,	8,	0,	3,	FUNC_RLA,	false	},
	{ 0x34,	"SKB",	2,	6,	4,	0,	0,	FUNC_SKB,	false	},
	{ 0x35,	"AND",	2,	6,	4,	0,	2,	FUNC_AND,	true	},
	{ 0x36,	"ROL",	2,	6,	6,	0,	3,	FUNC_ROL,	true	},
	{ 0x37,	"RLA",	2,	6,	6,	0,	3,	FUNC_RLA,	false	},
	{ 0x38,	"SEC",	1,	0,	2,	0,	0,	FUNC_SEC,	true	},
	{ 0x39,	"AND",	3,	10,	4,	1,	2,	FUNC_AND,	true	},
	{ 0x3A,	"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	false	},
	{ 0x3B,	"RLA",	3,	10,	7,	0,	3,	FUNC_RLA,	false	},
	{ 0x3C,	"SKW",	3,	9,	4,	1,	2,	FUNC_SKW,	false	},
	{ 0x3D,	"AND",	3,	9,	4,	1,	2,	FUNC_AND,	true	},
	{ 0x3E,	"ROL",	3,	9,	7,	0,	3,	FUNC_ROL,	true	},
	{ 0x3F,	"RLA",	3,	9,	7,	0,	3,	FUNC_RLA,	false	},
	{ 0x40,	"RTI",	1,	0,	6,	0,	0,	FUNC_RTI,	true	},
	{ 0x41,	"EOR",	2,	12,	6,	0,	2,	FUNC_EOR,	true	},
	{ 0x42,	"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x43,	"LSE",	2,	12,	8,	0,	3,	FUNC_LSE,	false	},
	{ 0x44,	"SKB",	2,	5,	3,	0,	0,	FUNC_SKB,	false	},
	{ 0x45,	"EOR",	2,	5,	3,	0,	2,	FUNC_EOR,	true	},
	{ 0x46,	"LSR",	2,	5,	5,	0,	3,	FUNC_LSR,	true	},
	{ 0x47,	"LSE",	2,	5,	5,	0,	3,	FUNC_LSE,	false	},
	{ 0x48,	"PHA",	1,	0,	3,	0,	0,	FUNC_PHA,	true	},
	{ 0x49,	"EOR",	2,	3,	2,	0,	6,	FUNC_EOR,	true	},
	{ 0x4A,	"LSR",	1,	2,	2,	0,	1,	FUNC_LSR,	true	},
	{ 0x4B,	"ALR",	2,	3,	2,	0,	1,	FUNC_ALR,	false	},
	{ 0x4C,	"JMP",	3,	4,	3,	0,	0,	FUNC_JMP,	true	},
	{ 0x4D,	"EOR",	3,	8,	4,	0,	2,	FUNC_EOR,	true	},
	{ 0x4E,	"LSR",	3,	8,	6,	0,	3,	FUNC_LSR,	true	},
	{ 0x4F,	"LSE",	3,	8,	6,	0,	3,	FUNC_LSE,	false	},
	{ 0x50,	"BVC",	2,	1,	2,	1,	5,	FUNC_BVC,	true	},
	{ 0x51,	"EOR",	2,	13,	5,	1,	2,	FUNC_EOR,	true	},
	{ 0x52,	"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x53,	"LSE",	2,	13,	8,	0,	3,	FUNC_LSE,	false	},
	{ 0x54,	"SKB",	2,	6,	4,	0,	0,	FUNC_SKB,	false	},
	{ 0x55,	"EOR",	2,	6,	4,	0,	2,	FUNC_EOR,	true	},
	{ 0x56,	"LSR",	2,	6,	6,	0,	3,	FUNC_LSR,	true	},
	{ 0x57,	"LSE",	2,	6,	6,	0,	3,	FUNC_LSE,	false	},
	{ 0x58,	"CLI",	1,	0,	2,	0,	0,	FUNC_CLI,	true	},
	{ 0x59,	"EOR",	3,	10,	4,	1,	2,	FUNC_EOR,	true	},
	{ 0x5A,	"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	false	},
	{ 0x5B,	"LSE",	3,	10,	7,	0,	3,	FUNC_LSE,	false	},
	{ 0x5C,	"SKW",	3,	9,	4,	1,	2,	FUNC_SKW,	false	},
	{ 0x5D,	"EOR",	3,	9,	4,	1,	2,	FUNC_EOR,	true	},
	{ 0x5E,	"LSR",	3,	9,	7,	0,	3,	FUNC_LSR,	true	},
	{ 0x5F,	"LSE",	3,	9,	7,	0,	3,	FUNC_LSE,	false	},
	{ 0x60,	"RTS",	1,	0,	6,	0,	0,	FUNC_RTS,	true	},
	{ 0x61,	"ADC",	2,	12,	6,	0,	2,	FUNC_ADC,	true	},
	{ 0x62,	"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x63,	"RRA",	2,	12,	8,	0,	3,	FUNC_RRA,	false	},
	{ 0x64,	"SKB",	2,	5,	3,	0,	0,	FUNC_SKB,	false	},
	{ 0x65,	"ADC",	2,	5,	3,	0,	2,	FUNC_ADC,	true	},
	{ 0x66,	"ROR",	2,	5,	5,	0,	3,	FUNC_ROR,	true	},
	{ 0x67,	"RRA",	2,	5,	5,	0,	3,	FUNC_RRA,	false	},
	{ 0x68,	"PLA",	1,	0,	4,	0,	0,	FUNC_PLA,	true	},
	{ 0x69,	"ADC",	2,	3,	2,	0,	6,	FUNC_ADC,	true	},
	{ 0x6A,	"ROR",	1,	2,	2,	0,	1,	FUNC_ROR,	true	},
	{ 0x6B,	"ARR",	2,	3,	2,	0,	2,	FUNC_ARR,	false	},
	{ 0x6C,	"JMP",	3,	11,	5,	1,	0,	FUNC_JMP,	true	},
	{ 0x6D,	"ADC",	3,	8,	4,	0,	2,	FUNC_ADC,	true	},
	{ 0x6E,	"ROR",	3,	8,	6,	0,	3,	FUNC_ROR,	true	},
	{ 0x6F,	"RRA",	3,	8,	6,	0,	3,	FUNC_RRA,	false	},
	{ 0x70,	"BVS",	2,	1,	2,	1,	5,	FUNC_BVS,	true	},
	{ 0x71,	"ADC",	2,	13,	5,	1,	2,	FUNC_ADC,	true	},
	{ 0x72,	"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x73,	"RRA",	2,	13,	8,	0,	3,	FUNC_RRA,	false	},
	{ 0x74,	"SKB",	2,	6,	4,	0,	0,	FUNC_SKB,	false	},
	{ 0x75,	"ADC",	2,	6,	4,	0,	2,	FUNC_ADC,	true	},
	{ 0x76,	"ROR",	2,	6,	6,	0,	3,	FUNC_ROR,	true	},
	{ 0x77,	"RRA",	2,	6,	6,	0,	3,	FUNC_RRA,	false	},
	{ 0x78,	"SEI",	1,	0,	2,	0,	0,	FUNC_SEI,	true	},
	{ 0x79,	"ADC",	3,	10,	4,	1,	2,	FUNC_ADC,	true	},
	{ 0x7A,	"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	false	},
	{ 0x7B,	"RRA",	3,	10,	7,	0,	3,	FUNC_RRA,	false	},
	{ 0x7C,	"SKW",	3,	9,	4,	1,	2,	FUNC_SKW,	false	},
	{ 0x7D,	"ADC",	3,	9,	4,	1,	2,	FUNC_ADC,	true	},
	{ 0x7E,	"ROR",	3,	9,	7,	0,	3,	FUNC_ROR,	true	},
	{ 0x7F,	"RRA",	3,	9,	7,	0,	3,	FUNC_RRA,	false	},
	{ 0x80,	"SKB",	2,	3,	2,	0,	0,	FUNC_SKB,	false	},
	{ 0x81,	"STA",	2,	12,	6,	0,	4,	FUNC_STA,	true	},
	{ 0x82,	"SKB",	2,	3,	2,	0,	0,	FUNC_SKB,	false	},
	{ 0x83,	"AXS",	2,	12,	6,	0,	4,	FUNC_AXS,	false	},
	{ 0x84,	"STY",	2,	5,	3,	0,	4,	FUNC_STY,	true	},
	{ 0x85,	"STA",	2,	5,	3,	0,	4,	FUNC_STA,	true	},
	{ 0x86,	"STX",	2,	5,	3,	0,	4,	FUNC_STX,	true	},
	{ 0x87,	"AXS",	2,	5,	3,	0,	4,	FUNC_AXS,	false	},
	{ 0x88,	"DEY",	1,	0,	2,	0,	0,	FUNC_DEY,	true	},
	{ 0x89,	"SKB",	2,	3,	2,	0,	0,	FUNC_SKB,	false	},
	{ 0x8A,	"TXA",	1,	0,	2,	0,	0,	FUNC_TXA,	true	},
	{ 0x8B,	"XAA",	2,	3,	2,	0,	0,	FUNC_XAA,	false	},
	{ 0x8C,	"STY",	3,	8,	4,	0,	4,	FUNC_STY,	true	},
	{ 0x8D,	"STA",	3,	8,	4,	0,	4,	FUNC_STA,	true	},
	{ 0x8E,	"STX",	3,	8,	4,	0,	4,	FUNC_STX,	true	},
	{ 0x8F,	"AXS",	3,	8,	4,	0,	4,	FUNC_AXS,	false	},
	{ 0x90,	"BCC",	2,	1,	2,	1,	5,	FUNC_BCC,	true	},
	{ 0x91,	"STA",	2,	13,	6,	0,	4,	FUNC_STA,	true	},
	{ 0x92,	"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0x93,	"AXA",	2,	13,	6,	0,	3,	FUNC_AXA,	false	},
	{ 0x94,	"STY",	2,	6,	4,	0,	4,	FUNC_STY,	true	},
	{ 0x95,	"STA",	2,	6,	4,	0,	4,	FUNC_STA,	true	},
	{ 0x96,	"STX",	2,	7,	4,	0,	4,	FUNC_STX,	true	},
	{ 0x97,	"AXS",	2,	7,	4,	0,	4,	FUNC_AXS,	false	},
	{ 0x98,	"TYA",	1,	0,	2,	0,	0,	FUNC_TYA,	true	},
	{ 0x99,	"STA",	3,	10,	5,	0,	4,	FUNC_STA,	true	},
	{ 0x9A,	"TXS",	1,	0,	2,	0,	0,	FUNC_TXS,	true	},
	{ 0x9B,	"TAS",	3,	10,	5,	0,	0,	FUNC_TAS,	false	},
	{ 0x9C,	"SAY",	3,	14,	5,	0,	3,	FUNC_SAY,	false	},
	{ 0x9D,	"STA",	3,	9,	5,	0,	4,	FUNC_STA,	true	},
	{ 0x9E,	"XAS",	3,	10,	5,	0,	4,	FUNC_XAS,	false	},
	{ 0x9F,	"AXA",	3,	10,	5,	0,	3,	FUNC_AXA,	false	},
	{ 0xA0,	"LDY",	2,	3,	2,	0,	6,	FUNC_LDY,	true	},
	{ 0xA1,	"LDA",	2,	12,	6,	0,	2,	FUNC_LDA,	true	},
	{ 0xA2,	"LDX",	2,	3,	2,	0,	6,	FUNC_LDX,	true	},
	{ 0xA3,	"LAX",	2,	12,	6,	0,	2,	FUNC_LAX,	false	},
	{ 0xA4,	"LDY",	2,	5,	3,	0,	2,	FUNC_LDY,	true	},
	{ 0xA5,	"LDA",	2,	5,	3,	0,	2,	FUNC_LDA,	true	},
	{ 0xA6,	"LDX",	2,	5,	3,	0,	2,	FUNC_LDX,	true	},
	{ 0xA7,	"LAX",	2,	5,	3,	0,	2,	FUNC_LAX,	false	},
	{ 0xA8,	"TAY",	1,	0,	2,	0,	0,	FUNC_TAY,	true	},
	{ 0xA9,	"LDA",	2,	3,	2,	0,	6,	FUNC_LDA,	true	},
	{ 0xAA,	"TAX",	1,	0,	2,	0,	0,	FUNC_TAX,	true	},
	{ 0xAB,	"OAL",	2,	3,	2,	0,	0,	FUNC_OAL,	false	},
	{ 0xAC,	"LDY",	3,	8,	4,	0,	2,	FUNC_LDY,	true	},
	{ 0xAD,	"LDA",	3,	8,	4,	0,	2,	FUNC_LDA,	true	},
	{ 0xAE,	"LDX",	3,	8,	4,	0,	2,	FUNC_LDX,	true	},
	{ 0xAF,	"LAX",	3,	8,	4,	0,	2,	FUNC_LAX,	false	},
	{ 0xB0,	"BCS",	2,	1,	2,	1,	5,	FUNC_BCS,	true	},
	{ 0xB1,	"LDA",	2,	13,	5,	1,	2,	FUNC_LDA,	true	},
	{ 0xB2,	"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0xB3,	"LAX",	2,	13,	5,	1,	2,	FUNC_LAX,	false	},
	{ 0xB4,	"LDY",	2,	6,	4,	0,	2,	FUNC_LDY,	true	},
	{ 0xB5,	"LDA",	2,	6,	4,	0,	2,	FUNC_LDA,	true	},
	{ 0xB6,	"LDX",	2,	7,	4,	0,	2,	FUNC_LDX,	true	},
	{ 0xB7,	"LAX",	2,	7,	4,	0,	2,	FUNC_LAX,	false	},
	{ 0xB8,	"CLV",	1,	0,	2,	0,	0,	FUNC_CLV,	true	},
	{ 0xB9,	"LDA",	3,	10,	4,	1,	2,	FUNC_LDA,	true	},
	{ 0xBA,	"TSX",	1,	0,	2,	0,	0,	FUNC_TSX,	true	},
	{ 0xBB,	"LAS",	3,	10,	4,	1,	2,	FUNC_LAS,	false	},
	{ 0xBC,	"LDY",	3,	9,	4,	1,	2,	FUNC_LDY,	true	},
	{ 0xBD,	"LDA",	3,	9,	4,	1,	2,	FUNC_LDA,	true	},
	{ 0xBE,	"LDX",	3,	10,	4,	1,	2,	FUNC_LDX,	true	},
	{ 0xBF,	"LAX",	3,	10,	4,	1,	2,	FUNC_LAX,	false	},
	{ 0xC0,	"CPY",	2,	3,	2,	0,	6,	FUNC_CPY,	true	},
	{ 0xC1,	"CMP",	2,	12,	6,	0,	2,	FUNC_CMP,	true	},
	{ 0xC2,	"SKB",	2,	3,	2,	0,	0,	FUNC_SKB,	false	},
	{ 0xC3,	"DCM",	2,	12,	8,	0,	3,	FUNC_DCM,	false	},
	{ 0xC4,	"CPY",	2,	5,	3,	0,	2,	FUNC_CPY,	true	},
	{ 0xC5,	"CMP",	2,	5,	3,	0,	2,	FUNC_CMP,	true	},
	{ 0xC6,	"DEC",	2,	5,	5,	0,	3,	FUNC_DEC,	true	},
	{ 0xC7,	"DCM",	2,	5,	5,	0,	3,	FUNC_DCM,	false	},
	{ 0xC8,	"INY",	1,	0,	2,	0,	0,	FUNC_INY,	true	},
	{ 0xC9,	"CMP",	2,	3,	2,	0,	6,	FUNC_CMP,	true	},
	{ 0xCA,	"DEX",	1,	0,	2,	0,	0,	FUNC_DEX,	true	},
	{ 0xCB,	"SAX",	2,	3,	2,	0,	0,	FUNC_SAX,	false	},
	{ 0xCC,	"CPY",	3,	8,	4,	0,	2,	FUNC_CPY,	true	},
	{ 0xCD,	"CMP",	3,	8,	4,	0,	2,	FUNC_CMP,	true	},
	{ 0xCE,	"DEC",	3,	8,	6,	0,	3,	FUNC_DEC,	true	},
	{ 0xCF,	"DCM",	3,	8,	6,	0,	3,	FUNC_DCM,	false	},
	{ 0xD0,	"BNE",	2,	1,	2,	1,	5,	FUNC_BNE,	true	},
	{ 0xD1,	"CMP",	2,	13,	5,	1,	2,	FUNC_CMP,	true	},
	{ 0xD2,	"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0xD3,	"DCM",	2,	13,	8,	0,	3,	FUNC_DCM,	false	},
	{ 0xD4,	"SKB",	2,	6,	4,	0,	0,	FUNC_SKB,	false	},
	{ 0xD5,	"CMP",	2,	6,	4,	0,	2,	FUNC_CMP,	true	},
	{ 0xD6,	"DEC",	2,	6,	6,	0,	3,	FUNC_DEC,	true	},
	{ 0xD7,	"DCM",	2,	6,	6,	0,	3,	FUNC_DCM,	false	},
	{ 0xD8,	"CLD",	1,	0,	2,	0,	0,	FUNC_CLD,	true	},
	{ 0xD9,	"CMP",	3,	10,	4,	1,	2,	FUNC_CMP,	true	},
	{ 0xDA,	"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	false	},
	{ 0xDB,	"DCM",	3,	10,	7,	0,	3,	FUNC_DCM,	false	},
	{ 0xDC,	"SKW",	3,	9,	4,	1,	2,	FUNC_SKW,	false	},
	{ 0xDD,	"CMP",	3,	9,	4,	1,	2,	FUNC_CMP,	true	},
	{ 0xDE,	"DEC",	3,	9,	7,	0,	3,	FUNC_DEC,	true	},
	{ 0xDF,	"DCM",	3,	9,	7,	0,	3,	FUNC_DCM,	false	},
	{ 0xE0,	"CPX",	2,	3,	2,	0,	6,	FUNC_CPX,	true	},
	{ 0xE1,	"SBC",	2,	12,	6,	0,	2,	FUNC_SBC,	true	},
	{ 0xE2,	"SKB",	2,	3,	2,	0,	0,	FUNC_SKB,	false	},
	{ 0xE3,	"INS",	2,	12,	8,	0,	3,	FUNC_INS,	false	},
	{ 0xE4,	"CPX",	2,	5,	3,	0,	2,	FUNC_CPX,	true	},
	{ 0xE5,	"SBC",	2,	5,	3,	0,	2,	FUNC_SBC,	true	},
	{ 0xE6,	"INC",	2,	5,	5,	0,	3,	FUNC_INC,	true	},
	{ 0xE7,	"INS",	2,	5,	5,	0,	3,	FUNC_INS,	false	},
	{ 0xE8,	"INX",	1,	0,	2,	0,	0,	FUNC_INX,	true	},
	{ 0xE9,	"SBC",	2,	3,	2,	0,	6,	FUNC_SBC,	true	},
	{ 0xEA,	"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	true	},
	{ 0xEB,	"SBC",	2,	3,	2,	0,	2,	FUNC_SBC,	false	},
	{ 0xEC,	"CPX",	3,	8,	4,	0,	2,	FUNC_CPX,	true	},
	{ 0xED,	"SBC",	3,	8,	4,	0,	2,	FUNC_SBC,	true	},
	{ 0xEE,	"INC",	3,	8,	6,	0,	3,	FUNC_INC,	true	},
	{ 0xEF,	"INS",	3,	8,	6,	0,	3,	FUNC_INS,	false	},
	{ 0xF0,	"BEQ",	2,	1,	2,	1,	5,	FUNC_BEQ,	true	},
	{ 0xF1,	"SBC",	2,	13,	5,	1,	2,	FUNC_SBC,	true	},
	{ 0xF2,	"HLT",	1,	0,	2,	0,	0,	FUNC_HLT,	false	},
	{ 0xF3,	"INS",	2,	13,	8,	0,	3,	FUNC_INS,	false	},
	{ 0xF4,	"SKB",	2,	6,	4,	0,	0,	FUNC_SKB,	false	},
	{ 0xF5,	"SBC",	2,	6,	4,	0,	2,	FUNC_SBC,	true	},
	{ 0xF6,	"INC",	2,	6,	6,	0,	3,	FUNC_INC,	true	},
	{ 0xF7,	"INS",	2,	6,	6,	0,	3,	FUNC_INS,	false	},
	{ 0xF8,	"SED",	1,	0,	2,	0,	0,	FUNC_SED,	true	},
	{ 0xF9,	"SBC",	3,	10,	4,	1,	2,	FUNC_SBC,	true	},
	{ 0xFA,	"NOP",	1,	0,	2,	0,	0,	FUNC_NOP,	false	},
	{ 0xFB,	"INS",	3,	10,	7,	0,	3,	FUNC_INS,	false	},
	{ 0xFC,	"SKW",	3,	9,	4,	1,	2,	FUNC_SKW,	false	},
	{ 0xFD,	"SBC",	3,	9,	4,	1,	2,	FUNC_SBC,	true	},
	{ 0xFE,	"INC",	3,	9,	7,	0,	3,	FUNC_INC,	true	},
	{ 0xFF,	"INS",	3,	9,	7,	0,	3,	FUNC_INS,	false	}
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
