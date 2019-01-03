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
	void			ProdToolGL_GenerateTexture(unsigned char* _data, unsigned int _width, unsigned int _height, unsigned int& _textureID);
	void			ProdToolGL_DeleteTexture(unsigned int* _textureID);
}