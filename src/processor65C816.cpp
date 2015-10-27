

#include "stdafx.h"
#include "processor65C816.h"
#include "opcodes65C816.h"
#include "gui/app.h"
#include "SnesMainboard.h"

/*
     Native Mode           6502 Emulation Mode
     -----------------------------------------
     IRQ   $FFEE-$FFEF     IRQ/BRK $FFFE-$FFFF
                           RESET   $FFFC-$FFFD
     NMI   $FFEA-$FFEB     NMI     $FFFA-$FFFB
     ABORT $FFE8-$FFE9     ABORT   $FFF8-$FFF9
     BRK   $FFE6-$FFE7
     COP   $FFE5-$FFE6     COP     $FFF4-$FFF5

Notice that there is a separate BRK vector for Native mode, and no need to
poll bit 5 for the brk flag. However when running in emulation mode,
remember that bit 5 is still the BRK flag, and your IRQ will still need to
check for the source of the IRQ.
*/

static const UInt32_t RESET_ADDRESS_65C816 = 0x0000FFFC;
static const UInt32_t NMI_ADDRESS_65C816 = 0x0000FFEA;
static const UInt32_t NMI_ADDRESS_65C816_EMU = 0x0000FFFA;
static const UInt32_t IRQ_ADDRESS_65C816 = 0x0000FFEE;
static const UInt32_t IRQ_ADDRESS_65C816_EMU = 0x0000FFFE;
static const UInt32_t COP_ADDRESS_65C816 = 0x0000FFE5;
static const UInt32_t COP_ADDRESS_65C816_EMU = 0x0000FFF4;
static const UInt32_t BRK_ADDRESS_65C816 = 0x0000FFE6;
static const UInt32_t ABORT_ADDRESS_65C816 = 0x0000FFE8;
static const UInt32_t ABORT_ADDRESS_65C816_EMU = 0x0000FFF8;

static const Byte_t InstructionSize65C816[ 256 ] = {
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	3, 2, 4, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	1, 2, 2, 2, 3, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4,
	1, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	2, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4,
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4,
	2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4
 };

static const char* InstructionName65C816[ 256 ] = { 
	"BRK","ORA","COP","ORA","TSB","ORA","ASL","ORA","PHP","ORA","ASL","PHD","TSB","ORA","ASL","ORA",
	"BPL","ORA","ORA","ORA","TRB","ORA","ASL","ORA","CLC","ORA","INC","TCS","TRB","ORA","ASL","ORA",
	"JSR","AND","JSL","AND","BIT","AND","ROL","AND","PLP","AND","ROL","PLD","BIT","AND","ROL","AND",
	"BMI","AND","AND","AND","BIT","AND","SEC","AND","SEC","AND","DEC","TSC","BIT","AND","ROL","AND",
	"RTI","EOR","UNK","EOR","MVP","EOR","LSR","EOR","PHA","EOR","LSR","PHK","JMP","EOR","LSR","EOR",
	"BVC","EOR","EOR","EOR","MVN","EOR","LSR","EOR","CLI","EOR","PHY","TCD","JMP","EOR","LSR","EOR",
	"RTS","ADC","PER","ADC","STZ","ADC","ROR","ADC","PLA","ADC","ROR","RTL","JMP","ADC","ROR","ADC",
	"BVS","ADC","ADC","ADC","STZ","ADC","ROR","ADC","SEI","ADC","PLY","TDC","JMP","ADC","ROR","ADC",
	"BRA","STA","BRL","STA","STY","STA","STX","STA","DEY","BIT","TXA","PHB","STY","STA","STX","STA",
	"BCC","STA","STA","STA","STY","STA","STX","STA","TYA","STA","TXS","TXY","STZ","STA","STZ","STA",
	"LDY","LDA","LDX","LDA","LDY","LDA","LDX","LDA","TAY","LDA","TAX","PLB","LDY","LDA","LDX","LDA",
	"BCS","LDA","LDA","LDA","LDY","LDA","LDX","LDA","CLV","LDA","TSX","TYX","LDY","LDA","LDX","LDA",
	"CPY","CMP","REP","CMP","CPY","CMP","DEC","CMP","INY","CMP","DEX","WAI","CPY","CMP","DEC","CMP",
	"BNE","CMP","CMP","CMP","PEI","CMP","DEC","CMP","CLD","CMP","PHX","STP","JML","CMP","DEC","CMP",
	"CPX","SBC","SEP","SBC","CPX","SBC","INC","SBC","INX","SBC","NOP","XBA","CPX","SBC","INC","SBC",
	"BEQ","SBC","SBC","SBC","PEA","SBC","INC","SBC","SED","SBC","PLX","XCE","JSR","SBC","INC","SBC"
};

static const Byte_t InstructionCycles65C816_16bit[ 256 ] = { 
	7, 6, 7, 4, 5, 3, 5, 6, 3, 2, 2, 4, 6, 4, 6, 5,
	2, 5, 5, 7, 5, 4, 6, 6, 2, 4, 2, 2, 6, 4, 7, 5,
	6, 6, 8, 4, 3, 3, 5, 6, 4, 2, 2, 5, 4, 4, 6, 5,
	2, 5, 5, 7, 4, 4, 6, 6, 2, 4, 2, 2, 4, 4, 7, 5,
	6, 6, 0, 4, 0, 3, 5, 6, 3, 2, 2, 3, 3, 4, 6, 5,
	2, 5, 5, 7, 0, 4, 6, 6, 2, 4, 3, 2, 4, 4, 7, 5,
	6, 6, 6, 4, 3, 3, 5, 6, 4, 2, 2, 6, 5, 4, 6, 5,
	2, 5, 5, 7, 4, 4, 6, 6, 2, 4, 4, 2, 6, 4, 7, 5,
	3, 6, 4, 4, 3, 3, 3, 6, 2, 2, 2, 3, 4, 4, 4, 5,
	2, 6, 5, 7, 4, 4, 4, 6, 2, 5, 2, 2, 4, 5, 5, 5,
	2, 6, 2, 4, 3, 3, 3, 6, 2, 2, 2, 4, 4, 4, 4, 5,
	2, 5, 5, 7, 4, 4, 4, 6, 2, 4, 2, 2, 4, 4, 4, 5,
	2, 6, 3, 4, 3, 3, 5, 6, 2, 2, 2, 3, 4, 4, 6, 5,
	2, 5, 5, 7, 6, 4, 6, 6, 2, 4, 3, 3, 6, 4, 7, 5,
	2, 6, 3, 4, 3, 3, 5, 6, 2, 2, 2, 3, 4, 4, 6, 5,
	2, 5, 5, 7, 5, 4, 6, 6, 2, 4, 4, 2, 8, 4, 7, 5,
 };

static const Byte_t InstructionAddressingModes65C816[256] = { 
	AMS_STACK, AMS_INDIRECTX, AMS_STACK, AMS_STACKRELATIVE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE_INDIRECT_LONG,
	AMS_STACK, AMS_IMMEDIATE_EMU, AMS_ACCUMULATOR, AMS_STACK, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTELONG,

	AMS_RELATIVE, AMS_INDIRECTY, AMS_ZEROPAGE_INDIRECT, AMS_STACKRELATIVEINDEXED, AMS_ZEROPAGE, AMS_ZEROPAGEX, AMS_ZEROPAGEX, AMS_ZEROPAGE_INDIRECT_LONGY,
	AMS_IMPLIED, AMS_ABSOLUTEY, AMS_ACCUMULATOR, AMS_IMPLIED, AMS_ABSOLUTE, AMS_ABSOLUTEX, AMS_ABSOLUTEX, AMS_ABSOLUTELONGX,

	AMS_ABSOLUTE, AMS_INDIRECTX, AMS_ABSOLUTELONG, AMS_STACKRELATIVE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE_INDIRECT_LONG,
	AMS_STACK, AMS_IMMEDIATE_EMU, AMS_ACCUMULATOR, AMS_STACK, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTELONG,

	AMS_RELATIVE, AMS_INDIRECTY, AMS_ZEROPAGE_INDIRECT, AMS_STACKRELATIVEINDEXED, AMS_ZEROPAGEX, AMS_ZEROPAGEX, AMS_ZEROPAGEX, AMS_ZEROPAGE_INDIRECT_LONGY,
	AMS_IMPLIED, AMS_ABSOLUTEY, AMS_ACCUMULATOR, AMS_IMPLIED, AMS_ABSOLUTEX, AMS_ABSOLUTEX, AMS_ABSOLUTEX, AMS_ABSOLUTELONGX,

	AMS_STACK, AMS_INDIRECTX, AMS_IMPLIED, AMS_STACKRELATIVE, AMS_BLOCKMOVE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE_INDIRECT_LONG,
	AMS_STACK, AMS_IMMEDIATE_EMU, AMS_ACCUMULATOR, AMS_STACK, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTELONG,

	AMS_RELATIVE, AMS_INDIRECTY, AMS_ZEROPAGE_INDIRECT, AMS_STACKRELATIVEINDEXED, AMS_BLOCKMOVE, AMS_ZEROPAGEX, AMS_ZEROPAGEX, AMS_ZEROPAGE_INDIRECT_LONGY,
	AMS_IMPLIED, AMS_ABSOLUTEY, AMS_STACK, AMS_IMPLIED, AMS_ABSOLUTELONG, AMS_ABSOLUTEX, AMS_ABSOLUTEX, AMS_ABSOLUTELONGX,

	AMS_STACK, AMS_INDIRECTX, AMS_STACK, AMS_STACKRELATIVE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE_INDIRECT_LONG, 
	AMS_STACK, AMS_IMMEDIATE_EMU, AMS_ACCUMULATOR, AMS_STACK, AMS_ABSOLUTE_INDIRECT, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTELONG,

	AMS_RELATIVE, AMS_INDIRECTY, AMS_ZEROPAGE_INDIRECT, AMS_STACKRELATIVEINDEXED, AMS_ZEROPAGEX, AMS_ZEROPAGEX, AMS_ZEROPAGEX, AMS_ZEROPAGE_INDIRECT_LONGY,
	AMS_IMPLIED, AMS_ABSOLUTEY, AMS_STACK, AMS_IMPLIED, AMS_ABSOLUTE_INDIRECTX, AMS_ABSOLUTEX, AMS_ABSOLUTEX, AMS_ABSOLUTELONGX,

	AMS_RELATIVE, AMS_INDIRECTX, AMS_RELATIVELONG, AMS_STACKRELATIVE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE_INDIRECT_LONG,
	AMS_IMPLIED, AMS_IMMEDIATE_EMU, AMS_IMPLIED, AMS_STACK, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTELONG,

	AMS_RELATIVE, AMS_INDIRECTY, AMS_ZEROPAGE_INDIRECT, AMS_STACKRELATIVEINDEXED, AMS_ZEROPAGEX, AMS_ZEROPAGEX, AMS_ZEROPAGEY, AMS_ZEROPAGE_INDIRECT_LONGY,
	AMS_IMPLIED, AMS_ABSOLUTEY, AMS_IMPLIED, AMS_IMPLIED, AMS_ABSOLUTE, AMS_ABSOLUTEX, AMS_ABSOLUTEX, AMS_ABSOLUTELONGX,

	AMS_IMMEDIATE_INDEX, AMS_INDIRECTX, AMS_IMMEDIATE_INDEX, AMS_STACKRELATIVE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE_INDIRECT_LONG,
	AMS_IMPLIED, AMS_IMMEDIATE_EMU, AMS_IMPLIED, AMS_STACK, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTELONG,

	AMS_RELATIVE, AMS_INDIRECTY, AMS_ZEROPAGE_INDIRECT, AMS_STACKRELATIVEINDEXED, AMS_ZEROPAGEX, AMS_ZEROPAGEX, AMS_ZEROPAGEY, AMS_ZEROPAGE_INDIRECT_LONGY,
	AMS_IMPLIED, AMS_ABSOLUTEY, AMS_IMPLIED, AMS_IMPLIED, AMS_ABSOLUTEX, AMS_ABSOLUTEX, AMS_ABSOLUTEY, AMS_ABSOLUTELONGX,

	AMS_IMMEDIATE_INDEX, AMS_INDIRECTX, AMS_IMMEDIATE8, AMS_STACKRELATIVE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE_INDIRECT_LONG,
	AMS_IMPLIED, AMS_IMMEDIATE_EMU, AMS_IMPLIED, AMS_IMPLIED, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTELONG,

	AMS_RELATIVE, AMS_INDIRECTY, AMS_ZEROPAGE_INDIRECT, AMS_STACKRELATIVEINDEXED, AMS_STACK_DIRECTPAGEINDIRECT, AMS_ZEROPAGEX, AMS_ZEROPAGEX, AMS_ZEROPAGE_INDIRECT_LONGY,
	AMS_IMPLIED, AMS_ABSOLUTEY, AMS_STACK, AMS_IMPLIED, AMS_ABSOLUTE_INDIRECT_DIRECT, AMS_ABSOLUTEX, AMS_ABSOLUTEX, AMS_ABSOLUTELONGX,

	AMS_IMMEDIATE_INDEX, AMS_INDIRECTX, AMS_IMMEDIATE8, AMS_STACKRELATIVE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE, AMS_ZEROPAGE_INDIRECT_LONG,
	AMS_IMPLIED, AMS_IMMEDIATE_EMU, AMS_IMPLIED, AMS_IMPLIED, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTE, AMS_ABSOLUTELONG,

	AMS_RELATIVE, AMS_INDIRECTY, AMS_ZEROPAGE_INDIRECT, AMS_STACKRELATIVEINDEXED, AMS_IMMEDIATE16, AMS_ZEROPAGEX, AMS_ZEROPAGEX, AMS_ZEROPAGE_INDIRECT_LONGY,
	AMS_IMPLIED, AMS_ABSOLUTEY, AMS_STACK, AMS_IMPLIED, AMS_ABSOLUTE_INDIRECTX, AMS_ABSOLUTEX, AMS_ABSOLUTEX, AMS_ABSOLUTELONGX,
 };


static const char* AddressingModeNames65C816[] = {
	"IMPLIED",
	"IMMEDIATE8",
	"IMMEDIATE16",
	"IMMEDIATE_EMU",
	"IMMEDIATE_INDEX",
	"ACCUMULATOR",
	"ZEROPAGE",
	"ZEROPAGEX",
	"ZEROPAGEY",
	"INDIRECTX",
	"INDIRECTY",
	"ZEROPAGE_INDIRECT",
	"ZEROPAGE_INDIRECT_LONG",
	"ZEROPAGE_INDIRECT_LONGY",
	"ABSOLUTE",
	"ABSOLUTELONG",
	"ABSOLUTELONGX",
	"ABSOLUTEX",
	"ABSOLUTEY",
	"ABSOLUTE_INDIRECT",
	"ABSOLUTE_INDIRECT_DIRECT",
	"ABSOLUTE_INDIRECTX",
	"STACK",
	"STACKRELATIVE",
	"STACKRELATIVEINDEXED",
	"STACK_DIRECTPAGEINDIRECT",
	"RELATIVE",
	"RELATIVELONG",
	"BLOCKMOVE",
};



FORCE_INLINE UInt16_t Wrap8( UInt16_t t )
{
	return t % 0x100;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Processor65C816::Processor65C816()
{
	programCounter = 0;
	directPagePointer = 0;
	mDataBankRegister = 0;
	mProgramBankRegister = 0;
	mStackPointer = 0;
	mEmulationMode = true;
	mWaitingForInterrupt = false;
	mIrqPending = false;
	mStoppedUntilReset = false;
	mSubCycleMasterTicks = 0;

	mResetPending = false;
	mIrqLineLow = false;
	mNmiPending = false;

	mValidProgramAddressLine = false;
	mValidDataAddressLine = false;

	mStatusRegisterPtr = reinterpret_cast<Byte_t*>( &statusRegister );

	SetDefaultVariables();
}


void Processor65C816::ClearSubCycle()
{
	mValidDataAddressLine = mValidProgramAddressLine = false;
}


void Processor65C816::ProcessSubCycle( bool vda, bool vpa, int cpuCycleCount )
{
	mValidDataAddressLine = vda;
	mValidProgramAddressLine = vpa;
	mSubCycleMasterTicks += cpuCycleCount; // TODO: calculate proper master tick count
}


int Processor65C816::MasterTicksToAccessMemory( UInt32_t memoryAddress ) const
{
	//The SNES's CPU is a 65c816 based processor. While its clock speed is at about 21 MHz, it's effective speed is considerably lower, at 3.58 MHz for quick access
	//	(i.e. hardware registers at $2100-$21FF in banks $00-$3F), 2.68 MHz for slow access (i.e. ROM and RAM) and 1.79 MHz for very slow access
	//	(i.e. hardware registers at $4000-$41FFF in banks $00 through $3F). 
	const UInt8_t bank = ( memoryAddress & 0xFF0000 ) >> 16;
	if ( IS_INT_BETWEEN( bank, 0x00, 0x40 ) )
	{
		return 0;
	}
	else
	{
		return SNES_MASTER_TICKS_PER_MEM_ACCESS_SLOW;
	}
}


int Processor65C816::HandlePendingInterrupts( SnesCpuMemory* cpuMemory )
{
	if ( mResetPending )
	{
//
//	   D  = 0000		SH = 01
//	   DB = 00		XH = 00
//	   PB = 00		YH = 00
//
//		    N V M X D I Z C/E
//	   P =  * * 1 1 0 1 * */1
//
//		* = Not Initialized
//		STP and WAI instructions are cleared.
//
//
//	              Signals
//
//	   E    = 1		VDA = 0
//	   M/X  = 1		/VP = 1
//	   R/W  = 1		VPA = 0
//	   SYNC = 0
//
//
//When Reset is brought high, an interrupt sequence is initiated:
//* R/W remains in the high stale during the stack address cycles.
//* The Reset vector address is 00FFFC,D.
		
		if ( !mEmulationMode )
			PushStack8( cpuMemory, mProgramBankRegister );
		PushStack16( cpuMemory, programCounter );
		PushStack8( cpuMemory, *mStatusRegisterPtr );

		mEmulationMode = true;
		statusRegister.Decimal = false;
		statusRegister.Interrupt = true;
		statusRegister.IndexRegister8bit = true;
		statusRegister.Accumulator8bit = true;

		mProgramBankRegister = 0;
		programCounter = cpuMemory->Read16( RESET_ADDRESS_65C816 );
		if ( programCounter == 0 )
			programCounter = 0x8000;
		
#ifdef LOG_PROCESSOR_INSTRUCTIONS
		if ( Log::IsTypeEnabled( LOG_CPU ) )
			Log::Write( LOG_CPU, ( boost::format( "[%1$02X%2$04X] RESET triggered" ) % (int)mProgramBankRegister % (int)programCounter ).str().c_str() );
#endif

		mResetPending = false;
		mWaitingForInterrupt = false;
		mStoppedUntilReset = false;

		return 0;
	}

	if ( mNmiPending )
	{
		//if ( triggerNmiAfterNextInstruction )
		//{
		//	triggerNmiAfterNextInstruction = false;
		//	return 0;
		//}

		// NMI interrupt
		if ( !mEmulationMode )
			PushStack8( cpuMemory, mProgramBankRegister );
		PushStack16( cpuMemory, programCounter );
		PushStack8( cpuMemory, *mStatusRegisterPtr );

		statusRegister.Decimal = false;

		mProgramBankRegister = 0;
		programCounter = cpuMemory->Read16( mEmulationMode ? NMI_ADDRESS_65C816_EMU : NMI_ADDRESS_65C816 );
		if ( programCounter == 0 )
			programCounter = 0x8000;

		//statusRegister.Break = false;
		//statusRegister.Interrupt = true;
		//programCounter = cpuMemory->Read16( NMI_ADDRESS, false );

		mNmiPending = false;

#ifdef LOG_PROCESSOR_INSTRUCTIONS
		if ( Log::IsTypeEnabled( LOG_CPU ) )
			Log::Write( LOG_CPU, ( boost::format( "[%1$02X%2$04X] NMI triggered" ) % (int)mProgramBankRegister % (int)programCounter ).str().c_str() );
#endif

		return 7;
	}

	if ( this->mIrqPending && /*this->irqReadyForExecution &&*/ !statusRegister.Interrupt )
	{
		// NMI interrupt
		if ( !mEmulationMode )
			PushStack8( cpuMemory, mProgramBankRegister );
		PushStack16( cpuMemory, programCounter );
		PushStack8( cpuMemory, *mStatusRegisterPtr );

		statusRegister.Decimal = false;

		mProgramBankRegister = 0;
		programCounter = cpuMemory->Read16( mEmulationMode ? IRQ_ADDRESS_65C816_EMU : IRQ_ADDRESS_65C816 );
		if ( programCounter == 0 )
			programCounter = 0x8000;

		//statusRegister.Break = false;
		//statusRegister.Interrupt = true;
		//programCounter = cpuMemory->Read16( NMI_ADDRESS, false );

		mIrqPending = false;

#ifdef LOG_PROCESSOR_INSTRUCTIONS
		if ( Log::IsTypeEnabled( LOG_CPU ) )
			Log::Write( LOG_CPU, ( boost::format( "[%1$02X%2$04X] IRQ interrupt triggered" ) % (int)mProgramBankRegister % (int)programCounter ).str().c_str() );
#endif

		return 7;
	}

	return 0;
}


void Processor65C816::OperationBRK( SnesCpuMemory* cpuMemory )
{
	if ( mEmulationMode )
	{
		PushStack8( cpuMemory, (programCounter >> 8) & 0xFF );
		PushStack8( cpuMemory, programCounter & 0xFF );
		PushStack8( cpuMemory, *mStatusRegisterPtr );

		statusRegister.Interrupt = true;
		programCounter = cpuMemory->Read16( IRQ_ADDRESS_65C816_EMU );
		statusRegister.Decimal = false;
	}
	else
	{
		PushStack8( cpuMemory, mProgramBankRegister );
		programCounter += 2;
		PushStack16( cpuMemory, programCounter );
		PushStack8( cpuMemory, *mStatusRegisterPtr );
		statusRegister.Interrupt = true;
		statusRegister.Decimal = false;
		mProgramBankRegister = 0;
		programCounter = cpuMemory->Read16( BRK_ADDRESS_65C816 );
	}
}


void Processor65C816::OperationCOP( SnesCpuMemory* cpuMemory ) // NEW
{
	if ( mEmulationMode )
	{
		programCounter += 2;
		PushStack16( cpuMemory, programCounter );
		PushStack8( cpuMemory, *mStatusRegisterPtr );
		statusRegister.Interrupt = true;
		programCounter = cpuMemory->Read16( COP_ADDRESS_65C816_EMU );
		statusRegister.Decimal = false;
	}
	else
	{
		PushStack8( cpuMemory, mProgramBankRegister );
		programCounter += 2;
		PushStack16( cpuMemory, programCounter );
		PushStack8( cpuMemory, *mStatusRegisterPtr );
		statusRegister.Interrupt = true;
		mProgramBankRegister = 0;
		programCounter = cpuMemory->Read16( COP_ADDRESS_65C816 );
		statusRegister.Decimal = false; // TODO: decimal flag is cleared after COP executed
	}
}


int Processor65C816::ExecuteInstructionFast( SnesCpuMemory* cpuMemory )
{
	if ( mWaitingForInterrupt )
		return 0; // TODO: make sure this'll work

	ClearSubCycle();
	Byte_t opcode = cpuMemory->Read8( mProgramBankRegister, programCounter );
	ProcessSubCycle( true, true );

	MultiSizeInt pcArgument;

	/*
Direct: 	lda $12 	a = ram[$00 : d + $0012]
Direct indexed: 	lda $12,x
lda $12,y 	a = ram[$00 : d + $0012 + x]
a = ram[$00 : d + $0012 + y]
Direct indirect: 	lda ($12); pei ($12) 	a = ram[db: ramw[$00 : d + $0012]]
Direct indexed indirect: 	lda ($12,x) 	a = ram[db: ramw[$00 : d + $0012 + x]]
Direct indirect indexed: 	lda ($12),y 	a = ram[db: ramw[$00 : d + $0012] + y]
Direct indirect long: 	lda [$12] 	a = ram[raml[$00 : d + $0012]]
Direct indirect indexed long: 	lda [$12],y 	a = ram[raml[$00 : d + $0012] + y]
Absolute: 	lda $1234 	a = ram[db : $1234]
Absolute indexed: 	lda $1234,x
lda $1234,y 	a = ram[db : $1234 + x]
a = ram[db : $1234 + y]
Absolute long: 	lda $123456 	a = ram[$12 : $3456]
Absolute indexed long: 	lda $123456,x 	a = ram[$12 : $3456 + x]
Stack-relative: 	lda $12,s 	a = ram[$00 : $0012 + s]
Stack-relative indirect indexed: 	lda ($12,s),y 	a = ram[db : ramb[$00 : $0012 + s] + y]
Absolute indirect: 	lda ($1234) 	a = ram[db : ramw[db : $1234]]
Absolute indirect long: 	lda [$1234] 	a = ram[raml[db : $1234]]
Absolute indexed indirect: 	lda ($1234,x) 	a = ram[raml[db : $1234 + x]]
	*/

	switch ( InstructionAddressingModes65C816[ opcode ] )
	{
	case AMS_IMPLIED:
		break;
	case AMS_IMMEDIATE8: // Immediate Addressing -- #
		pcArgument.Data.Part8.b1 = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 );
		ProcessSubCycle( false, true );
		break;
	case AMS_IMMEDIATE16:
		pcArgument.Data.Part16.w1 = cpuMemory->Read16( mProgramBankRegister, programCounter + 1 );
		ProcessSubCycle( false, true, 2 );
		break;
	case AMS_IMMEDIATE_EMU: // Immediate Addressing -- #
		if ( Use8bitAccumulator() )
		{
			pcArgument.Data.Part8.b1 = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 );
			ProcessSubCycle( false, true );
		}
		else
		{
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( mProgramBankRegister, programCounter + 1 );
			ProcessSubCycle( false, true, 2 );
		}
		break;
	case AMS_IMMEDIATE_INDEX: // Immediate Addressing -- #
		if ( Use8bitIndexReg() )
		{
			pcArgument.Data.Part8.b1 = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 );
			ProcessSubCycle( false, true );
		}
		else
		{
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( mProgramBankRegister, programCounter + 1 );
			ProcessSubCycle( false, true, 2 );
		}
		break;
	case AMS_ACCUMULATOR: // Accumulator -- A
		pcArgument.Data.Part16.w1 = mAccumulator.Data.Part16.w1; // TODO: Only use 8 bits in 8 bit accum mode?
		ProcessSubCycle( false, false );
		break;
	case AMS_ZEROPAGE: // Direct -- d
		pcArgument.Data.Part16.w1 = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 ) + directPagePointer;
		ProcessSubCycle( false, true );
		if ( ( directPagePointer & 0xFF ) != 0 )
			ProcessSubCycle( false, false );
		break;
	case AMS_ZEROPAGEX: // Direct Indexed With X -- d,x
		if ( mEmulationMode )
			pcArgument.Data.Part16.w1 = Wrap8( cpuMemory->Read8( mProgramBankRegister, programCounter + 1 ) + directPagePointer + mXRegister.Data.Part16.w1 );
		else
			pcArgument.Data.Part16.w1 = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 ) + directPagePointer + mXRegister.Data.Part16.w1;
		break;
	case AMS_ZEROPAGEY: // Direct Indexed With Y -- d,y
		if ( mEmulationMode )
			pcArgument.Data.Part16.w1 = Wrap8( cpuMemory->Read8( mProgramBankRegister, programCounter + 1 ) + directPagePointer + mYRegister.Data.Part16.w1 );
		else
			pcArgument.Data.Part16.w1 = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 ) + directPagePointer + mYRegister.Data.Part16.w1;
		break;
	case AMS_INDIRECTX: // Direct Indexed Indirect -- (d,x)
		{
			UInt16_t tempAddress = 0;
			if ( mEmulationMode )
				tempAddress = Wrap8( cpuMemory->Read8( mProgramBankRegister, programCounter + 1 ) + directPagePointer + mXRegister.Data.Part16.w1 );
			else
				tempAddress = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 ) + directPagePointer + mXRegister.Data.Part16.w1;
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( 0, tempAddress );
			pcArgument.Data.Part8.b3 = mDataBankRegister;
		}
		break;
	case AMS_INDIRECTY: // Direct Indirect Indexed -- (d),y
		{
			UInt16_t tempAddress = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 ) + directPagePointer;
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( 0, tempAddress ) + mYRegister.Data.Part16.w1;
			pcArgument.Data.Part8.b3 = mDataBankRegister;
		}
		break;
	case AMS_ZEROPAGE_INDIRECT: // Direct Indirect -- (d) a = ram[db: ramw[$00 : d + $0012]]
		{
			UInt16_t tempAddress = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 ) + directPagePointer;
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( 0, tempAddress );
			pcArgument.Data.Part8.b3 = mDataBankRegister;
		}
		break;
	case AMS_ZEROPAGE_INDIRECT_LONG: // Direct Indirect Long -- [d]  	a = ram[raml[$00 : d + $0012]] 
		{
			UInt16_t tempAddress = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 ) + directPagePointer;
			pcArgument.Data.Part32 = cpuMemory->Read24( 0, tempAddress );
		}
		break;
	case AMS_ZEROPAGE_INDIRECT_LONGY: // Direct Indirect Indexed Long -- [d],y
		{
			UInt16_t tempAddress = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 ) + directPagePointer;
			pcArgument.Data.Part32 = cpuMemory->Read24( 0, tempAddress ) + mYRegister.Data.Part16.w1;
		}
		break;
	case AMS_ABSOLUTE: // Absolute -- a
		{
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( mProgramBankRegister, programCounter + 1 );
			pcArgument.Data.Part8.b3 = mDataBankRegister;
		}
		break;
	case AMS_ABSOLUTELONG: // Absolute Long -- al
		{
			pcArgument.Data.Part32 = cpuMemory->Read24( mProgramBankRegister, programCounter + 1 );
		}
		break;
	case AMS_ABSOLUTELONGX: // Absolute Long Indexed With X -- al,x
		{
			pcArgument.Data.Part32 = cpuMemory->Read24( mProgramBankRegister, programCounter + 1 );
			pcArgument.Data.Part16.w1 += mXRegister.Data.Part16.w1;
		}
		break;
	case AMS_ABSOLUTEX: // Absolute Indexed With X -- a,x
		{
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( mProgramBankRegister, programCounter + 1 ) + mXRegister.Data.Part16.w1;
			pcArgument.Data.Part8.b3 = mDataBankRegister;
		}
		break;
	case AMS_ABSOLUTEY: // Absolute Indexed With Y -- a,y
		{
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( mProgramBankRegister, programCounter + 1 ) + mYRegister.Data.Part16.w1;
			pcArgument.Data.Part8.b3 = mDataBankRegister;
		}
		break;
	case AMS_ABSOLUTE_INDIRECT: // Absolute indirect -- (a)
		{
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( mProgramBankRegister, programCounter + 1 );
			pcArgument.Data.Part8.b3 = mDataBankRegister;
		}
		break;
	case AMS_ABSOLUTE_INDIRECT_DIRECT: // Absolute indirect -- (a) [ uses direct bank instead of program ]
		{
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( mProgramBankRegister, programCounter + 1 );
			pcArgument.Data.Part8.b3 = directPagePointer & 0xFF;
		}
		break;
	case AMS_ABSOLUTE_INDIRECTX: // Absolute Indexed Indirect -- (a,x)  	a = ram[raml[db : $1234 + x]]
		{
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( mProgramBankRegister, programCounter + 1 ) + mXRegister.Data.Part16.w1;
			pcArgument.Data.Part8.b3 = mDataBankRegister;
		}
		break;
	case AMS_STACK: // Stack -- s
		break;
	case AMS_STACKRELATIVE: // Stack Relative -- d,s
		{
			pcArgument.Data.Part32 = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 ) + GetStackAddress() - 1; // Note: This is an address to data manipulated
		}
		break;
	case AMS_STACKRELATIVEINDEXED: // Stack Relative Indirect Indexed -- (d,s),y
		{
			UInt16_t tempAddress = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 ) + GetStackAddress() - 1;
			pcArgument.Data.Part16.w1 = cpuMemory->Read8( 0, tempAddress ) + mYRegister.Data.Part16.w1;
			pcArgument.Data.Part8.b3 = mDataBankRegister;
		}
		break;
	case AMS_STACK_DIRECTPAGEINDIRECT:
		{
			const Byte_t effectiveAddress = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 );
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( mDataBankRegister, effectiveAddress );
		}
		break;
	case AMS_RELATIVE: // Program counter relative -- r -- used by branch instructions
		{
			pcArgument.Data.Part8.b1 = cpuMemory->Read8( mProgramBankRegister, programCounter + 1 );
		}
		break;
	case AMS_RELATIVELONG: // Program counter relative long -- rl -- used by branch instructions
		{
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( mProgramBankRegister, programCounter + 1 );
		}
		break;
	case AMS_BLOCKMOVE: // Block move -- xya -- used by MVP and MVN instructions
		{
			pcArgument.Data.Part16.w1 = cpuMemory->Read16( mProgramBankRegister, programCounter + 1 );
		}
		break;
	}

	if ( Log::IsTypeEnabled( LOG_CPU ) )
	{
		/* n v m x d i z c */
		const std::string statusRegisterString = ( boost::format( "%1%%2%%3%%4%%5%%6%%7%%8%" )
			% ( statusRegister.Sign ? "N" : "n" )
			% ( statusRegister.Overflow ? "V" : "v" ) 
			% ( Use8bitAccumulator() ? "M" : "m" )
			% ( Use8bitIndexReg() ? "X" : "x" )
			% ( statusRegister.Decimal ? "D" : "d" )
			% ( statusRegister.Interrupt ? "I" : "i" )
			% ( statusRegister.Zero ? "Z" : "z" )
			% ( statusRegister.Carry ? "C" : "c" ) ).str();
		const std::string logMsg = ( boost::format( "[%1$02X%2$04X]\t%3$02X\t%4%\t[ARG:%5$06X]\t[%7%][%8%]\t%6%" ) % (int)mProgramBankRegister % programCounter % (int)opcode %
			InstructionName65C816[ opcode ] % (int)pcArgument.Data.Part32 % AddressingModeNames65C816[ InstructionAddressingModes65C816[ opcode ] ] %
			( mEmulationMode ? "E" : "N" ) % statusRegisterString ).str();
		PWLOG0( LOG_CPU, logMsg );

		wxGetApp().Sleep( 1 ); // sleep for half a second after each instruction if logging
		wxGetApp().DoWindowsEvents();
	}

	int addForPageBoundary = 0; // TODO: implement

	int instructionSize = InstructionSize65C816[ opcode ];
	if ( !Use8bitAccumulator() )
	{
		switch ( opcode )
		{
		case OPS_ORA_IMMEDIATE:
		case OPS_AND_IMMEDIATE:
		case OPS_EOR_IMMEDIATE:
		case OPS_ADC_IMMEDIATE:
		case OPS_BIT_IMMEDIATE:
		case OPS_LDA_IMMEDIATE:
		case OPS_CMP_IMMEDIATE:
		case OPS_SBC_IMMEDIATE:
			instructionSize++;
			break;
		}
	}
	if ( !Use8bitIndexReg() )
	{
		switch ( opcode )
		{
		case OPS_LDY_IMMEDIATE:
		case OPS_LDX_IMMEDIATE:
		case OPS_CPY_IMMEDIATE:
		case OPS_CPX_IMMEDIATE:
			instructionSize++;
			break;
		}
	}

	// Program counter should roll around from 0xFFFF -> 0x0000 without incrementing program bank reg
	programCounter += instructionSize;

	bool branchTaken = false;

	switch ( opcode )
	{
	default:
		throw std::runtime_error( "Invalid instruction detected" );

	// Add memory to accumulator with carry
	case OPS_ADC_IMMEDIATE:
		{
			OperationADC( pcArgument );
		}
		break;
	case OPS_ADC_ZEROPAGE:
	case OPS_ADC_ZEROPAGEX:
	case OPS_ADC_ABSOLUTE:
	case OPS_ADC_ABSOLUTEX:
	case OPS_ADC_ABSOLUTEY:
	case OPS_ADC_INDIRECTX: // Direct Indexed Indirect -- (d,x)
	case OPS_ADC_INDIRECTY: // Direct Indirect Indexed -- (d),y
	case OPS_ADC_STACKRELATIVE: // NEW: stack relative
	case OPS_ADC_ZEROPAGE_INDIRECT_LONG: // NEW: direct indirect long
	case OPS_ADC_ABSOLUTELONG: // NEW: absolute long
	case OPS_ADC_ZEROPAGE_INDIRECT: // NEW: direct Indirect
	case OPS_ADC_STACKRELATIVEINDEXED: // NEW: (d,s),y	stack relative indirect Indexed
	case OPS_ADC_ZEROPAGE_INDIRECT_LONGY: // NEW: [d],y		direct indirect indexed long
	case OPS_ADC_ABSOLUTELONGX: // NEW: al,x		absolute indexed long
		{
			OperationADC( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		}
		break;

	// "AND" memory with accumulator
	case OPS_AND_IMMEDIATE: // NEW: check
		OperationAND( pcArgument );
		break;
	case OPS_AND_ZEROPAGE:
	case OPS_AND_ZEROPAGEX:
	case OPS_AND_ABSOLUTE:
	case OPS_AND_ABSOLUTEX:
	case OPS_AND_ABSOLUTEY:
	case OPS_AND_INDIRECTX:
	case OPS_AND_INDIRECTY:
	case OPS_AND_STACKRELATIVE: // NEW: check d,s		stack relative
	case OPS_AND_ZEROPAGE_INDIRECT_LONG: // NEW: check direct indirect long
	case OPS_AND_ABSOLUTELONG: // NEW: check absolute long
	case OPS_AND_ABSOLUTELONGX: // NEW: check absolute indexed long
	case OPS_AND_ZEROPAGE_INDIRECT: // NEW: (d)		direct Indirect
	case OPS_AND_STACKRELATIVEINDEXED: // NEW: (d,s),y	stack relative indirect Indexed
	case OPS_AND_ZEROPAGE_INDIRECT_LONGY: // NEW: [d],y		direct indirect indexed long
		{
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
			OperationAND( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		}
		break;

	// ASL Shift Left One Bit (Memory or Accumulator)
	case OPS_ASL_ACCUMULATOR:
		if ( Use8bitAccumulator() )
			mAccumulator.Data.Part8.b1 = OperationASL( mAccumulator ).Data.Part8.b1;
		else
			mAccumulator.Data.Part16.w1 = OperationASL( mAccumulator ).Data.Part16.w1;
		break;
	case OPS_ASL_ZEROPAGE:
	case OPS_ASL_ZEROPAGEX:
	case OPS_ASL_ABSOLUTE:
	case OPS_ASL_ABSOLUTEX:
		{
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 3, false );
			MultiSizeInt b = OperationASL( cpuMemory->Read16( pcArgument.Data.Part32 ) );
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
			if ( Use8bitAccumulator() )
				cpuMemory->Write8( pcArgument.Data.Part32, b.Data.Part8.b1 );
			else
				cpuMemory->Write16( pcArgument.Data.Part32, b.Data.Part16.w1 );
		}
		break;

	// BCC Branch on Carry Clear
	case OPS_BCC:
		branchTaken = OperationBCC( pcArgument );
		break;

	// BCS Branch on carry set
	case OPS_BCS:
		branchTaken = OperationBCS( pcArgument );
		break;

	// BEQ Branch on result zero
	case OPS_BEQ:
		branchTaken = OperationBEQ( pcArgument );
		break;

	// BIT Test bits in memory with accumulator
	case OPS_BIT_IMMEDIATE: // NEW: check
		ProcessSubCycle( false, false );
		OperationBIT( pcArgument );
		break;
	case OPS_BIT_ZEROPAGE:
	case OPS_BIT_ABSOLUTE:
	case OPS_BIT_ZEROPAGEX: // NEW: direct indexed (with x)
	case OPS_BIT_ABSOLUTEX: // NEW: absolute indexed (with x)
		{
			const MultiSizeInt data = Use8bitAccumulator() ? cpuMemory->Read8( pcArgument.Data.Part32 ) : cpuMemory->Read16( pcArgument.Data.Part32 );
			ProcessSubCycle( false, false, Use8bitAccumulator() ? 1 : 2 );
			OperationBIT( data );
		}
		break;

	// BMI Branch on result minus
	case OPS_BMI:
		branchTaken = OperationBMI( pcArgument );
		break;

	// BNE Branch on result not zero
	case OPS_BNE:
		branchTaken = OperationBNE( pcArgument ); 
		break;

	// BPL Branch on result plus
	case OPS_BPL:
		branchTaken = OperationBPL( pcArgument ); 
		break;

	// NEW: Branch always
	case OPS_BRA:
		branchTaken = OperationBRA( pcArgument ); 
		break;

	// BRK Force Break
	case OPS_BRK:
		OperationBRK( cpuMemory );
		break;

	// NEW: Branch always long
	case OPS_BRL:
		branchTaken = OperationBRL( pcArgument );
		break;

	// BVC Branch on overflow clear
	case OPS_BVC:
		branchTaken = OperationBVC( pcArgument );
		break;

	// BVS Branch on overflow set
	case OPS_BVS:
		branchTaken = OperationBVS( pcArgument );
		break;

	// CLC Clear carry flag
	case OPS_CLC:
		OperationCLC();
		break;

	// CLD Clear decimal mode
	case OPS_CLD:
		OperationCLD();
		break;

	// CLI Clear interrupt disable bit
	case OPS_CLI:
		OperationCLI();
		break;

	// CLV Clear overflow flag
	case OPS_CLV:
		OperationCLV();
		break;

	// CMP Compare memory and accumulator
	case OPS_CMP_IMMEDIATE:
		OperationCMP( pcArgument );
		break;
	case OPS_CMP_ZEROPAGE:
	case OPS_CMP_ZEROPAGEX:
	case OPS_CMP_ABSOLUTE:
	case OPS_CMP_ABSOLUTEX:
	case OPS_CMP_ABSOLUTEY:
	case OPS_CMP_INDIRECTX:
	case OPS_CMP_INDIRECTY:
	case OPS_CMP_STACKRELATIVE: // NEW: d,s		stack relative
	case OPS_CMP_ZEROPAGE_INDIRECT_LONG: // NEW:  [d]		direct indirect long
	case OPS_CMP_ABSOLUTELONG: // NEW: CMP al
	case OPS_CMP_ZEROPAGE_INDIRECT: // NEW: CMP (d)
	case OPS_CMP_STACKRELATIVEINDEXED: // NEW: CMP(d,s),y
	case OPS_CMP_ZEROPAGE_INDIRECT_LONGY: // NEW: CMP [d],y
	case OPS_CMP_ABSOLUTELONGX: // NEW: CMP al,x
//		SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
		OperationCMP( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		break;

	// NEW: COP co-processor
	case OPS_COP:
		OperationCOP( cpuMemory );
		break;

	// CPX Compare Memory and Index X
	case OPS_CPX_IMMEDIATE:
		OperationCPX( pcArgument );
		break;
	case OPS_CPX_ZEROPAGE:
	case OPS_CPX_ABSOLUTE:
//		SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
		OperationCPX( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		break;

	// CPY Compare memory and index Y
	case OPS_CPY_IMMEDIATE:
		OperationCPY( pcArgument );
		break;
	case OPS_CPY_ZEROPAGE:
	case OPS_CPY_ABSOLUTE:
//		SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
		OperationCPY( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		break;

	// DEC Decrement memory by one
	case OPS_DEC_ACCUMULATOR: // NEW: check
		if ( Use8bitAccumulator() )
			mAccumulator.Data.Part8.b1 = OperationDEC( mAccumulator ).Data.Part8.b1;
		else
			mAccumulator.Data.Part16.w1 = OperationDEC( mAccumulator ).Data.Part16.w1;
		break;
	case OPS_DEC_ZEROPAGE:
	case OPS_DEC_ZEROPAGEX:
	case OPS_DEC_ABSOLUTE:
	case OPS_DEC_ABSOLUTEX:
		{
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 3, false );
			MultiSizeInt b = OperationDEC( cpuMemory->Read16( pcArgument.Data.Part32 ) );
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
			if ( Use8bitAccumulator() )
				cpuMemory->Write8( pcArgument.Data.Part32, b.Data.Part8.b1 );
			else
				cpuMemory->Write16( pcArgument.Data.Part32, b.Data.Part16.w1 );
		}
		break;

	// DEX Decrement index X by one
	case OPS_DEX:
		OperationDEX();
		break;

	// DEY Decrement index Y by one
	case OPS_DEY:
		OperationDEY();
		break;

	// EOR "Exclusive-Or" memory with accumulator
	case OPS_EOR_IMMEDIATE:
		OperationEOR( pcArgument );
		break;
	case OPS_EOR_ZEROPAGE:
	case OPS_EOR_ZEROPAGEX:
	case OPS_EOR_ABSOLUTE:
	case OPS_EOR_ABSOLUTEX:
	case OPS_EOR_ABSOLUTEY:
	case OPS_EOR_INDIRECTX:
	case OPS_EOR_INDIRECTY:
	case OPS_EOR_STACKRELATIVE: // NEW: stack relative
	case OPS_EOR_ZEROPAGE_INDIRECT_LONG: // NEW: direct indirect long
	case OPS_EOR_ABSOLUTELONG: // NEW: absolute long
	case OPS_EOR_ZEROPAGE_INDIRECT: // NEW: direct indirect
	case OPS_EOR_STACKRELATIVEINDEXED: // NEW: (d,s),y	stack relative indirect Indexed
	case OPS_EOR_ZEROPAGE_INDIRECT_LONGY: // NEW: [d],y		direct indirect indexed long
	case OPS_EOR_ABSOLUTELONGX: // NEW: absolute indexed long
//		SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
		OperationEOR( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		break;

	// INC Increment memory by one
	case OPS_INC_ACCUMULATOR: // NEW: check
		if ( Use8bitAccumulator() )
			mAccumulator.Data.Part8.b1 = OperationINC( mAccumulator ).Data.Part8.b1;
		else
			mAccumulator.Data.Part16.w1 = OperationINC( mAccumulator ).Data.Part16.w1;
		break;
	case OPS_INC_ZEROPAGE:
	case OPS_INC_ZEROPAGEX:
	case OPS_INC_ABSOLUTE:
	case OPS_INC_ABSOLUTEX:
		{
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 3, false );
			MultiSizeInt b = OperationINC( cpuMemory->Read16( pcArgument.Data.Part32 ) );
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
			if ( Use8bitAccumulator() )
				cpuMemory->Write8( pcArgument.Data.Part32, b.Data.Part8.b1 );
			else
				cpuMemory->Write16( pcArgument.Data.Part32, b.Data.Part16.w1 );
		}
		break;

	// INX Increment Index X by one
	case OPS_INX:
		OperationINX();
		break;

	// INY Increment Index Y by one
	case OPS_INY:
		OperationINY();
		break;

	// NEW: JML Jump long
	case OPS_JML: // Absolute long addressing mode
		OperationJML( cpuMemory, pcArgument );
		break;

	case OPS_JMP_ABSOLUTELONG: // NEW: check this
		OperationJMP24( pcArgument );
		break;

	// JMP Jump to new location
	case OPS_JMP_ABSOLUTE:
	case OPS_JMP_INDIRECT:
	case OPS_JMP_ABSOLUTE_INDIRECTX: // NEW: check this
		{
			OperationJMP( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		}
		break;

	// NEW: JSL Jump Subroutine Long
	case OPS_JSL:
		OperationJSL( cpuMemory, pcArgument );
		break;

	// JSR Jump to new location saving return address
	case OPS_JSR:
	case OPS_JSR_ABSOLUTE_INDIRECTX: // NEW: check this
		OperationJSR( cpuMemory, pcArgument );
		break;

	// LDA Load accumulator with memory
	case OPS_LDA_IMMEDIATE:
		OperationLDA( pcArgument );
		break;
	case OPS_LDA_ZEROPAGE:
	case OPS_LDA_ZEROPAGEX:
	case OPS_LDA_ABSOLUTE:
	case OPS_LDA_ABSOLUTEX:
	case OPS_LDA_ABSOLUTEY:
	case OPS_LDA_INDIRECTX:
	case OPS_LDA_INDIRECTY:
	case OPS_LDA_STACKRELATIVE: // NEW: d,s
	case OPS_LDA_ZEROPAGE_INDIRECT_LONG: // NEW: [d]
	case OPS_LDA_ABSOLUTELONG: // NEW: absolute long
	case OPS_LDA_ZEROPAGE_INDIRECT: // NEW: (d)
	case OPS_LDA_STACKRELATIVEINDEXED: // NEW: (d,s),y	stack relative indirect Indexed
	case OPS_LDA_ZEROPAGE_INDIRECT_LONGY: // NEW: [d],y		direct indirect indexed long
	case OPS_LDA_ABSOLUTELONGX: // NEW: al,x		absolute indexed long
		{
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
			OperationLDA( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		}
		break;

	// LDX Load index X with memory
	case OPS_LDX_IMMEDIATE:
		OperationLDX( pcArgument );
		break;
	case OPS_LDX_ZEROPAGE:
	case OPS_LDX_ZEROPAGEY:
	case OPS_LDX_ABSOLUTE:
	case OPS_LDX_ABSOLUTEY:
//		SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
		OperationLDX( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		break;

	// LDY Load index Y with memory
	case OPS_LDY_IMMEDIATE:
		OperationLDY( pcArgument );
		break;
	case OPS_LDY_ZEROPAGE:
	case OPS_LDY_ZEROPAGEX:
	case OPS_LDY_ABSOLUTE:
	case OPS_LDY_ABSOLUTEX:
//		SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
		OperationLDY( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		break;

	// LSR Shift right one bit (memory or accumulator)
	case OPS_LSR_ACCUMULATOR:
		if ( Use8bitAccumulator() )
			mAccumulator.Data.Part8.b1 = OperationLSR( mAccumulator ).Data.Part8.b1;
		else
			mAccumulator.Data.Part16.w1 = OperationLSR( mAccumulator ).Data.Part16.w1;
		break;
	case OPS_LSR_ZEROPAGE:
	case OPS_LSR_ZEROPAGEX:
	case OPS_LSR_ABSOLUTE:
	case OPS_LSR_ABSOLUTEX:
		{
//			SetSubCycle( InstructionCycles[ opcode ] - 3, false );
			MultiSizeInt b = OperationLSR( cpuMemory->Read16( pcArgument.Data.Part32 ) );
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
			if ( Use8bitIndexReg() )
				cpuMemory->Write8( pcArgument.Data.Part32, b.Data.Part8.b1 );
			else
				cpuMemory->Write16( pcArgument.Data.Part32, b.Data.Part16.w1 );
		}
		break;

	// NEW: MVN Block Move Negative
	case OPS_MVN:
		OperationMVN( cpuMemory, pcArgument );
		break;

	// NEW: MVP	Block Move Positive
	case OPS_MVP:
		OperationMVP( cpuMemory, pcArgument );
		break;

	// NOP No operation
	case OPS_NOP:
		break;

	// ORA "OR" memory with accumulator
	case OPS_ORA_IMMEDIATE:
		OperationORA( pcArgument );
		break;
	case OPS_ORA_ZEROPAGE:
	case OPS_ORA_ZEROPAGEX:
	case OPS_ORA_ABSOLUTE:
	case OPS_ORA_ABSOLUTEX:
	case OPS_ORA_ABSOLUTEY:
	case OPS_ORA_INDIRECTX:
	case OPS_ORA_INDIRECTY:
	case OPS_ORA_STACKRELATIVE: // NEW: check this??
	case OPS_ORA_ZEROPAGE_INDIRECT_LONG: // NEW: def. check this!
	case OPS_ORA_ABSOLUTELONG: // NEW: check
	case OPS_ORA_ZEROPAGE_INDIRECT: // NEW: (d) ( direct Indirect )
	case OPS_ORA_STACKRELATIVEINDEXED: // NEW: (d,s),y	stack relative indirect Indexed
	case OPS_ORA_ZEROPAGE_INDIRECT_LONGY: // NEW: [d],y		direct indirect indexed long
	case OPS_ORA_ABSOLUTELONGX: // NEW: al, x absolute indexed long
		{
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
			OperationORA( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		}
		break;

	// NEW: PEA	Push Effective Absolute Address on Stack (or Push Immediate Data on Stack)
	case OPS_PEA:
		OperationPEA( cpuMemory, pcArgument );
		break;

	// NEW: PEI	Push Effective Indirect Address on Stack (add one cycle if DL f 0)
	case OPS_PEI:
		OperationPEI( cpuMemory, pcArgument );
		break;

	// NEW: PER	Push Effective Program Counter Relative Address on Stack
	case OPS_PER:
		OperationPER( cpuMemory, pcArgument );
		break;

	// PHA Push accumulator on stack
	case OPS_PHA:
		OperationPHA( cpuMemory );
		break;

	// NEW: PHB	Push Data Bank Register on Stack
	case OPS_PHB:
		OperationPHB( cpuMemory );
		break;

	// NEW: PHD	Push Direct Register on Stack
	case OPS_PHD:
		OperationPHD( cpuMemory );
		break;

	// NEW: PHK	Push Program Bank Register on Stack
	case OPS_PHK:
		OperationPHK( cpuMemory );
		break;

	// PHP Push processor status on stack
	case OPS_PHP:
		OperationPHP( cpuMemory );
		break;

	// NEW: PHX	Push Index X on Stack
	case OPS_PHX:
		OperationPHX( cpuMemory );
		break;

	// NEW: PHY	Push index Y on Stack
	case OPS_PHY:
		OperationPHY( cpuMemory );
		break;

	// PLA Pull accumulator from stack
	case OPS_PLA:
		OperationPLA( cpuMemory );
		break;

	// NEW: PLB	Pull Data Bank Register from Stack
	case OPS_PLB:
		OperationPLB( cpuMemory );
		break;

	// NEW: PLD	Pull Direct Register from Stack
	case OPS_PLD:
		OperationPLD( cpuMemory );
		break;

	// PLP Pull processor status from stack
	case OPS_PLP:
		OperationPLP( cpuMemory );
		break;

	// NEW: PLX	Pull Index X from Stack
	case OPS_PLX:
		OperationPLX( cpuMemory );
		break;

	// NEW: PLY	Pull Index Y form Stack
	case OPS_PLY:
		OperationPLY( cpuMemory );
		break;

	// NEW: REP	Reset Status Bits
	case OPS_REP:
		OperationREP( pcArgument );
		break;

	// ROL Rotate one bit left (memory or accumulator)
	case OPS_ROL_ACCUMULATOR:
		if ( Use8bitAccumulator() )
			mAccumulator.Data.Part8.b1 = OperationROL( mAccumulator ).Data.Part8.b1;
		else
			mAccumulator.Data.Part16.w1 = OperationROL( mAccumulator ).Data.Part16.w1;
		break;
	case OPS_ROL_ZEROPAGE:
	case OPS_ROL_ZEROPAGEX:
	case OPS_ROL_ABSOLUTE:
	case OPS_ROL_ABSOLUTEX:
		{
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 3, false );
			MultiSizeInt b = OperationROL( cpuMemory->Read16( pcArgument.Data.Part32 ) );
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
			if ( Use8bitIndexReg() )
				cpuMemory->Write8( pcArgument.Data.Part32, b.Data.Part8.b1 );
			else
				cpuMemory->Write16( pcArgument.Data.Part32, b.Data.Part16.w1 );
		}
		break;

	// ROR Rotate one bit right (memory or accumulator)
	case OPS_ROR_ACCUMULATOR:
		if ( Use8bitAccumulator() )
			mAccumulator.Data.Part8.b1 = OperationROR( mAccumulator ).Data.Part8.b1;
		else
			mAccumulator.Data.Part16.w1 = OperationROR( mAccumulator ).Data.Part16.w1;
		break;
	case OPS_ROR_ZEROPAGE:
	case OPS_ROR_ZEROPAGEX:
	case OPS_ROR_ABSOLUTE:
	case OPS_ROR_ABSOLUTEX:
		{
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 3, false );
			MultiSizeInt b = OperationROR( cpuMemory->Read16( pcArgument.Data.Part32 ) );
//			SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
			if ( Use8bitIndexReg() )
				cpuMemory->Write8( pcArgument.Data.Part32, b.Data.Part8.b1 );
			else
				cpuMemory->Write16( pcArgument.Data.Part32, b.Data.Part16.w1 );
		}
		break;

	// RTI Return from interrupt
	case OPS_RTI:
		OperationRTI( cpuMemory );
		break;

	// NEW: RTL Return from Subroutine Long
	case OPS_RTL:
		OperationRTL( cpuMemory );
		break;

	// RTS Return from subroutine
	case OPS_RTS:
		OperationRTS( cpuMemory );
		break;

	// SBC Subtract memory from accumulator with borrow
	case OPS_SBC_IMMEDIATE:
		OperationSBC( pcArgument );
		break;
	case OPS_SBC_ZEROPAGE:
	case OPS_SBC_ZEROPAGEX:
	case OPS_SBC_ABSOLUTE:
	case OPS_SBC_ABSOLUTEX:
	case OPS_SBC_ABSOLUTEY:
	case OPS_SBC_INDIRECTX:
	case OPS_SBC_INDIRECTY:
	case OPS_SBC_STACKRELATIVE: // NEW: SBC d,s
	case OPS_SBC_ZEROPAGE_INDIRECT_LONG: // NEW: SBC [d]
	case OPS_SBC_ABSOLUTELONG: // NEW: SBC al
	case OPS_SBC_ZEROPAGE_INDIRECT: // NEW SBC (d)
	case OPS_SBC_STACKRELATIVEINDEXED: // NEW: SBC(d,s),y
	case OPS_SBC_ZEROPAGE_INDIRECT_LONGY: // NEW: SBC [d],y
	case OPS_SBC_ABSOLUTELONGX: // NEW: SBC al,x
//		SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
		OperationSBC( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		break;

	// SEC Set carry flag
	case OPS_SEC:
		OperationSEC();
		break;

	// SED Set decimal mode
	case OPS_SED:
		OperationSED();
		break;

	// SEI Set interrupt disable status
	case OPS_SEI:
		OperationSEI();
		break;

	// NEW: SEP	Set Processor Status Bits
	case OPS_SEP:
		OperationSEP( pcArgument );
		break;

	// STA Store accumulator in memory
	case OPS_STA_ZEROPAGE:
	case OPS_STA_ZEROPAGEX:
	case OPS_STA_ABSOLUTE:
	case OPS_STA_ABSOLUTEX:
	case OPS_STA_ABSOLUTEY:
	case OPS_STA_INDIRECTX:
	case OPS_STA_INDIRECTY:
	case OPS_STA_STACKRELATIVE: // NEW: d,s		stack relative
	case OPS_STA_ZEROPAGE_INDIRECT_LONG: // NEW: [d]		direct indirect long
	case OPS_STA_ABSOLUTELONG: // NEW: al		absolute long
	case OPS_STA_ZEROPAGE_INDIRECT: // NEW: direct indirect
	case OPS_STA_STACKRELATIVEINDEXED: // (d,s),y	stack relative indirect Indexed
	case OPS_STA_ZEROPAGE_INDIRECT_LONGY: // [d],y		direct indirect indexed long
	case OPS_STA_ABSOLUTELONGX: //  al,x		absolute indexed long
//		SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
		if ( Use8bitAccumulator() )
			cpuMemory->Write8( pcArgument.Data.Part32, OperationSTA().Data.Part8.b1 );
		else
			cpuMemory->Write16( pcArgument.Data.Part32, OperationSTA().Data.Part16.w1 );
		break;

	// NEW: STP	Stop the Clock
	case OPS_STP:
		OperationSTP();
		break;

	// STX Store index X in memory
	case OPS_STX_ZEROPAGE:
	case OPS_STX_ZEROPAGEY:
	case OPS_STX_ABSOLUTE:
//		SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
		if ( Use8bitIndexReg() )
			cpuMemory->Write8( pcArgument.Data.Part32, OperationSTX().Data.Part8.b1 );
		else
			cpuMemory->Write16( pcArgument.Data.Part32, OperationSTX().Data.Part16.w1 );
		break;

	// STY Store index Y in memory
	case OPS_STY_ZEROPAGE:
	case OPS_STY_ZEROPAGEX:
	case OPS_STY_ABSOLUTE:
//		SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
		if ( Use8bitIndexReg() )
			cpuMemory->Write8( pcArgument.Data.Part32, OperationSTY().Data.Part8.b1 );
		else
			cpuMemory->Write16( pcArgument.Data.Part32, OperationSTY().Data.Part16.w1 );
		break;

	// NEW: STZ	Store Zero in Memory
	case OPS_STZ_ZEROPAGE:
	case OPS_STZ_ZEROPAGEX:
	case OPS_STZ_ABSOLUTE:
	case OPS_STZ_ABSOLUTEX:
//		SetSubCycle( InstructionCycles[ opcode ] + addForPageBoundary - 1, true );
		if ( Use8bitAccumulator() )
			cpuMemory->Write8( pcArgument.Data.Part32, OperationSTZ().Data.Part8.b1 );
		else
			cpuMemory->Write16( pcArgument.Data.Part32, OperationSTZ().Data.Part16.w1 );
		break;

	// TAX Transfer accumulator to index X
	case OPS_TAX:
		OperationTAX();
		break;

	// TAY Transfer accumulator to index Y
	case OPS_TAY:
		OperationTAY();
		break;

	// NEW: TCD*	Transfer Accumulator to Direct Register
	case OPS_TCD:
		OperationTCD();
		break;

	// NEW: TCS*	Transfer Accumulator to Stack Pointer Register
	case OPS_TCS:
		OperationTCS();
		break;

	// NEW: TDC*	Transfer Direct Register to Accumulator
	case OPS_TDC:
		OperationTDC();
		break;

	// NEW: TRB	Test and Reset Bit
	case OPS_TRB_ZEROPAGE:
	case OPS_TRB_ABSOLUTE:
		OperationTRB( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		break;

	// NEW: TSB	Test and Set Bit
	case OPS_TSB_ZEROPAGE:
	case OPS_TSB_ABSOLUTE:
		OperationTSB( cpuMemory->Read16( pcArgument.Data.Part32 ) );
		break;

	// NEW: TSC*	Transfer Stack Pointer Register to Accumulator
	case OPS_TSC:
		OperationTSC();
		break;

	// TSX Transfer stack pointer to index X
	case OPS_TSX:
		OperationTSX();
		break;

	// TXA Transfer index X to accumulator
	case OPS_TXA:
		OperationTXA();
		break;

	// TXS Transfer index X to stack pointer
	case OPS_TXS:
		OperationTXS();
		break;

	// NEW: TXY	Transfer Index X to Index Y
	case OPS_TXY:
		OperationTXY();
		break;

	// TYA Transfer index Y to accumulator
	case OPS_TYA:
		OperationTYA();
		break;

	// NEW: TYX 	Transfer Index Y to Index X
	case OPS_TYX:
		OperationTYX();
		break;

	// NEW: WAI	Wait for Interrupt
	case OPS_WAI:
		OperationWAI();
		break;

	// NEW: XBA*	Exchange AH and AL
	case OPS_XBA:
		OperationXBA();
		break;

	// NEW: XCE	Exchange Carry and Emulation Bits
	case OPS_XCE:
		OperationXCE();
		break;
	}

	int ticks = InstructionCycles65C816_16bit[ opcode ];

	if ( InstructionAddressingModes65C816[ opcode ] == AMS_RELATIVE )
	{
		if ( branchTaken )
			ticks += ( addForPageBoundary + 1 );
	}
	else
		ticks += addForPageBoundary;

	return ticks;
}


void Processor65C816::SetDefaultVariables()
{
	mEmulationMode = true;
	statusRegister.Interrupt = true;
	statusRegister.IndexRegister8bit = true;
	statusRegister.Accumulator8bit = true;
	mStackPointer = 0x1FF;
}


void Processor65C816::Reset( bool cold )
{
	SetDefaultVariables();
}


void Processor65C816::OutputAllInstructions()
{
	for ( size_t i=0; i<256; ++i )
	{
		OPCODE_65C816 opcode = (OPCODE_65C816)i;
		const std::string logMsg = ( boost::format( "%1$02X\t%2%\t\t[%3%]\t\t\t[Size:%4%] [Cycles:%5%]" ) % (int)opcode %
			InstructionName65C816[ opcode ] % AddressingModeNames65C816[ InstructionAddressingModes65C816[ opcode ] ] %
			(int)InstructionSize65C816[ opcode ] % (int)InstructionCycles65C816_16bit[ opcode ] ).str();
		PWLOG0( LOG_MISC, logMsg );
	}
}



