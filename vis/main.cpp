//@	{
//@		"target":
//@		{
//@			"name":"main.o",
//@			"dependencies": [
//@				{"ref":"glfw3", "origin":"pkg-config"},
//@				{"ref":"glew", "origin":"pkg-config"},
//@				{"ref":"GL", "origin":"system", "rel":"external"}
//@			]
//@		}
//@	}

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <memory>

struct GlfwWindowDeleter
{
	void operator()(GLFWwindow* handle)
	{
		glfwDestroyWindow(handle);
	}
};

using WindowHandle = std::unique_ptr<GLFWwindow, GlfwWindowDeleter>;

WindowHandle createWindow()
{
	static bool initialized = false;
	if(!initialized)
	{
		if(!glfwInit())
		{
			return nullptr;
		}
		initialized = true;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	return WindowHandle{glfwCreateWindow(800, 600, "Terraformer", nullptr, nullptr)};
}

int main()
{
	auto window = createWindow();
	glfwMakeContextCurrent(window.get());

	if(glewInit() != GLEW_OK)
	{
		printf("GLEW init failed\n");
		return 1;
	}

	auto gl_version = glGetString(GL_VERSION);
	if(gl_version == nullptr)
	{
		printf("Failed to initialize OpenGL\n");
		return 1;
	}
	printf("OpenGL version: %s\n", reinterpret_cast<char const*>(gl_version));

	glfwSetFramebufferSizeCallback(window.get(), [](GLFWwindow*, int w, int h){
		glViewport(0, 0, w, h);
	});

	while(!glfwWindowShouldClose(window.get()))
	{
		glfwPollEvents();
		glfwSwapBuffers(window.get());
	}

	window.reset();
	glfwTerminate();

	return 0;
}
