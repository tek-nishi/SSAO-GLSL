#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class VertexColorApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void VertexColorApp::setup()
{
}

void VertexColorApp::mouseDown( MouseEvent event )
{
}

void VertexColorApp::update()
{
}

void VertexColorApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( VertexColorApp, RendererGl )
