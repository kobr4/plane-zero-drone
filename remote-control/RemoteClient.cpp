#include "RemoteClient.h"

#include <SDL2/SDL_net.h>
#include <string.h>
#include "OutputConsole.h"

void RemoteClient::connect(const char * address, int port) {
	strcpy(this->address,address);
	this->port = port;
	if(SDLNet_ResolveHost(&ip,address,port)==-1) {
		OutputConsole::log("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
	}

	tcpsock=SDLNet_TCP_Open(&ip);

	if(!tcpsock) {
		OutputConsole::log("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
	} else {
		this->isConnected = true;
	}
}

void RemoteClient::send(const char * message) {
	if (this->isConnected) {
		int len=strlen(message)+1; // add one for the terminating NULL
		int result=SDLNet_TCP_Send(tcpsock,message,len);
		if(result<len) {
			OutputConsole::log("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		}
	}
}

void RemoteClient::disconnect() {
	SDLNet_TCP_Close(tcpsock);
}