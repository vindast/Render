#include "GLSparseTextureAtlasPage.h"

void CoreRender::SparseTextureMipMemory::initilaze()
{
	pTileCounter = new GLint * [iWidth];

	for (int i = 0; i < iWidth; i++)
	{
		pTileCounter[i] = new GLint[iHeight];

		for (int j = 0; j < iHeight; j++)
		{
			pTileCounter[i][j] = 0;
		}
	}
}

CoreRender::SparseTextureMipMemory::~SparseTextureMipMemory()
{
	if (pTileCounter)
	{
		for (int i = 0; i < iWidth; i++)
		{
			delete[] pTileCounter[i];
		}

		delete[] pTileCounter;
	}
}

CoreRender::GLSparseTextureAtlasPage::GLSparseTextureAtlasPage(int iWidth, int iHeight, TextureFiltring minFilter, TextureFiltring magFilter, TextureFormat format) :
	_iWidth(iWidth), _iHeight(iHeight)
{
	if (!CL::isPowerOf2(iWidth) || !CL::isPowerOf2(iHeight))
	{
		TException("GLSparseTextureAtlasPage() : iWidth и iHeight должны быть степенями двойки");
	}

	int iMaxResolution = CL::max(iWidth, iHeight);

	while (iMaxResolution > 1)
	{
		_iLevels++;
		iMaxResolution /= 2;
	}

	_iLevels++;



	GLError::cheakMessageLog("GLSparseTextureAtlasPage(): проверка перед генерацией текстуры");

	glGenTextures(1, &_glTextureId);
	glBindTexture(GL_TEXTURE_2D, _glTextureId);

	GLError::cheakMessageLog("GLSparseTextureAtlasPage(): проверка генерации текстуры");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SPARSE_ARB, GL_TRUE);
	GLint _nSize = -1;

	glGetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_NUM_VIRTUAL_PAGE_SIZES_ARB, 1, &_nSize);

	if (!_nSize)
	{
		TException("Невозможно создать текстуры такого типа...");
	}

	glTexParameteri(GL_TEXTURE_2D, GL_VIRTUAL_PAGE_SIZE_INDEX_ARB, 0);

	glGetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_VIRTUAL_PAGE_SIZE_X_ARB, 1, &_iPageSizeX);
	glGetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_VIRTUAL_PAGE_SIZE_Y_ARB, 1, &_iPageSizeY);

	if (_iPageSizeX != _iPageSizeY)
	{
		TException("GLSparseTextureAtlasPage(): _iPageSizeX != _iPageSizeY, необработанный случай");
	}

	if (!CL::isPowerOf2(_iPageSizeX) || !CL::isPowerOf2(_iPageSizeY))
	{
		TException("GLSparseTextureAtlasPage(): _iPageSizeX или _iPageSizeY не степень двойки, необработанный случай");
	}

	/*std::cout << "page size x = " << _iPageSizeX << ", y = " << _iPageSizeY << std::endl;*/

	GLError::cheakMessageLog("GLSparseTextureAtlasPage(): уставнока saprse параметров и получение размера страниц...");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, _iLevels - 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, translateTextureFiltring("GLSparseTextureAtlasPage()", minFilter));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, translateTextureFiltring("GLSparseTextureAtlasPage()", magFilter));

	GLError::cheakMessageLog("GLSparseTextureAtlasPage(): проверка установки параметров текстуры");

	_glFormat = translateTextureFormat("GLSparseTextureAtlasPage()", format);

	glTexStorage2D(GL_TEXTURE_2D, _iLevels, _glFormat.internalFormat, OpenGL_4_5_context::getInstance().getMaxTextureResolution(), OpenGL_4_5_context::getInstance().getMaxTextureResolution());

	GLError::cheakMessageLog("GLSparseTextureAtlasPage(): создание виртуальной текстуры glTexStorage2D(...)");

	glBindTexture(GL_TEXTURE_2D, 0);

	_iSizeX = OpenGL_4_5_context::getInstance().getMaxTextureResolution() / _iWidth;
	_iSizeY = OpenGL_4_5_context::getInstance().getMaxTextureResolution() / _iHeight;


	_pTiles = new SparseTextureTile[_iSizeX * _iSizeY];


	float fScaleX = 1.0f / float(_iSizeX);
	float fScaleY = 1.0f / float(_iSizeY);

	for (GLint x = 0; x < _iSizeX; x++)
	{
		for (GLint y = 0; y < _iSizeY; y++)
		{
			SparseTextureTile& tile = _pTiles[x + y * _iSizeX];

			tile.f4OffsetScale = glm::vec4(float(x) / float(_iSizeX), float(y) / float(_iSizeY), fScaleX, fScaleY);
			tile._pOwner = this;

			tile.iX = x;
			tile.iY = y;

			freeTiles.push_back(&tile);
		}
	}

	_pMipLevels = new glm::ivec2[_iLevels];

	GLint iW = iWidth;
	GLint iH = iHeight;

	for (GLint i = 0; i < _iLevels; i++)
	{
		_pMipLevels[i].x = iW;
		_pMipLevels[i].y = iH;

		//	std::cout << iW << " " << iH << std::endl;

		iW = CL::max(1, iW / 2);
		iH = CL::max(1, iH / 2);
	}


	createMipMemory();
}

bool CoreRender::GLSparseTextureAtlasPage::alocateTexture(HSparseTexture& hTexture)
{
	bool bAlocated = false;

	if (freeTiles.size() && !hTexture._pTile)
	{
		bAlocated = true;

		auto pTile = freeTiles.begin()();

		freeTiles.erase(freeTiles.begin());

		pTile->bFree = false;

		hTexture._pTile = pTile;

		glBindTexture(GL_TEXTURE_2D, _glTextureId);

		for (int iLevel = 0; iLevel < _iLevels; iLevel++)
		{
			GLint x = pTile->iX * _pMipLevels[iLevel].x / _mMipMemory[iLevel].iTileSize;
			GLint y = pTile->iY * _pMipLevels[iLevel].y / _mMipMemory[iLevel].iTileSize;

			if (!_mMipMemory[iLevel].pTileCounter[x][y])
			{
				glTexPageCommitmentARB(
					GL_TEXTURE_2D,
					iLevel,
					x * _mMipMemory[iLevel].iTileSize, y * _mMipMemory[iLevel].iTileSize, 0,
					_mMipMemory[iLevel].iTileSize, _mMipMemory[iLevel].iTileSize, 1,
					GL_TRUE
				);

				GLError::cheakMessageLog("GLSparseTextureAtlasPage::alocateTexture(): glTexPageCommitmentARB...");
			}

			_mMipMemory[iLevel].pTileCounter[x][y]++;
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		//	printMemMap();
	}

	return bAlocated;
}

bool CoreRender::GLSparseTextureAtlasPage::eraseTexture(HSparseTexture& hTexture)
{
	bool bErased = false;

	if (hTexture._pTile && hTexture._pTile->_pOwner == this)
	{
		auto pTile = hTexture._pTile;
		pTile->bFree = true;

		glBindTexture(GL_TEXTURE_2D, _glTextureId);

		for (int iLevel = 0; iLevel < _iLevels; iLevel++)
		{
			GLint x = pTile->iX * _pMipLevels[iLevel].x / _mMipMemory[iLevel].iTileSize;
			GLint y = pTile->iY * _pMipLevels[iLevel].y / _mMipMemory[iLevel].iTileSize;

			_mMipMemory[iLevel].pTileCounter[x][y]--;

			if (!_mMipMemory[iLevel].pTileCounter[x][y])
			{
				glTexPageCommitmentARB(
					GL_TEXTURE_2D,
					iLevel,
					x * _mMipMemory[iLevel].iTileSize, y * _mMipMemory[iLevel].iTileSize, 0,
					_mMipMemory[iLevel].iTileSize, _mMipMemory[iLevel].iTileSize, 1,
					GL_FALSE
				);

				GLError::cheakMessageLog("GLSparseTextureAtlasPage::eraseTexture(): glTexPageCommitmentARB...");
			}
		}

		glBindTexture(GL_TEXTURE_2D, 0);

		freeTiles.push_back(pTile);

		hTexture._pTile = nullptr;

		bErased = true;

		//	printMemMap();
	}

	return bErased;
}

//Обновляет данные в мип уровне iLevel в текстуре hTexture данными из pData
//Предполагается, что pData - корректен
//Вернет true, если pData != NULL, hTexture принадлежит этому атласу, iLevel >= 0 и iLevel < количесва мип уровней,
//иначе false

bool CoreRender::GLSparseTextureAtlasPage::writeMipLevel(const void* pData, const HSparseTexture& hTexture, GLint iLevel)
{
	bool bResult = false;

	if (pData && iLevel > -1 && iLevel < _iLevels && hTexture._pTile && hTexture._pTile->_pOwner == this)
	{
		bResult = true;

		auto pTile = hTexture._pTile;

		glBindTexture(GL_TEXTURE_2D, _glTextureId);

		glTexSubImage2D(
			GL_TEXTURE_2D,
			iLevel,
			pTile->iX * _pMipLevels[iLevel].x,
			pTile->iY * _pMipLevels[iLevel].y,
			_pMipLevels[iLevel].x,
			_pMipLevels[iLevel].y,
			_glFormat.baseFormat,
			_glFormat.type,
			pData
		);

		GLError::cheakMessageLog("GLSparseTextureAtlasPage::writeMipLevel(): glTexSubImage2D...");

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return bResult;
}

bool CoreRender::GLSparseTextureAtlasPage::alocateTexture(HSparseTexture& hTexture, const void* pData)
{
	bool bAlocated = false;

	if (freeTiles.size() && !hTexture._pTile)
	{
		bAlocated = true;

		auto pTile = freeTiles.begin()();

		freeTiles.erase(freeTiles.begin());

		pTile->bFree = false;

		hTexture._pTile = pTile;

		glBindTexture(GL_TEXTURE_2D, _glTextureId);

		for (int iLevel = 0; iLevel < _iLevels; iLevel++)
		{
			GLint x = pTile->iX * _pMipLevels[iLevel].x / _mMipMemory[iLevel].iTileSize;
			GLint y = pTile->iY * _pMipLevels[iLevel].y / _mMipMemory[iLevel].iTileSize;

			if (!_mMipMemory[iLevel].pTileCounter[x][y])
			{
				glTexPageCommitmentARB(
					GL_TEXTURE_2D,
					iLevel,
					x * _mMipMemory[iLevel].iTileSize, y * _mMipMemory[iLevel].iTileSize, 0,
					_mMipMemory[iLevel].iTileSize, _mMipMemory[iLevel].iTileSize, 1,
					GL_TRUE
				);

				GLError::cheakMessageLog("GLSparseTextureAtlasPage::alocateTexture(): glTexPageCommitmentARB...");
			}

			_mMipMemory[iLevel].pTileCounter[x][y]++;
		}

		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			pTile->iX * _pMipLevels[0].x,
			pTile->iY * _pMipLevels[0].y,
			_pMipLevels[0].x,
			_pMipLevels[0].y,
			_glFormat.baseFormat,
			_glFormat.type,
			pData
		);

		glGenerateMipmap(GL_TEXTURE_2D);

		GLError::cheakMessageLog("GLSparseTextureAtlasPage::alocateTexture(SparseTexture& hTexture, const void* pData): glTexSubImage2D...");

		glBindTexture(GL_TEXTURE_2D, 0);

		//	printMemMap();
	}

	return bAlocated;
}

CoreRender::GLSparseTextureAtlasPage::~GLSparseTextureAtlasPage()
{
	delete[] _mMipMemory;
	delete[] _pTiles;
	delete[] _pMipLevels;

	if (_glTextureId)
	{
		glDeleteTextures(1, &_glTextureId);
	} 
}

GLint CoreRender::GLSparseTextureAtlasPage::getLevels() const
{
	return _iLevels;
}

void CoreRender::GLSparseTextureAtlasPage::createMipMemory()
{
	_mMipMemory = new SparseTextureMipMemory[_iLevels];


	GLint iMipResolution = OpenGL_4_5_context::getInstance().getMaxTextureResolution();

	for (GLint iLevel = 0; iLevel < _iLevels; iLevel++)
	{
		SparseTextureMipMemory& mip = _mMipMemory[iLevel];

		mip.iTileSize = _pMipLevels[iLevel].x >= _iPageSizeX ? _pMipLevels[iLevel].x : iMipResolution;

		mip.iWidth = CL::max(1, iMipResolution / CL::max(_iPageSizeX, _pMipLevels[iLevel].x));
		mip.iHeight = CL::max(1, iMipResolution / CL::max(_iPageSizeY, _pMipLevels[iLevel].y));

		mip.initilaze();

		/*std::cout << "mip = " << iLevel << std::endl;
		std::cout << "tile size = " << mip.iTileSize << std::endl;
		std::cout << "w = " << mip.iWidth << ", h = " << mip.iHeight << std::endl;
		std::cout << "mip resolution = " << iMipResolution << std::endl;
		std::cout << "--------------------------------------------" << std::endl;*/

		iMipResolution /= 2;
	}

}
