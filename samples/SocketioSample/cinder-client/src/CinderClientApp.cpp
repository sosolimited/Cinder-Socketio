#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/log.h"

#include "sio_client.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderClientApp : public App {
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseUp( MouseEvent event ) override;
	void update() override;
	void draw() override;
	
	void handleMouseDownEvent(const sio::event &ev);
	
	string endpoint = "http://localhost:7778";
	shared_ptr<sio::client> sio_client;
	bool mouse_down = false;
	vec2 mouse_position;
};

// convenience functions for parsing socketio message objects
namespace {
	pair<vec2,bool> parsePositionMessage(sio::message::ptr msg){
		pair<vec2,bool> failure_pair;
		failure_pair.first = vec2(0,0);
		failure_pair.second = false;
		
		if( msg->get_flag() != sio::message::flag_object ){
			return failure_pair;
		}
		
		auto coords = msg->get_map()["pos"];
		
		// we're expecting ["pos"] to hold a vector with 2 items
		if( !coords || coords->get_flag() != sio::message::flag_array || coords->get_vector().size() != 2 ){
			return failure_pair;
		}
		
		auto x_msg = coords->get_vector()[0];
		auto y_msg = coords->get_vector()[1];
		
		// above items should be doubles
		if( x_msg->get_flag() != sio::message::flag_double || y_msg->get_flag() != sio::message::flag_double ){
			return failure_pair;
		}
		
		pair<vec2,bool> xy_pair;
		xy_pair.first = vec2( x_msg->get_double(), y_msg->get_double() );
		xy_pair.second = true;
		
		return xy_pair;
	}
}

void CinderClientApp::setup()
{
	sio_client = make_shared<sio::client>();
	
	//
	// setup socketio client connectivity events
	//
	sio_client->set_open_listener([this] {
		CI_LOG_I("Connected to socketio endpoint @ " << endpoint);
	});
	
	sio_client->set_reconnect_listener([this](unsigned, unsigned) {
		CI_LOG_I("Reconnected to socketio endpoint @ " << endpoint);
	});
	
	sio_client->set_close_listener([](sio::client::close_reason reason) {
		CI_LOG_E("Socketio connection closed for reason: " << reason);
	});
	
	sio_client->set_socket_close_listener([](const std::string &nsp) {
		CI_LOG_E("Socketio namespace closed: " << nsp);
	});
	
	//
	// Setup socketio message events.
	// Note: the "socket()" accessor is not referring to a physical socket, but rather
	// a socketio namespace, or logical channel. Our socketio client can have multiple
	// "sockets" (channels) but is always connected to a single physical socket.
	//
	sio_client->socket()->on("mouse down", [this](sio::event& ev){
		CI_LOG_V("received socketio mouse down event");
		
		// force code to run on the main thread
		dispatchAsync([this,ev]{
			handleMouseDownEvent(ev);
		});
	});
	
	sio_client->socket()->on("mouse up", [this](sio::event& ev){
		CI_LOG_V("received socketio mouse up event");
		
		// force code to run on the main thread
		dispatchAsync([this]{
			mouse_down = false;
		});
	});
	
	// connect to our socketio Node server
	sio_client->connect(endpoint);
}

void CinderClientApp::mouseDown( MouseEvent event ){
	// Tell the socketio server we pressed the mouse; attach data about the position.
	
	// What we're doing here is constructing an object to send over socketio, which
	// eventually transforms into a JSON string read by the socketio server.
	auto coords = sio::object_message::create();
	
	auto xy = sio::array_message::create();
	xy->get_vector().push_back( sio::double_message::create(event.getX()/(double)getWindowSize()[0]) );
	xy->get_vector().push_back( sio::double_message::create(event.getY()/(double)getWindowSize()[1]) );
	
	coords->get_map()["pos"] = xy;
	
	// send the object with the "mouse down" message identifier
	sio_client->socket()->emit("mouse down", coords);
}

void CinderClientApp::mouseUp( MouseEvent event ){
	// tell the socketio server we released the mouse
	sio_client->socket()->emit("mouse up");
}

void CinderClientApp::update()
{
}

void CinderClientApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	
	// draw mouse click position based on coordinates from socketio events
	if( mouse_down ){
		gl::ScopedModelMatrix scoped_model_matrix;
		
		gl::translate( mouse_position );
		gl::rotate(45 * M_PI / 180);
		
		gl::color( 1, 0, 0 );
		gl::drawSolidRect( Rectf(-20,-2,20,2) );
		gl::drawSolidRect( Rectf(-2,-20,2,20) );
	}
}

// Called when we receive a mouse down event from socketio.
// We have to carefully examine the data returned by the event to ensure it
// is what we expect to receive.
// expected format:
// {
//   "pos": [x,y]
// }
void CinderClientApp::handleMouseDownEvent(const sio::event &ev){
	auto msg = ev.get_message();
	
	auto xy_pair = parsePositionMessage(msg);
	
	// if failure flag set (invalid message format)
	if( !xy_pair.second ){
		CI_LOG_E("received socketio mouse down event message in unexpected format");
		return;
	}
	
	mouse_position.x = xy_pair.first.x * getWindowSize()[0];
	mouse_position.y = xy_pair.first.y * getWindowSize()[1];
	
	mouse_down = true;
}

CINDER_APP( CinderClientApp, RendererGl )
