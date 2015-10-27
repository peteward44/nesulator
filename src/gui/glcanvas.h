
#ifndef GLCANVAS_H
#define GLCANVAS_H

#include "gui_shared.h"
#include "wx/glcanvas.h"
#include "../irenderbuffer.h"


class GLCanvas : public wxGLCanvas
{
private:
	float zoomx, zoomy;
	IRenderBuffer* mAttachedRenderBuffer;

	DECLARE_EVENT_TABLE();


public:
	GLCanvas( wxWindow* parent, long style, int* attribList );
	virtual ~GLCanvas();

	void SyncWithOptions();

	void AttachRenderBuffer( IRenderBuffer* renderBuffer );

	void SetZoomSize( int w, int h );

	void OnIdle(wxIdleEvent& ev);
	void OnSize(wxSizeEvent& ev);

	void OnKeyUp(wxKeyEvent& ev);
	void OnKeyDown(wxKeyEvent& ev);

	void Render();

	void OnMouseLeftDown( wxMouseEvent& ev );
	void OnMouseLeftUp( wxMouseEvent& ev );

	void CoordinatesToCanvas( int x, int y, int& outx, int& outy ) const;
};


#endif

