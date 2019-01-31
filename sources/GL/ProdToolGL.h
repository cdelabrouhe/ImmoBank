#pragma once

struct GLFWwindow;

namespace ImmoBank
{
	GLFWwindow*		ProdToolGL_InitCreateWindow(int width, int height);
	void			ProdToolGL_Shutdown();
	void			ProdToolGL_InitImGui();
	bool			ProdToolGL_IsMinimized();
	int				ProdToolGL_ShouldClose();
	void			ProdToolGL_Render();
	void			ProdToolGL_GetHwnd(void* out_hwnd);
	void			ProdToolGL_NewFrame();
	bool			ProdToolGL_GenerateTextureFromJPEGBuffer(unsigned char* _buffer, const int _bufferSize, int& _width, int& _height, unsigned int& _textureID);
	bool			ProdToolGL_GenerateTextureFromFile(const char* _path, int& _width, int& _height, unsigned int& _textureID);
	void			ProdToolGL_GenerateTexture(unsigned char* _data, int _width, int _height, unsigned int& _textureID);
	void			ProdToolGL_DeleteTexture(unsigned int* _textureID);
}