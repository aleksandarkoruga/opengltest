
#pragma once
#include "utils/shader.h"
#include <thread>
#include <chrono>
#include <atomic>

//#define TEXTURE_X 1080
//#define TEXTURE_Y 1080

#include <array>

namespace scGraphics{
	static std::atomic<bool> glfwInitializing{ false }; // Indicates if initialization is in progress
	static std::atomic<bool> glfwInitialized{ false };  // Indicates if initialization succeeded


	struct Vertex
	{
		float x, y, z;
	};
	struct Index
	{
		int idx1,idx2,idx3;
	};


	class Shader
	{
	public:
		Shader(std::string shaderFolderPath) ;
		~Shader();

		inline GLuint& GetVertexBuffer() { return m_vertexBuffer; };
		inline GLuint* GetVertexBufferPtr() { return &m_vertexBuffer; };
		inline const std::vector<Vertex>& GetVertexArray() { return m_vertices; };
		inline const std::vector<Index>& GetIndexArray() { return m_indices; };
		inline GLuint* GetSSBO() { return &m_ssbo; };
		void SetData(const float* buf, int nSamples);
		bool Compile();
		GLint GetUniformLocation(const GLchar* name);
		GLint GetAttribLocation(const GLchar* name);
		void SetVertexBufferDynamic(const std::vector<Vertex>& vertices);
		inline GLuint& GetShaderProgram()
		{
			return m_shaderProgram;
		};
		inline GLuint& GetPostProcessShaderProgram()
		{
			return m_shaderPostProgram;
		};
		void CommitData();
		void ReleaseResources()
		{
			if(GetShaderProgram())
				glDeleteProgram(GetShaderProgram());
			if (m_vertexShader)
				glDeleteShader(m_vertexShader);
			if (m_fragmentShader)
			glDeleteShader(m_fragmentShader);


			if (GetPostProcessShaderProgram())
				glDeleteProgram(GetPostProcessShaderProgram());
			if (m_vertexPostShader)
				glDeleteShader(m_vertexPostShader);
			if (m_fragmentShader)
				glDeleteShader(m_fragmentPostShader);
		
		
		}
	private:
		void Shader::UploadSSBOData(const std::vector<float>& data);
		void Shader::UploadSSBOData(const float* data, const int size);


		bool CompileVertexShader(std::string& text, GLuint& vert);
		bool CompileFragmentShader(std::string& text, GLuint& vert);
		bool CompileProgram(GLuint& program, GLuint& vert, GLuint& frag);
		void ReleaseShader();
		std::string readShaderFile(const std::string& fileName);
		//void ExchangeVertexShader();
		//void ExchangeFragmentShader();

		
		
	protected:
		std::vector<Vertex> m_vertices;
		std::vector<Index> m_indices;
		GLuint m_ssbo;
		GLuint m_vertexBuffer;

		GLuint m_vertexShader;
		GLuint m_fragmentShader;
		GLuint m_shaderProgram;	

		GLuint m_vertexPostShader;
		GLuint m_fragmentPostShader;
		GLuint m_shaderPostProgram;

		std::string m_vertexShaderText;
		std::string m_fragmentShaderText;

		std::string m_vertexShaderPostText;
		std::string m_fragmentShaderPostText;

		std::vector<float> m_data;

	};


class GraphicsEngine{
public:
	GraphicsEngine(std::string shaderFolderPath, int widthBackBuffer, int heightBackBuffer);
	~GraphicsEngine();
	void RunEngine();
	void CalculateOneFrame();
	void SwapFBO();
	void SetData(const float* buf, int nSamples);
	const float* GetPixels() const {
		if (!m_bPixReady.load(std::memory_order_acquire)) return nullptr;

		const int idx = m_readIdx.load(std::memory_order_acquire);
		return (idx != -1) ? m_pixBuffers[idx].data() : nullptr;
	}
	
private:
	bool InitEngine();
	void TerminateEngine();

protected:

	std::string m_path;
	int m_width;
	int m_height;

	enum class PROGRAM_STATE { START, RUN, TERMINATE };
	std::atomic<PROGRAM_STATE> m_programState;

	std::thread m_graphicsThread;

	GLFWwindow* m_pWindow;

	
//	GLint m_mvpLocation;
	GLint m_vPosLocation;
	//GLint m_vColLocation;

	

	std::unique_ptr<Shader> m_shader;
	enum class MEMSTATE {READY,FINISHED};
	std::atomic<MEMSTATE> m_bSSBO;
	
	unsigned int m_VBO, m_VAO, m_EBO;
	
	std::vector<GLuint> m_frameBufferSwap;
	std::vector<GLuint> m_textures;
	bool m_bCurrentSwap; 

	//std::vector<float> m_arrPixels;
	//bool m_bPixReady;
	std::array<std::vector<float>, 3> m_pixBuffers; // Triple buffer
	std::atomic<int> m_writeIdx{ 0 }; // Graphics thread writes here
	std::atomic<int> m_readIdx{ -1 }; // Audio thread reads here (-1 = no data)
	std::atomic<bool> m_bPixReady{ false }; // Initial flag for safety
	
	
	//std::pair<GLuint, GLuint> m_frameBufferSwap;
	//std::pair<GLuint, GLuint> m_textures; //output tex
};

} //namespace scGraphics
