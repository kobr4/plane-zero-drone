#pragma once

#include <SDL2/SDL_net.h>

class RemoteClient {
private :
	char address[1024];
	unsigned int port;
	IPaddress ip;
	TCPsocket tcpsock;
	bool isConnected;
public :
	RemoteClient() {
		this->isConnected = false;
	}
	void connect(const char * address, int port);
	void send(const char * message);
	void disconnect();
};