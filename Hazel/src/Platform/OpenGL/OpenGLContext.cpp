#include "hzpch.h"
#include "Hazel/Log.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Hazel {

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		HZ_CORE_ASSERT(windowHandle, "Window handle is null");
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		HZ_CORE_ASSERT(status, "Failed to initialized GLAD!");

		 HZ_CORE_INFO("OpenGL Info:");
		 HZ_CORE_INFO(" Vendor: {0}", (const char*) glGetString(GL_VENDOR));
		 HZ_CORE_INFO(" Renderer: {0}", (const char*) glGetString(GL_RENDERER));

	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}