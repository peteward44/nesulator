
#ifndef IINPUTBUS_H
#define IINPUTBUS_H


class IInputBus
{
public:
	virtual ~IInputBus() {}

	virtual void OnKeyUp( int keyCode ) = 0 {}
	virtual void OnKeyDown( int keyCode ) = 0 {}
	virtual void OnMouseLeftUp( int x, int y ) = 0 {}
	virtual void OnMouseLeftDown( int x, int y ) = 0 {}


};


#endif

