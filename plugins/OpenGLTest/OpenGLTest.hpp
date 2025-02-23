// PluginOpenGLTest.hpp
// Aleksandar Koruga (aleksandar.koruga@gmail.com)

#pragma once
#include "GraphicsEngine.h"

#include "SC_PlugIn.hpp"

namespace OpenGLTest {

class OpenGLTest : public SCUnit {
public:
    OpenGLTest();

    // Destructor
    ~OpenGLTest();

private:




    // Calc function
    void next(int nSamples);
	/*GLFWwindow* m_pWindow;
	uint64_t m_samps;
	uint64_t m_prevFrame;
	
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;
    GLuint frameBuffer;*/

    float GetBilinear(const float& x, const float& y, const float* pixels);

    std::vector<float> m_buf;

    int m_cpyIdx;
    std::unique_ptr< scGraphics::GraphicsEngine> m_engine;
    
    int m_nVectors;
    int m_width, m_height;


    struct InterpolationState {
        int x = 0;
        int y = 0;
        int x1 = 0;
        int y1 = 0;
        float xCoord = 0.f;
        float yCoord = 0.f;
        float xFrac = 0.f;
        float yFrac = 0.f;
        int srcIdx00 = 0;
        int srcIdx10 = 0;
        int srcIdx01 = 0;
        int srcIdx11 = 0;
        float pixel00 = 0;
        float pixel10 = 0;
        float pixel01 = 0;
        float pixel11 = 0;
    } m_interpState;
    // Member variables
};

} // namespace OpenGLTest
