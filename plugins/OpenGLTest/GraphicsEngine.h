
#pragma once
#include "utils/shader.h"
#include <thread>
#include <chrono>
#include <atomic>
namespace scGraphics{

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
		Shader() ;
		~Shader();

		inline GLuint& GetVertexBuffer() { return m_vertexBuffer; };
		inline GLuint* GetVertexBufferPtr() { return &m_vertexBuffer; };
		inline const std::vector<Vertex>& GetVertexArray() { return m_vertices; };
		inline const std::vector<Index>& GetIndexArray() { return m_indices; };
		inline GLuint* GetSSBO() { return &m_ssbo; };
		void SetData(const float* buf, int nSamples);
		void Compile();
		GLint GetUniformLocation(const GLchar* name);
		GLint GetAttribLocation(const GLchar* name);
		void SetVertexBufferDynamic(const std::vector<Vertex>& vertices);
		inline GLuint& GetShaderProgram()
		{
			return m_shaderProgram;
		};
		void CommitData();
	private:
		void Shader::UploadSSBOData(const std::vector<float>& data);
		void Shader::UploadSSBOData(const float* data, const int size);


		bool CompileVertexShader();
		bool CompileFragmentShader();
		void CompileProgram();
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
		std::string m_vertexShaderText;
		std::string m_fragmentShaderText;
		std::vector<float> m_data;

	};


class GraphicsEngine{
public:
	GraphicsEngine();
	~GraphicsEngine();
	void RunEngine();
	void CalculateOneFrame();
	void SetData(const float* buf, int nSamples);
private:
	bool InitEngine();

protected:

	enum class PROGRAM_STATE { START, RUN, TERMINATE };
	std::atomic<PROGRAM_STATE> m_programState;

	std::thread m_graphicsThread;

	GLFWwindow* m_pWindow;

	
	GLint m_mvpLocation;
	GLint m_vPosLocation;
	//GLint m_vColLocation;

	GLuint m_frameBuffer;

	std::unique_ptr<Shader> m_shader;
	enum class MEMSTATE {READY,FINISHED};
	std::atomic<MEMSTATE> m_bSSBO;
	
	unsigned int m_VBO, m_VAO, m_EBO;
	
};

} //namespace scGraphics
