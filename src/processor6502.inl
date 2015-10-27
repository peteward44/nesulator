
#ifndef PROCESSOR6502_INL
#define PROCESSOR6502_INL


UInt16_t Processor6502::CalculateDummyReadAddress( UInt16_t add, Byte_t index )
{
	return (add & 0xFF00) | ((add + index) & 0xFF);
}


UInt16_t Processor6502::Read16IncSubCycles( CPUMemory* cpuMemory, UInt16_t offset, bool isLastSubCycle, bool zeropage )
{
	IncrementSubCycle();
	UInt16_t ret = cpuMemory->Read8( offset );
	IncrementSubCycle( isLastSubCycle );
	UInt16_t newoffset;
	if ( zeropage && (offset % 0x100) == 0xff)
		newoffset = offset - ( offset % 0x100 );
	else
		newoffset = offset+1;

	ret |= ( cpuMemory->Read8( newoffset ) << 8 );
	return ret;
}


void Processor6502::PerformDummyRead( const Instruction6502& instruction, UInt16_t baseAddress, Byte_t addition )
{
	mPageBoundaryCrossed = false;

	bool doDummyRead = true;
	if ( instruction.mOperationMode == OPERATION_READ )
	{ // dummy read is automatic for write and read/modify/write instructions, only read ops need to test cross boundary
		mPageBoundaryCrossed = ( ( baseAddress + addition ) & 0xff00 ) != ( baseAddress & 0xff00 );
		doDummyRead = mPageBoundaryCrossed && instruction.mPageBoundaryCycles > 0;
	}

	if ( doDummyRead )
	{ // dummy read for read operation instructions
		IncrementSubCycle();
		mMemory->Read8( CalculateDummyReadAddress( baseAddress, addition ) );
	}
}


void Processor6502::SetSubCycle( int s, bool last )
{
	subCycle = s;
	isLastCycle = last;
}


void Processor6502::IncrementSubCycle( bool last )
{
	subCycle++;
	if ( isLastCycle == true && last )
	{
		PWLOG6( LOG_ERROR, "Last cycle already set!: Op '%1%:%6$02X' Addressing mode '%2%' Correct cycles '%3%' Subcycles '%4%' Add for boundary '%5%'",
			mInstruction->mName, (int)mInstruction->mAddressingMode, (int)mInstruction->mBaseCycles, (int)subCycle, 0, (int)mInstruction->mOpcode );
		throw std::runtime_error( "" );
	}
	isLastCycle = last;
}


bool Processor6502::DecimalFlagEnabled() const
{
	return mDecimalModeEnabled && mStatusRegister.Status.Decimal;
}


void Processor6502::SetSign( Byte_t t )
{
	mStatusRegister.Status.Sign = (t & (0x01 << 7)) > 0;
}


void Processor6502::SetZero( Byte_t t )
{
	mStatusRegister.Status.Zero = t == 0;
}


void Processor6502::SetSignAndZero( Byte_t t )
{
	SetSign( t );
	SetZero( t );
}


void Processor6502::SetCarry( unsigned int t )
{
	mStatusRegister.Status.Carry = t > 0xff;
}


void Processor6502::SetCarrySubstract( unsigned int t )
{
	mStatusRegister.Status.Carry = t < 0x100;
}

//////////////////////////////////////////////////////////////


int Processor6502::CalculateRelativeDifference( Byte_t b )
{
	const bool isSigned = (b & 0x80) > 0;
	if ( isSigned )
	{
		const Byte_t inverse = (~b) + 1;
		return -static_cast< int >( inverse );
	}
	else
		return b;
}



//////////////////////////////////////////////////////////////



void Processor6502::OperationRTI( CPUMemory* cpuMemory )
{
	// dummy read
	IncrementSubCycle();
	cpuMemory->Read8( programCounter );

	IncrementSubCycle();
	DecrementStackCounter();

	IncrementSubCycle();
	mStatusRegister.Byte = PopStack( cpuMemory );
	DecrementStackCounter();
	
	IncrementSubCycle();
	programCounter = PopStack( cpuMemory );
	DecrementStackCounter();

	IncrementSubCycle( true );
	programCounter |= PopStack( cpuMemory ) << 8;

	mStatusRegister.Status.Break = true;
	mStatusRegister.Status.Unused = true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



MultiSizeInt Processor6502::OperationASO( MultiSizeInt src )
{
	mStatusRegister.Status.Carry = ( ( src.Data.Part8.b1 & 0x80 ) > 0 );
	Byte_t ret = src.Data.Part8.b1 << 1;
//	SetSignAndZero( ret );
	accumulatorRegister |= ret; 
	SetSignAndZero( accumulatorRegister );
	return ret;
}


void Processor6502::OperationADC( MultiSizeInt src )
{
	unsigned int temp = src.Data.Part8.b1 + accumulatorRegister + (mStatusRegister.Status.Carry ? 1 : 0);
	SetCarry( temp );

	if ( mCmosVersion || !DecimalFlagEnabled() )
	{ // Bug in non-cmos version: These flags aren't set if decimal mode is on
		SetSignAndZero( (Byte_t)temp );
		mStatusRegister.Status.Overflow = (!((accumulatorRegister ^ src.Data.Part8.b1) & 0x80) && ((accumulatorRegister ^ temp) & 0x80));
	}

	accumulatorRegister = (Byte_t)temp;
}


void Processor6502::OperationAND( MultiSizeInt src )
{
	accumulatorRegister &= src.Data.Part8.b1;
	SetSignAndZero( accumulatorRegister );
}


MultiSizeInt Processor6502::OperationASL( MultiSizeInt src )
{
	mStatusRegister.Status.Carry = ( ( src.Data.Part8.b1 & 0x80 ) > 0 );
	Byte_t ret = src.Data.Part8.b1 << 1;
	SetSignAndZero( ret );
	return ret;
}


bool Processor6502::OperationBCC()
{
	return !mStatusRegister.Status.Carry;
}


bool Processor6502::OperationBCS()
{
	return mStatusRegister.Status.Carry;
}


bool Processor6502::OperationBEQ()
{
	return mStatusRegister.Status.Zero;
}


void Processor6502::OperationBIT( MultiSizeInt src )
{
	SetSign( src.Data.Part8.b1 );
	SetZero( accumulatorRegister & src.Data.Part8.b1 );
	mStatusRegister.Status.Overflow = ( ( src.Data.Part8.b1 & 0x40) > 0 );	// Copy bit 6 to OVERFLOW flag.
}


bool Processor6502::OperationBMI()
{
	return mStatusRegister.Status.Sign;
}


bool Processor6502::OperationBNE()
{
	return !mStatusRegister.Status.Zero;
}


bool Processor6502::OperationBPL()
{
	return !mStatusRegister.Status.Sign;
}


bool Processor6502::OperationBVC()
{
	return !mStatusRegister.Status.Overflow;
}


bool Processor6502::OperationBVS()
{
	return mStatusRegister.Status.Overflow;
}


void Processor6502::OperationCLC()
{
	mStatusRegister.Status.Carry = false;
}


void Processor6502::OperationCLD()
{
	mStatusRegister.Status.Decimal = false;
}


void Processor6502::OperationCLI()
{
	mWaitOneInstructionAfterCli = mStatusRegister.Status.Interrupt == true;	
	mStatusRegister.Status.Interrupt = false;
}


void Processor6502::OperationCLV()
{
	mStatusRegister.Status.Overflow = false;
}


void Processor6502::OperationCMP( MultiSizeInt src )
{
	unsigned int t = accumulatorRegister - src.Data.Part8.b1;
	SetCarrySubstract( t );
	SetSignAndZero( (Byte_t)t );
}


void Processor6502::OperationCPX( MultiSizeInt src )
{
	unsigned int t = xRegister - src.Data.Part8.b1;
	SetCarrySubstract( t );
	SetSignAndZero( (Byte_t)t );
}


void Processor6502::OperationCPY( MultiSizeInt src )
{
	unsigned int t = yRegister - src.Data.Part8.b1;
	SetCarrySubstract( t );
	SetSignAndZero( (Byte_t)t );
}


MultiSizeInt Processor6502::OperationDEC( MultiSizeInt src )
{
	Byte_t t = src.Data.Part8.b1 - 1;
	SetSignAndZero( t );
	return t;
}


void Processor6502::OperationDEX()
{
	xRegister--;
	SetSignAndZero( xRegister );
}


void Processor6502::OperationDEY()
{
	yRegister--;
	SetSignAndZero( yRegister );
}


void Processor6502::OperationEOR( MultiSizeInt src )
{
	accumulatorRegister ^= src.Data.Part8.b1;
	SetSignAndZero( accumulatorRegister );
}


MultiSizeInt Processor6502::OperationINC( MultiSizeInt src )
{
	Byte_t ret = src.Data.Part8.b1 + 1;
	SetSignAndZero( ret );
	return ret;
}


void Processor6502::OperationINX()
{
	xRegister++;
	SetSignAndZero( xRegister );
}


void Processor6502::OperationINY()
{
	yRegister++;
	SetSignAndZero( yRegister );
}


void Processor6502::OperationJMP( UInt16_t src )
{
	programCounter = src;
}


void Processor6502::OperationJSR( CPUMemory* cpuMemory, UInt16_t src )
{
	const UInt16_t pushPc = programCounter - 1;
	IncrementSubCycle();
	PushStack( cpuMemory, ( pushPc >> 8 ) & 0xFF );
	IncrementStackCounter();

	IncrementSubCycle();
	PushStack( cpuMemory, pushPc & 0xFF );
	IncrementStackCounter();

	IncrementSubCycle( true );
	programCounter = src;
}


void Processor6502::OperationLDA( MultiSizeInt src )
{
	accumulatorRegister = src.Data.Part8.b1;
	SetSignAndZero( accumulatorRegister );
}


void Processor6502::OperationLDX( MultiSizeInt src )
{
	xRegister = src.Data.Part8.b1;
	SetSignAndZero( xRegister );
}


void Processor6502::OperationLDY( MultiSizeInt src )
{
	yRegister = src.Data.Part8.b1;
	SetSignAndZero( yRegister );
}


MultiSizeInt Processor6502::OperationLSR( MultiSizeInt src )
{
	mStatusRegister.Status.Carry = ((src.Data.Part8.b1 & 0x01) > 0);
	Byte_t temp = src.Data.Part8.b1 >> 1;
	SetSignAndZero( temp );
	return temp;
}



void Processor6502::OperationORA( MultiSizeInt src )
{
	accumulatorRegister |= src.Data.Part8.b1; 
	SetSignAndZero( accumulatorRegister );
}


void Processor6502::OperationPHA( CPUMemory* cpuMemory )
{
	IncrementSubCycle();
	cpuMemory->Read8( programCounter + 1 );

	IncrementSubCycle( true );
	PushStack( cpuMemory, accumulatorRegister );
	IncrementStackCounter();
}


void Processor6502::OperationPHP( CPUMemory* cpuMemory )
{
	IncrementSubCycle();
	cpuMemory->Read8( programCounter + 1 );

	IncrementSubCycle( true );
	PushStack( cpuMemory, (( mStatusRegister.Byte) | 0x10 ) );
	IncrementStackCounter();
}


void Processor6502::OperationPLA( CPUMemory* cpuMemory )
{
	IncrementSubCycle();
	cpuMemory->Read8( programCounter + 1 );

	IncrementSubCycle();
	DecrementStackCounter();

	IncrementSubCycle( true );
	accumulatorRegister = PopStack( cpuMemory );

	SetSignAndZero( accumulatorRegister );
}


void Processor6502::OperationPLP( CPUMemory* cpuMemory )
{
	mWaitOneInstructionAfterCli = mStatusRegister.Status.Interrupt == true;

	IncrementSubCycle();
	cpuMemory->Read8( programCounter + 1 );

	IncrementSubCycle();
	DecrementStackCounter();

	IncrementSubCycle( true );
	mStatusRegister.Byte = PopStack( cpuMemory );

	mStatusRegister.Status.Break = false;
	mStatusRegister.Status.Unused = true;

	mWaitOneInstructionAfterCli &= mStatusRegister.Status.Interrupt == false;
}


MultiSizeInt Processor6502::OperationROL( MultiSizeInt src )
{
	unsigned int t = ( src.Data.Part8.b1 << 1 ) | ( mStatusRegister.Status.Carry ? 1 : 0 );
	SetCarry( t );
	t &= 0xff;
	SetSignAndZero( t );
	return t;
}


MultiSizeInt Processor6502::OperationROR( MultiSizeInt src )
{
	Byte_t t = ( src.Data.Part8.b1 >> 1 ) | ( mStatusRegister.Status.Carry ? 0x80 : 0 );
	mStatusRegister.Status.Carry = ( src.Data.Part8.b1 & 0x1 );
	SetSignAndZero( t );
	return t;
}


void Processor6502::OperationRTS( CPUMemory* cpuMemory )
{
	// dummy read
	IncrementSubCycle();
	cpuMemory->Read8( programCounter );

	IncrementSubCycle();
	DecrementStackCounter();

	IncrementSubCycle();
	programCounter = PopStack( cpuMemory );
	DecrementStackCounter();

	IncrementSubCycle();
	programCounter |= PopStack( cpuMemory ) << 8;

	IncrementSubCycle( true );
	programCounter++;
}


void Processor6502::OperationSBC( MultiSizeInt src )
{
	unsigned int temp = accumulatorRegister - src.Data.Part8.b1 - (mStatusRegister.Status.Carry ? 0 : 1);

	if ( mCmosVersion || !DecimalFlagEnabled() )
	{ // Bug in non-cmos version: These flags aren't set if decimal mode is on
		SetSignAndZero( temp );
		mStatusRegister.Status.Overflow = (((accumulatorRegister ^ temp) & 0x80) && ((accumulatorRegister ^ src.Data.Part8.b1) & 0x80));
	}
	SetCarrySubstract( temp );
	accumulatorRegister = (temp & 0xff);
}


void Processor6502::OperationSEC()
{
	mStatusRegister.Status.Carry = true;
}


void Processor6502::OperationSED()
{
	mStatusRegister.Status.Decimal = true;
}


void Processor6502::OperationSEI()
{
	mStatusRegister.Status.Interrupt = true;
}


Byte_t Processor6502::OperationSTA( )
{
	return accumulatorRegister;
}


Byte_t Processor6502::OperationSTX( )
{
	return xRegister;
}


Byte_t Processor6502::OperationSTY( )
{
	return yRegister;
}


void Processor6502::OperationTAX()
{
	xRegister = accumulatorRegister;
	SetSignAndZero( xRegister );
}


void Processor6502::OperationTAY()
{
	yRegister = accumulatorRegister;
	SetSignAndZero( yRegister );
}


void Processor6502::OperationTSX()
{
	xRegister = stackPointer;
	SetSignAndZero( xRegister );
}


void Processor6502::OperationTXA()
{
	accumulatorRegister = xRegister;
	SetSignAndZero( accumulatorRegister );
}


void Processor6502::OperationTXS()
{
	stackPointer = xRegister;
}


void Processor6502::OperationTYA()
{
	accumulatorRegister = yRegister;
	SetSignAndZero( accumulatorRegister );
}

/////////////////////////////////////////////////////////
// Unofficial opcodes


void Processor6502::OperationSAX( MultiSizeInt val )
{
	/*
SAX ANDs the contents of the A and X registers (leaving the contents of A 
intact), subtracts an immediate value, and then stores the result in X.
... A few points might be made about the action of subtracting an immediate 
value.  It actually works just like the CMP instruction, except that CMP 
does not store the result of the subtraction it performs in any register.  
This subtract operation is not affected by the state of the Carry flag, 
though it does affect the Carry flag.  It does not affect the Overflow 
flag.
	*/
	//imm();
	//cpu._addr = (cpu.a & cpu.x) - cpu.tmp;
	//cpu.fz = cpu.fn = cpu.x = cpu._addr & 0xFF;
 //   cpu.fz = !cpu.fz
	//cpu.fc = !(cpu._addr & 0x100);

	unsigned int t = ( accumulatorRegister & xRegister ) - val.Data.Part8.b1;
	xRegister = t & 0xFF;
	SetCarrySubstract( t );
	SetSignAndZero( t );

	//UInt32_t temp = accumulatorRegister & xRegister;
	//temp -= val;
	//xRegister = temp & 0xFF;
	//mStatusRegister.Status.Carry = ( temp < 0x100 );
	//mStatusRegister.Status.Zero = ( (temp & 0xff) == 0 );
	//mStatusRegister.Status.Sign = ( (temp & (0x01 << 7)) > 0 );


	//// TODO: sort the above out, not important due to the fact it's an unofficial instruction
	//MultiSizeInt temp = OperationSTA();
	//OperationTXA();
	//OperationAND( temp );
	//OperationSEC();
	//OperationSBC( val, false );
	//OperationTAX();
	//OperationLDA( temp );
}


void Processor6502::OperationHLT()
{
}

// TODO: implement these instructions

Byte_t Processor6502::OperationTAS()
{
	stackPointer = xRegister & accumulatorRegister;
	return 0;
}


MultiSizeInt Processor6502::OperationSAY( MultiSizeInt val )
{
	const Byte_t result = yRegister & ( mSAYHighByte + 1 );
	return result;
}


Byte_t Processor6502::OperationXAS()
{
//	throw std::exception( "XAS not implemented" );
	return 0;
}


Byte_t Processor6502::OperationAXA()
{
//	throw std::exception( "AXA not implemented" );
	return 0;
}


void Processor6502::OperationLAS( MultiSizeInt val )
{
//	throw std::exception( "LAS not implemented" );
}


MultiSizeInt Processor6502::OperationRLA( MultiSizeInt val )
{
	unsigned int t = ( val.Data.Part8.b1 << 1 ) | ( mStatusRegister.Status.Carry ? 1 : 0 );
	SetCarry( t );
	t &= 0xff;
	accumulatorRegister &= t;
	SetSignAndZero( accumulatorRegister );
	return t;
}


MultiSizeInt Processor6502::OperationLSE( MultiSizeInt val )
{
	mStatusRegister.Status.Carry = ((val.Data.Part8.b1 & 0x01) > 0);
	const Byte_t temp = val.Data.Part8.b1 >> 1;
	accumulatorRegister ^= temp;
	SetSignAndZero( accumulatorRegister );
	return temp;
}


MultiSizeInt Processor6502::OperationRRA( MultiSizeInt val )
{
	Byte_t t = ( val.Data.Part8.b1 >> 1 ) | ( mStatusRegister.Status.Carry ? 0x80 : 0 );
	mStatusRegister.Status.Carry = ( val.Data.Part8.b1 & 0x1 );

	unsigned int newAccumulator = t + accumulatorRegister + (mStatusRegister.Status.Carry ? 1 : 0);
	SetCarry( newAccumulator );

	if ( mCmosVersion || !DecimalFlagEnabled() )
	{ // Bug in non-cmos version: These flags aren't set if decimal mode is on
		SetSignAndZero( (Byte_t)newAccumulator );
		mStatusRegister.Status.Overflow = (!((accumulatorRegister ^ t) & 0x80) && ((accumulatorRegister ^ newAccumulator) & 0x80));
	}

	accumulatorRegister = newAccumulator & 0xFF;

	return t;
}


Byte_t Processor6502::OperationAXS()
{
	return accumulatorRegister & xRegister;
}


void Processor6502::OperationLAX( MultiSizeInt val )
{
	accumulatorRegister = val.Data.Part8.b1;
	xRegister = val.Data.Part8.b1;
	SetSignAndZero( accumulatorRegister );
}


MultiSizeInt Processor6502::OperationDCM( MultiSizeInt val )
{
	const Byte_t decremented = val.Data.Part8.b1 - 1;

	unsigned int t = accumulatorRegister - decremented;
	SetCarrySubstract( t );
	SetSignAndZero( (Byte_t)t );
	return decremented;
}


MultiSizeInt Processor6502::OperationINS( MultiSizeInt val )
{
	Byte_t ret = val.Data.Part8.b1 + 1;
	SetSignAndZero( ret );

	unsigned int temp = accumulatorRegister - ret - (mStatusRegister.Status.Carry ? 0 : 1);

	if ( mCmosVersion || !DecimalFlagEnabled() )
	{ // Bug in non-cmos version: These flags aren't set if decimal mode is on
		SetSignAndZero( temp );
		mStatusRegister.Status.Overflow = (((accumulatorRegister ^ temp) & 0x80) && ((accumulatorRegister ^ ret) & 0x80));
	}
	SetCarrySubstract( temp );
	accumulatorRegister = (temp & 0xff);

	return ret;
}


void Processor6502::OperationALR( MultiSizeInt val )
{
	accumulatorRegister &= val.Data.Part8.b1;

	mStatusRegister.Status.Carry = ((accumulatorRegister & 0x01) > 0);
	accumulatorRegister = accumulatorRegister >> 1;
	SetSignAndZero( accumulatorRegister );
}


void Processor6502::OperationANC( MultiSizeInt val )
{
	accumulatorRegister &= val.Data.Part8.b1;
	SetSignAndZero( accumulatorRegister );
	mStatusRegister.Status.Carry = mStatusRegister.Status.Sign;
}


void Processor6502::OperationARR( MultiSizeInt val )
{
	accumulatorRegister &= val.Data.Part8.b1;
	
	accumulatorRegister = ( accumulatorRegister >> 1 ) | ( mStatusRegister.Status.Carry ? 0x80 : 0 );
	mStatusRegister.Status.Carry = ( accumulatorRegister & 0x1 );
	SetSignAndZero( accumulatorRegister );

	mStatusRegister.Status.Overflow = mStatusRegister.Status.Carry = false;
	switch ( accumulatorRegister & 0x60 )
	{
	case 0x20: mStatusRegister.Status.Overflow = true; break;
	case 0x40: mStatusRegister.Status.Overflow = mStatusRegister.Status.Carry = true; break;
	case 0x60: mStatusRegister.Status.Carry = true; break;
	}
}


/////////////////////////////////////////////////////////////////////////////////////////


void Processor6502::PushStack( CPUMemory* cpuMemory, MultiSizeInt val )
{
	cpuMemory->Write8( 0x100 + stackPointer, val.Data.Part8.b1 );
}


Byte_t Processor6502::PopStack( CPUMemory* cpuMemory )
{
	return cpuMemory->Read8( 0x100 + stackPointer );
}


void Processor6502::IncrementStackCounter()
{
	stackPointer--;
}


void Processor6502::DecrementStackCounter()
{
	stackPointer++;
}


/////////////////////////////////////////////////////////////////////////////////////////


#endif
