// PluginOpenGLTest.cpp
// Aleksandar Koruga (aleksandar.koruga@gmail.com)

#include "SC_PlugIn.hpp"
#include "OpenGLTest.hpp"

static InterfaceTable* ft;

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

namespace OpenGLTest {

	OpenGLTest::OpenGLTest() : m_engine(), m_pWindow(nullptr), m_samps(0ULL),m_prevFrame(0ULL) 
	{
		
		mCalcFunc = make_calc_function<OpenGLTest, &OpenGLTest::next>();
		next(1);

		glfwSetErrorCallback(error_callback);

		if(glfwInit())
		{
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

			m_pWindow = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
			if (!m_pWindow)
			{
				glfwTerminate();
			}
			else
			{
				glfwMakeContextCurrent(m_pWindow);
				gladLoadGL();
				glfwSwapInterval(1);

				glGenBuffers(1, &vertex_buffer);
				glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

				vertex_shader = glCreateShader(GL_VERTEX_SHADER);
				glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
				glCompileShader(vertex_shader);

				fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
				glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
				glCompileShader(fragment_shader);

				program = glCreateProgram();
				glAttachShader(program, vertex_shader);
				glAttachShader(program, fragment_shader);
				glLinkProgram(program);

				mvp_location = glGetUniformLocation(program, "MVP");
				vpos_location = glGetAttribLocation(program, "vPos");
				vcol_location = glGetAttribLocation(program, "vCol");

				glEnableVertexAttribArray(vpos_location);
				glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
					sizeof(vertices[0]), (void*)0);
				glEnableVertexAttribArray(vcol_location);
				glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
					sizeof(vertices[0]), (void*)(sizeof(float) * 2));


			}
		}

	}

	OpenGLTest::~OpenGLTest()
	{
		if(m_pWindow)
			glfwDestroyWindow(m_pWindow);

		glfwTerminate();
	}

void OpenGLTest::next(int nSamples) {
    const float* input = in(0);
    const float* gain = in(1);
    float* outbuf = out(0);

    // simple gain function
    for (int i = 0; i < nSamples; ++i) {

		++m_samps;
		auto nFrame = static_cast<uint64_t>( 30.0 * static_cast<double>( m_samps) / sampleRate());

		if (nFrame != m_prevFrame)
		{
			if (m_pWindow && (!glfwWindowShouldClose(m_pWindow)))
			{
				float ratio;
				int width, height;
				mat4x4 m, p, mvp;

				glfwGetFramebufferSize(m_pWindow, &width, &height);
				ratio = width / (float)height;

				glViewport(0, 0, width, height);
				glClear(GL_COLOR_BUFFER_BIT);

				mat4x4_identity(m);
				mat4x4_rotate_Z(m, m, (float)glfwGetTime());
				mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
				mat4x4_mul(mvp, p, m);
				
				glUseProgram(program);
				glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
				glDrawArrays(GL_TRIANGLES, 0, 3);

				glfwSwapBuffers(m_pWindow);
				glfwPollEvents();
			}
			m_prevFrame = nFrame;

		}

        outbuf[i] = input[i] * gain[i];
    }
}

} // namespace OpenGLTest

PluginLoad(OpenGLTestUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<OpenGLTest::OpenGLTest>(ft, "OpenGLTest", false);
}
