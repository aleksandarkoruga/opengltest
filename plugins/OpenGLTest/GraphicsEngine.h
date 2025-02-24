
#pragma once
#include "SCGraphics.hpp"
#include "Shader.hpp"



//#define TEXTURE_X 1080
//#define TEXTURE_Y 1080


namespace scGraphics{
	static std::atomic<bool> glfwInitializing{ false }; // Indicates if initialization is in progress
	static std::atomic<bool> glfwInitialized{ false };  // Indicates if initialization succeeded


class GraphicsEngine{
public:
	GraphicsEngine(std::string shaderFolderPath, int widthBackBuffer, int heightBackBuffer, int audioVectorSize, int nVectors);
	~GraphicsEngine();
	void RunEngine();
	void CalculateOneFrame();
	void SetData(const float* buf, const int& cpyIdx);

	const float* GetPixels() const {
		if (!m_bPixReady.load(std::memory_order_acquire)) return nullptr;

		const int idx = m_readIdx.load(std::memory_order_acquire);
		return (idx != -1) ? m_pixBuffers[idx].data() : nullptr;
	}

	const float* GetPixels(bool& isFrameNew) const {
		if (!m_bPixReady.load(std::memory_order_acquire)) return nullptr;

		const int idx = m_readIdx.load(std::memory_order_acquire);
		
		const int currentIdx = m_currentReadIdx.load(std::memory_order_acquire);
		
		const int lastIdx = m_lastReadIdx.load(std::memory_order_acquire);


		isFrameNew = false;

		if (idx != currentIdx)
		{
			m_currentReadIdx.store(idx, std::memory_order_relaxed);
			m_lastReadIdx.store(currentIdx, std::memory_order_relaxed);
			isFrameNew = true;
		}

		return (idx != -1) ? m_pixBuffers[idx].data() : nullptr;
	}
	
	const float* GetPrevPixels() const {
		
		if (!m_bPixReady.load(std::memory_order_acquire)) return nullptr;

		const int lastIdx = m_lastReadIdx.load(std::memory_order_acquire);

		return (lastIdx != -1) ? m_pixBuffers[lastIdx].data() : nullptr;
	}
	
private:
	bool InitEngine();
	void TerminateEngine();

protected:

	std::string m_path;
	int m_width;
	int m_height;
	float m_ratio;
	int m_audioVectorSize;
	int m_nVectors;
	int m_cpyIdx;

	enum class PROGRAM_STATE { START, RUN, TERMINATE };
	std::atomic<PROGRAM_STATE> m_programState;

	std::thread m_graphicsThread;

	GLFWwindow* m_pWindow;

	
//	GLint m_mvpLocation;
	GLint m_vPosLocation;
	//GLint m_vColLocation;

	

	std::unique_ptr<Shader> m_pShader;
	enum class MEMSTATE {READY,READING,WRITING,EMPTY};
	std::atomic<MEMSTATE> m_bSSBO;
	
	unsigned int m_VBO, m_VAO, m_EBO;
	
	std::vector<GLuint> m_frameBufferSwap;
	std::vector<GLuint> m_textures;
	bool m_bCurrentSwap; 

	//std::vector<float> m_arrPixels;
	//bool m_bPixReady;
	std::array<std::vector<float>, 8> m_pixBuffers; // Triple buffer
	std::atomic<int> m_writeIdx{ 0 }; // Graphics thread writes here
	std::atomic<int> m_readIdx{ -1 }; // Audio thread reads here (-1 = no data)
	std::atomic<bool> m_bPixReady{ false }; // Initial flag for safety
	mutable std::atomic<int> m_lastReadIdx{-1};
	mutable std::atomic<int> m_currentReadIdx{-1};

	
	//std::pair<GLuint, GLuint> m_frameBufferSwap;
	//std::pair<GLuint, GLuint> m_textures; //output tex
};

} //namespace scGraphics
