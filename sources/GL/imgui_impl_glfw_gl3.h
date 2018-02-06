#ifdef OPENGL3

// ImGui GLFW binding with OpenGL3 + shaders
// https://github.com/ocornut/imgui
// GLFW forward declaration
typedef struct GLFWwindow GLFWwindow;

IMGUI_API bool        ImGui_ImplGlfwGL_Init(GLFWwindow* window, bool install_callbacks);
IMGUI_API void        ImGui_ImplGlfwGL_Shutdown();
IMGUI_API void        ImGui_ImplGlfwGL_NewFrame();

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_API bool        ImGui_ImplGlfwGL_CreateDeviceObjects();

// GLFW callbacks (installed by default if you enable 'install_callbacks' during initialization)
// Provided here if you want to chain callbacks.
// You can also handle inputs yourself and use those as a reference.
IMGUI_API void        ImGui_ImplGlfwGL_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
IMGUI_API void        ImGui_ImplGlfwGL_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
IMGUI_API void        ImGui_ImplGlfwGL_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
IMGUI_API void        ImGui_ImplGlfwGL_CharCallback(GLFWwindow* window, unsigned int c);

#endif