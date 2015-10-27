

#ifndef INPUT_H
#define INPUT_H

#include "main.h"
#include "iinputbus.h"


enum INPUT_DEVICE
{
	INPUT_NONE,
	INPUT_JOYPAD,
	INPUT_ZAPPER,
};


class InputDevice : private boost::noncopyable, public HasState
{
private:

public:
	virtual ~InputDevice() {}

	virtual void WriteToRegister( UInt16_t offset, Byte_t data ) {}
	virtual Byte_t ReadFromRegister( UInt16_t offset ) { return 0; }

	virtual void SaveState( std::ostream& ostr ) {}
	virtual void LoadState( std::istream& istr ) {}

	virtual void OnKeyUp( int keyCode ) {}
	virtual void OnKeyDown( int keyCode ) {}

	virtual void OnMouseLeftDown( int x, int y ) {}
	virtual void OnMouseLeftUp( int x, int y ) {}

	virtual INPUT_DEVICE GetDeviceType() const = 0;

	virtual void OnConnection( int port ) {}
	virtual void OnDisconnection( int port ) {}
};


typedef boost::shared_ptr< InputDevice > InputDevicePtr_t;


enum JOYSTICK_BUTTON
{
	JS_A = 0,
	JS_B,
	JS_SELECT,
	JS_START,
	JS_UP,
	JS_DOWN,
	JS_LEFT,
	JS_RIGHT,
	JS_BUTTON_COUNT,
	JS_BUTTON_INVALID = 0xFFFFFFFF
};


class NullDevice : public InputDevice
{
private:

public:
	virtual INPUT_DEVICE GetDeviceType() const { return INPUT_NONE; }
};


class Joypad : public InputDevice
{
private:
	int nextJoystickButton;
	bool joystickHigh, joystickLow;
	int currentPort;

	bool buttons[ JS_BUTTON_COUNT ];

public:
	Joypad();

	virtual void OnConnection( int port );
	virtual void OnDisconnection( int port );

	virtual void WriteToRegister( UInt16_t offset, Byte_t data );
	virtual Byte_t ReadFromRegister( UInt16_t offset );

	virtual void SaveState( std::ostream& ostr );
	virtual void LoadState( std::istream& istr );

	virtual void OnKeyUp( int keyCode );
	virtual void OnKeyDown( int keyCode );

	virtual INPUT_DEVICE GetDeviceType() const { return INPUT_JOYPAD; }
};


class Zapper : public InputDevice
{
private:
	bool triggerHeld;

	bool IsSpriteInCrossHairs() const;
	bool IsWhitePixel( Byte_t paletteIndex ) const;

public:
	Zapper();

	virtual void WriteToRegister( UInt16_t offset, Byte_t data );
	virtual Byte_t ReadFromRegister( UInt16_t offset );

	virtual void SaveState( std::ostream& ostr );
	virtual void LoadState( std::istream& istr );

	virtual void OnMouseLeftDown( int x, int y );
	virtual void OnMouseLeftUp( int x, int y );

	virtual INPUT_DEVICE GetDeviceType() const { return INPUT_ZAPPER; }
};


class InputDeviceBus : private boost::noncopyable, public HasState, public IInputBus
{
private:
	InputDevicePtr_t devices[2];
	
public:
	InputDeviceBus();

	void SyncWithOptions();

	InputDevicePtr_t GetDevice( int port );

	void ConnectDevice( int port, InputDevicePtr_t device );
	void DisconnectDevice( int port );

	void WriteToRegister( UInt16_t offset, Byte_t data );
	Byte_t ReadFromRegister( UInt16_t offset );

	virtual void OnKeyUp( int keyCode );
	virtual void OnKeyDown( int keyCode );

	virtual void OnMouseLeftDown( int x, int y );
	virtual void OnMouseLeftUp( int x, int y );

	virtual void SaveState( std::ostream& ostr );
	virtual void LoadState( std::istream& istr );
};


extern InputDevicePtr_t CreateInputDevice( INPUT_DEVICE device );


#endif
