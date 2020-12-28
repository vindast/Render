#pragma once
#include "Texture.h"

#define TGL_FBO_RENDER_TARGET_COUNT_MAX 4

namespace CoreRender
{ 
	enum class RTType
	{
		RT_ERROR = -1,
		RT_TEXTURE_2D = 0,
		RT_TEXTURE_CUBE_MAP_POSITIVE_X = 1,
		RT_TEXTURE_CUBE_MAP_NEGATIVE_X = 2,
		RT_TEXTURE_CUBE_MAP_POSITIVE_Y = 3,
		RT_TEXTURE_CUBE_MAP_NEGATIVE_Y = 4,
		RT_TEXTURE_CUBE_MAP_POSITIVE_Z = 5,
		RT_TEXTURE_CUBE_MAP_NEGATIVE_Z = 6,
		RT_TEXTURE_2D_ARRAY = 7
	};

	struct RTdesc
	{
		RTType type = RTType::RT_ERROR;
		GLuint mip = 0;
		TextureBase* pTexture = nullptr;
		GLint iLayer = 0;
	}; 

	struct GLFBODesc
	{
		GLsizei nColorRT = 0;
		RTdesc mColorRT[TGL_FBO_RENDER_TARGET_COUNT_MAX];
		RTdesc depthRT;
	};

	bool isArray(const RTType type);
	GLenum translateRTType(const RTType type);

	class GLFBOBindControl
	{
	public:
		static void bind(GLuint iFBOId);
	private:
		static GLFBOBindControl& getInstance();
		GLFBOBindControl();
		GLFBOBindControl& operator = (const GLFBOBindControl&) = delete;
		GLFBOBindControl(const GLFBOBindControl&) = delete;

		GLuint _iCurrentFBOId = -1;
	};

	class GLFBO
	{
	public:
		GLFBO();
		static void createFBO(GLFBO& fbo, const GLFBODesc& desc);
		~GLFBO();

		GLuint _iGLFBOid = 0;
	private: 
		GLFBO(const GLFBO&) = delete;
		GLFBO& operator = (const GLFBO&) = delete;
		void clear();
		static void cheakRT(const RTdesc& desc, const std::string& sDebugMessage);
		static void setupRT(const RTdesc& desc, GLenum attacment, const std::string& sDebugMessage); 
	}; 
}