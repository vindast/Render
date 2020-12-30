#include "GLFBO.h"

CoreRender::GLFBO::GLFBO()
{

}

void CoreRender::GLFBO::createFBO(GLFBO& fbo, const GLFBODesc& desc)
{
	if (!desc.nColorRT && !desc.depthRT.pTexture)
	{
		TException("GLFBO::createFBO(): Инициализация пустого fbo");
	}

	if (desc.nColorRT >= TGL_FBO_RENDER_TARGET_COUNT_MAX)
	{
		TException("GLFBO::createFBO(): nColorRT больше чем TGL_FBO_RENDER_TARGET_COUNT_MAX");
	}

	GLenum mDrawBuffer[TGL_FBO_RENDER_TARGET_COUNT_MAX];

	glGenFramebuffers(1, &fbo._iGLFBOid);
	GLFBOBindControl::bind(fbo._iGLFBOid);

	if (desc.nColorRT)
	{
		for (GLenum i = 0; i < GLenum(desc.nColorRT); i++)
		{
			std::string sDebugMessage = "mColorRT[" + std::to_string(i) + "]";

			cheakRT(desc.mColorRT[i], sDebugMessage);

			mDrawBuffer[i] = GL_COLOR_ATTACHMENT0 + i;

			setupRT(desc.mColorRT[i], mDrawBuffer[i], sDebugMessage);
		}

		glDrawBuffers(desc.nColorRT, mDrawBuffer);
	}   

	if (desc.depthRT.pTexture)
	{
		std::string sDebugMessage = "depthRT";

		cheakRT(desc.depthRT, sDebugMessage); 

		setupRT(desc.depthRT, GL_DEPTH_ATTACHMENT, sDebugMessage);
	} 
	else
	{
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
	}

	auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::string mes = "ivalid framebuffer : ";

		switch (status)
		{
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: mes += "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: mes += "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
		case GL_FRAMEBUFFER_UNSUPPORTED: mes += "GL_FRAMEBUFFER_UNSUPPORTED"; break;
		default:
			mes += "unknow error";
			break;
		}

		TException(mes.c_str());
	}
}

CoreRender::GLFBO::~GLFBO()
{
	clear();
}

void CoreRender::GLFBO::clear()
{
	if (_iGLFBOid)
	{ 
		GLFBOBindControl::bind(0);

		glDeleteFramebuffers(1, &_iGLFBOid);
		_iGLFBOid = -1;
	}
}

void CoreRender::GLFBO::cheakRT(const RTdesc& desc, const std::string& sDebugMessage)
{
	if (!desc.pTexture)
	{
		std::string message;
		message += "ERROR: ";
		message += sDebugMessage;
		message += " is nullptr!";
		TException(message.c_str());
	}

	if (desc.mip < 0)
	{
		std::string message;
		message += "ERROR: at ";
		message += sDebugMessage;
		message += "has less then zero mip";
		TException(message.c_str());
	}

	if (desc.type == RTType::RT_ERROR)
	{
		std::string message;
		message += "ERROR: at ";
		message += sDebugMessage;
		message += "has RT_ERROR format";
		TException(message.c_str());
	}
}

void CoreRender::GLFBO::setupRT(const RTdesc& desc, GLenum attacment, const std::string& sDebugMessage)
{
	if (isArray(desc.type))
	{
		/*glFramebufferTexture3D(
			GL_FRAMEBUFFER,
			attacment,
			translateRTType(desc.type),
			desc.pTexture->_textureId,
			desc.mip,
			desc.iLayer
		);*/

		glFramebufferTextureLayer(
			GL_FRAMEBUFFER,
			attacment,
			desc.pTexture->_textureId,
			desc.mip,
			desc.iLayer
		);


		GLError::cheakMessageLog("GLFBO::setupRT(): attach array texture");
	}
	else
	{
		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			attacment,
			translateRTType(desc.type),
			desc.pTexture->_textureId,
			desc.mip
		);

		GLError::cheakMessageLog("GLFBO::setupRT(): attach texture");
	}
}

void CoreRender::GLFBOBindControl::bind(GLuint iFBOId)
{
#ifdef _TDEBUG
	if (iFBOId == -1)
	{
		TException("GLFBOBindControl::bind(): установка невалидного fbo")
	}
#endif

	if (iFBOId != getInstance()._iCurrentFBOId)
	{
		getInstance()._iCurrentFBOId = iFBOId;
		glBindFramebuffer(GL_FRAMEBUFFER, getInstance()._iCurrentFBOId);

#ifdef _TDEBUG
		GLError::cheakMessageLog("GLFBOBindControl::bind(): установка нового fbo");
#endif
	}
}

CoreRender::GLFBOBindControl& CoreRender::GLFBOBindControl::getInstance()
{
	static GLFBOBindControl c;
	return c;
}

CoreRender::GLFBOBindControl::GLFBOBindControl()
{
	_iCurrentFBOId = 0;
	glBindFramebuffer(GL_FRAMEBUFFER, _iCurrentFBOId);
}

bool CoreRender::isArray(const RTType type)
{
	return RTType::RT_TEXTURE_2D_ARRAY == type;
}

GLenum CoreRender::translateRTType(const RTType type)
{
	switch (type)
	{
	case RTType::RT_TEXTURE_2D:
	{
		return GL_TEXTURE_2D;
		break;
	}
	case RTType::RT_TEXTURE_CUBE_MAP_POSITIVE_X:
	{
		return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		break;
	}
	case RTType::RT_TEXTURE_CUBE_MAP_NEGATIVE_X:
	{
		return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
		break;
	}
	case RTType::RT_TEXTURE_CUBE_MAP_POSITIVE_Y:
	{
		return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
		break;
	}
	case RTType::RT_TEXTURE_CUBE_MAP_NEGATIVE_Y:
	{
		return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
		break;
	}
	case RTType::RT_TEXTURE_CUBE_MAP_POSITIVE_Z:
	{
		return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
		break;
	}
	case RTType::RT_TEXTURE_CUBE_MAP_NEGATIVE_Z:
	{
		return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
		break;
	}
	case RTType::RT_TEXTURE_2D_ARRAY:
	{
		return GL_TEXTURE_2D_ARRAY;
		break;
	}
	default:
	{
		TException(":GLFBO::translateRTType(): out of range");
		break;
	}
	}
}
