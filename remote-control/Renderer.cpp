/*
* Copyright (c) 2012, Nicolas My
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the <organization> nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Nicolas My ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Nicolas My BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
//#define GLEW_STATIC
#include "Renderer.h"
#define GLEW_STATIC
#include <SDL.h>
#undef main
#include <SDL_ttf.h>
#include <SDL_net.h>
#include <iostream>

#include <GL/glew.h>
#include "Shader.h"
#include "FrameBuffer.h"
#include <glm/glm.hpp>
#include <glm\gtx\rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm\gtx\intersect.hpp>
#include "Sprite.h"
#include "Texture.h"
#include "TextureVideo.h"
#include "FastMath.h"
#include "Camera.h"
#include "OutputConsole.h"
#include "TextureGenerator.h"
#include <time.h>       /* time */
#include "UIWidget.h"
#include "RemoteClient.h"
#include <windows.h>
/*
const char * fragment_lightmap_texturing =
#include "assets/fragment_lightmap_texturing.gl"
;

const char * vertex = 
#include "assets/vertex.gl"
;
	
const char * fragment_debug =
#include "assets/fragment_debug.gl"
;
*/
//ugliest hack ever !
const char test[] = {
#include "font.data"
};

const char * fragment_lightmap_texturing =
#include "assets/fragment_lightmap_texturing.gl"
;

const char * vertex = 
#include "assets/vertex.gl"
;
	
const char * fragment_debug =
#include "assets/fragment_debug.gl"
;

const char * fragment_vr =
#include "assets/fragment_vr.gl"
;

unsigned char g_texdata[]= { 255, 255, 255, 255, 255, 255, 255, 255,
							255, 255, 255, 255, 255, 255, 255, 255};


unsigned char  g_checker_texdata[]= { 255, 255, 255, 255, 0, 0, 0, 0,
							0, 0, 0, 0, 255, 255, 255, 255};

int thread_func(void *data);
SDL_Surface* CreateSurface(Uint32 flags,int width,int height,const SDL_Surface* display);


#include "Renderable.h"
//Renderable * g_renderable;
std::vector<Renderable*> g_renderableList = std::vector<Renderable*>();
Renderable * sphereRenderable;
Shader * g_shader_debug;
float * g_vertexBuffer = NULL;


SDL_Joystick * g_joystick = NULL; // on crée le joystick
bool g_collision_detection = true;
bool g_cullface = true;
bool g_postprocess = false;
volatile bool g_asyncload = false;


void func_joystick_cb(void * data) {
	if (g_joystick == NULL) {
		g_joystick = SDL_JoystickOpen(0);
	}
	OutputConsole::log("Force joystick detection");
}

void func_bool_collision_change_cb(bool newState, void * data) {
	g_collision_detection = newState;
}

void func_bool_cullface_change_cb(bool newState, void * data) {
	g_cullface = newState;
}

void func_bool_postprocess_change_cb(bool newState, void * data) {
	g_postprocess = newState;
}

void func_bool_stereo_change_cb(bool newState, void * data) {
	((Renderer*)data)->isStereo = !((Renderer*)data)->isStereo;
}


void func_exit_cb(void * data) {
	Renderer * renderer = (Renderer*)data;
	renderer->setExitState();

}

void func_resume_cb(void * data) {
	UIWidget::currentWidget->setActive(false);
	SDL_SetRelativeMouseMode(SDL_TRUE);
}

void func_back_cb(void * data) {
	UIWidget * widget = (UIWidget *)data;
	if (widget->getParent() != NULL && widget->getParent()->getParent() != NULL) {
		UIWidget::currentWidget = widget->getParent()->getParent();
	}

}

RemoteClient client = RemoteClient();

void func_connect_cb(void * data) {
	client.connect("192.168.1.1",1234);
}

void func_disconnect_cb(void * data) {
	client.disconnect();
}

void func_send_cb(void * data) {
	client.send("hello");
}

void func_startv_cb(void * data) {
	client.send("video:7211");
}
				  

static const int deadzone;
int xjaxis = 0;
int yjaxis = 0;
int zjaxis = 0;
int wjaxis = 0;

typedef struct {
 int max;
 int min;
} T_INTERVAL;

int getValue(int value, T_INTERVAL interval) {
	return (value + 32768) * (interval.max - interval.min) / 65536 + interval.min;
}


void process_input(int xaxis,int yaxis) {


	T_INTERVAL servo_wing;
	servo_wing.max = 120;
	servo_wing.min = 60;
	char str[255];
	char str2[255];
	int s1 = (yaxis + xaxis) / 2;
	int s2 = (-yaxis + xaxis) / 2;
	if (s1 < -32768) s1 = -32768;
	if (s1 > 32768) s1 = 32768;
	if (s2 < -32768) s2 = -32768;
	if (s2 > 32768) s2 = 32768;

	int left_servo = getValue(s1, servo_wing);

	int right_servo = getValue(s2, servo_wing);

	sprintf(str,"s1s2:%d:%d",left_servo,right_servo);
	client.send(str);
	//sprintf(str2,"s2:%d",right_servo);
	//client.send(str2);
}

void process_thrust(int waxis) {
	T_INTERVAL s2;
	s2.max = 200;
	s2.min = 100;
	int servo_motor = getValue(waxis, s2);
	char str[255];

	sprintf(str,"m1:%d\n",servo_motor);
	client.send(str);
	
	OutputConsole::log("Throttle = %d\%\n",(wjaxis - s2.min)*100/(s2.max-s2.min) );
}


void handleEvent(SDL_Event event) {

	T_INTERVAL s2;
	s2.max = 200;
	s2.min = 100;
	int xaxis_absolute = 0;
	int yaxis_absolute = 0;
	char str[255];
	switch( event.type )
	{
		case SDL_JOYAXISMOTION:
			if (event.jaxis.axis == 0) {
				xaxis_absolute = event.jaxis.value;
				OutputConsole::log("xjaxis_absolute = %d\n",xaxis_absolute);
				process_input(xaxis_absolute,yaxis_absolute);
			}
			else if (event.jaxis.axis == 1) {
				//yaxis_absolute = event.jaxis.value;
				//process_input(xaxis_absolute,yaxis_absolute);
			}

			else if (event.jaxis.axis == 2) {
				zjaxis = event.jaxis.value;
			}

			else if (event.jaxis.axis == 3) {

				//OutputConsole::log("wjaxis = %d\n",wjaxis);
			}
			else if (event.jaxis.axis == 5) {
				wjaxis = getValue(event.jaxis.value, s2);
				if ((wjaxis > s2.min) && (wjaxis < s2.max)) {
					sprintf(str,"m1:%d\n",wjaxis);
					client.send(str);
				}
				OutputConsole::log("wjaxis = %d\n",wjaxis);
				OutputConsole::log("Throttle = %d\%\n",(wjaxis - s2.min)*100/(s2.max-s2.min) );
			}			
			
			else {
				OutputConsole::log("unknown axis : %d\n", event.jaxis.axis);
				OutputConsole::log("value = %d\n",getValue(event.jaxis.value, s2));
			}

			break;
	}               
}



/*
void func_value_hudscale_change_cb(float newValue, void * data) {
	Renderer::hudScale = newValue;
}

void func_value_ipd_change_cb(float newValue, void * data) {
	Renderer::ipd = newValue;
}
*/

void copyvector(float * vertexBuffer, int offset, glm::vec3 pt, glm::vec2 ptex) {
	vertexBuffer[offset] = pt[0];
	vertexBuffer[offset+1] = pt[1];
	vertexBuffer[offset+2] = pt[2];
	vertexBuffer[offset+3] = ptex[0];
	vertexBuffer[offset+4] = ptex[1];
}

void Renderer::initializeContent() {
	/* initialize random seed: */
	srand (time(NULL));
	TextureVideo * texture = new TextureVideo(1280,720);
	spriteVideo = new Sprite((Texture*)texture,this->screenWidth,this->screenHeight,0,1,1,0);
	texture->init();
	g_asyncload = true;
	
	TextureVideo::processVideoFromUDP(texture,7211);
}

static int AsyncInitThread(void *ptr)
{
	Renderer * renderer = (Renderer*)ptr;
	renderer->initializeContent();
	return 0;
}


void Renderer::init(unsigned int screenWidth, unsigned int screenHeight, bool fullscreen)
{
	this->screenHeight = screenHeight;
	this->screenWidth = screenWidth;

	for (int i = 0;i < 10;i++) {
		effectList[i].duration = 0;
	}

	if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO| SDL_INIT_JOYSTICK) < 0 ) {
		fprintf(stderr, "Impossible d'initialiser SDL: %s\n", SDL_GetError());
		exit(1);
	}

	if(SDLNet_Init()==-1) {
		printf("SDLNet_Init: %s\n", SDLNet_GetError());
		exit(2);
	}

    g_joystick = SDL_JoystickOpen(0);

    atexit(SDL_Quit);

    // Version d'OpenGL
      
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
      
    //SDL_GL_SetSwapInterval(0); 
    // Double Buffer
      
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);


	displayWindow = SDL_CreateWindow("Korridor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->screenWidth, this->screenHeight, SDL_WINDOW_OPENGL  | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));

    // Création du contexte OpenGL
  
    contexteOpenGL = SDL_GL_CreateContext(displayWindow);
  
    if(contexteOpenGL == 0)
    {
        std::cout << SDL_GetError() << std::endl; //// >> AFFICHE : " the specified window isn't an OpenGL window"
        SDL_DestroyWindow(displayWindow);
		SDLNet_Quit();
        SDL_Quit();
  
        exit(-1); //// >> PLANTE ICI : return -1 ..
    }
      

	if (TTF_Init() < 0) {
		puts("ERROR : unable to initialize font library");
		exit(1);
	}

	// GLEW Initialisation
	glewInit();

	SDL_RWops* fontdataptr = SDL_RWFromConstMem(test,sizeof(test));
	this->font = TTF_OpenFontRW(fontdataptr, 1, 28);
	//this->font = TTF_OpenFont( "digital display tfb.ttf",28);
	if (this->font == NULL)
	{
		puts("ERROR : unable to load font");
		exit(1);
	}


#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    unsigned int rmask = 0xff000000;
    unsigned int gmask = 0x00ff0000;
    unsigned int bmask = 0x0000ff00;
    unsigned int amask = 0x000000ff;
#else
    unsigned int rmask = 0x000000ff;
    unsigned int gmask = 0x0000ff00;
    unsigned int bmask = 0x00ff0000;
    unsigned int amask = 0xff000000;
#endif
	this->textSurface = SDL_CreateRGBSurface( 0, this->screenWidth, this->screenHeight, 32, rmask, gmask, bmask, amask);
	bExit = false;

	shaderLightmapTexturing = new Shader();
	shaderLightmapTexturing->load_fragment_from_string(fragment_lightmap_texturing);
	shaderLightmapTexturing->load_vertex_from_string(vertex);
	//shaderLightmapTexturing->load_fragment("fragment_lightmap_texturing.gl");
	//shaderLightmapTexturing->load_vertex("vertex.gl");

	shaderTexturing = Shader::createBuiltin(SHADER_TEXTURING);

	Texture * textSurfaceTexture = new Texture(this->screenWidth,this->screenHeight,(unsigned char*)this->textSurface->pixels);
	this->spriteTextSurface = new Sprite(textSurfaceTexture,(float)this->screenWidth,(float)this->screenHeight,0,1,1,0);
	this->fbDrawing = NULL;
	this->fbDrawing2 = NULL;
	this->fbHalfRes = NULL;

	g_shader_debug = new Shader();
	g_shader_debug->load_fragment_from_string(fragment_debug);
	g_shader_debug->load_vertex_from_string(vertex);
	
	//g_shader_debug->load_fragment("fragment_debug.gl");
	//g_shader_debug->load_vertex("vertex.gl");

	shaderVr = new Shader();
	shaderVr->load_fragment_from_string(fragment_vr);
	shaderVr->load_vertex_from_string(vertex);

	camera = new Camera();
	camera->SetClipping(0.1f,200.f);
	//camera->SetPosition(glm::vec3(14.7f,1.9f,57.5f));
	camera->SetPosition(glm::vec3(0.0f,0.0f,-10.0f));
	camera->SetLookAt(glm::vec3(0.0f,0.0f,0.0f));

	OutputConsole::setRenderer(this);


	UIHeader * headWidget = new UIHeader();

	UIHeader * header3 = new UIHeader();
	header3->setLabel("Resume");
	header3->setOnClickCallback(&func_resume_cb,this);
	headWidget->addChild(header3);

	UIHeader * header1 = new UIHeader();
	header1->setLabel("Option");
	{
		UIHeader * headerOption1 = new UIHeader();
		headerOption1->setLabel("Joystick detection");
		headerOption1->setOnClickCallback(&func_joystick_cb,NULL);
		header1->addChild(headerOption1);

		UIBoolean * headerBoolean3 = new UIBoolean();
		headerBoolean3->setLabel("Stereo");
		headerBoolean3->setOnBoolChangeCallback(func_bool_stereo_change_cb,this);
		headerBoolean3->setState(false);
		header1->addChild(headerBoolean3);

		UIHeader * headerOptionBack = new UIHeader();
		headerOptionBack->setLabel("Back");
		headerOptionBack->setOnClickCallback(&func_back_cb,headerOptionBack);
		header1->addChild(headerOptionBack);
	}

    headWidget->addChild(header1);
	
	UIHeader * headerNet = new UIHeader();
	headerNet->setLabel("Network");
	{
		UIHeader * headerConnect = new UIHeader();
		headerConnect->setLabel("Connect");
		headerConnect->setOnClickCallback(&func_connect_cb,NULL);
		headerNet->addChild(headerConnect);

		UIHeader * headerSend = new UIHeader();
		headerSend->setLabel("Send hello");
		headerSend->setOnClickCallback(&func_send_cb,NULL);
		headerNet->addChild(headerSend);

		UIHeader * headerStartVideo = new UIHeader();
		headerStartVideo->setLabel("Start Video");
		headerStartVideo->setOnClickCallback(&func_startv_cb,NULL);
		headerNet->addChild(headerStartVideo);

		UIHeader * headerDisconnect = new UIHeader();
		headerDisconnect->setLabel("Disconnect");
		headerDisconnect->setOnClickCallback(&func_disconnect_cb,NULL);
		headerNet->addChild(headerDisconnect);

		UIHeader * headerOptionBack = new UIHeader();
		headerOptionBack->setLabel("Back");
		headerOptionBack->setOnClickCallback(&func_back_cb,headerOptionBack);
		headerNet->addChild(headerOptionBack);
	}
	headWidget->addChild(headerNet);
	
	UIHeader * header2 = new UIHeader();
	header2->setLabel("Quit");
	header2->setOnClickCallback(&func_exit_cb,this);
	headWidget->addChild(header2);

	UIWidget::currentWidget = headWidget;
	
	this->isStereo = false;

	spriteVideo = NULL;
	asyncInitThread = SDL_CreateThread(AsyncInitThread, "AsyncInitThread", (void *)this);
}


bool Renderer::exitstate()
{
	return bExit; 
}

void Renderer::setExitState() {
	bExit = true;
}

int thread_func(void *data)
{
	Renderer * renderer = (Renderer*)data;
    int last_value = 0;

    while ( renderer->exitstate() != true ) {
		renderer->draw();
    }
    printf("Thread quitting\n");
    return(0);
}


void Renderer::drawFps()
{
	char s[256];
	sprintf(s,"FPS %.3d",this->getFps());
	drawMessage(s,ALIGNRIGHT,ALIGNTOP);
}

void Renderer::drawMessage(const char * message,float x,float y) {
	if (strlen(message) == 0)
		return;

	SDL_Color fontcolor = {255, 255, 255};

	SDL_Surface * textSur = TTF_RenderText_Solid(this->font,message,fontcolor);	//set the text surface
	if (textSur == NULL)
	{
		
        fprintf(stderr, "Unable to create draw surface: %s\n", SDL_GetError());
		printf("%s\n",message);
        return;
	}

	drawMessage(textSur,x,y);
	SDL_FreeSurface(textSur);	
}

void Renderer::drawMessage(SDL_Surface * textSur,float x,float y) {
	SDL_Rect rect;
	rect.x = (int)x;
	rect.y = (int)y;
	SDL_BlitSurface(textSur,NULL,this->textSurface,&rect);
}

void Renderer::drawMessage(const char * message,RendererTextAlign hAlign,RendererTextAlign vAlign)
{
	
	SDL_Color fontcolor = {255, 255, 255};


	if (strlen(message) == 0)
		return;

	SDL_Surface * textSur = TTF_RenderText_Solid(this->font,message,fontcolor);	//set the text surface
	
	if (textSur == NULL)
	{
		
        fprintf(stderr, "Unable to create draw surface: %s\n", SDL_GetError());
		printf("%s\n",message);
        return;
	}
	SDL_Rect rect;
	float hudScale = 1.0f;

	switch (hAlign)
	{
	case ALIGNLEFT:
		rect.x = (int)(this->textSurface->w - this->textSurface->w* hudScale );
		break;
	case ALIGNRIGHT:
		rect.x = (int)(this->textSurface->w * hudScale - textSur->w) ;
		break;
	case ALIGNCENTER:
		rect.x = (int)(this->textSurface->w - textSur->w)/2;
		break;
	}

	switch (vAlign)
	{
	case ALIGNTOP:
		rect.y = (int)(this->textSurface->h - this->textSurface->h* hudScale );
		break;
	case ALIGNBOTTOM:
		rect.y = (int)(this->textSurface->h * hudScale - textSur->h) ;
		break;
	case ALIGNCENTER:
		rect.y = (int)(this->textSurface->h - textSur->h)/2;
		break;
	}
	
	drawMessage(textSur,rect.x,rect.y);
	
	SDL_FreeSurface(textSur);	
	
}

void swapFBdrawing(FrameBuffer **fb1, FrameBuffer **fb2) {
	FrameBuffer * fbtmp;
	fbtmp = *fb1;
	*fb1 = *fb2;
	*fb2 = fbtmp; 
}

void Renderer::draw()
{
	SDL_FillRect(this->textSurface, NULL, 0x000000);

	if (this->fbHalfRes == NULL){ 
		this->fbHalfRes = new FrameBuffer(this->screenWidth / 8,this->screenHeight / 8);
		this->fbHalfRes->do_register();
	}

	if (this->fbDrawing == NULL){
		this->fbDrawing = new FrameBuffer(this->screenWidth,this->screenHeight);
		this->fbDrawing->do_register();
	}


	if (this->fbDrawing2 == NULL){
		this->fbDrawing2 = new FrameBuffer(this->screenWidth,this->screenHeight);
		this->fbDrawing2->do_register();
	}
	
	this->fbDrawing->bind();
	
	//OpenGL setup
	if (g_cullface) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	glClearColor(0, 0, 0.1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 



	
	glViewport((GLint) (0),
				(GLint) (0),
				(GLsizei) this->screenWidth,
				(GLsizei) this->screenHeight);
	


	camera->SetViewport((GLint) (0),
				(GLint) (0),
				(GLsizei) this->screenWidth,
				(GLsizei) this->screenHeight);
	
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);
	
	glm::mat4 P = glm::mat4();
	glm::mat4 M = glm::mat4();
	glm::mat4 V = glm::mat4();

	camera->Update();
	camera->GetMatricies(P,V,M);

	memcpy(g_shader_debug->getProjectionMatrix(),glm::value_ptr(P),sizeof(float)*16);
	memcpy(g_shader_debug->getModelViewMatrix(),glm::value_ptr(V),sizeof(float)*16);
	
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	
	this->shaderTexturing->setProjectionMatrixToOrtho(this->screenWidth,this->screenHeight);
	this->shaderTexturing->setModelViewMatrixToIdentity();
	this->shaderTexturing->bind();
	
	if (g_postprocess && g_asyncload) {
		this->fbHalfRes->draw(screenWidth,screenHeight);	
	}
	
	if (spriteVideo != NULL) spriteVideo->draw();

	char s[1024];
	glm::vec3 cam_pos = camera->getPosition();
	sprintf(s,"Pos: %.1f %.1f %.1f",cam_pos[0],cam_pos[1],cam_pos[2]);
	this->drawMessage(s,RendererTextAlign::ALIGNRIGHT,RendererTextAlign::ALIGNBOTTOM);
	
	
	if (UIWidget::currentWidget->isActive()){
		UIWidget::currentWidget->drawChilds(this);
	}
	
	
	this->drawFps();

	OutputConsole::render();

	glEnable(GL_BLEND);
	this->spriteTextSurface->updateTexture();
	this->spriteTextSurface->draw();
	glDisable(GL_BLEND);	
	this->shaderTexturing->unbind();	

	this->fbDrawing->unbind(screenWidth,screenHeight);


	Shader * selectedShader = NULL;


	if (this->isStereo) this->fbDrawing2->bind();

	shaderTexturing->bind();
	shaderTexturing->setProjectionMatrixToOrtho(this->screenWidth,this->screenHeight);
	shaderTexturing->setModelViewMatrixToIdentity();
	shaderTexturing->bind_attributes();
	
	if (this->isStereo) {
		glViewport((GLint) (0),
			(GLint) (0),
			(GLsizei) this->screenWidth/2,
			(GLsizei) this->screenHeight);


		this->fbDrawing->draw(screenWidth,screenHeight);

		glViewport((GLint) (this->screenWidth/2),
		(GLint) (0),
		(GLsizei) this->screenWidth/2,
		(GLsizei) this->screenHeight);


		this->fbDrawing->draw(screenWidth,screenHeight);
	} else {
		this->fbDrawing->draw(screenWidth,screenHeight);
	}
	shaderTexturing->unbind();

	if (this->isStereo) this->fbDrawing2->unbind(screenWidth,screenHeight);

	if (this->isStereo) {
		shaderVr->bind();
		shaderVr->setProjectionMatrixToOrtho(this->screenWidth,this->screenHeight);
		shaderVr->setModelViewMatrixToIdentity();
		shaderVr->bind_attributes();
		this->fbDrawing2->draw(screenWidth,screenHeight);
		shaderVr->unbind();
	}
	SDL_GL_SwapWindow(displayWindow);
}

void Renderer::loop()
{
	SDL_Event event;
	T_EFFECT effect;
	int told = SDL_GetTicks();

	while (!bExit)
	{
		frameCounter++;
		if (frameCounter%10 == 0) {
			int tnew = SDL_GetTicks();
			int dt = tnew - told;
			told = tnew;

			if (dt != 0) {
				this->fps = 10000/dt;
			}
		}

		if (UIWidget::currentWidget->isActive() == false && g_joystick != NULL) {
			int x = SDL_JoystickGetAxis(g_joystick,0);
			int y = SDL_JoystickGetAxis(g_joystick,1);
			//OutputConsole::log("JOY[x,y] = [%d,%d]\n",x,y);
			process_input(x,y);

			int p = SDL_JoystickGetAxis(g_joystick,5);
			process_thrust(p);
		}


		while( SDL_PollEvent( &event ) )
		{
			if (UIWidget::currentWidget->isActive()) {
				UIWidget::currentWidget->handleEvent(event);
			} else {
				//camera->handleEvent(event);
				//handleEvent(event);
				switch( event.type )
				{
					case SDL_JOYBUTTONDOWN:
						if (event.jbutton.button == 4) {
							SDL_SetRelativeMouseMode(SDL_FALSE);
							UIWidget::currentWidget->setActive(true);
						}
						break;
					case SDL_KEYDOWN:
						switch( event.key.keysym.sym )
						{
							case SDLK_ESCAPE:
								SDL_SetRelativeMouseMode(SDL_FALSE);
								UIWidget::currentWidget->setActive(true);
								break;
							case SDLK_p:
								this->fbDrawing->bind();
								this->fbDrawing->writeToTGA("fbDrawing.tga");
								this->fbDrawing->unbind(screenWidth,screenHeight);
								this->fbHalfRes->bind();
								this->fbHalfRes->writeToTGA("fbHalfRes.tga");
								this->fbHalfRes->unbind(screenWidth,screenHeight);
								break;
							default:
								break;
						}
					break;
				}                
			}
		}
		
		this->draw();
	}

    

	SDL_FreeSurface(this->textSurface);
	SDL_GL_DeleteContext(contexteOpenGL);
    SDL_DestroyWindow(displayWindow);
    
	if (g_joystick != NULL) {
		SDL_JoystickClose(g_joystick);
	}
	SDL_Quit();
}

int Renderer::getFps()
{
	return this->fps;
}

void Renderer::processEffect(T_EFFECT &effect) {
	effect.duration--;
	switch (effect.effectType) {
	case 4 : {
				this->drawMessage(effect.str_param,effect.fl_param1,effect.fl_param2);
			 }
			 break;
	case 3 : {
			}
			break;
	case 2  :
			break;
	case 1 : 
			break;
	}
}

void Renderer::addEffect(T_EFFECT effect) {
	for(int i = 0;i < 10;i++) {
		if (effectList[i].duration == 0) {
			effectList[i].duration = effect.duration;
			effectList[i].effectType = effect.effectType;
			strcpy(effectList[i].str_param, effect.str_param);
			effectList[i].fl_param1 = effect.fl_param1;
			effectList[i].fl_param2 = effect.fl_param2;
			return;
		}
	}
}

Renderer::~Renderer() {

	//delete this->camera;
}