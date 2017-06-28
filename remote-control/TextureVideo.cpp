#include "TextureVideo.h"
#include <GL/glew.h>
#include <SDL_net.h>

extern "C"
{
	#include <libavcodec/avcodec.h>
}

#define INBUF_SIZE 4096

void GetRGBAPixel(unsigned char * resultMap,const AVFrame * frame, int xin, int yin)
{
	
    // Y component
    unsigned char yc = frame->data[0][frame->linesize[0]*yin + xin];

    // U, V components 
    xin /= 2;
	yin /= 2;
	float u = frame->data[1][frame->linesize[1]*yin + xin];
    float v = frame->data[2][frame->linesize[2]*yin + xin];

    // RGB conversion
    float r = (float)yc + 1.402*(float)(v-128);
    float g = (float)yc - 0.344*(float)(u-128) - 0.714*(v-128);
    float b = (float)yc + 1.772*(float)(u-128);

	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;

	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;

	resultMap[0] = (unsigned char)r;
	resultMap[1] = (unsigned char)g;
	resultMap[2] = (unsigned char)b;
	resultMap[3] = 0xff;
}

void TextureVideo::updateFromFrame() {
	unsigned char map4[4];
	for (int i = 0;i < height;i++) {
		for (int j = 0;j < width;j++) {
			GetRGBAPixel(map4,frame,j,i);
			int offset = (i * width + j) * 4;
			this->pixels[offset] = map4[0];
			this->pixels[offset+1] = map4[1];
			this->pixels[offset+2] = map4[2];
			this->pixels[offset+3] = map4[3];
		}
	}

	this->invalidate = true;
}

void TextureVideo::init() {
    this->codec = NULL;
    this->parser = NULL;
    this->c = NULL;

    int ret;

    avcodec_register_all();

    pkt = av_packet_alloc();
    if (!pkt) {
		puts("fail !");
        exit(1);
	}

    /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
    //memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    /* find the MPEG-1 video decoder */
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    parser = av_parser_init(codec->id);
    if (!parser) {
        fprintf(stderr, "parser not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */
	c->width = 1280;
	c->height = 720;

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
	puts("Init !");
}

int TextureVideo::processData(unsigned char * buffer, unsigned int size) {
    int ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                            buffer, size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
	if (ret < 0) {
        fprintf(stderr, "Error while parsing\n");
        exit(1);
    }
  

    if (pkt->size) {
        decode();
	}
	return ret;
}


void TextureVideo::decode() {
    //char buf[1024];
    int ret;

    ret = avcodec_send_packet(this->c, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        //exit(1);
		return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(this->c, this->frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            //exit(1);
			return;
        }

		updateFromFrame();
    }
}

void TextureVideo::processVideoFromFile(TextureVideo * texture, char * filename) {
    uint8_t *data;
    size_t   data_size;
	uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
	FILE * f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

	while (!feof(f)) {
        /* read raw data from the input file */
        data_size = fread(inbuf, 1, INBUF_SIZE, f);
        if (!data_size)
            break;

        /* use the parser to split the data into frames */
        data = inbuf;
        while (data_size > 0) {
			int ret = texture->processData(data,data_size);
            data      += ret;
            data_size -= ret;
        }
    }
	puts("Decoding finished !");
    /* flush the decoder */
    //decode(c, frame, NULL, outfilename);

    fclose(f);

}

void TextureVideo::processVideoFromUDP(TextureVideo * texture, int port) {

    UDPsocket udpsock;
    uint8_t *data;
   
	
	uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];	
	SDLNet_Init();

    //create the socket
    if(!(udpsock = SDLNet_UDP_Open(port))) {
        printf("Could not create socket\n");
        SDLNet_Quit();
        SDL_Quit();
        exit(1);
    }

    //get my address
    IPaddress* myaddress = SDLNet_UDP_GetPeerAddress(udpsock, -1);
    if(!myaddress) {
        printf("Could not get own port\n");
        exit(2);
    }

    UDPpacket * rcvP = SDLNet_AllocPacket(INBUF_SIZE);
    if(!rcvP) {
        printf("Could not allocate receiving packet\n");
        exit(3);
    }

	
	
	size_t   data_size = 0;
	while(1) {
		int numrecv;
		memset(inbuf, 0, INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE);
		do {
			numrecv=SDLNet_UDP_Recv(udpsock, rcvP);
			if(numrecv) {
				puts("Received");
				for (int i = 0;i < rcvP->len;i++) {
					inbuf[i+data_size] = rcvP->data[i];
				}

				data_size += rcvP->len;
			}

			
		} while (numrecv && data_size < INBUF_SIZE);

		

	
		//if (!data_size)
		//	break;

		/* use the parser to split the data into frames */
		data = inbuf;

		
		while (data_size > 0) {
			int ret = texture->processData(data,data_size);
			
			data      += ret;
			data_size -= ret;
		}
		

	}
}