
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



	GraphicsEngine::GraphicsEngine(std::string shaderFolderPath, int widthBackBuffer, int heightBackBuffer) :
		m_width(widthBackBuffer),
		m_height(heightBackBuffer),
		m_path(shaderFolderPath),
		m_programState({ PROGRAM_STATE::START }),
		m_graphicsThread(&GraphicsEngine::RunEngine,this),
		m_pWindow(nullptr),
		//m_mvpLocation(0),
		m_vPosLocation(0),
		//m_vColLocation(0),
		m_shader(nullptr),
		m_bSSBO({MEMSTATE::EMPTY}),
		m_VBO(0),
		m_VAO(0),
		m_EBO(0),
		m_frameBufferSwap({ (GLuint)0U, (GLuint)0U }),
		m_textures({ (GLuint)0U, (GLuint)0U }),
		m_bCurrentSwap(false),
		m_pixBuffers{  // Uniform initialization for std::array
				  std::vector<float>(widthBackBuffer * heightBackBuffer * 4, 0.0f),
				  std::vector<float>(widthBackBuffer * heightBackBuffer * 4, 0.0f),
				  std::vector<float>(widthBackBuffer * heightBackBuffer * 4, 0.0f)
					},
		m_bPixReady(false)

		
	{
	}

	GraphicsEngine::~GraphicsEngine()
	{
		m_programState.store(PROGRAM_STATE::TERMINATE);
		//std::this_thread::sleep_for(std::chrono::milliseconds(60));

		if (m_graphicsThread.joinable()) {
			m_graphicsThread.join(); // Ensure thread finishes cleanup
		}
	}

	void GraphicsEngine::RunEngine()
	{
		while (m_programState.load() != PROGRAM_STATE::TERMINATE)
		{


			switch (m_programState.load())
			{
			case PROGRAM_STATE::START:
			{
				if(InitEngine())
					m_programState.store(PROGRAM_STATE::RUN);
				else
					m_programState.store(PROGRAM_STATE::TERMINATE);
			}
			break;
			case PROGRAM_STATE::RUN:
				CalculateOneFrame();
			break;
			default:
				break;
			}

			if (m_programState.load()==PROGRAM_STATE::TERMINATE)
				break;
				//std::this_thread::sleep_for(std::chrono::milliseconds(33));
			//else
				//break;
		}

		TerminateEngine();
		


	}
	void GraphicsEngine::CalculateOneFrame()
	{
		if (m_pWindow && (!glfwWindowShouldClose(m_pWindow)) )
		{
			float ratio;
			//int width= TEXTURE_X, height= TEXTURE_Y;
			

			checkOpenGLError();


			
			//START RENDER TO SWAP FRAMEBUFFER

			glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferSwap[(int)m_bCurrentSwap]);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				std::cout << "Framebuffer is not complete!" << std::endl;
				return;
			}

			checkOpenGLError();

			ratio = m_width / (float)m_height;

			glViewport(0, 0, m_width, m_height);

			checkOpenGLError();
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			checkOpenGLError();

			//mat4x4_identity(m);
			//mat4x4_rotate_Z(m, m, (float)glfwGetTime());
			//mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
			//mat4x4_mul(mvp, p, m);

			checkOpenGLError();

			glUseProgram(m_shader->GetShaderProgram());


			//push previous frame
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D,m_textures[(int)(!m_bCurrentSwap)]);
			glUniform1i(glGetUniformLocation(m_shader->GetShaderProgram(), "previousFrame"), 4);
			
			//push the frame before the previous
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, m_textures[(int)(m_bCurrentSwap)]);
			glUniform1i(glGetUniformLocation(m_shader->GetShaderProgram(), "previousFrame2"), 5);
			
			
			//push to shader

			GLint resolutionLocation = glGetUniformLocation(m_shader->GetShaderProgram(), "resolution");
			glUniform2f(resolutionLocation, static_cast<float>(m_width), static_cast<float>(m_height)  );

			auto expected = MEMSTATE::READY;
			if(m_bSSBO.compare_exchange_weak(expected, MEMSTATE::READING))
				m_shader->CommitData();
			m_bSSBO.store(MEMSTATE::READY);

			checkOpenGLError();
			//glUniformMatrix4fv(m_mvpLocation, 1, GL_FALSE, (const GLfloat*)mvp);
			//checkOpenGLError();

			glBindVertexArray(m_VAO);

			//glBindVertexArray(m_vaoIdpos); 
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			glReadBuffer(GL_COLOR_ATTACHMENT0); // Ensure we read from color attachment 0

			int currentWriteIdx = m_writeIdx.load(std::memory_order_relaxed);
			glReadPixels(
				0, 0,                     // Start position (lower-left corner)
				m_width, m_height,     // Dimensions
				GL_RGBA,                  // Format
				GL_FLOAT,                 // Type
				m_pixBuffers[currentWriteIdx].data()         // Destination buffer
			);
			int expectedReadIdx = m_readIdx.load(std::memory_order_relaxed);
			if (expectedReadIdx != currentWriteIdx) 
			{ 
				m_readIdx.compare_exchange_strong(expectedReadIdx, currentWriteIdx); 
			}      
			// Cycle write index (0→1→2→0...)    
			m_writeIdx.store((currentWriteIdx + 1) % 3, std::memory_order_relaxed);      
			// Signal first valid frame (one-time)     
			if (!m_bPixReady.load()) 
				m_bPixReady.store(true);
			checkOpenGLError();

			glBindVertexArray(0);

			// END RENDER TO SWAP FRAMEBUFFER


			

			//START RENDER TO WINDOW
			glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Bind the default framebuffer (window)
			glClear(GL_COLOR_BUFFER_BIT);          // Clear the window's framebuffer


			int width, height;
			glfwGetFramebufferSize(m_pWindow, &width, &height);

			glViewport(0, 0, width, height);


			float winAspect = (float)width / (float)height;

			mat4x4 p, mvp;

			// Create an identity matrix
			mat4x4_identity(mvp);

			// Adjust scaling based on window aspect ratio
			if (winAspect > 1.0f) {
				// Wider window: squish X to preserve 1:1 aspect ratio
				mat4x4_scale_aniso(mvp, mvp, 1.0f / winAspect, 1.0f, 1.0f);
			}
			else {
				// Taller window: squish Y to preserve 1:1 aspect ratio
				mat4x4_scale_aniso(mvp, mvp, 1.0f, winAspect, 1.0f);
			}

			// Use a fixed orthographic projection (covers [-1, 1] in both axes)
			mat4x4_ortho(p, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
			mat4x4_mul(mvp, p, mvp); // Combine projection and scaling


			// Use the post-processing shader program
			glUseProgram(m_shader->GetPostProcessShaderProgram());

			// Pass the updated MVP matrix to the shader
			GLint mvpLocation = glGetUniformLocation(m_shader->GetPostProcessShaderProgram(), "MVP");
			glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (const GLfloat*)mvp);




			// Bind the texture from the swap framebuffer
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, m_textures[(int)m_bCurrentSwap]);
			
			// Set resolution for the window
			glUniform2f(resolutionLocation, static_cast<float>(width), static_cast<float>(height));

			// Render the quad with the texture to the window (default framebuffer)
			glBindVertexArray(m_VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			checkOpenGLError();

			glBindVertexArray(0);
			//END RENDER TO WINDOW
			glfwSwapBuffers(m_pWindow);
			glfwPollEvents();

			checkOpenGLError();

			SwapFBO();
			//m_bSSBO.store(MEMSTATE::FINISHED);
		}
		
	}

	void GraphicsEngine::SwapFBO()
	{
		m_bCurrentSwap = !m_bCurrentSwap;
		//m_frameBufferSwap = { m_frameBufferSwap.second, m_frameBufferSwap.first }; //swap buffers
		//m_textures = { m_textures.second, m_textures.first };
	}

	void GraphicsEngine::SetData(const float* buf, int nSamples)
	{
		if (!m_shader && m_programState.load() != PROGRAM_STATE::RUN)
			return;

		auto expected = MEMSTATE::READY;
		auto alternative = MEMSTATE::EMPTY;
		if ((m_bSSBO.compare_exchange_weak(expected, MEMSTATE::WRITING, std::memory_order_acquire)) 
			|| (m_bSSBO.compare_exchange_weak(alternative, MEMSTATE::WRITING, std::memory_order_acquire)))
		{
			m_shader->SetData(buf, nSamples);
			

			m_bSSBO.store(MEMSTATE::READY);
		}
	}
	bool GraphicsEngine::InitEngine()
	{
		glfwSetErrorCallback(error_callback);

		if (glfwInitialized.load(std::memory_order_acquire)) {
			goto create_window;
		}

		// Try to start initialization
		bool expected = false;
		if (glfwInitializing.compare_exchange_strong(expected, true,
			std::memory_order_acq_rel)) {
			// This thread handles initialization
			bool success = glfwInit();
			glfwInitialized.store(success, std::memory_order_release);
			glfwInitializing.store(false, std::memory_order_release);
		}
		else {
			// Another thread is initializing. Wait until done.
			while (glfwInitializing.load(std::memory_order_acquire)) {
				std::this_thread::yield(); // Reduce CPU usage
			}
		}

		if (!glfwInitialized.load(std::memory_order_acquire)) {
			// Initialization failed globally; abort
			return false;
		}

	create_window:

		//if (glfwInit())
		{
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


			//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
			//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

			m_pWindow = glfwCreateWindow(640, 480, "Supercollider Renderer", NULL, NULL);
			if (!m_pWindow)
			{
				return false;
			}
			else
			{
				glfwMakeContextCurrent(m_pWindow);
				gladLoadGL();
				glfwSwapInterval(1);

				checkOpenGLError();

				const char* renderer = (const char*)glGetString(GL_RENDERER);
				const char* vendor = (const char*)glGetString(GL_VENDOR);
				const char* version = (const char*)glGetString(GL_VERSION);

				std::cout << "GPU Vendor: " << vendor << std::endl;
				std::cout << "GPU Renderer: " << renderer << std::endl;
				std::cout << "OpenGL Version: " << version << std::endl;

				m_shader = std::make_unique<Shader>(m_path);

				checkOpenGLError();

				if (!m_shader->Compile()) 			
					return false;


				checkOpenGLError();

				//m_mvpLocation = m_shader->GetUniformLocation("MVP");
				
				m_vPosLocation = m_shader->GetAttribLocation("vPos");
				checkOpenGLError();

				//m_vColLocation = m_shader->GetAttribLocation("vCol");
				//checkOpenGLError();
				

				//////////
				//output framebuffer 

				//textures for feedback 

				glGenFramebuffers(2, m_frameBufferSwap.data());
				glGenTextures(2, m_textures.data());


				glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferSwap[0]);
				glBindTexture(GL_TEXTURE_2D, m_textures[0]);

				glTexImage2D(
					GL_TEXTURE_2D,
					0,
					GL_RGBA32F,          // Internal format: 32-bit float per channel
					m_width,
					m_height,
					0,
					GL_RGBA,             // Format: RGBA components
					GL_FLOAT,            // Type: Floating-point data
					nullptr              // No initial data
				);

				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_X, TEXTURE_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textures[0], 0);

				
				
				glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferSwap[1]);
				glBindTexture(GL_TEXTURE_2D, m_textures[1]);
				glTexImage2D(
					GL_TEXTURE_2D,
					0,
					GL_RGBA32F,          // Internal format: 32-bit float per channel
					m_width,
					m_height,
					0,
					GL_RGBA,             // Format: RGBA components
					GL_FLOAT,            // Type: Floating-point data
					nullptr              // No initial data
				);

				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_X, TEXTURE_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);				
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textures[1], 0);
				
				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
					std::cout << "Framebuffer is not complete!" << std::endl;
					return -1;
				}

				//
				
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
				return true;
			}
		}


		return false;
	}

	void GraphicsEngine::TerminateEngine()
	{
		//TERMINATE CALLED DELETE RESOURCES ON GRAPHICS THREAD
		/*
		for (auto frameBuffer : m_frameBufferSwap)
			if (glIsFramebuffer(frameBuffer)) {
				std::cout << "valid framebuffer: " << frameBuffer << std::endl;
			}
		*/
		if (glIsFramebuffer(m_frameBufferSwap[0]))
			glDeleteFramebuffers(2, &m_frameBufferSwap[0]);

		/*
		for (auto tex : m_textures)
			if (glIsTexture(tex)) {
				std::cout << "valid texture: " << tex << std::endl;
			}
		*/
		if (glIsTexture(m_textures[0]))
			glDeleteTextures(2, &m_textures[0]);

		m_shader->ReleaseResources();
		if (m_VBO && glIsBuffer(m_VBO))
			glDeleteBuffers(1, &m_VBO);
		if (m_VAO && glIsVertexArray(m_VAO))
			glDeleteBuffers(1, &m_VAO);
		if (m_EBO && glIsBuffer(m_EBO))
			glDeleteBuffers(1, &m_EBO);
	
		if (m_pWindow)
			glfwDestroyWindow(m_pWindow);
		//glfwTerminate();
	}

	Shader::Shader(std::string shaderFolderPath) :m_data({})
	{
		//std::filesystem::path cwd = std::filesystem::current_path();
		//std::cout << cwd;
		const std::string vertFName = shaderFolderPath +"shader.vert";
		const std::string fragFName = shaderFolderPath +"shader.frag";

		const std::string vertPostFName = shaderFolderPath + "post.vert";
		const std::string fragPostFName = shaderFolderPath + "post.frag";

		m_vertexShaderText = readShaderFile(vertFName);
		m_fragmentShaderText = readShaderFile(fragFName);

		m_vertexShaderPostText = readShaderFile(vertPostFName);
		m_fragmentShaderPostText = readShaderFile(fragPostFName);
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
						{1.0f,  1.0f, 0.0f},  // top right
						{1.0f, -1.0f, 0.0f },  // bottom right
						{-1.0f, -1.0f, 0.0f},  // bottom left
						{-1.0f,  1.0f, 0.0f}   // top left 
		};

		m_indices = {  // note that we start from 0!
			{0, 1, 3 },  // first Triangle
			{1, 2, 3 }  // second Triangle
		};

		
		m_vertexBuffer = 0;
		

		m_vertexShader  = glCreateShader(GL_VERTEX_SHADER);
		m_fragmentShader= glCreateShader(GL_FRAGMENT_SHADER);
		m_vertexPostShader = glCreateShader(GL_VERTEX_SHADER);
		m_fragmentPostShader = glCreateShader(GL_FRAGMENT_SHADER);
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

	bool Shader::Compile()
	{
		bool valid = true;
		valid &= CompileVertexShader(m_vertexShaderText, m_vertexShader);
		checkOpenGLError();
		valid &= CompileFragmentShader(m_fragmentShaderText, m_fragmentShader);
		checkOpenGLError();
		valid &= CompileProgram(m_shaderProgram, m_vertexShader, m_fragmentShader );
		checkOpenGLError();

		valid &= CompileVertexShader(m_vertexShaderPostText, m_vertexPostShader);
		checkOpenGLError();
		valid &= CompileFragmentShader(m_fragmentShaderPostText, m_fragmentPostShader);
		checkOpenGLError();
		valid &= CompileProgram(m_shaderPostProgram, m_vertexPostShader, m_fragmentPostShader);
		checkOpenGLError();

		if (!valid)
			ReleaseResources();
		return valid;


	}

	bool Shader::CompileVertexShader(std::string& text, GLuint& vert)
	{
		const char* str = text.c_str();
		glShaderSource(vert, 1, &str, NULL);
		glCompileShader(vert);

		GLint compileStatus;
		glGetShaderiv(vert, GL_COMPILE_STATUS, &compileStatus);

		if (compileStatus != GL_TRUE)
		{
			GLint logLength = 0;
			glGetShaderiv(vert, GL_INFO_LOG_LENGTH, &logLength);

			// Retrieve the error log
			std::vector<GLchar> errorLog(logLength);
			glGetShaderInfoLog(vert, logLength, nullptr, errorLog.data());
			std::string str(errorLog.begin(), errorLog.end());
			std::cout << "vertex shader failed to compile " << str;
		}

		return compileStatus == GL_TRUE;
	}

	bool Shader::CompileFragmentShader(std::string& text, GLuint& frag)
	{
		const char* str = text.c_str();
		glShaderSource(frag, 1, &str, NULL);
		glCompileShader(frag);

		GLint compileStatus;
		glGetShaderiv(frag, GL_COMPILE_STATUS, &compileStatus);

		if (compileStatus != GL_TRUE)
		{
			GLint logLength = 0;
			glGetShaderiv(frag, GL_INFO_LOG_LENGTH, &logLength);

			// Retrieve the error log
			std::vector<GLchar> errorLog(logLength);
			glGetShaderInfoLog(frag, logLength, nullptr, errorLog.data());
			std::string str(errorLog.begin(), errorLog.end());
			std::cout<< "fragment shader failed to compile"<< str;;
		}


		return compileStatus == GL_TRUE;
	}

	bool Shader::CompileProgram(GLuint& program, GLuint& vert, GLuint& frag)
	{
		program = glCreateProgram();
		glAttachShader(program, vert);
		glAttachShader(program, frag);
		glLinkProgram(program);

		GLint linkStatus;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

		if (linkStatus != GL_TRUE) {
			// Program linking failed, get the error log
			GLint logLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

			if (logLength > 0) {
				std::vector<char> infoLog(logLength);
				glGetProgramInfoLog(program, logLength, nullptr, infoLog.data());
				std::cout << "Shader program linking failed:\n" << infoLog.data() << std::endl;
			}
			else {
				std::cout << "Shader program linking failed with no additional info." << std::endl;
			}
		}
		else {
			std::cout << "Shader program linked successfully!" << std::endl;
		}
		return static_cast<bool>(linkStatus);
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
