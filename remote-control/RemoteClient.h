#pragma once
#include "SDL_net.h"

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
	void connect(char * address, int port);
	void send(char * message);
	void disconnect();
};