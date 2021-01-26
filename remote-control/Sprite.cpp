#include "Sprite.h"
#include "Texture.h"
#include "BufferData.h"
#include "Shader.h"
#include <GL/glew.h>
#include <cstring>
#include <stdio.h>

Sprite::Sprite(Texture * texture,float width,float height,int u,int v, int u2, int v2) {
	this->texture = texture;
	this->u = u;
	this->v = v;
	this->u2 = u2;
	this->v2 = v2;
	this->x = 0.f;
	this->y = 0.f;
	this->width = width;
	this->height = height;

	GLfloat vertices[] = {
	0.f,0.f,0.f,(float)u,(float)v2,
	0.f,height,0.f,(float)u,(float)v,
	width,height,0.f,(float)u2,(float)v,
	width,0.f,0.f,(float)u2,(float)v2
	};
	
	vb = (float *)malloc(sizeof(float)*20);
	
	memcpy(vb,vertices,sizeof(float)*20);
	this->bufferData = new BufferData(vb,20*sizeof(float));
}

void Sprite::draw() {
	glEnable(GL_TEXTURE_2D);
	
	this->texture->bind();

	this->bufferData->bind();	

	glVertexPointer (3,GL_FLOAT,5*sizeof(float),0);
	glTexCoordPointer (2,GL_FLOAT,5*sizeof(float),0);
	
	glVertexAttribPointer(Shader::vertexPositionHandle,3,GL_FLOAT,GL_FALSE,5*sizeof(float),0);
	glEnableVertexAttribArray(Shader::vertexPositionHandle);

	glVertexAttribPointer(Shader::texCoordHandle,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
	glEnableVertexAttribArray(Shader::texCoordHandle);

	glDrawArrays(GL_TRIANGLE_FAN,0,4);
	this->bufferData->unbind();

	this->texture->unbind();

	glDisable(GL_TEXTURE_2D);

}

void Sprite::updateQuad(float x1, float y1, float x2, float y2) {
	GLfloat vertices[] = {
	x1,y1,0.,(float)u,(float)v2,
	x1,y2,0.,(float)u,(float)v,
	x2,y2,0.,(float)u2,(float)v,
	x2,y1,0.,(float)u2,(float)v2
	};
	memcpy(vb,vertices,sizeof(float)*20);
	this->bufferData->updateBuffer();
}

void Sprite::updateQuad(float x1, float y1, float x2, float y2,float x3, float y3, float x4, float y4) {
	GLfloat vertices[] = {
	x1,y1,0.,(float)u,(float)v2,
	x2,y2,0.,(float)u,(float)v,
	x3,y3,0.,(float)u2,(float)v,
	x4,y4,0.,(float)u2,(float)v2
	};
	memcpy(vb,vertices,sizeof(float)*20);
	this->bufferData->updateBuffer();
}

void Sprite::updateTexture() {
	this->texture->update();
}


void SpriteTrail::draw() {
	glEnable(GL_TEXTURE_2D);
	
	this->texture->bind();

	this->bufferData->bind();	

	glVertexPointer (3,GL_FLOAT,5*sizeof(float),0);
	glTexCoordPointer (2,GL_FLOAT,5*sizeof(float),0);
	
	glVertexAttribPointer(Shader::vertexPositionHandle,3,GL_FLOAT,GL_FALSE,5*sizeof(float),0);
	glEnableVertexAttribArray(Shader::vertexPositionHandle);

	glVertexAttribPointer(Shader::texCoordHandle,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));
	glEnableVertexAttribArray(Shader::texCoordHandle);

	glDrawArrays (GL_TRIANGLE_FAN,0,4);

	this->bufferData->unbind();

	this->texture->unbind();

	glDisable(GL_TEXTURE_2D);

}

void SpriteLoop::draw() {
	current++;
	int l = this->spriteList.size();
	l = (current / this->frameLength)%l;
	this->spriteList[l]->draw();
}