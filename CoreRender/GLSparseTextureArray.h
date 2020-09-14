#pragma once
#include <common.h>
#include <glm.hpp>
#include <list.h>
#include "Texture.h" 
namespace CoreRender
{
	class GLSparseTexture2DArray;

	struct SparseTextureLayer
	{
		bool bFree = true;
		GLSparseTexture2DArray* _pParrent = nullptr;
		GLint iLayer = -1;
		GLint iMinMipLevel = -1;
		GLint iMaxMipLevel = -1;
	};

	struct HSparseTexture
	{
		SparseTextureLayer* _pTexture = nullptr;
	};

	class GLSparseTexture2DArray
	{
		friend class ShaderBase;
		friend class ShaderBuilder;
	public:

		GLSparseTexture2DArray(int iWidth, int iHeight, TextureFiltring minFilter, TextureFiltring magFilter, TextureFormat format);

		//Allocate texture in this array, load data to 0 mip-level and generate other mip levels.
		//return true, if allocation successful, outwise false
		bool allocateTexture(HSparseTexture& hTexture, const void* pData);

		//Allocate texture in this array
		//return true, if allocation successful, outwise false
		bool allocateTexture(HSparseTexture& hTexture);
		  
		bool writeMipLevel(HSparseTexture& hTexture, GLint iMipLevel, const void* pData);

		bool writeTexture(HSparseTexture& hTexture, const void* pData);

		bool freeTexture(HSparseTexture& hTexture);

		GLint getLayerCount() const;

		GLint getMipLevels() const;

		const glm::ivec2* getMipResolutions() const;

		const SparseTextureLayer* getLayers() const;

		SparseTextureLayer* getLayers();

		~GLSparseTexture2DArray();

		GLuint64 getHandle();
		 
	private:
		GLSparseTexture2DArray(const GLSparseTexture2DArray&) = delete;
		GLSparseTexture2DArray& operator = (const GLSparseTexture2DArray&) = delete;

		CL::List<SparseTextureLayer*> _freeTextures;
		 
		GLuint64 _glTextureHandle = 0;

		GLint _iLayerCount = -1;
		GLint _iLevels = -1;

		GLint _iWidth = 0, _iHeight = 0;

		GLuint _glTextureId = -1;
		GLTextureFormat _glFormat;

		glm::ivec2* _pMipLevels = nullptr;

		SparseTextureLayer* _pTextures = nullptr;
	};
};