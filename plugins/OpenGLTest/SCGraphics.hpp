
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

	inline void GetOrthoMVP(mat4x4* mvp, const int& width, const int& height)
	{
		float winAspect = (float)width / (float)height;

		mat4x4 p;

		// Create an identity matrix
		mat4x4_identity(*mvp);

		// Adjust scaling based on window aspect ratio
		if (winAspect > 1.0f) {
			// Wider window: squish X to preserve 1:1 aspect ratio
			mat4x4_scale_aniso(*mvp, *mvp, 1.0f / winAspect, 1.0f, 1.0f);
		}
		else {
			// Taller window: squish Y to preserve 1:1 aspect ratio
			mat4x4_scale_aniso(*mvp, *mvp, 1.0f, winAspect, 1.0f);
		}

		// Use a fixed orthographic projection (covers [-1, 1] in both axes)
		mat4x4_ortho(p, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
		mat4x4_mul(*mvp, p, *mvp); // Combine projection and scaling
	
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
