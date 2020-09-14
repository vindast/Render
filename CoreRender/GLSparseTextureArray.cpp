#include "GLSparseTextureArray.h"

#include <iostream>

CoreRender::GLSparseTexture2DArray::GLSparseTexture2DArray(int iWidth, int iHeight, TextureFiltring minFilter, TextureFiltring magFilter, TextureFormat format) :
	_iWidth(iWidth), _iHeight(iHeight)
{
	_glFormat = translateTextureFormat("GLSparseTexture2DArray()", format);
 	 
	/*if (OpenGL_4_5_context::getInstance().isAMDSparseTextureSupports())
	{
		_iLayerCount = OpenGL_4_5_context::getInstance().getMaxAmdSparseTextureArrayLayers();
	}
	else
	{*/
		_iLayerCount = OpenGL_4_5_context::getInstance().getMaxSparseTextureArrayLayers();
/*	}*/

//	_iLayerCount = 128;
	
//  _iLayerCount = CL::min(128, OpenGL_4_5_context::getInstance().getMaxSparseTextureArrayLayers() / 4);

	if (iWidth <= 0 || iHeight <= 0)
	{
		TException("GLSparseTexture2DArray() : iWidth & iHeight must be greather then 0");
	}

	if (!CL::isPowerOf2(iWidth) || !CL::isPowerOf2(iHeight))
	{
		TException("GLSparseTexture2DArray() : iWidth & iHeight must be the power of 2");
	}

	int iMaxResolution = CL::max(iWidth, iHeight);
 
	_iLevels = 0; 

	while (iMaxResolution > 0)
	{
		_iLevels++;
		iMaxResolution /= 2;
	}  

	std::cout << "w = " << iWidth << ", h = " << iHeight << ", _iLevels = " << _iLevels << std::endl;

	_pMipLevels = new glm::ivec2[_iLevels];

	GLint iW = iWidth;
	GLint iH = iHeight;

	_pTextures = new SparseTextureLayer[_iLayerCount];

	for (GLint i = 0; i < _iLayerCount; i++)
	{
		SparseTextureLayer& texture = _pTextures[i];

		_freeTextures.push_back(&texture);

		texture.iLayer = i;
		texture._pParrent = this;
	}

	for (GLint i = 0; i < _iLevels; i++)
	{
		_pMipLevels[i].x = iW;
		_pMipLevels[i].y = iH;

		std::cout << "mip: " << i << ", w = " << iW << ", h = " << iH << std::endl;
 
		iW = CL::max(1, iW / 2);
		iH = CL::max(1, iH / 2);
	}

	Logger::pushMessageFormated("try create GLSparseTexture2DArray(): w = %i, h = %i, levels = %i, layers = %i, format = %s, ", iWidth, iHeight, _iLevels, _iLayerCount, getTextureFormatSTR(format).c_str());

	std::cout << "_iLayerCount (GL_MAX_SPARSE_ARRAY_TEXTURE_LAYERS_ARB) = " << _iLayerCount << std::endl;

	glGenTextures(1, &_glTextureId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);
	 
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SPARSE_ARB, GL_TRUE);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, _iLevels - 1);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, translateTextureFiltring("GLSparseTextureAtlasPage()", minFilter));
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, translateTextureFiltring("GLSparseTextureAtlasPage()", magFilter));

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/*if (OpenGL_4_5_context::getInstance().isAMDSparseTextureSupports())
	{
		std::cout << "glTexStorageSparseAMD..." << std::endl;

		glTexStorageSparseAMD(GL_TEXTURE_2D_ARRAY, _glFormat.internalFormat, _iWidth, _iHeight, 1, _iLayerCount, GL_TEXTURE_STORAGE_SPARSE_BIT_AMD);
	}
	else
	{ */
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, _iLevels, _glFormat.internalFormat, _iWidth, _iHeight, _iLayerCount);
	/*}*/
	

	GLError::cheakMessageLog("GLSparseTexture2DArray(): glTexStorage3D()");

	for (GLint layer = 0; layer < _iLayerCount; layer++)
	{
		for (int iLevel = 0; iLevel < _iLevels && iLevel < 1; iLevel++)
		{
			glTexPageCommitmentARB(
				GL_TEXTURE_2D_ARRAY,
				iLevel,
				0, 0, layer,
				_pMipLevels[iLevel].x, _pMipLevels[iLevel].y, 1,
				GL_FALSE
			);

			GLError::cheakMessageLog("GLSparseTexture2DArray::allocateTexture(): glTexPageCommitmentARB...");
		}
	}

	

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	std::cout << "GLSparseTexture2DArray created " << std::endl;

	Logger::write("GLSparseTexture2DArray(): ok");
}

bool CoreRender::GLSparseTexture2DArray::writeMipLevel(HSparseTexture& hTexture, GLint iMipLevel, const void* pData)
{
	bool bMipLevelUpdated = false;

	std::cout << "writeMipLevel(HSparseTexture& hTexture, GLint iMipLevel, const void* pData)..." << std::endl;

	if (hTexture._pTexture && hTexture._pTexture->_pParrent == this && iMipLevel > -1 && iMipLevel < _iLevels)
	{
		if (hTexture._pTexture->iMinMipLevel == -1)
		{
			hTexture._pTexture->iMinMipLevel = iMipLevel;
			hTexture._pTexture->iMaxMipLevel = iMipLevel;

			bMipLevelUpdated = true;
		}

		if (hTexture._pTexture->iMinMipLevel == iMipLevel + 1 || hTexture._pTexture->iMaxMipLevel == iMipLevel - 1)
		{
			hTexture._pTexture->iMinMipLevel = CL::min(hTexture._pTexture->iMinMipLevel, iMipLevel);
			hTexture._pTexture->iMaxMipLevel = CL::max(hTexture._pTexture->iMaxMipLevel, iMipLevel);

			bMipLevelUpdated = true;
		}

		if (bMipLevelUpdated)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

			std::cout << "glTexSubImage3D..." << std::endl;

			glTexSubImage3D(
				GL_TEXTURE_2D_ARRAY,
				iMipLevel,
				0, 0, hTexture._pTexture->iLayer,
				_pMipLevels[iMipLevel].x,
				_pMipLevels[iMipLevel].y,
				1,
				_glFormat.baseFormat,
				_glFormat.type,
				pData
			);

			std::cout << "glTexSubImage3D : ok" << std::endl;


			GLError::cheakMessageLog("GLSparseTexture2DArray::writeMipLevel(HSparseTexture& hTexture, GLint iMipLevel, const void* pData): glTexSubImage3D...");

			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		}
	}

	return bMipLevelUpdated;
}

bool CoreRender::GLSparseTexture2DArray::writeTexture(HSparseTexture& hTexture, const void* pData)
{
	bool bUpdated = false;

	std::cout << "try writeTexture(HSparseTexture& hTexture, const void* pData)" << std::endl;

	if (hTexture._pTexture && hTexture._pTexture->_pParrent == this)
	{
		bUpdated = true;

		hTexture._pTexture->iMaxMipLevel = _iLayerCount - 1;
		hTexture._pTexture->iMinMipLevel = 0;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

		std::cout << "glTexSubImage3D..." << std::endl;

		glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,
			0, 0, hTexture._pTexture->iLayer,
			_pMipLevels[0].x,
			_pMipLevels[0].y,
			1,
			_glFormat.baseFormat,
			_glFormat.type,
			pData
		);

		std::cout << "glTexSubImage3D : ok" << std::endl;
		 
		glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

		std::cout << "glGenerateMipmap : ok" << std::endl;

		GLError::cheakMessageLog("GLSparseTexture2DArray::writeTexture(HSparseTexture& hTexture, const void* pData): glTexSubImage3D...");

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	return bUpdated;
}

bool CoreRender::GLSparseTexture2DArray::freeTexture(HSparseTexture& hTexture)
{
	bool bFree = false;

	if (hTexture._pTexture && hTexture._pTexture->_pParrent == this)
	{
		bFree = true;

		auto pTexture = hTexture._pTexture;

		pTexture->iMaxMipLevel = -1;
		pTexture->iMinMipLevel = -1;
		pTexture->bFree = true;

		_freeTextures.push_back(pTexture);

		hTexture._pTexture = nullptr;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

		for (int iLevel = 0; iLevel < _iLevels; iLevel++)
		{
			glTexPageCommitmentARB(
				GL_TEXTURE_2D_ARRAY,
				iLevel,
				0, 0, pTexture->iLayer,
				_pMipLevels[iLevel].x, _pMipLevels[iLevel].y, 1,
				GL_FALSE
			);

			GLError::cheakMessageLog("GLSparseTexture2DArray::freeTexture(HSparseTexture& hTexture): glTexPageCommitmentARB...");
		}

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	return bFree;
}

GLint CoreRender::GLSparseTexture2DArray::getLayerCount() const
{
	return _iLayerCount;
}

GLint CoreRender::GLSparseTexture2DArray::getMipLevels() const
{
	return _iLevels;
}

const glm::ivec2* CoreRender::GLSparseTexture2DArray::getMipResolutions() const
{
	return _pMipLevels;
}

const CoreRender::SparseTextureLayer* CoreRender::GLSparseTexture2DArray::getLayers() const
{
	return _pTextures;
}

CoreRender::SparseTextureLayer* CoreRender::GLSparseTexture2DArray::getLayers()
{
	return _pTextures;
}

CoreRender::GLSparseTexture2DArray::~GLSparseTexture2DArray()
{
	delete[] _pTextures;
	delete[] _pMipLevels;

	if (_glTextureHandle)
	{
		glMakeTextureHandleNonResidentARB(_glTextureHandle);
	}

	if (_glTextureId)
	{
		glDeleteTextures(1, &_glTextureId);
	}
}

GLuint64 CoreRender::GLSparseTexture2DArray::getHandle()
{ 
	if (!_glTextureHandle)
	{
		_glTextureHandle = glGetTextureHandleARB(_glTextureId);
		glMakeTextureHandleResidentARB(_glTextureHandle);

		GLError::cheakMessageLog("GLSparseTexture2DArray(): glGetTextureHandleARB()");
	}

	return _glTextureHandle;
}

bool CoreRender::GLSparseTexture2DArray::allocateTexture(HSparseTexture& hTexture, const void* pData)
{
	bool bAlocated = false;

	if (_freeTextures.size() && !hTexture._pTexture)
	{
		bAlocated = true;

		auto pTexture = _freeTextures.begin()();

		_freeTextures.erase(_freeTextures.begin());

		pTexture->bFree = false;
		pTexture->iMinMipLevel = 0;
		pTexture->iMaxMipLevel = _iLevels - 1;

		hTexture._pTexture = pTexture;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

		for (int iLevel = 0; iLevel < _iLevels; iLevel++)
		{
			glTexPageCommitmentARB(
				GL_TEXTURE_2D_ARRAY,
				iLevel,
				0, 0, pTexture->iLayer,
				_pMipLevels[iLevel].x, _pMipLevels[iLevel].y, 1,
				GL_TRUE
			);

			GLError::cheakMessageLog("GLSparseTexture2DArray::allocateTexture(): glTexPageCommitmentARB...");
		}

		glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,
			0,
			0, 0, pTexture->iLayer,
			_pMipLevels[0].x,
			_pMipLevels[0].y,
			1,
			_glFormat.baseFormat,
			_glFormat.type,
			pData
		);

		GLError::cheakMessageLog("GLSparseTexture2DArray::allocateTexture(HSparseTexture& hTexture, const void* pData): glTexSubImage3D...");

		glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
 
		GLError::cheakMessageLog("GLSparseTexture2DArray::allocateTexture(HSparseTexture& hTexture, const void* pData): glGenerateMipmap...");

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	return bAlocated;
}

//Allocate texture in this array
//return true, if allocation successful, outwise false

bool CoreRender::GLSparseTexture2DArray::allocateTexture(HSparseTexture& hTexture)
{
	bool bAlocated = false;

	std::cout << "try allocateTexture(HSparseTexture& hTexture)" << std::endl;

	if (_freeTextures.size() && !hTexture._pTexture)
	{
		bAlocated = true;

		auto pTexture = _freeTextures.begin()();

		_freeTextures.erase(_freeTextures.begin());

		pTexture->bFree = false;

		hTexture._pTexture = pTexture;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

		for (int iLevel = 0; iLevel < _iLevels; iLevel++)
		{
			glTexPageCommitmentARB(
				GL_TEXTURE_2D_ARRAY,
				iLevel,
				0, 0, pTexture->iLayer,
				_pMipLevels[iLevel].x, _pMipLevels[iLevel].y, 1,
				GL_TRUE
			);

			std::cout << "iLevel = " << iLevel << ", w = " << _pMipLevels[iLevel].x << ", h = " << _pMipLevels[iLevel].y << ", layer = " << pTexture->iLayer << std::endl;

			GLError::cheakMessageLog("GLSparseTexture2DArray::allocateTexture(HSparseTexture& hTexture): glTexPageCommitmentARB...");
		}

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

		std::cout << "texture allocated..." << std::endl;
	}

	return bAlocated;
}

