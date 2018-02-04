#pragma once

struct GLFWwindow;

GLFWwindow*		ProdToolGL_InitCreateWindow(int width, int height);
void			ProdToolGL_Shutdown();
void			ProdToolGL_InitImGui();
bool			ProdToolGL_IsMinimized();
int				ProdToolGL_ShouldClose();
void			ProdToolGL_Render();
void			ProdToolGL_GetHwnd(void* out_hwnd);
void			ProdToolGL_NewFrame();
