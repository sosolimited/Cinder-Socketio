# Socketio Sample
This sample demonstrates how to write C++ code to talk to a Socketio server. Included is a Cinder app that acts as a Socketio client, a small webpage that acts as an additional Socketio client, and the Socketio server itself (Node script).

## What is Socketio?
Socketio is a realtime communication framework built around web technologies. The Socketio project provides server software for Node applications, and client software for various languages, including Javascript and C++.

Socketio communication works by passing around messages between clients and servers, optionally with attached JSON objects. Both the server and clients "subscribe" to message types (e.g. "mouse clicked") and the server pushes these messages to the client in real time. Communication is two-way; clients can message servers and servers can message clients.

The socketio software manages the lifetime of connections, so in the event of an unintended disconnection the software will reconnect automatically.

There are some optional features to the protocol, like broadcasting messages to channels (groups of clients), requesting acknowledgement of message receipt, and request-response functionality.

## Sample Usage
1. Install Node package prereqs: `cd server && npm install`
2. Run the Node server: `node server.js`
3. Open your browser to `http://127.0.0.1:7778/`
4. Build and run the Cinder app

When you click the canvas in one client, the other client will be notified of the click position and will draw a red X in the same position. This is facilitated by socketio message passing.

## Resources
[Socketio javascript documentation](http://socket.io/docs/)

[Socketio C++ API overview](https://github.com/socketio/socket.io-client-cpp/blob/master/API.md)
