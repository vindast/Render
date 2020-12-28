#pragma once 
#include <common.h>
#include <glm.hpp>
#include <list.h>
#include "Texture.h"
  
//#include <iostream>
 
namespace CoreRender
{  
	class GLSparseTextureAtlasPage;

	struct SparseTextureTile final
	{
		bool bFree = true;
		glm::vec4 f4OffsetScale = glm::vec4(0.0f);
		GLSparseTextureAtlasPage* _pOwner = nullptr;
		GLint iX = 0;
		GLint iY = 0;
	};

	struct SparseTextureMipMemory final
	{
		//������ �����
		GLint iTileSize = 0; 

		//������ ������
		GLint iWidth = 0;
		GLint iHeight = 0;

		GLint** pTileCounter = nullptr; 

		void initilaze();

		~SparseTextureMipMemory();
	};

	struct HSparseTextureAtlasTile final
	{  
		SparseTextureTile* _pTile = nullptr;
	};
	 
	class GLSparseTextureAtlasPage
	{
		friend class ShaderBase;
		friend class ShaderBuilder;
	public:

		/*void printMemMap() const 
		{
			for (GLint iLevel = 0; iLevel < _iLevels; iLevel++)
			{
				std::cout << "level = " << iLevel << std::endl;

				const SparseTextureMipMemory& mem = _mMipMemory[iLevel];

				for (int y = 0; y < mem.iHeight; y++)
				{
					for (int x = 0; x < mem.iWidth; x++)
					{
						std::cout << mem.pTileCounter[x][y];
					}

					std::cout << std::endl;
				}
			} 
		}*/
		 
		//������� GLSparseTextureAtlasPage
		//iWidth � iHeight ������ ���� ��������� ������, �� ����������� �������
		//������ ��� ��� ������ ���������� �������������
		GLSparseTextureAtlasPage(int iWidth, int iHeight, TextureFiltring minFilter, TextureFiltring magFilter, TextureFormat format);
		 
		//��������� ��������
		//������ true ���� �������� �������, ����� false
		bool allocateTexture(HSparseTextureAtlasTile& hTexture);

		//������ �������� hTexture �� ������. ���������� ������, ������� ������������ ������ hTexture 
		//������ true, ���� hTexture ������������ � ������ ������, ����� false 
		bool eraseTexture(HSparseTextureAtlasTile& hTexture);

		//��������� ������ � ��� ������ iLevel � �������� hTexture ������� �� pData
		//��������������, ��� pData - ���������
		//������ true, ���� pData != NULL, hTexture ����������� ����� ������, iLevel >= 0 � iLevel < ��������� ��� �������,
		//����� false
		bool writeMipLevel(const void* pData, const HSparseTextureAtlasTile& hTexture, GLint iLevel);

		//��������� ��������, ������� 0 ��� ������� �� pData, ����������� ��� ������ 
		//(����� ��� ������ ������������� � ������� ������� ������, ����� ��������)
		//��������������, ��� pData - ���������
		//������ true ���� �������� �������, ����� false
		bool allocateTexture(HSparseTextureAtlasTile& hTexture, const void* pData);

		//������� ������ � ��������, ����������� ��� ������
		//(����� ��� ������ ������������� � ������� ������� ������, ����� ��������)
		//������ true ���� hTexture �� ����� ������, ����� false
		bool writeTexture(HSparseTextureAtlasTile& hTexture, const void* pData);

		~GLSparseTextureAtlasPage();

		GLint getLevels() const;
		 

		GLuint64 _glTextureHandle = 0;

	private:
		GLTextureFormat _glFormat;
		GLuint _glTextureId = -1;

		SparseTextureTile* _pTiles = nullptr;
		glm::ivec2* _pMipLevels = nullptr; 
		SparseTextureMipMemory* _mMipMemory = nullptr;

		CL::List<SparseTextureTile*> _freeTiles;

		GLint _iPageSizeX = 0;
		GLint _iPageSizeY = 0;
		GLint _iAlocatedLevels = 0;
		GLint _iLevels = 0;
		GLint _iWidth = 0, _iHeight = 0;
		GLint _iSizeX = 0, _iSizeY = 0;

		GLSparseTextureAtlasPage(const GLSparseTextureAtlasPage&) = delete;
		GLSparseTextureAtlasPage& operator = (const GLSparseTextureAtlasPage&) = delete;

		void createMipMemory();
	};
};

