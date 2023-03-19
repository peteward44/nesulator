

#ifndef MAINBOARD_H
#define MAINBOARD_H


#include "ppu.h"
#include "apu.h"
#include "cpumemory.h"
#include "processor6502.h"
#include "mappers.h"
#include "input.h"
#include "cartridge.h"
#include "renderbuffer.h"
#include "modeconstants.h"
#include "options.h"
#include "imainboard.h"
#include "MainboardCallback.h"
#include "boost/cstdint.hpp"

// MainBoard - represents the NES itself.
// All subcomponents (CPU, PPU, etc, are housed here)


class MainBoard : public IMainBoard, private boost::noncopyable
{
private:
	CartridgePtr_t cartridge;
	Synchroniser* synchroniser;
	InputDeviceBus inputDeviceBus;

	IModeConstants* modeConstants;
	CPUMemory* cpumemory;
	PictureProcessingUnit* ppu;
	APU* apu;
	Processor6502* processor;

	boost::uint32_t ticksLastFrame, targetTicks, ticksThisSecond, ticksIncrement;
	int frameCount;

	bool running, paused, stepFrame, takeScreenshot;

	std::wstring GetScreenshotFilename();

	RenderBuffer* renderBuffer;
	MainBoardCallbackInterface* callbackInterface;


public:
	typedef boost::signals2::signal< void (bool) > ResetEvent_t;
	ResetEvent_t ResetEvent;


	MainBoard( MainBoardCallbackInterface* callbackInterface );
	~MainBoard();
		
	virtual void LoadCartridge( const std::wstring& path );
	virtual ICartridge* GetCartridge() { return cartridge.get(); }

	void CreateComponents();

	virtual std::wstring GetName() const
	{ return cartridge.get() != NULL ? cartridge->GetName() : L""; }

	virtual IInputBus* GetInputBus() { return &inputDeviceBus; }

	bool StartNewFrame();

	FORCE_INLINE CPUMemory* GetCPUMemory()
	{ return cpumemory; }

	FORCE_INLINE PictureProcessingUnit* GetPPU()
	{ return ppu; }

	FORCE_INLINE APU* GetAPU()
	{ return apu; }

	FORCE_INLINE Processor6502* GetProcessor()
	{ return processor; }
	
	FORCE_INLINE MemoryMapperPtr_t GetMemoryMapper()
	{ return cartridge->GetMemoryMapper(); }

	FORCE_INLINE InputDeviceBus* GetInputDeviceBus()
	{ return &inputDeviceBus; }

	FORCE_INLINE Synchroniser* GetSynchroniser()
	{ return synchroniser; }

	virtual IRenderBuffer* GetRenderBuffer() { return renderBuffer; }
	FORCE_INLINE RenderBuffer* GetNesRenderBuffer() { return renderBuffer; }

	FORCE_INLINE const IModeConstants* GetModeConstants() const
	{ return modeConstants; }

	FORCE_INLINE void Pause( bool pause ) { paused = pause; }
	FORCE_INLINE bool IsPaused() const { return paused; }

	FORCE_INLINE void StepFrame( bool step ) { stepFrame = step; }
	FORCE_INLINE bool IsStepFrame() const { return stepFrame; }

	FORCE_INLINE void TakeScreenshot() { takeScreenshot = true; }

	virtual void Reset( bool cold );

	virtual void OnChangeSpeed();

	virtual void DoLoop();
	virtual void StopLoop();

	virtual void Shutdown();

	virtual void SaveState( const std::wstring& filename );
	virtual void LoadState( const std::wstring& filename );

	virtual void SetTerritoryMode( TERRITORY_MODE mode );
	virtual TERRITORY_MODE GetTerritoryMode() const { return modeConstants->Mode(); }

	virtual bool IsRunning() const { return running; }
	FORCE_INLINE void SleepTime( int milli ) { callbackInterface->Sleep( milli ); }

};


extern MainBoard* g_nesMainboard;


#endif

