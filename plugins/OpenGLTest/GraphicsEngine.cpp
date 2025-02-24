
#pragma once
#include "GraphicsEngine.h"

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




	GraphicsEngine::GraphicsEngine(std::string shaderFolderPath, int widthBackBuffer, int heightBackBuffer, int audioVectorSize, int nVectors) :
		m_path(shaderFolderPath),
		m_width(widthBackBuffer),
		m_height(heightBackBuffer),
		m_ratio(m_width / static_cast<float>(m_height)),
		m_audioVectorSize(audioVectorSize),
		m_nVectors(nVectors),
		m_cpyIdx(0),
		m_programState({ PROGRAM_STATE::START }),
		m_graphicsThread(&GraphicsEngine::RunEngine,this),
		m_pWindow(nullptr),
		//m_mvpLocation(0),
		m_vPosLocation(0),
		//m_vColLocation(0),
		m_pShader(nullptr),
		m_bSSBO({MEMSTATE::EMPTY}),
		m_VBO(0),
		m_VAO(0),
		m_EBO(0),
		m_frameBufferSwap({ (GLuint)0U, (GLuint)0U }),
		m_textures({ (GLuint)0U, (GLuint)0U }),
		m_bCurrentSwap(false),
		m_pixBuffers{  // Uniform initialization for std::array
				  std::vector<float>(widthBackBuffer* heightBackBuffer * 4, 0.0f),
				  std::vector<float>(widthBackBuffer* heightBackBuffer * 4, 0.0f),
				  std::vector<float>(widthBackBuffer * heightBackBuffer * 4, 0.0f),
				  std::vector<float>(widthBackBuffer * heightBackBuffer * 4, 0.0f),
				  std::vector<float>(widthBackBuffer* heightBackBuffer * 4, 0.0f),
				  std::vector<float>(widthBackBuffer* heightBackBuffer * 4, 0.0f),
				  std::vector<float>(widthBackBuffer* heightBackBuffer * 4, 0.0f),
				  std::vector<float>(widthBackBuffer* heightBackBuffer * 4, 0.0f)
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
			//int width= TEXTURE_X, height= TEXTURE_Y;
			

			checkOpenGLError();


			
			//START RENDER TO SWAP FRAMEBUFFER

			glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferSwap[(int)m_bCurrentSwap]);
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				std::cout << "Framebuffer is not complete!" << std::endl;
				return;
			}

			checkOpenGLError();


			glViewport(0, 0, m_width, m_height);

			checkOpenGLError();

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			checkOpenGLError();

			glUseProgram(m_pShader->GetShaderProgram());


			//push previous frame
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D,m_textures[(int)(!m_bCurrentSwap)]);
			glUniform1i(glGetUniformLocation(m_pShader->GetShaderProgram(), "previousFrame"), 4);
			
			//push the frame before the previous
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, m_textures[(int)(m_bCurrentSwap)]);
			glUniform1i(glGetUniformLocation(m_pShader->GetShaderProgram(), "previousFrame2"), 5);
			
			
			//push to shader

			GLint resolutionLocation = glGetUniformLocation(m_pShader->GetShaderProgram(), "resolution");
			glUniform2f(resolutionLocation, static_cast<float>(m_width), static_cast<float>(m_height)  );

			

			GLint idxLocation = glGetUniformLocation(m_pShader->GetShaderProgram(), "lastWriteIdx");
			glUniform1i(idxLocation, static_cast<int>(m_cpyIdx * m_audioVectorSize));
			checkOpenGLError();

			auto expected = MEMSTATE::READY;
			if(m_bSSBO.compare_exchange_weak(expected, MEMSTATE::READING))
				m_pShader->CommitData();
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

			int lastRead = m_lastReadIdx.load(std::memory_order_relaxed);
			int currentRead = m_currentReadIdx.load(std::memory_order_relaxed);
			
			int nextWriteIdx = currentWriteIdx;
			while ((nextWriteIdx == currentRead) || (nextWriteIdx == lastRead) || (nextWriteIdx == currentWriteIdx))
				nextWriteIdx = (nextWriteIdx + 1) & 7;

			m_writeIdx.store(nextWriteIdx, std::memory_order_relaxed);
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

			mat4x4 mvp;
			GetOrthoMVP(&mvp, width, height);

			// Use the post-processing shader program
			glUseProgram(m_pShader->GetPostProcessShaderProgram());

			// Pass the updated MVP matrix to the shader
			GLint mvpLocation = glGetUniformLocation(m_pShader->GetPostProcessShaderProgram(), "MVP");
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

			m_bCurrentSwap = !m_bCurrentSwap;

			//m_bSSBO.store(MEMSTATE::FINISHED);
		}
		
	}

	//cpyIxd is the nth vector last writen in the audio engine 
	void GraphicsEngine::SetData(const float* buf,const int& cpyIdx)
	{
		if (!m_pShader && m_programState.load() != PROGRAM_STATE::RUN)
			return;

		auto expected = MEMSTATE::READY;
		auto alternative = MEMSTATE::EMPTY;
		if ((m_bSSBO.compare_exchange_weak(expected, MEMSTATE::WRITING, std::memory_order_acquire)) 
			|| (m_bSSBO.compare_exchange_weak(alternative, MEMSTATE::WRITING, std::memory_order_acquire)))
		{

			float* pData = m_pShader->GetDataVector().data();

			if (pData) 
			{
				//we need to reach cpyIdx

				if (m_cpyIdx != cpyIdx)
				{
					//wrap around end of buffer 
					if (m_cpyIdx > cpyIdx)
					{
						auto nCpy = (m_nVectors - m_cpyIdx);
						memcpy(pData + m_cpyIdx * m_audioVectorSize, buf + m_cpyIdx * m_audioVectorSize, m_audioVectorSize * nCpy * sizeof(float));
						m_cpyIdx = 0;
					}
					//both are not 0
					if (m_cpyIdx != cpyIdx) //redundant check if cpyIdx is 0 or cpyIdx<m_cpyidx but less repeated code
					{
						//copy until cpy 
						auto nCpy = (cpyIdx - m_cpyIdx);
						memcpy(pData + m_cpyIdx * m_audioVectorSize, buf + m_cpyIdx * m_audioVectorSize, m_audioVectorSize * nCpy * sizeof(float));
						m_cpyIdx += nCpy;
						m_cpyIdx &= (m_nVectors - 1);
					}
				}
				//in case of same index copy whole buffer
				else
				{
					memcpy(pData, buf, m_audioVectorSize * m_nVectors * sizeof(float));
					m_cpyIdx = 0;

				}

				m_bSSBO.store(MEMSTATE::READY);
			}
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

				m_pShader = std::make_unique<Shader>(m_path, m_nVectors*m_audioVectorSize);

				checkOpenGLError();

				if (!m_pShader->Compile()) 			
					return false;


				checkOpenGLError();

				//m_mvpLocation = m_pShader->GetUniformLocation("MVP");
				
				m_vPosLocation = m_pShader->GetAttribLocation("vPos");
				checkOpenGLError();

				//m_vColLocation = m_pShader->GetAttribLocation("vCol");
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
					return false;
				}

				//
				
				glGenVertexArrays(1, &m_VAO);
				glGenBuffers(1, &m_VBO);
				glGenBuffers(1, &m_EBO);

				glGenBuffers(1, m_pShader->GetSSBO());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, *m_pShader->GetSSBO());
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, *m_pShader->GetSSBO()); // Binding point 0 matches shader
				// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
				glBindVertexArray(m_VAO);

				glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(m_pShader->GetVertexArray()[0]) * m_pShader->GetVertexArray().size(), m_pShader->GetVertexArray().data(), GL_STATIC_DRAW);


				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_pShader->GetIndexArray()[0]) * m_pShader->GetIndexArray().size(), m_pShader->GetIndexArray().data(), GL_STATIC_DRAW);

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

		m_pShader->ReleaseResources();
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

}
