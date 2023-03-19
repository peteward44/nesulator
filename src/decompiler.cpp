

#include "main.h"
#include "decompiler.h"
#include "mainboard.h"
#include <sstream>
#include <boost/format.hpp>


Decompiler::Decompiler()
: labelnum(0)
{
}


std::wstring Decompiler::GenerateLabelName()
{
	std::wstringstream ss;
	ss << L"label_" << ++labelnum;
	return ss.str();
}


void Decompiler::Save( const std::wstring& romFile, const std::wstring& outputFile )
{
	labelnum = 0;

	CartridgePtr_t cart = CreateCartridgeFromROM( romFile, false );
	cart->GetMemoryMapper()->OnReset( true );

	UInt16_t resetAddress = MAKE_WORD( cart->GetMemoryMapper()->Read8PrgRom( 0xFFFC ), cart->GetMemoryMapper()->Read8PrgRom( 0xFFFC + 1 ) );

	DecompileFunction( cart, L"reset", resetAddress );

	out.open( outputFile.c_str() );

	CodeMap_t::iterator it = code.begin(), itEnd = code.end();

	for ( ; it != itEnd; ++it )
	{
		UInt16_t address = it->first;
		
		LabelMap_t::iterator findit;
		if ( ( findit = labels.find( address ) ) != labels.end() )
		{ // print label
			out << findit->second << L":" << std::endl;
		}

		// print instruction
		out << L"\t" << it->second;
	}

	out.close();
}


void Decompiler::DecompileFunction( CartridgePtr_t cart, const std::wstring& label, UInt16_t address )
{
	if ( address < 0x8000 )
		return;

	labels.insert( std::make_pair( address, label ) );

	//for ( int counter=address; counter<0x10000; )
	//{
	//	// if address is already in code, end function
	//	CodeMap_t::iterator findit;
	//	if ( ( findit = code.find( counter ) ) != code.end() )
	//	{
	//		break;
	//	}

	//	Byte_t opcode = cart->GetMemoryMapper()->Read8PrgRom( counter );
	//	const Instruction6502& instruction = Instructions[ opcode ];

	//	Byte_t arg1 = 0, arg2 = 0;
	//	if ( InstructionSize[ opcode ] > 1 )
	//		arg1 = cart->GetMemoryMapper()->Read8PrgRom( counter + 1 );
	//	if ( InstructionSize[ opcode ] > 2 )
	//		arg2 = cart->GetMemoryMapper()->Read8PrgRom( counter + 2 );

	//	std::stringstream sstream;
	//	sstream << g_nesMainboard->GetProcessor()->FormatLogString( counter, opcode, arg1, arg2, 0 ) << "\n";
	//	code.insert( std::make_pair( counter, sstream.str() ) );

	//	counter += InstructionSize[ opcode ];

	//	if ( opcode == OP_JSR )
	//	{
	//		DecompileFunction( cart, GenerateLabelName(), MAKE_WORD( arg1, arg2 ) ); 
	//	}
	//	else if ( opcode == OP_JMP_ABSOLUTE )
	//	{
	//		DecompileFunction( cart, GenerateLabelName(), MAKE_WORD( arg1, arg2 ) );
	//		break;
	//	}
	//	else if ( opcode == OP_JMP_INDIRECT )
	//	{ // TODO: allow for indirect jmps
	//		break;
	//	}
	//	else if ( /*opcode == OP_RTI ||*/ opcode == OP_RTS )
	//	{
	//		break;
	//	}
	//	else if ( opcode == OP_BCC || opcode == OP_BCS || opcode == OP_BEQ
	//		|| opcode == OP_BMI || opcode == OP_BNE || opcode == OP_BPL
	//		|| opcode == OP_BVC || opcode == OP_BVS )
	//	{
	//		DecompileFunction( cart, GenerateLabelName(), address + Processor6502::CalculateRelativeDifference( arg1 ) ); 
	//	}
	//}
}

