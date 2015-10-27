
#ifndef MAINBOARDCALLBACK_H
#define MAINBOARDCALLBACK_H


class IMainBoard;


// Used to seperate GUI & core code by letting the GUI call these functions as appropriate. This is passed to the mainboard constructor
class MainBoardCallbackInterface
{
public:
	virtual ~MainBoardCallbackInterface() = 0 {}

	virtual void SetFps( float fps ) = 0;
	virtual bool DoWindowsEvents() = 0;
	virtual void Render() = 0;

	virtual void OnCartridgeLoad( IMainBoard* mainBoard ) = 0;

	virtual void OnStopRunning( IMainBoard* mainBoard ) = 0;

	virtual void OnReadyCommand( IMainBoard* mainBoard ) = 0;

	virtual void Sleep( int milliSeconds ) = 0;
	virtual int GetTicks() = 0;
};



#endif

