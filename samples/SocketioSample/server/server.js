var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);

var port = 7778;

app.get('/', function(req,res) {
	res.sendFile(__dirname + '/static/index.html');
});

io.on('connection', function(socket){
	var sid = socket.id.substr(2);

	console.log("(io) client %s connected", sid);

	socket.on("mouse down", function(msg){
		console.log("(io) client %s clicked [%d,%d]", sid, msg.pos[0], msg.pos[1]);
		io.sockets.emit("mouse down", msg);
	});

	socket.on("mouse up", function(){
		console.log("(io) client %s released mouse", sid);
		io.sockets.emit("mouse up");
	});

	socket.on("disconnect", function(){
		console.log("(io) client %s disconnected", sid);
	});
});

http.listen(port, function(){
	console.log("Server listening @ http://localhost:" + port + "/");
});