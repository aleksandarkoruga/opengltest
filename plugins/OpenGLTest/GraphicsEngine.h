
#pragma once
#include "utils/shader.h"
#include <thread>

namespace scGraphics{

	class Shader
	{
	public:
		Shader();
		~Shader();

		inline GLuint& GetVertexBuffer() { return m_vertexBuffer; };

		void Compile();
		GLint& GetUniformLocation(const GLchar* name);
		GLint& GetAttribLocation(const GLchar* name);

	private:
		void CompileVertexShader();
		void CompileFragmentShader();
		void CompileProgram();
		void ReleaseShader();
		//void ExchangeVertexShader();
		//void ExchangeFragmentShader();

		
		
	protected:
		GLuint m_vertexBuffer;
		GLuint m_vertexShader;
		GLuint m_fragmentShader;
		GLuint m_shaderProgram;	




	};


class GraphicsEngine{
public:
	GraphicsEngine();
	~GraphicsEngine();
	void RunEngine();

private:
	bool InitEngine();


	std::thread m_graphicsThread;

protected:
	GLFWwindow* m_pWindow;

	
	GLint m_mvpLocation;
	GLint m_vPosLocation;
	GLint m_vColLocation;

	GLuint m_frameBuffer;

	Shader m_shader;


};

} //namespace scGraphics
