#include <cvt/io/Camera.h>
#include <cvt/util/Time.h>
#include <cvt/util/Exception.h>

#include <cvt/gui/Application.h>
#include <cvt/gui/Window.h>
#include <cvt/gui/WidgetLayout.h>
#include <cvt/gui/Moveable.h>
#include <cvt/gui/Button.h>
#include <cvt/gui/BasicTimer.h>
#include <cvt/gui/TimeoutHandler.h>
#include "FeatureView.h"

#include <cvt/io/ImageIO.h>

#include <cvt/vision/FAST.h>
#include <cvt/vision/AGAST.h>

using namespace cvt;

class CameraTimeout : public TimeoutHandler
{
	public:
		CameraTimeout( Camera * cam, Moveable* mov, FeatureView * view ) :
			TimeoutHandler(),
			_cam( cam ),
			_moveable( mov ),
			_view( view ),
			_image( cam->width(), cam->height(), IFormat::BGRA_UINT8 ),
			_gray( cam->width(), cam->height(), IFormat::GRAY_UINT8 ),
			_frames( 0.0f ),
			//_detector( SEGMENT_9 )
			//_detector( AGAST_5_8 )
			//_detector( AGAST_7_12D )
			_detector( AGAST_7_12S )
			//_detector( OAST_9_16 )
		{
			_cam->startCapture();
			_timer.reset();
			_processingTime.reset();
			_iter = 0;
			_timeSum = 0;
			//_detector.setMinScore( 25 );
			//_detector.setThreshold( 25 );
		}

		~CameraTimeout()
		{
			_cam->stopCapture();
		}

		void onTimeout()
		{
			_cam->nextFrame();

			_cam->frame().convert( _gray );
			
			{
				static int i = 0;
				char buf[200];
				sprintf( buf, "out_%05d.png", i++ );
				ImageIO::savePNG( gray, buf );								
			}
			
			_view->setImage( _gray );

			std::vector<Feature2D> features;

			_processingTime.reset();
			_detector.extract( _gray, features );

			_timeSum += _processingTime.elapsedMilliSeconds();

			_iter++;
			if( (_iter % 100) == 0 )
				std::cout << "Avg. Feature extraction time: " << _timeSum / _iter << "ms" << std::endl;

			_view->setFeatures( features );

			_frames++;
			if( _timer.elapsedSeconds() > 5.0f ) {
				char buf[ 200 ];
				sprintf( buf,"Camera FPS: %.2f", _frames / _timer.elapsedSeconds() );
				_moveable->setTitle( buf );
				_frames = 0;
				_timer.reset();
			}
		}

	private:
		Camera *		_cam;
		Moveable*		_moveable;
		FeatureView *	_view;
		Image			_image;
		Image			_gray;
		Time			_timer;
		Time			_processingTime;
		size_t			_iter;
		double			_timeSum;
		float			_frames;
		//FAST			_detector;
		AGAST			_detector;
};

int main( )
{
	Camera::updateInfo();
	size_t numCams = Camera::count();

	std::cout << "Overall number of Cameras: " << numCams << std::endl;
	if( numCams == 0 ){
		std::cout << "Please connect a camera!" << std::endl;
		return 0;
	}

	for( size_t i = 0; i < numCams; i++ ){
		const CameraInfo & info = Camera::info( i );
		std::cout << "Camera " << i << ": " << info << std::endl;
	}

	size_t selection = numCams;
	std::cout << "Select camera: ";
	std::cin >> selection;
	while ( selection >= numCams ){
		std::cout << "Index out of bounds -> select camera in Range:";
		std::cin >> selection;
	}

	Window w( "Camera Test" );
	w.setSize( 800, 600 );
	w.setVisible( true );
	w.setMinimumSize( 320, 240 );

	Button button( "Quit" );
	Delegate<void ()> dquit( &Application::exit );
	button.clicked.add( &dquit );
    WidgetLayout wl;
    wl.setAnchoredRight( 10, 50 );
    wl.setAnchoredBottom( 10, 20 );
    w.addWidget( &button, wl );

	Camera * cam = 0;

	try {
		cam = Camera::get( selection, 640, 480, 60, IFormat::UYVY_UINT8 );
		FeatureView camView;
		Moveable m( &camView );
		m.setTitle( "Camera" );
		m.setSize( 320, 240 );
		w.addWidget( &m );

		CameraTimeout camTimeOut( cam, &m, &camView );
		uint32_t timerId = Application::registerTimer( 25, &camTimeOut );
		Application::run();
		Application::unregisterTimer( timerId );

	} catch( cvt::Exception e ) {
		std::cout << e.what() << std::endl;
	}


	if(cam)
		delete cam;

	return 0;
}