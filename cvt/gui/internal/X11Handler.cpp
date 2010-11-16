#include <cvt/gui/internal/X11Handler.h>

namespace cvt {

	void X11Handler::onDataReadable()
	{
		XEvent xevent;
		Event* e;
		WidgetImplWinGLX11* win;

		while( XPending( _dpy ) ) {
			XNextEvent( _dpy, &xevent );
			switch( xevent.type ) {
				case ConfigureNotify:
					{
						while( XCheckTypedWindowEvent( _dpy, xevent.xconfigure.window, ConfigureNotify, &xevent ) )
							;
						win = (*_windows)[ xevent.xconfigure.window ];
						int oldwidth = win->rect.width;
						int oldheight = win->rect.height;

						if( oldwidth != xevent.xconfigure.width || oldheight != xevent.xconfigure.height ) {
							ResizeEvent re( xevent.xconfigure.width, xevent.xconfigure.height, oldwidth, oldheight );
							win->resizeEvent( &re );
						}
						if( xevent.xconfigure.send_event ) {
							int oldx = win->rect.x;
							int oldy = win->rect.y;

							if( oldx != xevent.xconfigure.x  || oldy != xevent.xconfigure.y ) {
								MoveEvent me( xevent.xconfigure.x, xevent.xconfigure.y, oldx, oldy );
								win->moveEvent( &me );
							}
						}
					}
					break;
				case ReparentNotify:
					{
						win = (*_windows)[ xevent.xreparent.window ];
						::Window child;
						int gx, gy;
						int oldx = win->rect.x;
						int oldy = win->rect.y;

						XTranslateCoordinates( _dpy, xevent.xreparent.window, RootWindow( _dpy, DefaultScreen( _dpy ) ), 0, 0, &gx, &gy, &child );
						if( oldx != gx  || oldy != gy ) {
							MoveEvent me( gx, gy, 0, 0 );
							win->moveEvent( &me );
						}
					}
					break;
				case MapNotify:
					{
						ShowEvent se;

						win = (*_windows)[ xevent.xmap.window ];
						win->showEvent( &se );
					}
					break;
				case UnmapNotify:
					{
						HideEvent he;

						win = (*_windows)[ xevent.xmap.window ];
						win->hideEvent( &he );
					}
					break;
				case Expose:
					{
						win = (*_windows)[ xevent.xexpose.window ];

						// Compress resize/motions before sending expose events
						// FIXME
/*						int cfevent = 0;
						XEvent xevent2;
						while( ( cfevent = XCheckTypedWindowEvent( _dpy, xevent.xexpose.window, ConfigureNotify, &xevent2 ) ) )
							;
						if( cfevent ) {
							ResizeEvent re( xevent2.xconfigure.width, xevent2.xconfigure.height, 0, 0 );
							enqueue( xevent2.xconfigure.window, e );
							if( xevent2.xconfigure.send_event ) {
								e = new MoveEvent( xevent2.xconfigure.x, xevent2.xconfigure.y, 0, 0 );
								enqueue( xevent2.xconfigure.window, e );
							}
						}*/

						// Compress expose
						// FIXME: fix the rect information
						while( XCheckTypedWindowEvent( _dpy, xevent.xexpose.window, Expose, &xevent ) )
							;
						PaintEvent pe( xevent.xexpose.x, xevent.xexpose.y, xevent.xexpose.width, xevent.xexpose.height );
						win->paintEvent( &pe );
					}
					break;
				case ButtonPress:
					{
						win = ( *_windows )[ xevent.xbutton.window ];
						MousePressEvent mp( xevent.xbutton.x, xevent.xbutton.y, xevent.xbutton.button );
						( ( Window* ) win->widget )->mousePressEvent( &mp );
					}
					break;
				case ButtonRelease:
					{
						win = ( *_windows )[ xevent.xbutton.window ];
						MouseReleaseEvent mr( xevent.xbutton.x, xevent.xbutton.y, xevent.xbutton.button );
						( ( Window* ) win->widget )->mouseReleaseEvent( &mr );
					}
					break;
				case MotionNotify:
					{
						win = ( *_windows )[ xevent.xmotion.window ];
						while( XCheckTypedWindowEvent( _dpy, xevent.xmotion.window, MotionNotify, &xevent ) )
							;
						MouseMoveEvent mme( xevent.xmotion.x, xevent.xmotion.y );
						( ( Window* ) win->widget )->mouseMoveEvent( &mme );
					}
					break;
				case ClientMessage:
					{
						if( xevent.xclient.message_type == xatom_wmproto && ( ::Atom ) xevent.xclient.data.l[0] == xatom_wmdelete ) {
							CloseEvent ce;
							win = ( *_windows )[ xevent.xclient.window ];
							( ( Window* ) win->widget )->closeEvent( &ce );
						}
					}
					break;
				default:
					break;
			}
		}
	}

}
