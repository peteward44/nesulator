
#ifndef APP_H
#define APP_H

#include "gui_shared.h"
#include "../imainboard.h"
#include "../MainboardCallback.h"
#include "../input.h"
#include "mainframe.h"
#include "boost/signals2.hpp"

extern IMainBoard* g_activeMainboard;


class NesulatorApp : public wxApp, public MainBoardCallbackInterface
{
private:
	MainFrame* mainFrame;
	wxStopWatch stopWatch;
	bool mQuitNow;

	boost::signals2::signal< void (IMainBoard*) > ReadyState;

public:
	static void DisplayError( const std::string& error );
	static std::wstring GetApplicationPathWithSep();
	static std::wstring GetConfigFilename();


	NesulatorApp();

	void LoadRomFile( const std::wstring& filename );

	void StartShutdown();

	virtual bool OnInit();
	virtual int OnExit();

	virtual int OnRun();

	virtual void SetFps( float fps );
	virtual bool DoWindowsEvents();
	virtual void OnCartridgeLoad( IMainBoard* mainBoard );
	virtual void OnStopRunning( IMainBoard* mainBoard );
	virtual void Render();

	virtual void OnReadyCommand( IMainBoard* mainBoard );

	virtual void Sleep( int milliSeconds );
	virtual int GetTicks();

	FORCE_INLINE MainFrame* GetMainFrame() const { return mainFrame; }

	template < class FUNCTOR >
	FORCE_INLINE void ConnectToReadySignal( FUNCTOR functor ) { ReadyState.connect( functor ); }
};


DECLARE_APP(NesulatorApp);


#endif
