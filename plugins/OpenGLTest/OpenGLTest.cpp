// PluginOpenGLTest.cpp
// Aleksandar Koruga (aleksandar.koruga@gmail.com)

#include "SC_PlugIn.hpp"
#include "OpenGLTest.hpp"

static InterfaceTable* ft;



namespace OpenGLTest {

#define AUDIO_INPUT  0
#define AUDIO_GAIN   1
#define X_INPUT_READ 2
#define Y_INPUT_READ 3
#define TEX_WIDTH    4
#define TEX_HEIGHT   5
#define N_VECTORS    6
#define PATH_INPUT   7

	OpenGLTest::OpenGLTest() :
		m_buf() ,
		m_cpyIdx(0),
		m_engine(nullptr),
		m_width(512), 
		m_height(512),
		m_interpState()
		
		
	{
		m_nVectors = std::pow(2, static_cast<int>( in0(N_VECTORS) ));

		m_buf = std::vector<float>(mWorld->mBufLength * m_nVectors, 0.f);


		m_width = in0(TEX_WIDTH);
		m_height = in0(TEX_HEIGHT);

		int filename_length = in0(PATH_INPUT);
		
		auto path = std::string(filename_length, '!');
		for (int i = 0; i < filename_length; i++) {
			path[i] = static_cast<char>(in0(i + 1 + PATH_INPUT));
		}
		m_engine= std::make_unique<scGraphics::GraphicsEngine>(path, m_width, m_height, mWorld->mBufLength, m_nVectors);
		mCalcFunc = make_calc_function<OpenGLTest, &OpenGLTest::next>();
		next(1);
		

	}

	OpenGLTest::~OpenGLTest()
	{
		
	}

void OpenGLTest::next(int nSamples) {
    const float* input = in(AUDIO_INPUT);
	const float* gain = in(AUDIO_GAIN);
	const float* pix_x = in(X_INPUT_READ);
	const float* pix_y = in(Y_INPUT_READ);


    float* outbuf = out(0);

	memcpy(m_buf.data() + nSamples * m_cpyIdx, input, sizeof(float) * nSamples);
	++m_cpyIdx;
	m_cpyIdx &= (m_nVectors-1);

	m_engine->SetData(m_buf.data(), m_cpyIdx);

    // simple gain function


	//const float* pixels = nullptr;
	bool newFrame = false;

	const auto* pixels = m_engine->GetPixels(newFrame);
//		pixels = m_engine->GetPixels();
	const float* prevPixels = newFrame ? m_engine->GetPrevPixels() : nullptr;
	

    for (int i = 0; i < nSamples; ++i) {
		
		if (pixels)
		{
			if ((!newFrame) || (!prevPixels))
			{
				outbuf[i] = GetBilinear(pix_x[i], pix_y[i], pixels) * gain[i];
			}
			else
			{
				float crossFade = i/static_cast<float>(nSamples);
				outbuf[i] = ((1.0f-crossFade) * GetBilinear(pix_x[i], pix_y[i], prevPixels) + crossFade* GetBilinear(pix_x[i], pix_y[i], pixels) )* gain[i];
			}
		}
		else
			outbuf[i] = 0;
    }



}

float OpenGLTest::GetBilinear(const float& x, const float& y, const float* pixels)
{
	if (!pixels)
		return 0;

	m_interpState.xCoord = x * static_cast<float>(m_width);
	m_interpState.yCoord = y * static_cast<float>(m_height);

	m_interpState.xFrac = sc_frac(m_interpState.xCoord);
	m_interpState.yFrac = sc_frac(m_interpState.yCoord);


	m_interpState.x = sc_wrap(static_cast<int>(sc_floor(m_interpState.xCoord)), 0, m_width - 1);
	m_interpState.y = sc_wrap(static_cast<int>(sc_floor(m_interpState.yCoord)), 0, m_height - 1);

	m_interpState.x1 = sc_wrap(static_cast<int>(sc_floor(m_interpState.xCoord)) + 1, 0, m_width - 1);
	m_interpState.y1 = sc_wrap(static_cast<int>(sc_floor(m_interpState.yCoord)) + 1, 0, m_height - 1);

	// Calculate the indices for the four surrounding points
	m_interpState.srcIdx00 = 4 * (m_interpState.y * m_width + m_interpState.x);      // (x, y)
	m_interpState.srcIdx10 = 4 * (m_interpState.y * m_width + m_interpState.x1);     // (x1, y)
	m_interpState.srcIdx01 = 4 * (m_interpState.y1 * m_width + m_interpState.x);     // (x, y1)
	m_interpState.srcIdx11 = 4 * (m_interpState.y1 * m_width + m_interpState.x1);    // (x1, y1)



	// Retrieve the pixel values at the four surrounding points
	m_interpState.pixel00 = pixels[m_interpState.srcIdx00];
	m_interpState.pixel10 = pixels[m_interpState.srcIdx10];
	m_interpState.pixel01 = pixels[m_interpState.srcIdx01];
	m_interpState.pixel11 = pixels[m_interpState.srcIdx11];

	// Perform bilinear interpolation
	return
		m_interpState.pixel00 * (1.0f - m_interpState.xFrac) * (1.0f - m_interpState.yFrac) +
		m_interpState.pixel10 * m_interpState.xFrac * (1.0f - m_interpState.yFrac) +
		m_interpState.pixel01 * (1.0f - m_interpState.xFrac) * m_interpState.yFrac +
		m_interpState.pixel11 * m_interpState.xFrac * m_interpState.yFrac;

}

#undef AUDIO_INPUT
#undef AUDIO_GAIN  
#undef X_INPUT_READ 
#undef Y_INPUT_READ 
#undef TEX_WIDTH 
#undef TEX_HEIGHT
#undef N_VECTORS
#undef PATH_INPUT 

} // namespace OpenGLTest

PluginLoad(OpenGLTestUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<OpenGLTest::OpenGLTest>(ft, "OpenGLTest", false);
}


