
#include "stdafx.h"
#include "main.h"
#include "SnesMainboard.h"



SnesMainboard* g_snesMainboard = NULL;



SnesMainboard::SnesMainboard( MainBoardCallbackInterface* callbackInterface )
{
	mIsRunning = false;
	mCallbackInterface = callbackInterface;
	mCpuMemory = NULL;
	mPpu = NULL;
	mProcessor = NULL;
	mSynchroniser = NULL;
}


SnesMainboard::~SnesMainboard()
{
	delete mCpuMemory;
	mCpuMemory = NULL;
	delete mPpu;
	mPpu = NULL;
	delete mProcessor;
	mProcessor = NULL;
	delete mSynchroniser;
	mSynchroniser = NULL;
}


void SnesMainboard::CreateComponents()
{
	mSynchroniser = new SnesSynchroniser;
	mCpuMemory = new SnesCpuMemory;
	mPpu = new SnesPpu;
	mProcessor = new Processor65C816;
}


void SnesMainboard::LoadCartridge( const std::string& path )
{
	mCartridge = SnesCartidgePtr_t( new SnesCartridge( path, false ) );
}


void SnesMainboard::DoLoop()
{
	mIsRunning = true;

	Reset( true );

	while ( mIsRunning )
	{
		mSynchroniser->RunCycle();
		mCallbackInterface->OnReadyCommand( this );
	}
}


void SnesMainboard::StopLoop()
{
	mIsRunning = false;
}


void SnesMainboard::Shutdown()
{
}


void SnesMainboard::Reset( bool cold )
{
	Log::Write( LOG_MISC, ( boost::format( "%1% reset" ) % ( cold ? "Cold" : "Warm" ) ).str() );
	ResetEvent( cold );
//	renderBuffer->ClearBuffer();
	mProcessor->ResetInterrupt();
}

