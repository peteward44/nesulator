

#ifndef SNESMAINBOARD_H
#define SNESMAINBOARD_H

#include "SnesCartridge.h"
#include "SnesCpuMemory.h"
#include "SnesPpu.h"
#include "processor65C816.h"
#include "SnesSynchroniser.h"
#include "MainboardCallback.h"
#include "imainboard.h"
#include "irenderbuffer.h"
#include "iinputbus.h"


class SnesMainboard : public IMainBoard, public IInputBus
{
	SnesCartidgePtr_t mCartridge;

	SnesPpu *mPpu;
	SnesCpuMemory* mCpuMemory;
	SnesSynchroniser* mSynchroniser;
	Processor65C816* mProcessor;

	bool mIsRunning;

	MainBoardCallbackInterface* mCallbackInterface;

public:
	typedef boost::signal1< void, bool > ResetEvent_t;
	ResetEvent_t ResetEvent;


	SnesMainboard( MainBoardCallbackInterface* callbackInterface );
	~SnesMainboard();

	void CreateComponents();

	FORCE_INLINE Processor65C816* GetProcessor() { return mProcessor; }
	FORCE_INLINE SnesCpuMemory* GetCPUMemory() { return mCpuMemory; }
	FORCE_INLINE SnesPpu* GetPPU() { return mPpu; }
	FORCE_INLINE SnesSynchroniser* GetSynchroniser() { return mSynchroniser; }

	virtual void LoadCartridge( const std::string& path );
	virtual ICartridge* GetCartridge() { return mCartridge.get(); }
	virtual SnesCartidgePtr_t GetSnesCartridge() { return mCartridge; }

	virtual void DoLoop();
	virtual void StopLoop();
	virtual void Shutdown();

	virtual bool IsRunning() const { return true; }

	
	virtual void SaveState( const std::string& filename ) {}
	virtual void LoadState( const std::string& filename ) {}

	virtual IInputBus* GetInputBus() { return this; }

	virtual void OnKeyUp( int keyCode ) {}
	virtual void OnKeyDown( int keyCode ) {}
	virtual void OnMouseLeftUp( int x, int y ) {}
	virtual void OnMouseLeftDown( int x, int y ) {}

	virtual IRenderBuffer* GetRenderBuffer() { return NULL; }

	virtual void OnChangeSpeed() {}
	virtual void Reset( bool cold );

	virtual void SetTerritoryMode( TERRITORY_MODE mode ) {}
	virtual TERRITORY_MODE GetTerritoryMode() const { return TERRITORY_NTSC; }

};



extern SnesMainboard* g_snesMainboard;



#endif

