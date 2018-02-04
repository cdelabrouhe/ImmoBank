#include "ProdToolGL.h"
#include <stdio.h>
#include <string>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#include "Tools/ImGui/imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include "UI/UIManager.h"
#include "ProdToolTypes.h"

//-------------------------------------------------------------------------------------------------
// DATA
//-------------------------------------------------------------------------------------------------
GLFWwindow*		s_Window;

//-------------------------------------------------------------------------------------------------
// FUNCTIONS
//-------------------------------------------------------------------------------------------------

GLFWwindow*		ProdToolGL_InitCreateWindow(int width, int height)
{
	glfwSetErrorCallback([](int error, const char* description) { fprintf(stderr, "Error %d: %s\n", error, description); });
	if (!glfwInit())
		return NULL;

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	glfwWindowHint(GLFW_DECORATED, 1);
	glfwWindowHint(GLFW_RESIZABLE, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	const char* additional_text = "";
	if (_MSC_VER == 1900)
	{
		if (sizeof(void*) == 8)
			additional_text = "(VS2015 x64)";
		else
			additional_text = "(VS2015)";
	}

	std::string name = "ProdTool";
	GLFWwindow* window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
	glfwSetWindowPos(window, (mode->width - width) / 2, (mode->height - height) / 2);
	glfwMakeContextCurrent(window);

	gl3wInit();

	s_Window = window;

	return window;
}

void ProdToolGL_Shutdown()
{
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
}

// FIXME-IMGUI: Add to imgui somehow (split into helpers)
void AddReplacementGlyph(ImFont* font, unsigned short missing_glyph_char, unsigned short replace_glyph_char)
{
	const ImFont::Glyph* missing_glyph = font->FindGlyph(missing_glyph_char);
	if (missing_glyph == font->FallbackGlyph)
		missing_glyph = NULL;

	const ImFont::Glyph* replace_glyph = font->FindGlyph(replace_glyph_char);
	if (replace_glyph == font->FallbackGlyph)
		replace_glyph = NULL;

	if (missing_glyph)      // Not missing
		return;
	if (!replace_glyph)     // No target
		return;

	int lookup_size = font->IndexLookup.size();
	if (lookup_size < missing_glyph_char + 1)
	{
		font->IndexAdvanceX.resize(missing_glyph_char + 1);
		font->IndexLookup.resize(missing_glyph_char + 1);
		for (int i = lookup_size; i < missing_glyph_char + 1; i++)
		{
			font->IndexAdvanceX[i] = -1.0f;
			font->IndexLookup[i] = -1;
		}
	}

	font->IndexAdvanceX[missing_glyph_char] = font->IndexAdvanceX[replace_glyph_char];
	font->IndexLookup[missing_glyph_char] = font->IndexLookup[replace_glyph_char];
}

void ProdToolGL_InitImGui()
{
	ASSERT(s_Window != NULL);

	// Setup ImGui binding
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfwGL3_Init(s_Window, true);

	// Load fonts next to executable file
	std::string fontPath = "DroidSans.ttf";
	UIManager::getSingleton()->FontDefault = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);

	// Setup style
	ImGui::StyleColorsDark();
	/*ImGuiStyle& style = ImGui::GetStyle();
	style.Alpha = 1.0f;
	style.FrameRounding = 4;
	style.IndentSpacing = 12.0f;*/

	ImGui_ImplGlfwGL3_CreateDeviceObjects();

	for (auto& font : io.Fonts->Fonts)
		AddReplacementGlyph(font, 0x2019, '\'');
}

bool	ProdToolGL_IsMinimized()
{
	HWND hwnd = glfwGetWin32Window(s_Window);
	return hwnd && IsIconic(hwnd);
}

int ProdToolGL_ShouldClose()
{
	return glfwWindowShouldClose(s_Window);
}

void	ProdToolGL_GetHwnd(void* out_hwnd)
{
	HWND hwnd = glfwGetWin32Window(s_Window);
	*(HWND*)out_hwnd = hwnd;
}

void ProdToolGL_NewFrame()
{
	glfwPollEvents();

	Sleep(15);

	ImGui_ImplGlfwGL3_NewFrame();
}

void	ProdToolGL_Render()
{
	// Rendering
	int display_w, display_h;
	glfwGetFramebufferSize(s_Window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);

	// We don't need to clear, our main ImGui window takes all the screen
	//ImVec4 clear_color = ImColor(114, 144, 154);
	//glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	//glClear(GL_COLOR_BUFFER_BIT);

	ImGui::Render();
	glfwSwapBuffers(s_Window);
}