main: BufferData.o UIWidget.o TextureVideo.o TextureGenerator.o Texture.o Sprite.o Shader.o Renderer.o Renderable.o RemoteClient.o OutputConsole.o Main.o FrameBuffer.o FastMath.o Camera.o
	g++ -o main BufferData.o UIWidget.o TextureVideo.o TextureGenerator.o Texture.o Sprite.o Shader.o Renderer.o Renderable.o RemoteClient.o OutputConsole.o Main.o FrameBuffer.o FastMath.o Camera.o -lSDL2 -lSDL2_ttf -lSDL2_net -lGL -lGLU -lGLEW -lavcodec -lavutil 
BufferData.o: BufferData.cpp
	g++ -o BufferData.o -c BufferData.cpp
UIWidget.o: UIWidget.cpp
	g++ -o UIWidget.o -c UIWidget.cpp
TextureVideo.o: TextureVideo.cpp
	g++ -o TextureVideo.o -c TextureVideo.cpp
TextureGenerator.o: TextureGenerator.cpp
	g++ -o TextureGenerator.o -c TextureGenerator.cpp
Texture.o: Texture.cpp
	g++ -o Texture.o -c Texture.cpp
Sprite.o: Sprite.cpp
	g++ -o Sprite.o -c Sprite.cpp
Shader.o: Shader.cpp
	g++ -o Shader.o -c Shader.cpp
Renderer.o: Renderer.cpp
	g++ -o Renderer.o -c Renderer.cpp
Renderable.o: Renderable.cpp
	g++ -o Renderable.o -c Renderable.cpp
RemoteClient.o: RemoteClient.cpp
	g++ -o RemoteClient.o -c RemoteClient.cpp
OutputConsole.o: OutputConsole.cpp
	g++ -o OutputConsole.o -c OutputConsole.cpp
Main.o: Main.cpp
	g++ -o Main.o -c Main.cpp
FrameBuffer.o: FrameBuffer.cpp
	g++ -o FrameBuffer.o -c FrameBuffer.cpp
FastMath.o: FastMath.cpp
	g++ -o FastMath.o -c FastMath.cpp
Camera.o: Camera.cpp
	g++ -o Camera.o -c Camera.cpp
clean:
	rm *.o