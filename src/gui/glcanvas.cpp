
#include "stdafx.h"
#include "app.h"
#include "glcanvas.h"
#include "../mainboard.h"



BEGIN_EVENT_TABLE(GLCanvas, wxGLCanvas)
		EVT_IDLE(GLCanvas::OnIdle)
		EVT_SIZE(GLCanvas::OnSize)

		EVT_KEY_DOWN(GLCanvas::OnKeyDown)
		EVT_KEY_UP(GLCanvas::OnKeyUp)

		EVT_LEFT_DOWN( GLCanvas::OnMouseLeftDown )
		EVT_LEFT_UP( GLCanvas::OnMouseLeftUp )

END_EVENT_TABLE()


GLCanvas::GLCanvas( wxWindow* parent, long style, int* attribList )
	: wxGLCanvas( parent, -1, wxDefaultPosition, wxDefaultSize, style, "GLCanvas", attribList)
{
	mAttachedRenderBuffer = NULL;
	zoomx = zoomy = 1.0f;

	SetZoomSize( GetClientSize().GetWidth(), GetClientSize().GetHeight() );

	g_options->AttachToSyncEvent( boost::bind( &GLCanvas::SyncWithOptions, this ) );
}


GLCanvas::~GLCanvas()
{
}


void GLCanvas::SyncWithOptions()
{
	SetZoomSize( GetClientSize().GetWidth(), GetClientSize().GetHeight() );
}


void GLCanvas::CoordinatesToCanvas( int x, int y, int& outx, int& outy ) const
{
	outx = (int)((float)x / zoomx);
	outy = (int)((float)y / zoomy);
}


void GLCanvas::OnIdle(wxIdleEvent& ev)
{
}


void GLCanvas::AttachRenderBuffer( IRenderBuffer* renderBuffer )
{
	mAttachedRenderBuffer = renderBuffer;
}


void GLCanvas::SetZoomSize( int w, int h )
{
	int x = 256, y = 240;
	if ( mAttachedRenderBuffer != NULL )
		mAttachedRenderBuffer->GetBufferSize( x, y );

	zoomx = ((float)w) / ((float)x);
	zoomy = ((float)h) / ((float)y);
}


void GLCanvas::OnSize(wxSizeEvent& ev)
{
	SetZoomSize( ev.GetSize().GetWidth(), ev.GetSize().GetHeight() );
}


void GLCanvas::OnKeyUp(wxKeyEvent& ev)
{
	if ( g_activeMainboard != NULL )
		g_activeMainboard->GetInputBus()->OnKeyUp( ev.GetKeyCode() );
}


void GLCanvas::OnKeyDown(wxKeyEvent& ev)
{
	if ( g_activeMainboard != NULL )
		g_activeMainboard->GetInputBus()->OnKeyDown( ev.GetKeyCode() );
}


void GLCanvas::OnMouseLeftDown( wxMouseEvent& ev )
{
	int x, y;
	CoordinatesToCanvas( ev.GetX(), ev.GetY(), x, y );
	if ( g_activeMainboard != NULL )
		g_activeMainboard->GetInputBus()->OnMouseLeftDown( x, y );
	ev.Skip();
}


void GLCanvas::OnMouseLeftUp( wxMouseEvent& ev )
{
	int x, y;
	CoordinatesToCanvas( ev.GetX(), ev.GetY(), x, y );
	if ( g_activeMainboard != NULL )
		g_activeMainboard->GetInputBus()->OnMouseLeftUp( x, y );
}


void GLCanvas::Render()
{
	if ( GetParent()->IsShown() && mAttachedRenderBuffer != NULL )
	{
		SetCurrent();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glDisable(GL_DITHER);

		int w = this->GetSize().GetWidth();
		int h = this->GetSize().GetHeight();

		int x = 256, y = 240;
		mAttachedRenderBuffer->GetBufferSize( x, y );

		glViewport(0, 0, (GLsizei)w, (GLsizei)h );

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho( 0, w, 0, h, -1, 1 );

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRasterPos2f( 0.0f, 0.0f );
		glPixelZoom( zoomx, zoomy );

		glDrawPixels( x, y, GL_RGB, GL_UNSIGNED_BYTE, mAttachedRenderBuffer->GetRGBBuffer( true ) );

		SwapBuffers();
	}
}

