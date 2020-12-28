#include "GLSparseTextureArray.h"

#include <iostream>

bool CoreRender::GLSparseTexture2DArray::freeMipLevel(GLSparseTexture2DArrayMemoryDeallocator& allocator)
{
	bool bMipLevelFree = false;

	if (allocator.pTexture && allocator.pTexture->_pParrent == this)
	{
//		CL::Logger::pushMessageFormated("GLSparseTexture2DArray::freeMipLevel(GLSparseTexture2DArrayMemoryDeallocator& allocator): input params: iMipLevel = %i, allocator.iCellId = %i, w = %i, h = %i",		allocator.iMipLevel, allocator.iCellId, _pMipLevels[allocator.iMipLevel].x, _pMipLevels[allocator.iMipLevel].y);

		const int maxX = glm::max(_pMipLevels[allocator.iMipLevel].x / _iPageSizeX, 1);
		const int maxY = glm::max(_pMipLevels[allocator.iMipLevel].y / _iPageSizeY, 1);
		const int iCellIdMax = maxX * maxY;
		int x = _pMipLevels[allocator.iMipLevel].x > _iPageSizeX ? allocator.iCellId % maxX : 0;
		int y = _pMipLevels[allocator.iMipLevel].y > _iPageSizeY ? allocator.iCellId / maxY : 0;

		if (allocator.iCellId == 0)
		{
			if (allocator.iMipLevel == allocator.pTexture->_pTexture->iMinMipLevel)
			{
				allocator.pTexture->_pTexture->iMinMipLevel = allocator.iMipLevel + 1;
			}
			else
			{ 
				TException("Wrong argumetn");
			}
		} 

		if (allocator.iCellId++ < iCellIdMax && allocator.iMipLevel < _iMipLevelsAllocated)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

			glTexPageCommitmentARB(
				GL_TEXTURE_2D_ARRAY,
				allocator.iMipLevel,
				x * _iPageSizeX, y * _iPageSizeY, allocator.pTexture->_pTexture->iLayer,
				glm::min(_pMipLevels[allocator.iMipLevel].x, _iPageSizeX), glm::min(_pMipLevels[allocator.iMipLevel].y, _iPageSizeY), 1,
				GL_FALSE
			);

			GLError::cheakMessageLog("GLSparseTexture2DArray::freeMipLevel(GLSparseTexture2DArrayMemoryDeallocator& allocator): glTexPageCommitmentARB...");
	//		CL::Logger::pushMessageFormated("GLSparseTexture2DArray::freeMipLevel(GLSparseTexture2DArrayMemoryDeallocator& allocator): updated, x= %i, y = %i, iCellIdMax = %i", x, y, iCellIdMax);

			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		}
		else
		{
			bMipLevelFree = true;
//			CL::Logger::pushMessageFormated("GLSparseTexture2DArray::freeMipLevel(GLSparseTexture2DArrayMemoryDeallocator& allocator): alocated, x= %i, y = %i, iCellIdMax = %i", x, y, iCellIdMax);
		}
	}
	else
	{
//		CL::Logger::write("GLSparseTexture2DArray::freeMipLevel(GLSparseTexture2DArrayMemoryDeallocator& allocator): invalid argument");
	}

	return bMipLevelFree;
}

CoreRender::GLSparseTexture2DArrayMemoryAllocator CoreRender::GLSparseTexture2DArray::beginMemoryAllocator(HSparseTexture& hTexture, GLint iMipLevel)
{
	GLSparseTexture2DArrayMemoryAllocator allocator;

	if (hTexture._pTexture && hTexture._pTexture->_pParrent == this)
	{
		allocator.pTexture = &hTexture;
		allocator.iMipLevel = iMipLevel;
		allocator.iCellId = 0;
	}

	return allocator;
}

bool CoreRender::GLSparseTexture2DArray::allocateMipLevel(GLSparseTexture2DArrayMemoryAllocator& allocator)
{
	bool bMipLevelAlocated = false;

	if (allocator.pTexture && allocator.pTexture->_pParrent == this && allocator.iMipLevel < this->_iMipLevels)
	{  
		//CL::Logger::pushMessageFormated("GLSparseTexture2DArray::allocateTexture(GLSparseTexture2DArrayMemoryAllocator& allocator): input params: iMipLevel = %i, allocator.iCellId = %i, w = %i, h = %i",allocator.iMipLevel, allocator.iCellId, _pMipLevels[allocator.iMipLevel].x, _pMipLevels[allocator.iMipLevel].y);
		 
		if (_lowMipLevelsAllocator.iLayer > allocator.pTexture->_pTexture->iLayer)
		{ 
			const int maxX = glm::max(_pMipLevels[allocator.iMipLevel].x / _iPageSizeX, 1);
			const int maxY = glm::max(_pMipLevels[allocator.iMipLevel].y / _iPageSizeY, 1);
			const int iCellIdMax = maxX * maxY;
			int x = _pMipLevels[allocator.iMipLevel].x > _iPageSizeX ? allocator.iCellId % maxX : 0;
			int y = _pMipLevels[allocator.iMipLevel].y > _iPageSizeY ? allocator.iCellId / maxY : 0;

			if (allocator.iCellId++ < iCellIdMax && allocator.iMipLevel < _iMipLevelsAllocated)
			{
			//	CL::Logger::pushMessageFormated("GLSparseTexture2DArray::allocateTexture(GLSparseTexture2DArrayMemoryAllocator& allocator): begin, x= %i, y = %i, iCellIdMax = %i", x, y, iCellIdMax);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

				glTexPageCommitmentARB(
					GL_TEXTURE_2D_ARRAY,
					allocator.iMipLevel,
					x * _iPageSizeX, y * _iPageSizeY, allocator.pTexture->_pTexture->iLayer,
					glm::min(_pMipLevels[allocator.iMipLevel].x, _iPageSizeX), glm::min(_pMipLevels[allocator.iMipLevel].y, _iPageSizeY), 1,
					GL_TRUE
				);

				GLError::cheakMessageLog("GLSparseTexture2DArray::allocateTexture(GLSparseTexture2DArrayMemoryAllocator& allocator& hTexture): glTexPageCommitmentARB...");
		//		CL::Logger::write("GLSparseTexture2DArray::allocateTexture(GLSparseTexture2DArrayMemoryAllocator& allocator): updated...");

				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			}
			else
			{
				bMipLevelAlocated = true;
				CL::Logger::pushMessageFormated("GLSparseTexture2DArray::allocateTexture(GLSparseTexture2DArrayMemoryAllocator& allocator): alocated, x= %i, y = %i, iCellIdMax = %i", x, y, iCellIdMax);
			}
		}
		else
		{  
		//	CL::Logger::pushMessageFormated("GLSparseTexture2DArray::allocateTexture(GLSparseTexture2DArrayMemoryAllocator& allocator): beign low levels allocation, iLayer = %i", _lowMipLevelsAllocator.iLayer);

			if (_lowMipLevelsAllocator.iMip >= _iMipLevelsAllocated)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

				glTexPageCommitmentARB(
					GL_TEXTURE_2D_ARRAY,
					_lowMipLevelsAllocator.iMip,
					0, 0, _lowMipLevelsAllocator.iLayer,
					_pMipLevels[_lowMipLevelsAllocator.iMip].x, _pMipLevels[_lowMipLevelsAllocator.iMip].y, 1,
					GL_TRUE
				);

				GLError::cheakMessageLog("GLSparseTexture2DArray(): glTexPageCommitmentARB()");

				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

			//	CL::Logger::pushMessageFormated("GLSparseTexture2DArray::allocateTexture(GLSparseTexture2DArrayMemoryAllocator& allocator): low layer %i mip %i created", _lowMipLevelsAllocator.iLayer, _lowMipLevelsAllocator.iMip);

				_lowMipLevelsAllocator.iMip--;

			}
			else
			{
			//	CL::Logger::pushMessageFormated("GLSparseTexture2DArray::allocateTexture(GLSparseTexture2DArrayMemoryAllocator& allocator): low layer %i created", _lowMipLevelsAllocator.iLayer);

				_lowMipLevelsAllocator.iLayer++;
				_lowMipLevelsAllocator.iMip = _iMipLevels - 1;
			} 
			
		} 
	}
	else
	{
	//	CL::Logger::write("GLSparseTexture2DArray::allocateTexture(GLSparseTexture2DArrayMemoryAllocator& allocator): invalid argument"); 
	}

	return bMipLevelAlocated;
}

bool CoreRender::GLSparseTexture2DArray::writeMipLevel(HSparseTexture& hTexture, GLint iMipLevel, const void* pData)
{
	bool bMipLevelUpdated = false;

	if (hTexture._pTexture && hTexture._pTexture->_pParrent == this && iMipLevel > -1 && iMipLevel < _iMipLevels)
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

		if (bMipLevelUpdated && iMipLevel < _iMipLevelsAllocated)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);
 
			glTexPageCommitmentARB(
				GL_TEXTURE_2D_ARRAY,
				iMipLevel,
				0, 0, hTexture._pTexture->iLayer,
				_pMipLevels[iMipLevel].x, _pMipLevels[iMipLevel].y, 1,
				GL_TRUE
			);
			 
			GLError::cheakMessageLog("GLSparseTexture2DArray::allocateTexture(HSparseTexture& hTexture): glTexPageCommitmentARB...");
			 
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
 
			GLError::cheakMessageLog("GLSparseTexture2DArray::writeMipLevel(HSparseTexture& hTexture, GLint iMipLevel, const void* pData): glTexSubImage3D...");

			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		} 
	}

	return bMipLevelUpdated;
}

bool CoreRender::GLSparseTexture2DArray::freeMipLevel(HSparseTexture& hTexture, GLint iMipLevel)
{
	bool bFree = false;

	if (hTexture._pTexture && hTexture._pTexture->_pParrent == this)
	{
		bool bIsMipLast = hTexture._pTexture->iMinMipLevel == hTexture._pTexture->iMaxMipLevel;

		if (iMipLevel == hTexture._pTexture->iMinMipLevel)
		{
			hTexture._pTexture->iMinMipLevel = iMipLevel + 1;
			bFree = true;
		} 

		if (bFree)
		{
			if (bIsMipLast)
			{
				hTexture._pTexture->iMinMipLevel = -1;
				hTexture._pTexture->iMaxMipLevel = -1;
			}
 
			if (iMipLevel < _iMipLevelsAllocated)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

				if (_pMipLevels[iMipLevel].x > _iPageSizeX&& _pMipLevels[iMipLevel].y > _iPageSizeY)
				{
					glTexPageCommitmentARB(
						GL_TEXTURE_2D_ARRAY,
						iMipLevel,
						0, 0, hTexture._pTexture->iLayer,
						_pMipLevels[iMipLevel].x, _pMipLevels[iMipLevel].y, 1,
						GL_FALSE
					);
				}

				GLError::cheakMessageLog("GLSparseTexture2DArray::freeTexture(HSparseTexture& hTexture): glTexPageCommitmentARB...");

				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			}
		}
	}

	return bFree;
}

bool CoreRender::GLSparseTexture2DArray::freeTexture(HSparseTexture& hTexture)
{
	bool bFree = false;

	if (hTexture._pTexture && hTexture._pTexture->_pParrent == this)
	{
		bFree = true;

		auto pTexture = hTexture._pTexture;
		hTexture._pParrent = nullptr;
		pTexture->iAllocatorId = -1;
		pTexture->iMaxMipLevel = -1;
		pTexture->iMinMipLevel = -1;
		pTexture->bFree = true; 

		_freeTextures.push_back(pTexture);

		hTexture._pTexture = nullptr;
			
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

		for (int iLevel = 0; iLevel < _iMipLevelsAllocated; iLevel++)
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
	return _iMipLevels;
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
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

	for (GLint layer = 0; layer < _iLayerCount; layer++)
	{
		for (int iLevel = 0; iLevel < _iMipLevels; iLevel++)
		{
			glTexPageCommitmentARB(
				GL_TEXTURE_2D_ARRAY,
				iLevel,
				0, 0, layer,
				_pMipLevels[iLevel].x, _pMipLevels[iLevel].y, 1,
				GL_FALSE
			); 
		}
	} 

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

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

CoreRender::GLSparseTexture2DArray::GLSparseTexture2DArray(int iWidth, int iHeight, TextureFiltring minFilter, TextureFiltring magFilter, TextureFormat format, TextureAnisotropyLevel anisotropyLevel) :
	_iWidth(iWidth), _iHeight(iHeight)
{
	CL::Logger::pushMessageFormated("GLSparseTexture2DArray(): input params: iWidth = %i, iHeight = %i", iWidth, iHeight);

	_glFormat = translateTextureFormat("GLSparseTexture2DArray()", format);

	_iLayerCount = OpenGL_4_5_context::getInstance().getMaxSparseTextureArrayLayers(); //чертово говно не рабоет на амд
	_iLayerCount = 128;

	if (iWidth <= 0 || iHeight <= 0)
	{
		TException("GLSparseTexture2DArray() : iWidth & iHeight must be greather then 0");
	}
	if (!CL::isPowerOf2(iWidth) || !CL::isPowerOf2(iHeight))
	{
		TException("GLSparseTexture2DArray() : iWidth & iHeight must be the power of 2");
	}

	int iMaxResolution = CL::max(iWidth, iHeight);

	_iMipLevels = 0;
	while (iMaxResolution > 0)
	{
		_iMipLevels++;
		iMaxResolution /= 2;
	}
	_lowMipLevelsAllocator.iMip = _iMipLevels - 1;
	_pMipLevels = new glm::ivec2[_iMipLevels];

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

	for (GLint i = 0; i < _iMipLevels; i++)
	{
		_pMipLevels[i].x = iW;
		_pMipLevels[i].y = iH;
		iW = CL::max(1, iW / 2);
		iH = CL::max(1, iH / 2);
	}

	CL::Logger::pushMessageFormated("try create : w = %i, h = %i, levels = %i, layers = %i, format = %s, ", iWidth, iHeight, _iMipLevels, _iLayerCount, getTextureFormatSTR(format).c_str());

	glGenTextures(1, &_glTextureId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SPARSE_ARB, GL_TRUE);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, _iMipLevels - 1);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, translateTextureFiltring("GLSparseTextureAtlasPage()", minFilter));
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, translateTextureFiltring("GLSparseTextureAtlasPage()", magFilter));
	 
	if (anisotropyLevel != ANISOTROPY_DISABLE)
	{
		glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, static_cast<float>(anisotropyLevel)); 
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_VIRTUAL_PAGE_SIZE_INDEX_ARB, 0);

	glGetInternalformativ(GL_TEXTURE_2D_ARRAY, _glFormat.internalFormat, GL_VIRTUAL_PAGE_SIZE_X_ARB, 1, &_iPageSizeX);
	glGetInternalformativ(GL_TEXTURE_2D_ARRAY, _glFormat.internalFormat, GL_VIRTUAL_PAGE_SIZE_Y_ARB, 1, &_iPageSizeY);

	if (_iPageSizeX != _iPageSizeY)
	{
		TException("GLSparseTextureAtlasPage(): _iPageSizeX != _iPageSizeY, необработанный случай");
	}

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, _iMipLevels, _glFormat.internalFormat, _iWidth, _iHeight, _iLayerCount);
	GLError::cheakMessageLog("GLSparseTexture2DArray(): glTexStorage3D()");

	_iMipLevelsAllocated = 0;
	while ((_pMipLevels[_iMipLevelsAllocated].x > _iPageSizeX || _pMipLevels[_iMipLevelsAllocated].y > _iPageSizeY) && _iMipLevels > _iMipLevelsAllocated)
	{
		_iMipLevelsAllocated++;
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	CL::Logger::pushMessageFormated("_iPageSizeX = %i, _iPageSizeY = %i, _iMipLevelsAllocated = %i", _iPageSizeX, _iPageSizeY, _iMipLevelsAllocated);
	CL::Logger::write("GLSparseTexture2DArray(): created");
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
		pTexture->iMaxMipLevel = _iMipLevels - 1;

		hTexture._pTexture = pTexture;
		hTexture._pParrent = this;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

		for (int iLevel = _iMipLevelsAllocated + 1; iLevel >= 0; iLevel--)
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

	if (_freeTextures.size() && !hTexture._pTexture)
	{
		bAlocated = true;

		auto pTexture = _freeTextures.begin()();

		_freeTextures.erase(_freeTextures.begin());

		pTexture->bFree = false;

		hTexture._pTexture = pTexture;
		hTexture._pParrent = this;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	return bAlocated;
}

bool CoreRender::GLSparseTexture2DArray::createMipLevel(HSparseTexture& hTexture, GLint iMipLevel)
{
	bool bMipLevelUpdated = false;
	if (hTexture._pTexture && hTexture._pTexture->_pParrent == this && iMipLevel > -1 && iMipLevel < _iMipLevels)
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
	}

	return bMipLevelUpdated;
}

void CoreRender::GLSparseTexture2DArray::writeMipLevelData(HSparseTexture& hTexture, GLint iMipLevel, const void* pData, GLint x, GLint y, GLint w, GLint h)
{
	if (hTexture._pTexture && hTexture._pTexture->_pParrent == this)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, _glTextureId);

		glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,
			iMipLevel,
			x, y, hTexture._pTexture->iLayer,
			w, h, 1,
			_glFormat.baseFormat,
			_glFormat.type,
			pData
		);

		GLError::cheakMessageLog("GLSparseTexture2DArray::writeMipLevel(HSparseTexture& hTexture, GLint iMipLevel, const void* pData): glTexSubImage3D...");

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}
}

CoreRender::GLSparseTexture2DArrayMemoryDeallocator CoreRender::GLSparseTexture2DArray::beginMemoryDeallocator(CoreRender::HSparseTexture& hTexture, GLint iMipLevel)
{
	GLSparseTexture2DArrayMemoryDeallocator deallocator;

	if (hTexture._pTexture && hTexture._pTexture->_pParrent == this)
	{
		deallocator.pTexture = &hTexture;
		deallocator.iMipLevel = iMipLevel;
		deallocator.iCellId = 0;
	}

	return deallocator;
}

void CoreRender::HSparseTexture::free()
{
	if (_pParrent)
	{
		_pParrent->freeTexture(*this);
	}
}

bool CoreRender::SparseTextureLayer::isFullLoaded() const
{
	return _pParrent
		&& iMinMipLevel == 0
		&& iMaxMipLevel == _pParrent->getMipLevels() - 1;
}

CoreRender::GLTexture2DArrayShadow::GLTexture2DArrayShadow(int iWidth, int iLayerCount) :
	_iWidth(iWidth)
{
	CL::Logger::pushMessageFormated("GLTexture2DArrayShadow(): input params: iWidth = %i", iWidth);
	_glFormat = translateTextureFormat("GLTexture2DArrayShadow()", DEPTH_32f);

	//	_iLayerCount = OpenGL_4_5_context::getInstance().getMaxSparseTextureArrayLayers(); //чертово говно не рабоет на амд
	_iLayerCount = iLayerCount;

	if (iWidth <= 0)
	{
		TException("GLSparseTexture2DArray() : iWidth must be greather then 0");
	}

	if (!CL::isPowerOf2(iWidth))
	{
		TException("GLSparseTexture2DArray() : iWidth & iHeight must be the power of 2");
	}

	_pTextures = new SparseShadow2DTextureLayer[_iLayerCount];

	for (GLint i = 0; i < _iLayerCount; i++)
	{
		SparseShadow2DTextureLayer& texture = _pTextures[i];

		_freeTextures.push_back(&texture);

		texture.iLayer = i;
		texture._pParrent = this;
	}

	glGenTextures(1, &_textureId);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textureId);

	GLError::cheakMessageLog("GLSparseTexture2DArray(): glTexStorage3D()");

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/*glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_SPARSE_ARB, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_VIRTUAL_PAGE_SIZE_INDEX_ARB, 0);
	glGetInternalformativ(GL_TEXTURE_2D_ARRAY, _glFormat.internalFormat, GL_VIRTUAL_PAGE_SIZE_X_ARB, 1, &_iPageSizeX);
	glGetInternalformativ(GL_TEXTURE_2D_ARRAY, _glFormat.internalFormat, GL_VIRTUAL_PAGE_SIZE_Y_ARB, 1, &_iPageSizeY);

	if (_iPageSizeX != _iPageSizeY)
	{
	TException("GLSparseTextureAtlasPage(): _iPageSizeX != _iPageSizeY, необработанный случай");
	}
	CL::Logger::pushMessageFormated("_iPageSizeX = %i, _iPageSizeY = %i", _iPageSizeX, _iPageSizeY);
	*/

	GLError::cheakMessageLog("GLSparseTexture2DArray(): glTexStorage3D()");

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, _glFormat.internalFormat, _iWidth, _iWidth, _iLayerCount);
	GLError::cheakMessageLog("GLSparseTexture2DArray(): glTexStorage3D()");

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);


	CL::Logger::write("GLSparseTexture2DArray(): created");
}

CoreRender::HTextureArray2Shadow CoreRender::GLTexture2DArrayShadow::alocateShadowMap()
{
	HTextureArray2Shadow h;

	if (_freeTextures.size())
	{
		h._pParrent = this;
		h._pTexture = _freeTextures.begin()();
		_freeTextures.erase(_freeTextures.begin());

		h._pTexture->bFree = false;

		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, _textureId);

		glTexPageCommitmentARB(
			GL_TEXTURE_2D_ARRAY,
			0,
			0, 0, h._pTexture->iLayer,
			_iWidth, _iWidth, 1,
			GL_TRUE
		);

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);*/
	}

	return h;
}

bool CoreRender::GLTexture2DArrayShadow::free(HTextureArray2Shadow& hTexture)
{
	bool bFree = false;

	if (hTexture._pParrent == this && hTexture._pTexture != nullptr && !hTexture._pTexture->bFree)
	{
		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, _textureId);

		glTexPageCommitmentARB(
			GL_TEXTURE_2D_ARRAY,
			0,
			0, 0, hTexture._pTexture->iLayer,
			_iWidth, _iWidth, 1,
			GL_FALSE
		);

		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);*/

		hTexture._pParrent = false;
		hTexture._pTexture->bFree = true;
		_freeTextures.push_back(hTexture._pTexture);
		hTexture._pTexture = nullptr;
		bFree = true;
	}

	return bFree;
}

CoreRender::GLTexture2DArrayShadow::~GLTexture2DArrayShadow()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _textureId);

	for (GLint layer = 0; layer < _iLayerCount; layer++)
	{
		glTexPageCommitmentARB(
			GL_TEXTURE_2D_ARRAY,
			0,
			0, 0, layer,
			_iWidth, _iWidth, 1,
			GL_FALSE
		);
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glDeleteTextures(1, &_textureId);

	delete[] _pTextures;
}
