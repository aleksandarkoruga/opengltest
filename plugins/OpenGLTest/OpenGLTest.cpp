// PluginOpenGLTest.cpp
// Aleksandar Koruga (aleksandar.koruga@gmail.com)

#include "SC_PlugIn.hpp"
#include "OpenGLTest.hpp"

static InterfaceTable* ft;


namespace OpenGLTest {

	OpenGLTest::OpenGLTest() :
		m_buf(mWorld->mBufLength*32,0.f) ,
		m_cpyIdx(0),
		m_engine(nullptr),
		m_width(512), 
		m_height(512)
		
		//, m_pWindow(nullptr), m_samps(0ULL),m_prevFrame(0ULL) 
	{

		int filename_length = in0(4);
		//std::cout << filename_length << std::endl;
		// char path[filename_length];
		auto path = std::string(filename_length, '!');
		for (int i = 0; i < filename_length; i++) {
			path[i] = static_cast<char>(in0(i + 1 + 4));
		}
		m_engine= std::make_unique<scGraphics::GraphicsEngine>(path, m_width, m_height);
		mCalcFunc = make_calc_function<OpenGLTest, &OpenGLTest::next>();
		next(1);
		/*
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
		}*/

	}

	OpenGLTest::~OpenGLTest()
	{
		//if(m_pWindow)
		//	glfwDestroyWindow(m_pWindow);

		//glfwTerminate();
	}

void OpenGLTest::next(int nSamples) {
    const float* input = in(0);
	const float* gain = in(1);
	const float* pix_x = in(2);
	const float* pix_y = in(3);


    float* outbuf = out(0);

	memcpy(m_buf.data() + nSamples * m_cpyIdx, input, sizeof(float) * nSamples);
	++m_cpyIdx;
	m_cpyIdx &= 31;

	m_engine->SetData(m_buf.data(), m_buf.size());

    // simple gain function


	const float* pixels = nullptr;
	
	if(m_engine->IsPixReady())
		pixels = m_engine->GetPixels().data();
	int x, y;

    for (int i = 0; i < nSamples; ++i) {

		x = static_cast<int>( std::floor(sc_wrap(pix_x[i], 0.0f, 1.0f)*(m_width-1)));
		y = static_cast<int>(std::floor(sc_wrap(pix_y[i], 0.0f, 1.0f)*(m_height-1)));

		const int srcIdx = 4 * (y * m_width + x);

		
		if (pixels)
			outbuf[i] = pixels[srcIdx] * gain[i];
		else
			outbuf[i] = 0;
    }
}

} // namespace OpenGLTest

PluginLoad(OpenGLTestUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<OpenGLTest::OpenGLTest>(ft, "OpenGLTest", false);
}
