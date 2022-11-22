// PluginOpenGLTest.hpp
// Aleksandar Koruga (aleksandar.koruga@gmail.com)

#pragma once

#include "SC_PlugIn.hpp"

namespace OpenGLTest {

class OpenGLTest : public SCUnit {
public:
    OpenGLTest();

    // Destructor
    // ~OpenGLTest();

private:
    // Calc function
    void next(int nSamples);

    // Member variables
};

} // namespace OpenGLTest
