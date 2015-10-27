
#ifndef IMAINBOARD_H
#define IMAINBOARD_H

#include "main.h"
#include "irenderbuffer.h"
#include "icartridge.h"
#include "iinputbus.h"


class IMainBoard
{
public:
	virtual ~IMainBoard() {}

	virtual void LoadCartridge( const std::string& path ) = 0 {}
	virtual ICartridge* GetCartridge() = 0 {}

	virtual void DoLoop() = 0 {}
	virtual void StopLoop() = 0 {}
	virtual void Shutdown() = 0 {}

	virtual bool IsRunning() const = 0 {}

	virtual void SaveState( const std::string& filename ) = 0 {}
	virtual void LoadState( const std::string& filename ) = 0 {}

	virtual IInputBus* GetInputBus() = 0 {}
	virtual IRenderBuffer* GetRenderBuffer() = 0 {}

	virtual void OnChangeSpeed() = 0 {}

	virtual void Reset( bool cold ) = 0 {}

	virtual void TakeScreenshot() {}

	virtual void SetTerritoryMode( TERRITORY_MODE mode ) = 0 {}
	virtual TERRITORY_MODE GetTerritoryMode() const = 0 {}

};



#endif

