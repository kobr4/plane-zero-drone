#pragma once;
#include <stdlib.h>
class Texture {
protected :
	unsigned int width;
	unsigned int height;
	unsigned int textureName;
	unsigned char * pixels;
	volatile bool invalidate;
public :
	Texture(int width, int height,unsigned char * pixels) {
		this->width = width;
		this->height = height;
		this->pixels = pixels;
		this->textureName = 0;
		this->invalidate = false;
	}

	Texture(int width, int height,int textureName) {
		this->width = width;
		this->height = height;
		this->textureName = textureName;
		this->invalidate = false;
	}

	Texture(int width, int height) {
		this->width = width;
		this->height = height;
		this->pixels = (unsigned char *)malloc(sizeof(unsigned char)*width*height*4);
		this->textureName = 0;
		this->invalidate = false;
	}

	void packTexture(Texture * texture,int top_x,int top_y);
	void blur();
	void merge (Texture * texture);
	void bind();
	void unbind();
	void do_register();
	void update();
	unsigned char *  getPixels();
};