


#include "Shader.hpp"

namespace scGraphics {



	Shader::Shader(std::string shaderFolderPath, int ssboSize) :m_data(ssboSize, 0)
	{
		//std::filesystem::path cwd = std::filesystem::current_path();
		//std::cout << cwd;
		const std::string vertFName = shaderFolderPath + "shader.vert";
		const std::string fragFName = shaderFolderPath + "shader.frag";

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


		m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
		m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
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

	bool Shader::Compile()
	{
		bool valid = true;

		valid &= (m_vertexShaderText != "") && (m_fragmentShaderText != "") && (m_vertexShaderPostText != "") && (m_fragmentShaderPostText != "");

		if (valid) {
			valid &= CompileVertexShader(m_vertexShaderText, m_vertexShader);
			checkOpenGLError();
			valid &= CompileFragmentShader(m_fragmentShaderText, m_fragmentShader);
			checkOpenGLError();
			valid &= CompileProgram(m_shaderProgram, m_vertexShader, m_fragmentShader);
			checkOpenGLError();

			valid &= CompileVertexShader(m_vertexShaderPostText, m_vertexPostShader);
			checkOpenGLError();
			valid &= CompileFragmentShader(m_fragmentShaderPostText, m_fragmentPostShader);
			checkOpenGLError();
			valid &= CompileProgram(m_shaderPostProgram, m_vertexPostShader, m_fragmentPostShader);
			checkOpenGLError();
		}
		else 
		{
			std::cout << "failed to open shader file.";
		}

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
			std::cout << "fragment shader failed to compile" << str;;
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
			return "";//std::cout("Unable to open the file: " + fileName);
		}

		std::stringstream buffer;
		buffer << file.rdbuf(); // Read the entire file into the buffer
		return buffer.str(); // Return the contents as a string
	}

	GLint Shader::GetUniformLocation(const GLchar* name)
	{
		return glGetUniformLocation(m_shaderProgram, name);
	}

	GLint Shader::GetAttribLocation(const GLchar* name)
	{
		return glGetAttribLocation(m_shaderProgram, name);
	}

}//namespace scGraphics
