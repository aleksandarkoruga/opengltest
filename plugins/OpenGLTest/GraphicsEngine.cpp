#include "GraphicsEngine.h"
#include <functional>
namespace scGraphics{ 
	
	//TODO: Import vertex and file open from rendertotexture project
	static const struct
	{
		float x, y;
		float r, g, b;
	} vertices[3] =
	{
		{ -0.6f, -0.4f, 1.f, 0.f, 0.f },
		{  0.6f, -0.4f, 0.f, 1.f, 0.f },
		{   0.f,  0.6f, 0.f, 0.f, 1.f }
	};

	static const char* vertex_shader_text =
		"#version 110\n"
		"uniform mat4 MVP;\n"
		"attribute vec3 vCol;\n"
		"attribute vec2 vPos;\n"
		"varying vec3 color;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
		"    color = vCol;\n"
		"}\n";

	static const char* fragment_shader_text =
		"#version 110\n"
		"varying vec3 color;\n"
		"void main()\n"
		"{\n"
		"    gl_FragColor = vec4(color, 1.0);\n"
		"}\n";


	static void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}




	GraphicsEngine::GraphicsEngine() : 
		m_graphicsThread(std::bind(&RunEngine, this)),
		m_shader()
	{
	}

	GraphicsEngine::~GraphicsEngine()
	{
		m_graphicsThread.join();
	}

	void GraphicsEngine::RunEngine()
	{





	}

	bool GraphicsEngine::InitEngine()
	{
		glfwSetErrorCallback(error_callback);

		if (glfwInit())
		{
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

			m_pWindow = glfwCreateWindow(640, 480, "Supercollider Renderer", NULL, NULL);
			if (!m_pWindow)
			{
				glfwTerminate();
			}
			else
			{
				glfwMakeContextCurrent(m_pWindow);
				gladLoadGL();
				glfwSwapInterval(1);

				m_shader.Compile();


				m_mvpLocation = m_shader.GetUniformLocation("MVP");
				//position 
				m_vPosLocation = m_shader.GetAttribLocation("vPos");
				//color
				m_vColLocation = m_shader.GetAttribLocation("vCol");


				glEnableVertexAttribArray(m_vPosLocation);
				glVertexAttribPointer(m_vPosLocation, 2, GL_FLOAT, GL_FALSE,
					sizeof(vertices[0]), (void*)0);
				glEnableVertexAttribArray(m_vColLocation);
				glVertexAttribPointer(m_vColLocation, 3, GL_FLOAT, GL_FALSE,
					sizeof(vertices[0]), (void*)(sizeof(float) * 2));


			}
		}


		return false;
	}

	Shader::Shader()
	{
		glGenBuffers(1, &m_vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		m_vertexShader  = glCreateShader(GL_VERTEX_SHADER);
		m_fragmentShader= glCreateShader(GL_FRAGMENT_SHADER);


	}

	Shader::~Shader()
	{
	}

	void Shader::Compile()
	{
		CompileVertexShader();
		CompileFragmentShader();
		CompileProgram();
	}

	void Shader::CompileVertexShader()
	{
		glShaderSource(m_vertexShader, 1, &vertex_shader_text, NULL);
		glCompileShader(m_vertexShader);
	}

	void Shader::CompileFragmentShader()
	{
		glShaderSource(m_fragmentShader, 1, &fragment_shader_text, NULL);
		glCompileShader(m_fragmentShader);
	}

	void Shader::CompileProgram()
	{
		m_shaderProgram = glCreateProgram();
		glAttachShader(m_shaderProgram, m_vertexShader);
		glAttachShader(m_shaderProgram, m_fragmentShader);
		glLinkProgram(m_shaderProgram);
	}

	void Shader::ReleaseShader()
	{
	}

	GLint & Shader::GetUniformLocation(const GLchar * name)
	{
		glGetUniformLocation(m_shaderProgram, name);
	}

	GLint & Shader::GetAttribLocation(const GLchar * name)
	{
		glGetAttribLocation(m_shaderProgram, name);
	}

}
