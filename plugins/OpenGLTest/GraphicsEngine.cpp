
#pragma once
#include "GraphicsEngine.h"
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace scGraphics{ 
	
	//TODO: Import vertex and file open from rendertotexture project
	
	/*
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
	*/

	static void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}
	void checkOpenGLError() {
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cout << "OpenGL Error: " << error << std::endl;
		}
	}



	GraphicsEngine::GraphicsEngine() : 
		m_programState({ PROGRAM_STATE::START }),
		m_graphicsThread(&GraphicsEngine::RunEngine,this),
		m_pWindow(nullptr),
		m_mvpLocation(0),
		m_vPosLocation(0),
		//m_vColLocation(0),
		m_frameBuffer(0),
		m_shader(nullptr),
		m_bSSBO({MEMSTATE::FINISHED})
	{
	}

	GraphicsEngine::~GraphicsEngine()
	{
		m_programState.store(PROGRAM_STATE::TERMINATE);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		m_graphicsThread.join();
		
		if (m_pWindow)
			glfwDestroyWindow(m_pWindow);
		glfwTerminate();
	}

	void GraphicsEngine::RunEngine()
	{
		while (m_programState.load() != PROGRAM_STATE::TERMINATE)
		{


			switch (m_programState.load())
			{
			case PROGRAM_STATE::START:
			{
				InitEngine();
				m_programState.store(PROGRAM_STATE::RUN);
			}
			break;
			case PROGRAM_STATE::RUN:
				CalculateOneFrame();
			break;
			default:
				break;
			}

			if (m_programState.load()!=PROGRAM_STATE::TERMINATE)
				std::this_thread::sleep_for(std::chrono::milliseconds(33));
			else
				break;
		}
	}
	void GraphicsEngine::CalculateOneFrame()
	{
		if (m_pWindow && (!glfwWindowShouldClose(m_pWindow)) )
		{
			float ratio;
			int width, height;
			mat4x4 m, p, mvp;

			checkOpenGLError();

			glfwGetFramebufferSize(m_pWindow, &width, &height);

			checkOpenGLError();

			ratio = width / (float)height;

			glViewport(0, 0, width, height);

			checkOpenGLError();

			glClear(GL_COLOR_BUFFER_BIT);

			checkOpenGLError();

			mat4x4_identity(m);
			//mat4x4_rotate_Z(m, m, (float)glfwGetTime());
			mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
			mat4x4_mul(mvp, p, m);

			checkOpenGLError();


			checkOpenGLError();

			glUseProgram(m_shader->GetShaderProgram());

			GLint resolutionLocation = glGetUniformLocation(m_shader->GetShaderProgram(), "resolution");
			glUniform2f(resolutionLocation, static_cast<float>(width), static_cast<float>(height)  );

			if(m_bSSBO.load() == MEMSTATE::READY)
				m_shader->CommitData();

			checkOpenGLError();
			glUniformMatrix4fv(m_mvpLocation, 1, GL_FALSE, (const GLfloat*)mvp);
			checkOpenGLError();

			glBindVertexArray(m_VAO);

			//glBindVertexArray(m_vaoIdpos); 
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			checkOpenGLError();

			glBindVertexArray(0);

			glfwSwapBuffers(m_pWindow);
			glfwPollEvents();

			checkOpenGLError();


			m_bSSBO.store(MEMSTATE::FINISHED);
		}
		
	}
	void GraphicsEngine::SetData(const float* buf, int nSamples)
	{
		if (m_bSSBO.load() == MEMSTATE::FINISHED && m_shader!=nullptr && m_programState.load()==PROGRAM_STATE::RUN)
		{
			m_shader->SetData(buf, nSamples);
			

			m_bSSBO.store(MEMSTATE::READY);
		}
	}
	bool GraphicsEngine::InitEngine()
	{
		glfwSetErrorCallback(error_callback);

		if (glfwInit())
		{
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


			//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
			//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

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

				checkOpenGLError();

				m_shader = std::make_unique<Shader>();

				checkOpenGLError();

				m_shader->Compile();

				checkOpenGLError();

				m_mvpLocation = m_shader->GetUniformLocation("MVP");
				
				m_vPosLocation = m_shader->GetAttribLocation("vPos");
				checkOpenGLError();

				//m_vColLocation = m_shader->GetAttribLocation("vCol");
				//checkOpenGLError();
				

				//////////

				
				glGenVertexArrays(1, &m_VAO);
				glGenBuffers(1, &m_VBO);
				glGenBuffers(1, &m_EBO);

				glGenBuffers(1, m_shader->GetSSBO());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, *m_shader->GetSSBO());
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, *m_shader->GetSSBO()); // Binding point 0 matches shader
				// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
				glBindVertexArray(m_VAO);

				glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(m_shader->GetVertexArray()[0]) * m_shader->GetVertexArray().size(), m_shader->GetVertexArray().data(), GL_STATIC_DRAW);


				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_shader->GetIndexArray()[0]) * m_shader->GetIndexArray().size(), m_shader->GetIndexArray().data(), GL_STATIC_DRAW);

				glVertexAttribPointer(m_vPosLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);

				// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
				glBindBuffer(GL_ARRAY_BUFFER, 0);

				// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
				//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
				// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
				glBindVertexArray(0);
				
				
				/////

				

				checkOpenGLError();

			}
		}


		return false;
	}

	Shader::Shader() :m_data({})
	{
		//std::filesystem::path cwd = std::filesystem::current_path();
		//std::cout << cwd;
		const std::string vertFName = "shader.vert";
		const std::string fragFName = "shader.frag";

		m_vertexShaderText = readShaderFile(vertFName);
		m_fragmentShaderText = readShaderFile(fragFName);
		/*m_vertices =
		{
			// First triangle
			{ -1.0f, -1.0f, 1.f, 0.f, 0.f }, // Bottom-left (red)
			{  1.0f, -1.0f, 0.f, 1.f, 0.f }, // Bottom-right (green)
			{ -1.0f,  1.0f, 0.f, 0.f, 1.f }, // Top-left (blue)

			// Second triangle
			{  1.0f, -1.0f, 0.f, 1.f, 0.f }, // Bottom-right (green)
			{  1.0f,  1.0f, 1.f, 1.f, 0.f }, // Top-right (yellow)
			{ -1.0f,  1.0f, 0.f, 0.f, 1.f }  // Top-left (blue)
		};
		*/
		m_vertices = {
			{0.5f,  0.5f, 0.0f},  // top right
			{0.5f, -0.5f, 0.0f },  // bottom right
			{-0.5f, -0.5f, 0.0f},  // bottom left
			{-0.5f,  0.5f, 0.0f}   // top left 
		};

		m_indices = {  // note that we start from 0!
			{0, 1, 3 },  // first Triangle
			{1, 2, 3 }  // second Triangle
		};

		
		m_vertexBuffer = 0;
		

		m_vertexShader  = glCreateShader(GL_VERTEX_SHADER);
		m_fragmentShader= glCreateShader(GL_FRAGMENT_SHADER);
		m_shaderProgram = 0;



	}

	
	Shader::~Shader()
	{
	}

	void Shader::SetVertexBufferDynamic(const std::vector<Vertex>& vertices)
	{
		m_vertices = vertices;
		glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(), m_vertices.data(), GL_DYNAMIC_DRAW);

	}

	void Shader::SetData(const float* buf, int nSamples)
	{
		if (m_data.size() != nSamples)
			m_data = std::vector<float>(nSamples, 0);
		memcpy(m_data.data(), buf, nSamples * sizeof(float));
	}

	void Shader::Compile()
	{
		CompileVertexShader();
		checkOpenGLError();
		CompileFragmentShader();
		checkOpenGLError();
		CompileProgram();
		checkOpenGLError();

	}

	bool Shader::CompileVertexShader()
	{
		const char* str = m_vertexShaderText.c_str();
		glShaderSource(m_vertexShader, 1, &str, NULL);
		glCompileShader(m_vertexShader);

		GLint compileStatus;
		glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &compileStatus);

		if (compileStatus != GL_TRUE)
		{
			GLint logLength = 0;
			glGetShaderiv(m_vertexShader, GL_INFO_LOG_LENGTH, &logLength);

			// Retrieve the error log
			std::vector<GLchar> errorLog(logLength);
			glGetShaderInfoLog(m_vertexShader, logLength, nullptr, errorLog.data());
			std::string str(errorLog.begin(), errorLog.end());
			std::cout << "vertex shader failed to compile " << str;
		}

		return compileStatus == GL_TRUE;
	}

	bool Shader::CompileFragmentShader()
	{
		const char* str = m_fragmentShaderText.c_str();
		glShaderSource(m_fragmentShader, 1, &str, NULL);
		glCompileShader(m_fragmentShader);

		GLint compileStatus;
		glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &compileStatus);

		if (compileStatus != GL_TRUE)
		{
			GLint logLength = 0;
			glGetShaderiv(m_fragmentShader, GL_INFO_LOG_LENGTH, &logLength);

			// Retrieve the error log
			std::vector<GLchar> errorLog(logLength);
			glGetShaderInfoLog(m_fragmentShader, logLength, nullptr, errorLog.data());
			std::string str(errorLog.begin(), errorLog.end());
			std::cout<< "fragment shader failed to compile"<< str;;
		}


		return compileStatus == GL_TRUE;
	}

	void Shader::CompileProgram()
	{
		m_shaderProgram = glCreateProgram();
		glAttachShader(m_shaderProgram, m_vertexShader);
		glAttachShader(m_shaderProgram, m_fragmentShader);
		glLinkProgram(m_shaderProgram);

		GLint linkStatus;
		glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &linkStatus);

		if (linkStatus != GL_TRUE) {
			// Program linking failed, get the error log
			GLint logLength = 0;
			glGetProgramiv(m_shaderProgram, GL_INFO_LOG_LENGTH, &logLength);

			if (logLength > 0) {
				std::vector<char> infoLog(logLength);
				glGetProgramInfoLog(m_shaderProgram, logLength, nullptr, infoLog.data());
				std::cout << "Shader program linking failed:\n" << infoLog.data() << std::endl;
			}
			else {
				std::cout << "Shader program linking failed with no additional info." << std::endl;
			}
		}
		else {
			std::cout << "Shader program linked successfully!" << std::endl;
		}

	}

	void Shader::CommitData()
	{
		UploadSSBOData(m_data);
	}

	void Shader::UploadSSBOData(const std::vector<float>& data) {
		checkOpenGLError();

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
		checkOpenGLError();

		glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(float), data.data(), GL_DYNAMIC_DRAW);

		checkOpenGLError();


		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		checkOpenGLError();

		GLint ssboSizeLocation = glGetUniformLocation(m_shaderProgram, "ssboSize");
		checkOpenGLError();

		glUniform1i(ssboSizeLocation, data.size());

		checkOpenGLError();
	}

	void Shader::UploadSSBOData(const float* data, const int size)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(float), data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		GLint ssboSizeLocation = glGetUniformLocation(m_shaderProgram, "ssboSize");
		glUniform1i(ssboSizeLocation, size);
		checkOpenGLError();
	}

	void Shader::ReleaseShader()
	{
	}

	std::string Shader::readShaderFile(const std::string& fileName)
	{
		std::ifstream file(fileName); // Open the file
		if (!file.is_open()) { // Check if the file is successfully opened
			throw std::runtime_error("Unable to open the file: " + fileName);
		}

		std::stringstream buffer;
		buffer << file.rdbuf(); // Read the entire file into the buffer
		return buffer.str(); // Return the contents as a string
	}

	GLint Shader::GetUniformLocation(const GLchar * name)
	{
		return glGetUniformLocation(m_shaderProgram, name);
	}

	GLint Shader::GetAttribLocation(const GLchar * name)
	{
		return glGetAttribLocation(m_shaderProgram, name);
	}

}
