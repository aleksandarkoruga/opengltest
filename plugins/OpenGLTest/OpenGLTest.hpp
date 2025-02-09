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
    std::vector<float> m_buf;
    int m_cpyIdx;
    std::unique_ptr< scGraphics::GraphicsEngine> m_engine;
    
    int m_width, m_height;
    // Member variables
};

} // namespace OpenGLTest
