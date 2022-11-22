// PluginOpenGLTest.cpp
// Aleksandar Koruga (aleksandar.koruga@gmail.com)

#include "SC_PlugIn.hpp"
#include "OpenGLTest.hpp"

static InterfaceTable* ft;

namespace OpenGLTest {

OpenGLTest::OpenGLTest() {
    mCalcFunc = make_calc_function<OpenGLTest, &OpenGLTest::next>();
    next(1);
}

void OpenGLTest::next(int nSamples) {
    const float* input = in(0);
    const float* gain = in(1);
    float* outbuf = out(0);

    // simple gain function
    for (int i = 0; i < nSamples; ++i) {
        outbuf[i] = input[i] * gain[i];
    }
}

} // namespace OpenGLTest

PluginLoad(OpenGLTestUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<OpenGLTest::OpenGLTest>(ft, "OpenGLTest", false);
}
