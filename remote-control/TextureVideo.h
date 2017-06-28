#pragma once;
#include <stdlib.h>
#include "Texture.h"
class AVFrame;
class AVCodec;
class AVCodecParserContext;
class AVCodecContext;
class AVPacket;

class TextureVideo : Texture {
public :
    const AVCodec *codec;
    AVCodecParserContext *parser;
    AVCodecContext *c;
	AVPacket *pkt;
	AVFrame * frame;
	void updateFromFrame();
	void init();
	int processData(unsigned char * buffer, unsigned int size);
	void decode();

	TextureVideo(int width, int height) : Texture(width,height) {
		this->width = width;
		this->height = height;
		this->pixels = (unsigned char *)malloc(sizeof(unsigned char)*width*height*4);
		this->textureName = 0;
	}

	static void processVideoFromFile(TextureVideo * texture, char * filename);
	static void processVideoFromUDP(TextureVideo * texture, int port);
};

