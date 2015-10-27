
#ifndef PROCESSOR6502_H
#define PROCESSOR6502_H


#include "main.h"
#include "opcodes6502.h"
#include "mappers.h"
#include <stack>
#include <vector>

#include "synchroniser.h"
#include "multisizeint.h"

// Main processor class


class Processor6502 : public HasState
{

	FORCE_INLINE bool DecimalFlagEnabled() const;

	FORCE_INLINE void SetSign( Byte_t t );
	FORCE_INLINE void SetZero( Byte_t t );
	FORCE_INLINE void SetSignAndZero( Byte_t t );
	FORCE_INLINE void SetCarry( unsigned int t );
	FORCE_INLINE void SetCarrySubstract( unsigned int t );

	FORCE_INLINE UInt16_t Read16IncSubCycles( CPUMemory* cpuMemory, UInt16_t offset, bool isLastSubCycle = false, bool zeropage = false );

	FORCE_INLINE void PerformDummyRead( const Instruction6502& instruction, UInt16_t baseAddress, Byte_t addition );
	FORCE_INLINE UInt16_t CalculateDummyReadAddress( UInt16_t add, Byte_t index );

	FORCE_INLINE void OperationADC( MultiSizeInt src );
	FORCE_INLINE MultiSizeInt OperationASO( MultiSizeInt src );
	FORCE_INLINE void OperationAND( MultiSizeInt src );
	FORCE_INLINE MultiSizeInt OperationASL( MultiSizeInt src );
	FORCE_INLINE bool OperationBCC();
	FORCE_INLINE bool OperationBCS();
	FORCE_INLINE bool OperationBEQ();
	FORCE_INLINE void OperationBIT( MultiSizeInt src );
	FORCE_INLINE bool OperationBMI();
	FORCE_INLINE bool OperationBNE();
	FORCE_INLINE bool OperationBPL();
	void OperationBRK( CPUMemory* cpuMemory );
	FORCE_INLINE bool OperationBVC();
	FORCE_INLINE bool OperationBVS();
	FORCE_INLINE void OperationCLC();
	FORCE_INLINE void OperationCLD();
	FORCE_INLINE void OperationCLI();
	FORCE_INLINE void OperationCLV();
	FORCE_INLINE void OperationCMP( MultiSizeInt src );
	FORCE_INLINE void OperationCPX( MultiSizeInt src );
	FORCE_INLINE void OperationCPY( MultiSizeInt src );
	FORCE_INLINE MultiSizeInt OperationDEC( MultiSizeInt src );
	FORCE_INLINE void OperationDEX();
	FORCE_INLINE void OperationDEY();
	FORCE_INLINE void OperationEOR( MultiSizeInt src );
	FORCE_INLINE MultiSizeInt OperationINC( MultiSizeInt src );
	FORCE_INLINE void OperationINX();
	FORCE_INLINE void OperationINY();
	FORCE_INLINE void OperationJMP( UInt16_t src );
	FORCE_INLINE void OperationJSR( CPUMemory* cpuMemory, UInt16_t src );
	FORCE_INLINE void OperationLDA( MultiSizeInt src );
	FORCE_INLINE void OperationLDX( MultiSizeInt src );
	FORCE_INLINE void OperationLDY( MultiSizeInt src );
	FORCE_INLINE MultiSizeInt OperationLSR( MultiSizeInt src );
	FORCE_INLINE void OperationORA( MultiSizeInt src );
	FORCE_INLINE void OperationPHA( CPUMemory* cpuMemory );
	FORCE_INLINE void OperationPHP( CPUMemory* cpuMemory );
	FORCE_INLINE void OperationPLA( CPUMemory* cpuMemory );
	FORCE_INLINE void OperationPLP( CPUMemory* cpuMemory );
	FORCE_INLINE MultiSizeInt OperationROL( MultiSizeInt src );
	FORCE_INLINE MultiSizeInt OperationROR( MultiSizeInt src );
	FORCE_INLINE void OperationRTI( CPUMemory* cpuMemory );
	FORCE_INLINE void OperationRTS( CPUMemory* cpuMemory );
	FORCE_INLINE void OperationSBC( MultiSizeInt src );
	FORCE_INLINE void OperationSEC();
	FORCE_INLINE void OperationSED();
	FORCE_INLINE void OperationSEI();
	FORCE_INLINE Byte_t OperationSTA( );
	FORCE_INLINE Byte_t OperationSTX( );
	FORCE_INLINE Byte_t OperationSTY( );
	FORCE_INLINE void OperationTAX();
	FORCE_INLINE void OperationTAY();
	FORCE_INLINE void OperationTSX();
	FORCE_INLINE void OperationTXA();
	FORCE_INLINE void OperationTXS();
	FORCE_INLINE void OperationTYA();

	FORCE_INLINE void OperationSAX( MultiSizeInt val );
	FORCE_INLINE void OperationHLT();
	FORCE_INLINE Byte_t OperationTAS();
	FORCE_INLINE MultiSizeInt OperationSAY( MultiSizeInt val );
	FORCE_INLINE Byte_t OperationXAS();
	FORCE_INLINE Byte_t OperationAXA();
	FORCE_INLINE MultiSizeInt OperationRLA( MultiSizeInt val );
	FORCE_INLINE MultiSizeInt OperationLSE( MultiSizeInt val );
	FORCE_INLINE MultiSizeInt OperationRRA( MultiSizeInt val );
	FORCE_INLINE Byte_t OperationAXS();
	FORCE_INLINE MultiSizeInt OperationDCM( MultiSizeInt val );
	FORCE_INLINE MultiSizeInt OperationINS( MultiSizeInt val );
	FORCE_INLINE void OperationLAX( MultiSizeInt val );
	FORCE_INLINE void OperationLAS( MultiSizeInt val );
	FORCE_INLINE void OperationALR( MultiSizeInt val );
	FORCE_INLINE void OperationANC( MultiSizeInt val );
	FORCE_INLINE void OperationARR( MultiSizeInt val );
	
	FORCE_INLINE void SetSubCycle( int s, bool last = false );
	FORCE_INLINE void IncrementSubCycle( bool last = false );

	FORCE_INLINE void PushStack( CPUMemory* cpuMemory, MultiSizeInt val );
	FORCE_INLINE Byte_t PopStack( CPUMemory* cpuMemory );
	FORCE_INLINE void IncrementStackCounter();
	FORCE_INLINE void DecrementStackCounter();
	
	FORCE_INLINE static int CalculateRelativeDifference( Byte_t b );

	bool mCmosVersion;
	bool mDecimalModeEnabled;

	const Instruction6502* mInstruction;

	union
	{
		struct
		{
			bool Carry : 1;
			bool Zero : 1;
			bool Interrupt : 1;
			bool Decimal : 1;
			bool Break : 1;
			bool Unused : 1;
			bool Overflow : 1;
			bool Sign : 1;
		} Status;
		Byte_t Byte;
	} mStatusRegister;

private:
	boost::signals::connection resetConnection;
	CPUMemory* mMemory;

	Byte_t accumulatorRegister, xRegister, yRegister, stackPointer;
	UInt16_t programCounter;
	bool irqLineLow, mWaitOneInstructionAfterCli, nmiPending, resetPending; // Interrupts
	bool triggerNmiAfterNextInstruction;
	int subCycle;
	bool isLastCycle;
	Byte_t mSAYHighByte;
	
	void OnReset( bool cold );
	void SetDefaultVariables();

	bool mPageBoundaryCrossed;
	bool mBranchTaken;

private: // logging variables
	int addressLoopFrom, addressLoopTo, loopCount;

	typedef std::vector< UInt16_t > AddressList_t;
	AddressList_t addressList; // list of previous addresses executed - used to detect loops
	void LogProcessorMessage( UInt16_t address, Byte_t opcode, Byte_t arg1, Byte_t arg2 );
	std::string FormatLogString( int programCounter, Byte_t opcode, Byte_t arg1, Byte_t arg2, int memval ) const;

public:
	Processor6502();
	virtual ~Processor6502();

	void NonMaskableInterrupt( int ppuMasterTickCount ); // NMI interrupt
	FORCE_INLINE void HoldIRQLineLow( bool low );

	FORCE_INLINE void SetCMOSVersion( bool on ) { mCmosVersion = on; }
	FORCE_INLINE void EnableDecimalMode( bool on ) { mDecimalModeEnabled = on; }
	
	int HandlePendingInterrupts( CPUMemory* cpuMemory );
	int ExecuteInstructionFast( CPUMemory* cpuMemory );

	FORCE_INLINE int GetSubCycle() const { return subCycle; }

	virtual void SaveState( std::ostream& ostr );
	virtual void LoadState( std::istream& istr );

	static void OutputAllInstructions();
};


//////////////////////////////////////////////////////////////


void Processor6502::HoldIRQLineLow( bool low )
{
//	Log::Write( LOG_CPU, "CPU IRQ line held low" );
	irqLineLow = low;
}


#endif // #ifndef PROCESSOR6502_H

