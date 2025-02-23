
#pragma once

#include "SCGraphics.hpp"

namespace scGraphics {

class Shader
{
public:
	Shader(std::string shaderFolderPath, int ssboSize);
	~Shader();

	inline GLuint& GetVertexBuffer() { return m_vertexBuffer; };
	inline GLuint* GetVertexBufferPtr() { return &m_vertexBuffer; };
	inline const std::vector<Vertex>& GetVertexArray() { return m_vertices; };
	inline const std::vector<Index>& GetIndexArray() { return m_indices; };
	inline GLuint* GetSSBO() { return &m_ssbo; };
	//void SetData(const float* buf, int nSamples);
	bool Compile();
	GLint GetUniformLocation(const GLchar* name);
	GLint GetAttribLocation(const GLchar* name);
	void SetVertexBufferDynamic(const std::vector<Vertex>& vertices);
	inline std::vector<float>& GetDataVector() { return m_data; };
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
		if (GetShaderProgram())
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



} //namespace scGraphics
