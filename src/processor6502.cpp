

#include "main.h"
#include "processor6502.h"
#include "mainboard.h"
#include <cmath>
#include <sstream>
#include "gui/app.h"
#include <boost/format.hpp>

#include "processor6502.inl"

using namespace boost::placeholders;

#define PMIN( a, b )      ( (a) > (b) ? (b) : (a) )


const UInt16_t RESET_ADDRESS = 0xFFFC;
const UInt16_t IRQ_ADDRESS = 0xFFFE;
const UInt16_t NMI_ADDRESS = 0xFFFA;




#define IMMEDIATE_OPERATION( func ) \
{ \
	func( value ); \
}


#define ACCUMULATOR_OPERATION( func ) \
{ \
	IncrementSubCycle( true ); \
	accumulatorRegister = func( value.Data.Part8.b1 ).Data.Part8.b1; \
}


#define READ_OPERATION( func ) \
{ \
	IncrementSubCycle( true ); \
	const Byte_t b = memory->Read8( value.Data.Part16.w1 ); \
	func( b ); \
}


#define READ_MODIFY_WRITE_OPERATION( func ) \
{ \
	IncrementSubCycle(); \
	const Byte_t data = memory->Read8( value.Data.Part16.w1 ); \
	IncrementSubCycle(); \
	memory->Write8( value.Data.Part16.w1, data ); \
	\
	IncrementSubCycle( true ); \
	const MultiSizeInt a = func( data ); \
	memory->Write8( value.Data.Part16.w1, a.Data.Part8.b1 ); \
}

#define WRITE_OPERATION( func ) \
{ \
	IncrementSubCycle( true ); \
	const Byte_t b = func(); \
	memory->Write8( value.Data.Part16.w1, b ); \
}


#define BRANCH_OPERATION( func ) \
{ \
	mBranchTaken = func(); \
	if ( mBranchTaken ) \
	{ \
		IncrementSubCycle(); \
		value = memory->Read8( programCounter + 1 ); \
		const int diff = CalculateRelativeDifference( value.Data.Part8.b1 ); \
		if ( diff != 0 ) \
		{ \
			mPageBoundaryCrossed = ( ((UInt16_t)(programCounter + 2) & 0xff00 ) != ( ((UInt16_t)(diff + programCounter + 2) ) & 0xff00 ) ); \
		} \
		if ( mPageBoundaryCrossed ) \
			IncrementSubCycle(); \
		IncrementSubCycle( true ); \
		programCounter += diff + instruction.mSize; \
	} \
	else \
	{ \
		IncrementSubCycle( true ); \
		memory->Read8( programCounter + 1 ); \
		programCounter += instruction.mSize; \
	} \
}



// TODO: Decimal mode (not used in the NES, so not important)


FORCE_INLINE UInt16_t Wrap( UInt16_t t )
{
	return t % 0x100;
}



Processor6502::Processor6502()
{
	SetDefaultVariables();
	mDecimalModeEnabled = false;
	mCmosVersion = false;
	mMemory = NULL;

	resetConnection = g_nesMainboard->ResetEvent.connect( boost::bind( &Processor6502::OnReset, this, _1 ) );
}


void Processor6502::SetDefaultVariables()
{
	addressLoopFrom = addressLoopTo = loopCount = -1;

	accumulatorRegister = xRegister = yRegister = stackPointer = 0;
	irqLineLow = false;
	mWaitOneInstructionAfterCli = false;
	programCounter = 0;
	nmiPending = resetPending = false;
	subCycle = 0;
	isLastCycle = triggerNmiAfterNextInstruction = false;
	mPageBoundaryCrossed = false;
	mBranchTaken = false;

	mStatusRegister.Byte = 0;
	mStatusRegister.Status.Unused = true;
	mStatusRegister.Status.Break = true;
}


Processor6502::~Processor6502()
{
	resetConnection.disconnect();
}


void Processor6502::OnReset( bool cold )
{
	SetDefaultVariables();
	resetPending = true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int Processor6502::HandlePendingInterrupts( CPUMemory* cpuMemory )
{
	mMemory = cpuMemory;

	// TODO: if an NMI interrupt is interrupted by a BRK, dont execute the BRK (6502 bug - fixed in the CMOS version)
	if ( resetPending )
	{
		for ( int i=0; i<3; ++i )
			IncrementStackCounter(); // increment stack pointer but dont write to memory

		mStatusRegister.Status.Break = false;
		mStatusRegister.Status.Interrupt = true;

		if ( mCmosVersion )
			mStatusRegister.Status.Decimal = false;

		programCounter = cpuMemory->Read16( RESET_ADDRESS, false );
		//programCounter = 0xC000;

#ifdef LOG_PROCESSOR_INSTRUCTIONS
		if ( Log::IsTypeEnabled( LOG_CPU ) )
			Log::Write( LOG_CPU, ( boost::wformat( L"%1% RESET triggered" ) % (int)programCounter ).str().c_str() );
#endif

		resetPending = false;

		return 0;
	}

	if ( nmiPending )
	{
		if ( triggerNmiAfterNextInstruction )
		{
			triggerNmiAfterNextInstruction = false;
			return 0;
		}

		// NMI interrupt
		PushStack( cpuMemory, ( programCounter >> 8 ) & 0xFF );
		IncrementStackCounter();
		PushStack( cpuMemory, programCounter & 0xFF );
		IncrementStackCounter();

		mStatusRegister.Status.Break = false;

		PushStack( cpuMemory, mStatusRegister.Byte );
		IncrementStackCounter();

		mStatusRegister.Status.Interrupt = true;
		if ( mCmosVersion )
			mStatusRegister.Status.Decimal = false;
		programCounter = cpuMemory->Read16( NMI_ADDRESS, false );

		nmiPending = false;

#ifdef LOG_PROCESSOR_INSTRUCTIONS
		if ( Log::IsTypeEnabled( LOG_CPU ) )
			Log::Write( LOG_CPU, ( boost::wformat( L"%1% NMI triggered" ) % programCounter ).str().c_str() );
#endif

		return 7;
	}

	if ( this->irqLineLow && !this->mWaitOneInstructionAfterCli && !this->mStatusRegister.Status.Interrupt )
	{
		// IRQ interrupt
		PushStack( cpuMemory, ( programCounter >> 8 ) & 0xFF );
		IncrementStackCounter();
		PushStack( cpuMemory, programCounter & 0xFF );
		IncrementStackCounter();

		mStatusRegister.Status.Break = false;

		PushStack( cpuMemory, mStatusRegister.Byte );
		IncrementStackCounter();

		mStatusRegister.Status.Interrupt = true;
		if ( mCmosVersion )
			mStatusRegister.Status.Decimal = false;
		programCounter = cpuMemory->Read16( IRQ_ADDRESS, false );

//#ifdef LOG_PROCESSOR_INSTRUCTIONS
//		if ( Log::IsTypeEnabled( LOG_MISC ) )
//			Log::Write( LOG_MISC, ( boost::format( "IRQ triggered, MTC '%1%'" ) % g_nesMainboard->GetSynchroniser()->GetCpuMTC() ).str().c_str() );
//#endif

		return 7;
	}

	return 0;
}



void Processor6502::OperationBRK( CPUMemory* cpuMemory )
{
	// dummy read of opcode after brk
	IncrementSubCycle();
	cpuMemory->Read8( programCounter );
	programCounter++;

	IncrementSubCycle();
	PushStack( cpuMemory, ( programCounter >> 8 ) & 0xFF );
	IncrementStackCounter();

	IncrementSubCycle();
	PushStack( cpuMemory, programCounter & 0xFF );
	IncrementStackCounter();

	//mStatusRegister.Status.Break = true;
	//mStatusRegister.Status.Unused = true;

	IncrementSubCycle();
	PushStack( cpuMemory, ( mStatusRegister.Byte | 0x30 ) );
	IncrementStackCounter();

	programCounter = Read16IncSubCycles( cpuMemory, IRQ_ADDRESS, true );
	
	mStatusRegister.Status.Interrupt = true;
	if ( mCmosVersion ) // Decimal flag is cleared in the CMOS version
		mStatusRegister.Status.Decimal = false;
}


void Processor6502::NonMaskableInterrupt( int ppuMasterTickCount )
{
	nmiPending = true;
	if ( isLastCycle || g_nesMainboard->GetSynchroniser()->GetCpuMTC() - g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU() <= ppuMasterTickCount )
		triggerNmiAfterNextInstruction = true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Processor6502::SaveState( std::ostream& ostr )
{
	WriteType< Byte_t >( ostr, mStatusRegister.Byte );
	WriteType< Byte_t >( ostr, accumulatorRegister );
	WriteType< Byte_t >( ostr, xRegister );
	WriteType< Byte_t >( ostr, yRegister );
	WriteType< Byte_t >( ostr, stackPointer );
	WriteType< UInt16_t >( ostr, programCounter );
	WriteType< boost::uint8_t >( ostr, irqLineLow ? 1 : 0 );
	WriteType< boost::uint8_t >( ostr, mWaitOneInstructionAfterCli ? 1 : 0 );
	WriteType< boost::uint8_t >( ostr, nmiPending ? 1 : 0 );
	WriteType< boost::uint8_t >( ostr, resetPending ? 1 : 0 );
	WriteType< boost::uint8_t >( ostr, triggerNmiAfterNextInstruction ? 1 : 0 );
}


void Processor6502::LoadState( std::istream& istr )
{
	mStatusRegister.Byte = ReadType< Byte_t >( istr );
	accumulatorRegister = ReadType< Byte_t >( istr );
	xRegister = ReadType< Byte_t >( istr );
	yRegister = ReadType< Byte_t >( istr );
	stackPointer = ReadType< Byte_t >( istr );
	programCounter = ReadType< UInt16_t >( istr );
	irqLineLow = ReadType< boost::uint8_t >( istr ) > 0;
	mWaitOneInstructionAfterCli = ReadType< boost::uint8_t >( istr ) > 0;
	nmiPending = ReadType< boost::uint8_t >( istr ) > 0;
	resetPending = ReadType< boost::uint8_t >( istr ) > 0;
	triggerNmiAfterNextInstruction = ReadType< boost::uint8_t >( istr ) > 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int Processor6502::ExecuteInstructionFast( CPUMemory* memory )
{
	if ( mWaitOneInstructionAfterCli )
		mWaitOneInstructionAfterCli = false;

	// fetch opcode and increment PC always takes one cycle
	SetSubCycle( 0 );
	Byte_t opcode = memory->Read8( programCounter );

	const Instruction6502& instruction = GetInstruction6502( opcode );
	mInstruction = &instruction;

	MultiSizeInt value;
	mPageBoundaryCrossed = false;
	mBranchTaken = false;

	switch ( instruction.mAddressingMode )
	{
	default:
		case ADDRESS_NONE:
			break;
		case ADDRESS_ACCUMULATOR:
			value = accumulatorRegister;
			break;
		case ADDRESS_RELATIVE:
			{
			}
			break;
		case ADDRESS_IMMEDIATE:
			{
				IncrementSubCycle( instruction.mBaseCycles == 2 );
				value = memory->Read8( programCounter + 1 );
			}
			break;
		case ADDRESS_IMMEDIATE16: // Used only by JSR and JMP
			value = Read16IncSubCycles( memory, programCounter + 1, opcode == OP_JMP_ABSOLUTE );
			break;
		case ADDRESS_INDIRECT: // only used by indirect JMP
			{
				const UInt16_t address = Read16IncSubCycles( memory, programCounter + 1 );
				value = Read16IncSubCycles( memory, address, true, mCmosVersion ? false : true ); // CMOS version doesn't wrap on zero page
				break;
			}
		case ADDRESS_ZEROPAGE:
			IncrementSubCycle();
			value = memory->Read8( programCounter + 1 );
			break;
		case ADDRESS_ZEROPAGEX:
			{
				IncrementSubCycle();
				const UInt16_t arg = memory->Read8( programCounter + 1 );
				value = Wrap( arg + xRegister );
				IncrementSubCycle();
				memory->Read8( CalculateDummyReadAddress( arg, xRegister ) );
			}
			break;
		case ADDRESS_ZEROPAGEY:
			{
				IncrementSubCycle();
				const UInt16_t arg = memory->Read8( programCounter + 1 );
				value = Wrap( arg + yRegister );
				IncrementSubCycle();
				memory->Read8( CalculateDummyReadAddress( arg, yRegister ) );
			}
			break;
		case ADDRESS_ABSOLUTE:
			value = Read16IncSubCycles( memory, programCounter + 1 );
			break;
		case ADDRESS_ABSOLUTEX:
			{
				const UInt16_t address = Read16IncSubCycles( memory, programCounter + 1 );
				value = address + xRegister;

				PerformDummyRead( instruction, address, xRegister );
				break;
			}
		case ADDRESS_ABSOLUTEY:
			{
				const UInt16_t address = Read16IncSubCycles( memory, programCounter + 1 );
				value = address + yRegister;

				PerformDummyRead( instruction, address, yRegister );
				break;
			}
		case ADDRESS_INDIRECTX:
			{
				IncrementSubCycle();
				const Byte_t arg = memory->Read8( programCounter + 1 );
				IncrementSubCycle();
				const UInt16_t address = Wrap( arg + xRegister );
				value = Read16IncSubCycles( memory, address, false, true );
				break;
			}
		case ADDRESS_INDIRECTY:
			{
				IncrementSubCycle();
				const Byte_t arg = memory->Read8( programCounter + 1 );
				const UInt16_t address = Read16IncSubCycles( memory, arg, false, true );
				value = address + yRegister;
				PerformDummyRead( instruction, address, yRegister );
				break;
			}
		case ADDRESS_SAY:
			{
				IncrementSubCycle();
				UInt16_t address = memory->Read8( programCounter + 1 );
				address |= ( memory->Read8( programCounter + 2 ) << 8 );
				value = address + xRegister; // SAY writes to absolute X but needs the high byte of the address to operate on
				break;
			}
	}

#ifdef LOG_PROCESSOR_INSTRUCTIONS
	LogProcessorMessage( programCounter, opcode, /*instruction.mSize > 1 ? memory->Read8( programCounter + 1 ) :*/ 0,
		/*instruction.mSize > 2 ? memory->Read8( programCounter + 2 ) :*/ 0 );
	if ( Log::IsTypeEnabled( LOG_CPU ) )
	{
//		wxGetApp().Sleep( 0 ); // sleep for half a second after each instruction if logging
//		wxGetApp().DoWindowsEvents();
	}
#endif

	if ( instruction.mAddressingMode != ADDRESS_RELATIVE )
		programCounter += instruction.mSize;

	switch ( opcode )
	{
	// Add memory to accumulator with carry
	case OP_ADC_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationADC );
		break;
	case OP_ADC_ZEROPAGE:
	case OP_ADC_ZEROPAGEX:
	case OP_ADC_ABSOLUTE:
	case OP_ADC_ABSOLUTEX:
	case OP_ADC_ABSOLUTEY:
	case OP_ADC_INDIRECTX:
	case OP_ADC_INDIRECTY:
		READ_OPERATION( OperationADC );
		break;
	
	// "AND" memory with accumulator
	case OP_AND_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationAND );
		break;
	case OP_AND_ZEROPAGE:
	case OP_AND_ZEROPAGEX:
	case OP_AND_ABSOLUTE:
	case OP_AND_ABSOLUTEX:
	case OP_AND_ABSOLUTEY:
	case OP_AND_INDIRECTX:
	case OP_AND_INDIRECTY:
		READ_OPERATION( OperationAND );
		break;
	
	// ASL Shift Left One Bit (Memory or Accumulator)
	case OP_ASL_ACCUMULATOR:
		ACCUMULATOR_OPERATION( OperationASL );
		break;
	case OP_ASL_ZEROPAGE:
	case OP_ASL_ZEROPAGEX:
	case OP_ASL_ABSOLUTE:
	case OP_ASL_ABSOLUTEX:
		READ_MODIFY_WRITE_OPERATION( OperationASL );
		break;

	// BCC Branch on Carry Clear
	case OP_BCC:
		BRANCH_OPERATION( OperationBCC );
		break;
	
	// BCS Branch on carry set
	case OP_BCS:
		BRANCH_OPERATION( OperationBCS );
		break;
	
	// BEQ Branch on result zero
	case OP_BEQ:
		BRANCH_OPERATION( OperationBEQ );
		break;
	
	// BIT Test bits in memory with accumulator
	case OP_BIT_ZEROPAGE:
	case OP_BIT_ABSOLUTE:
		READ_OPERATION( OperationBIT );
		break;
	
	// BMI Branch on result minus
	case OP_BMI:
		BRANCH_OPERATION( OperationBMI );
		break;
	
	// BNE Branch on result not zero
	case OP_BNE:
		BRANCH_OPERATION( OperationBNE );
		break;
	
	// BPL Branch on result plus
	case OP_BPL:
		BRANCH_OPERATION( OperationBPL );
		break;
	
	// BRK Force Break
	case OP_BRK:
		OperationBRK( memory );
		break;
	
	// BVC Branch on overflow clear
	case OP_BVC:
		BRANCH_OPERATION( OperationBVC );
		break;
	
	// BVS Branch on overflow set
	case OP_BVS:
		BRANCH_OPERATION( OperationBVS );
		break;
	
	// CLC Clear carry flag
	case OP_CLC:
		IncrementSubCycle( true );
		OperationCLC();
		break;
	
	// CLD Clear decimal mode
	case OP_CLD:
		IncrementSubCycle( true );
		OperationCLD();
		break;
	
	// CLI Clear interrupt disable bit
	case OP_CLI:
		IncrementSubCycle( true );
		OperationCLI();
		break;
	
	// CLV Clear overflow flag
	case OP_CLV:
		IncrementSubCycle( true );
		OperationCLV();
		break;
	
	// CMP Compare memory and accumulator
	case OP_CMP_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationCMP );
		break;
	case OP_CMP_ZEROPAGE:
	case OP_CMP_ZEROPAGEX:
	case OP_CMP_ABSOLUTE:
	case OP_CMP_ABSOLUTEX:
	case OP_CMP_ABSOLUTEY:
	case OP_CMP_INDIRECTX:
	case OP_CMP_INDIRECTY:
		READ_OPERATION( OperationCMP );
		break;
	
	// CPX Compare Memory and Index X
	case OP_CPX_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationCPX );
		break;
	case OP_CPX_ZEROPAGE:
	case OP_CPX_ABSOLUTE:
		READ_OPERATION( OperationCPX );
		break;
	
	// CPY Compare memory and index Y
	case OP_CPY_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationCPY );
		break;
	case OP_CPY_ZEROPAGE:
	case OP_CPY_ABSOLUTE:
		READ_OPERATION( OperationCPY );
		break;
	
	// DEC Decrement memory by one
	case OP_DEC_ZEROPAGE:
	case OP_DEC_ZEROPAGEX:
	case OP_DEC_ABSOLUTE:
	case OP_DEC_ABSOLUTEX:
		READ_MODIFY_WRITE_OPERATION( OperationDEC );
		break;

	// DEX Decrement index X by one
	case OP_DEX:
		IncrementSubCycle( true );
		OperationDEX();
		break;
	
	// DEY Decrement index Y by one
	case OP_DEY:
		IncrementSubCycle( true );
		OperationDEY();
		break;
	
	// EOR "Exclusive-Or" memory with accumulator
	case OP_EOR_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationEOR );
		break;
	case OP_EOR_ZEROPAGE:
	case OP_EOR_ZEROPAGEX:
	case OP_EOR_ABSOLUTE:
	case OP_EOR_ABSOLUTEX:
	case OP_EOR_ABSOLUTEY:
	case OP_EOR_INDIRECTX:
	case OP_EOR_INDIRECTY:
		READ_OPERATION( OperationEOR );
		break;
	
	// INC Increment memory by one
	case OP_INC_ZEROPAGE:
	case OP_INC_ZEROPAGEX:
	case OP_INC_ABSOLUTE:
	case OP_INC_ABSOLUTEX:
		READ_MODIFY_WRITE_OPERATION( OperationINC );
		break;

	// INX Increment Index X by one
	case OP_INX:
		IncrementSubCycle( true );
		OperationINX();
		break;
	
	// INY Increment Index Y by one
	case OP_INY:
		IncrementSubCycle( true );
		OperationINY();
		break;

	// JMP Jump to new location
	case OP_JMP_ABSOLUTE:
	case OP_JMP_INDIRECT:
		OperationJMP( value.Data.Part16.w1 );
		break;
	
	// JSR Jump to new location saving return address
	case OP_JSR:
		OperationJSR( memory, value.Data.Part16.w1 );
		break;
	
	// LDA Load accumulator with memory
	case OP_LDA_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationLDA );
		break;
	case OP_LDA_ZEROPAGE:
	case OP_LDA_ZEROPAGEX:
	case OP_LDA_ABSOLUTE:
	case OP_LDA_ABSOLUTEX:
	case OP_LDA_ABSOLUTEY:
	case OP_LDA_INDIRECTX:
	case OP_LDA_INDIRECTY:
		READ_OPERATION( OperationLDA );
		break;
	
	// LDX Load index X with memory
	case OP_LDX_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationLDX );
		break;
	case OP_LDX_ZEROPAGE:
	case OP_LDX_ZEROPAGEY:
	case OP_LDX_ABSOLUTE:
	case OP_LDX_ABSOLUTEY:
		READ_OPERATION( OperationLDX );
		break;
	
	// LDY Load index Y with memory
	case OP_LDY_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationLDY );
		break;
	case OP_LDY_ZEROPAGE:
	case OP_LDY_ZEROPAGEX:
	case OP_LDY_ABSOLUTE:
	case OP_LDY_ABSOLUTEX:
		READ_OPERATION( OperationLDY );
		break;
	
	// LSR Shift right one bit (memory or accumulator)
	case OP_LSR_ACCUMULATOR:
		ACCUMULATOR_OPERATION( OperationLSR );
		break;
	case OP_LSR_ZEROPAGE:
	case OP_LSR_ZEROPAGEX:
	case OP_LSR_ABSOLUTE:
	case OP_LSR_ABSOLUTEX:
		READ_MODIFY_WRITE_OPERATION( OperationLSR );
		break;

	// NOP No operation
	case OP_NOP:
		IncrementSubCycle( true );
		break;
	
	// ORA "OR" memory with accumulator
	case OP_ORA_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationORA );
		break;
	case OP_ORA_ZEROPAGE:
	case OP_ORA_ZEROPAGEX:
	case OP_ORA_ABSOLUTE:
	case OP_ORA_ABSOLUTEX:
	case OP_ORA_ABSOLUTEY:
	case OP_ORA_INDIRECTX:
	case OP_ORA_INDIRECTY:
		READ_OPERATION( OperationORA );
		break;
	
	// PHA Push accumulator on stack
	case OP_PHA:
		OperationPHA( memory );
		break;
	
	// PHP Push processor status on stack
	case OP_PHP:
		OperationPHP( memory );
		break;
	
	// PLA Pull accumulator from stack
	case OP_PLA:
		OperationPLA( memory );
		break;
	
	// PLP Pull processor status from stack
	case OP_PLP:
		OperationPLP( memory );
		break;
	
	// ROL Rotate one bit left (memory or accumulator)
	case OP_ROL_ACCUMULATOR:
		ACCUMULATOR_OPERATION( OperationROL );
		break;
	case OP_ROL_ZEROPAGE:
	case OP_ROL_ZEROPAGEX:
	case OP_ROL_ABSOLUTE:
	case OP_ROL_ABSOLUTEX:
		READ_MODIFY_WRITE_OPERATION( OperationROL );
		break;

	// ROR Rotate one bit right (memory or accumulator)
	case OP_ROR_ACCUMULATOR:
		ACCUMULATOR_OPERATION( OperationROR );
		break;
	case OP_ROR_ZEROPAGE:
	case OP_ROR_ZEROPAGEX:
	case OP_ROR_ABSOLUTE:
	case OP_ROR_ABSOLUTEX:
		READ_MODIFY_WRITE_OPERATION( OperationROR );
		break;

	// RTI Return from interrupt
	case OP_RTI:
		OperationRTI( memory );
		break;
	
	// RTS Return from subroutine
	case OP_RTS:
		OperationRTS( memory );
		break;
	
	// SBC Subtract memory from accumulator with borrow
	case OP_SBC_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationSBC );
		break;
	case OP_SBC_ZEROPAGE:
	case OP_SBC_ZEROPAGEX:
	case OP_SBC_ABSOLUTE:
	case OP_SBC_ABSOLUTEX:
	case OP_SBC_ABSOLUTEY:
	case OP_SBC_INDIRECTX:
	case OP_SBC_INDIRECTY:
		READ_OPERATION( OperationSBC );
		break;
	
	// SEC Set carry flag
	case OP_SEC:
		IncrementSubCycle( true );
		OperationSEC();
		break;
	
	// SED Set decimal mode
	case OP_SED:
		IncrementSubCycle( true );
		OperationSED();
		break;
	
	// SEI Set interrupt disable status
	case OP_SEI:
		IncrementSubCycle( true );
		OperationSEI();
		break;
	
	// STA Store accumulator in memory
	case OP_STA_ZEROPAGE:
	case OP_STA_ZEROPAGEX:
	case OP_STA_ABSOLUTE:
	case OP_STA_ABSOLUTEX:
	case OP_STA_ABSOLUTEY:
	case OP_STA_INDIRECTX:
	case OP_STA_INDIRECTY:
		WRITE_OPERATION( OperationSTA );
		break;

	// STX Store index X in memory
	case OP_STX_ZEROPAGE:
	case OP_STX_ZEROPAGEY:
	case OP_STX_ABSOLUTE:
		WRITE_OPERATION( OperationSTX );
		break;
	
	// STY Store index Y in memory
	case OP_STY_ZEROPAGE:
	case OP_STY_ZEROPAGEX:
	case OP_STY_ABSOLUTE:
		WRITE_OPERATION( OperationSTY );
		break;
	
	// TAX Transfer accumulator to index X
	case OP_TAX:
		IncrementSubCycle( true );
		OperationTAX();
		break;
	
	// TAY Transfer accumulator to index Y
	case OP_TAY:
		IncrementSubCycle( true );
		OperationTAY();
		break;
	
	// TSX Transfer stack pointer to index X
	case OP_TSX:
		IncrementSubCycle( true );
		OperationTSX();
		break;
	
	// TXA Transfer index X to accumulator
	case OP_TXA:
		IncrementSubCycle( true );
		OperationTXA();
		break;
	
	// TXS Transfer index X to stack pointer
	case OP_TXS:
		IncrementSubCycle( true );
		OperationTXS();
		break;
	
	// TYA Transfer index Y to accumulator
	case OP_TYA:
		IncrementSubCycle( true );
		OperationTYA();
		break;

		////////////////////////////////////////////
		// UNOFFICIAL OPCODES

	// This opcode ASLs the contents of a memory location and then ORs the result 
	// with the accumulator.  
	case OP_ASO_ABSOLUTE:
	case OP_ASO_ZEROPAGE:
	case OP_ASO_ZEROPAGEX:
	case OP_ASO_INDIRECTX:
	case OP_ASO_INDIRECTY:
	case OP_ASO_ABSOLUTEX:
	case OP_ASO_ABSOLUTEY:
		READ_MODIFY_WRITE_OPERATION( OperationASO );
		break;

	// RLA ROLs the contents of a memory location and then ANDs the result with 
	// the accumulator.
	case OP_RLA_ABSOLUTE:
	case OP_RLA_ABSOLUTEX:
	case OP_RLA_ABSOLUTEY:
	case OP_RLA_ZEROPAGE:
	case OP_RLA_ZEROPAGEX:
	case OP_RLA_INDIRECTX:
	case OP_RLA_INDIRECTY:
		READ_MODIFY_WRITE_OPERATION( OperationRLA );
		break;

	// LSE LSRs the contents of a memory location and then EORs the result with 
	// the accumulator.
	case OP_LSE_ABSOLUTE:
	case OP_LSE_ABSOLUTEX:
	case OP_LSE_ABSOLUTEY:
	case OP_LSE_ZEROPAGE:
	case OP_LSE_ZEROPAGEX:
	case OP_LSE_INDIRECTX:
	case OP_LSE_INDIRECTY:
		READ_MODIFY_WRITE_OPERATION( OperationLSE );
		break;

	// RRA RORs the contents of a memory location and then ADCs the result with 
	// the accumulator.
	case OP_RRA_ABSOLUTE:
	case OP_RRA_ABSOLUTEX:
	case OP_RRA_ABSOLUTEY:
	case OP_RRA_ZEROPAGE:
	case OP_RRA_ZEROPAGEX:
	case OP_RRA_INDIRECTX:
	case OP_RRA_INDIRECTY:
		READ_MODIFY_WRITE_OPERATION( OperationRRA );
		break;

	// AXS ANDs the contents of the A and X registers (without changing the 
	// contents of either register) and stores the result in memory.
	// AXS does not affect any flags in the processor status register.
	case OP_AXS_ABSOLUTE:
	case OP_AXS_ZEROPAGE:
	case OP_AXS_ZEROPAGEY:
	case OP_AXS_INDIRECTX:
		WRITE_OPERATION( OperationAXS );
		break;

	// This opcode loads both the accumulator and the X register with the contents 
	// of a memory location.
	case OP_LAX_ABSOLUTE:
	case OP_LAX_ABSOLUTEY:
	case OP_LAX_ZEROPAGE:
	case OP_LAX_ZEROPAGEY:
	case OP_LAX_INDIRECTX:
	case OP_LAX_INDIRECTY:
		READ_OPERATION( OperationLAX );
		break;

	// This opcode DECs the contents of a memory location and then CMPs the result 
	// with the A register.
	case OP_DCM_ABSOLUTE:
	case OP_DCM_ABSOLUTEX:
	case OP_DCM_ABSOLUTEY:
	case OP_DCM_ZEROPAGE:
	case OP_DCM_ZEROPAGEX:
	case OP_DCM_INDIRECTX:
	case OP_DCM_INDIRECTY:
		READ_MODIFY_WRITE_OPERATION( OperationDCM );
		break;

	// This opcode INCs the contents of a memory location and then SBCs the result 
	// from the A register.
	case OP_INS_ABSOLUTE:
	case OP_INS_ABSOLUTEX:
	case OP_INS_ABSOLUTEY:
	case OP_INS_ZEROPAGE:
	case OP_INS_ZEROPAGEX:
	case OP_INS_INDIRECTX:
	case OP_INS_INDIRECTY:
		READ_MODIFY_WRITE_OPERATION( OperationINS );
		break;

	// This opcode ANDs the contents of the A register with an immediate value and 
	// then LSRs the result.
	case OP_ALR_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationALR );
		break;

	// This opcode ANDs the contents of the A register with an immediate value and 
	// then RORs the result.
	// Similar to AND #i then ROR A, except sets the flags differently. N and Z are normal, but C is bit 6 and V is bit 6 xor bit 5. 
	case OP_ARR_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationARR );
		break;

	// XAA transfers the contents of the X register to the A register and then 
	// ANDs the A register with an immediate value.
	case OP_XAA_IMMEDIATE:
		OperationTXA();
		OperationAND( value );
		break;

	// This opcode ORs the A register with #$EE, ANDs the result with an immediate 
	// value, and then stores the result in both A and X.
	case OP_OAL_IMMEDIATE:
/* Opcode $AB is known to work differently depending on the machine. Possible outcomes I've seen reported:

1. OR with $EE always
2. OR with $EE, $EF, $FE, or $FF based on the contents of other registers
3. OR with a seemingly random value (possibly influenced by DMA transfers)
4. Perform no OR operation at all (go straight to the AND step)

I think this opcode is triggering a bus conflict, causing the bits in A to get forced high in some cases (machine-dependent).
Unless people test this opcode on multiple NES's to see if it works the same on all of them or not,
we should assume the exact behavior to be unpredictable.

Exactly what is causing errors with $9C and $9E? Specifically, do problems show up during page boundary crossing,
when there's no page crossing, or both? I suspect that since the store value
is affected by the upper byte of the target address (the "fixed" version, with 1 added to it),
it's possible that a page crossing would also cause an issue with the upper address lines during the store cycle.
I haven't seen any docs that give light to this possibility, but that may be because no one tested it.

		*/
//		OperationORA( 0xEE );
		OperationORA( 0xFF );
		OperationAND( value );
		OperationTAX();
		break;

	// SAX ANDs the contents of the A and X registers (leaving the contents of A 
	// intact), subtracts an immediate value, and then stores the result in X.
	// ... A few points might be made about the action of subtracting an immediate 
	// value.  It actually works just like the CMP instruction, except that CMP 
	// does not store the result of the subtraction it performs in any register.  
	// This subtract operation is not affected by the state of the Carry flag, 
	// though it does affect the Carry flag.  It does not affect the Overflow flag.
	case OP_SAX_IMMEDIATE:
		IMMEDIATE_OPERATION( OperationSAX );
		break;

	case OP_NOP_1:
	case OP_NOP_2:
	case OP_NOP_3:
	case OP_NOP_4:
	case OP_NOP_5:
	case OP_NOP_6:
		IncrementSubCycle( true );
		break;

	// SKB stands for skip next byte.
	case OP_SKB_1:
	case OP_SKB_2:
	case OP_SKB_3:
	case OP_SKB_4:
	case OP_SKB_E: // Under "Opcode 89" in the docs - It requires 2 cycles to execute.
		break;

	case OP_SKB_5:
	case OP_SKB_6:
	case OP_SKB_7:
	case OP_SKB_8:
	case OP_SKB_9:
	case OP_SKB_A:
	case OP_SKB_B:
	case OP_SKB_C:
	case OP_SKB_D:
		IncrementSubCycle( true );
		break;

	// SKW skips next word (two bytes).
	case OP_SKW_1:
	case OP_SKW_2:
	case OP_SKW_3:
	case OP_SKW_4:
	case OP_SKW_5:
	case OP_SKW_6:
	case OP_SKW_7:
		IncrementSubCycle( true );
		break;

	// HLT crashes the microprocessor.  When this opcode is executed, program 
	// execution ceases.  No hardware interrupts will execute either.  The author 
	// has characterized this instruction as a halt instruction since this is the 
	// most straightforward explanation for this opcode's behaviour.  Only a reset 
	// will restart execution.  This opcode leaves no trace of any operation 
	// performed!  No registers affected.
	case OP_HLT_1:
	case OP_HLT_2:
	case OP_HLT_3:
	case OP_HLT_4:
	case OP_HLT_5:
	case OP_HLT_6:
	case OP_HLT_7:
	case OP_HLT_8:
	case OP_HLT_9:
	case OP_HLT_A:
	case OP_HLT_B:
	case OP_HLT_C:
		OperationHLT();
		break;

	// This opcode ANDs the contents of the A and X registers (without changing 
	// the contents of either register) and transfers the result to the stack 
	// pointer.  It then ANDs that result with the contents of the high byte of 
	// the target address of the operand +1 and stores that final result in memory.  
	case OP_TAS_ABSOLUTEY:
		WRITE_OPERATION( OperationTAS );
		break;

	// This opcode ANDs the contents of the Y register with  and stores the 
	// result in memory.
	case OP_SAY_ABSOLUTEX:
		READ_MODIFY_WRITE_OPERATION( OperationSAY );
		break;

	// This opcode ANDs the contents of the X register with  and stores the 
	// result in memory.
	case OP_XAS_ABSOLUTEY:
		WRITE_OPERATION( OperationXAS );
		break;

	// This opcode stores the result of A AND X AND the high byte of the target 
	// address of the operand +1 in memory.
	case OP_AXA_ABSOLUTEY:
	case OP_AXA_INDIRECTY:
		WRITE_OPERATION( OperationAXA );
		break;

	// ANC ANDs the contents of the A register with an immediate value and then 
	// moves bit 7 of A into the Carry flag.  This opcode works basically 
	// identically to AND #immed. except that the Carry flag is set to the same 
	// state that the Negative flag is set to.
	case OP_ANC_IMMEDIATE_1:
	case OP_ANC_IMMEDIATE_2:
		IMMEDIATE_OPERATION( OperationANC );
		break;

	// This opcode ANDs the contents of a memory location with the contents of the 
	// stack pointer register and stores the result in the accumulator, the X 
	// register, and the stack pointer.  Affected flags: N Z.
	case OP_LAS_ABSOLUTEY:
		READ_OPERATION( OperationLAS );
		break;

	// Opcode EB - Opcode EB seems to work exactly like SBC #immediate.  Takes 2 cycles.
	case OP_SBC_IMMEDIATE_1:
		IMMEDIATE_OPERATION( OperationSBC );
		break;

		///////////////////////////////////////

	default:
		// TODO: Do something about unknown instructions
		break;
	}

	int verifyTicks = instruction.mBaseCycles;
	if ( mPageBoundaryCrossed )
		verifyTicks += instruction.mPageBoundaryCycles;
	if ( mBranchTaken )
		verifyTicks++;

	const bool cyclesMatch = ( subCycle + 1 ) == verifyTicks;
	if ( !cyclesMatch )
	{
		PWLOG6( LOG_ERROR, L"Op '%1%:%6$02X' Addressing mode '%2%' Correct cycles '%3%' Subcycles '%4%' Add for boundary '%5%'",
			instruction.mName, (int)instruction.mAddressingMode, verifyTicks, (int)subCycle, mPageBoundaryCrossed ? "1" : "0", (int)opcode );
		throw std::runtime_error( "" );
	}

	if ( !isLastCycle )
	{
		PWLOG5( LOG_ERROR, L"Not last cycle: Op '%1%' Addressing mode '%2%' Correct cycles '%3%' Subcycles '%4%' Add for boundary '%5%'",
			instruction.mName, (int)instruction.mAddressingMode, verifyTicks, (int)subCycle, mPageBoundaryCrossed ? "1" : "0" );
		throw std::runtime_error( "" );
	}

	const int totalTicks = subCycle + 1;
	SetSubCycle( 0, false );
	return totalTicks;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////



void Processor6502::LogProcessorMessage( UInt16_t address, Byte_t opcode, Byte_t arg1, Byte_t arg2 )
{
	if ( !Log::IsTypeEnabled( LOG_CPU ) )
		return;

	//const int addressesToStore = 10; // number of previous addresses to store

	//AddressList_t::iterator it;
	//if ( ( it = std::find( addressList.begin(), addressList.end(), address ) ) != addressList.end() )
	//{ // if address is already contained within address list, it is possible we may have a loop / jump
	//	if ( addressLoopFrom == -1 && addressLoopTo == -1 )
	//	{ // new loop
	//		addressLoopTo = address;
	//		addressLoopFrom = addressList[ addressList.size()-1 ];
	//		loopCount = 1;
	//	}
	//	else if ( address == addressLoopTo && addressList[ addressList.size()-1 ] == addressLoopFrom )
	//	{ // continuing old loop
	//		loopCount++;
	//	}
	//}
	//else
	//{	
	//	// check if we broke out of a previous loop
	//	if ( addressLoopFrom >= 0 && addressLoopFrom < address )
	//	{
	//		std::stringstream ss;
	//		ss << "\nLOOP: " << std::hex << addressLoopFrom << " -> " << std::hex << addressLoopTo << " x " << std::dec << loopCount << "\n";
	//		Log::Write( LOG_CPU, ss.str() );

	//		addressLoopFrom = addressLoopTo = -1;
	//		loopCount = 0;
	//	}

	//	// add address to list
	//	if ( addressesToStore == addressList.size() )
	//		addressList.erase( addressList.begin() );
	//	addressList.push_back( address );

		Log::Write( LOG_CPU, FormatLogString( programCounter, opcode, arg1, arg2, 0 ).c_str() );
	//}

	// if logging enabled, breathe after each instruction - else goes too fast
	//g_nesMainboard->SleepTime( 2 );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::wstring PadWidthZeroes( int num, int width, bool hex, wchar_t c = '0' )
{
	int size = 0;
	for ( int i=1; i< 10; ++i )
	{
		if ( num < (int)pow((double)( hex ? 16 : 10 ), (double)i) )
		{
			size = i;
			break;
		}
	}

	int numzeros = ( size > width ) ? 0 : width - size;
	int strSize = size + numzeros + 1;
		
	wchar_t* str = new wchar_t[ strSize ];

	if ( hex )
		swprintf_s( str, strSize, L"%X", num );
	else
		swprintf_s( str, strSize, L"%i", num );

	std::wstring s = L"";

	for ( int i=0; i<numzeros; ++i )
	{
		s += c;
	}

	s += str;

	delete[] str;

	return s;
}


std::wstring Processor6502::FormatLogString( int programCounter, Byte_t opcode, Byte_t arg1, Byte_t arg2, int memval ) const
{
	std::wstringstream strstream;
	strstream << PadWidthZeroes( programCounter, 4, true ) << L"  ";
	strstream << PadWidthZeroes( (int)opcode, 2, true ) << L" ";

	const Instruction6502& instruction = GetInstruction6502( opcode );

	if ( instruction.mSize > 1 )
		strstream << PadWidthZeroes( (int)arg1, 2, true );
	else
	  strstream << "  ";

	strstream << " ";

	if ( instruction.mSize > 2 )
	{
		strstream << PadWidthZeroes( (int)arg2, 2, true );
	}
	else
	  strstream << "  ";

	strstream << "  ";

	strstream << (instruction.mIsLegal ? "" : "*") << instruction.mName << " ";
	switch ( instruction.mAddressingMode )
	{
	case ADDRESS_NONE:
			break;
		case ADDRESS_RELATIVE:
			strstream << "$" << PadWidthZeroes( (int)(programCounter + CalculateRelativeDifference( arg1 ) + instruction.mSize ), 4, true );
			break;
		case ADDRESS_ACCUMULATOR:
			break;
		case ADDRESS_IMMEDIATE:
			strstream << "#$" << PadWidthZeroes( (int)arg1, 2, true );
			break;
		case ADDRESS_IMMEDIATE16:
			strstream << "$" << PadWidthZeroes( (int)MAKE_WORD( arg1, arg2 ), 4, true );
			break;
		case ADDRESS_ZEROPAGE:
			strstream << "$" << PadWidthZeroes( (int)arg1, 2, true );
			strstream << " = " << PadWidthZeroes( memval, 2, true );
			break;
		case ADDRESS_ZEROPAGEX:
			strstream << "$"  << PadWidthZeroes( (int)arg1, 2, true ) << ", X";
			strstream << " = " << PadWidthZeroes( memval, 2, true );
			break;
		case ADDRESS_ZEROPAGEY:
			strstream << "$" << std::hex << PadWidthZeroes( (int)arg1, 2, true ) << ", Y";
			strstream << " = " << PadWidthZeroes( memval, 2, true );
			break;
		case ADDRESS_ABSOLUTE:
			strstream << "$" << std::hex << PadWidthZeroes( (int)MAKE_WORD( arg1, arg2 ), 4, true );
			strstream << " = " << PadWidthZeroes( memval, 2, true );
			break;
		case ADDRESS_ABSOLUTEX:
			strstream << "$" << std::hex << PadWidthZeroes( (int)MAKE_WORD( arg1, arg2 ), 4, true ) << ", X";
			strstream << " = " << PadWidthZeroes( memval, 2, true );
			break;
		case ADDRESS_ABSOLUTEY:
		   strstream << "$" << std::hex << PadWidthZeroes( (int)MAKE_WORD( arg1, arg2 ), 4, true ) << ", Y";
		   strstream << " = " << PadWidthZeroes( memval, 2, true );
			break;
		case ADDRESS_INDIRECT:
			strstream << "($" << std::hex << PadWidthZeroes( (int)MAKE_WORD( arg1, arg2 ), 4, true ) << ")";
			strstream << " = " << PadWidthZeroes( memval, 2, true );
			break;
		case ADDRESS_INDIRECTX:
			strstream << "($" << std::hex << PadWidthZeroes( (int)arg1, 2, true ) << ", X)";
			strstream << " = " << PadWidthZeroes( memval, 2, true );
			break;
		case ADDRESS_INDIRECTY:
			strstream << "($" << std::hex << PadWidthZeroes( (int)arg1, 2, true ) << "), Y";
			strstream << " = " << PadWidthZeroes( memval, 2, true );
			break;
	}

	int size = strstream.tellp();

	for ( int i=0; i<( 47 - size ); ++i )
	  strstream << " ";

	int x, y;
	g_nesMainboard->GetPPU()->TicksToScreenCoordinates( g_nesMainboard->GetSynchroniser()->GetCpuMTC(), &x, &y );

	strstream << "A:" << PadWidthZeroes( (int)accumulatorRegister, 2, true ) 
		<< " X:" << PadWidthZeroes( (int)xRegister, 2, true ) 
		<< " Y:" << PadWidthZeroes( (int)yRegister, 2, true )
		<< " P:" << PadWidthZeroes( (int)mStatusRegister.Byte, 2, true )
		<< " SP:" << PadWidthZeroes( (int)stackPointer, 2, true )
		<< " CYC:" << PadWidthZeroes( x, 3, false, ' ' )
		<< " SL:" << std::dec << y
		<< " F:" << std::dec << g_nesMainboard->GetPPU()->GetFrameCount()
		<< " S:" << PadWidthZeroes( g_nesMainboard->GetPPU()->GetS(), 2, true )
		<< " C1:" << PadWidthZeroes( g_nesMainboard->GetPPU()->GetC1(), 2, true )
		<< " C2:" << PadWidthZeroes( g_nesMainboard->GetPPU()->GetC2(), 2, true );

	return strstream.str();
}



void Processor6502::OutputAllInstructions()
{
	for ( size_t i=0; i<256; ++i )
	{
		OPCODE_6502 opcode = (OPCODE_6502)i;
		const Instruction6502& instruction = GetInstruction6502( opcode );

		const std::wstring logMsg = ( boost::wformat( L"{ 0x%1$02X,\t\"%2%\",\t%3%,\t%4%,\t%5%,\t%6%,\t%7%\t}," ) % (int)opcode %
			instruction.mName % (int)instruction.mSize %
			(int)instruction.mAddressingMode % (int)instruction.mBaseCycles %
			(int)instruction.mPageBoundaryCycles % ( instruction.mIsLegal ? "true" : "false" ) ).str();
		PWLOG0( LOG_MISC, logMsg );
	}
}

