
#pragma once
#include "utils/shader.h"

#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <atomic>
#include <array>

namespace scGraphics {

	struct Vertex
	{
		float x, y, z;
	};
	struct Index
	{
		int idx1, idx2, idx3;
	};


	static void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}

	inline void checkOpenGLError() {
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			std::cout << "OpenGL Error: " << error << std::endl;
		}
	}

} //namespace scGraphics
