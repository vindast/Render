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
		GLint iAllocatorId = -1;

		bool isFullLoaded() const;
	};

	struct HSparseTexture
	{
		SparseTextureLayer* _pTexture = nullptr;
		GLSparseTexture2DArray* _pParrent = nullptr;

		void free();
	};

	struct GLSparseTexture2DArrayLowMipLevelsMemoryAllocator
	{
		int iLayer = 0;
		int iMip = 0;
	};

	struct GLSparseTexture2DArrayMemoryAllocator
	{
		HSparseTexture* pTexture = nullptr;
		int iCellId = 0;
		int iMipLevel = 0;
	};

	struct GLSparseTexture2DArrayMemoryDeallocator
	{
		HSparseTexture* pTexture = nullptr;
		int iCellId = 0;
		int iMipLevel = 0;
	};

	class GLSparseTexture2DArray
	{
		friend class ShaderBase;
		friend class ShaderBuilder;
	public: 
		GLSparseTexture2DArray(int iWidth, int iHeight, TextureFiltring minFilter, TextureFiltring magFilter, TextureFormat format, TextureAnisotropyLevel anisotropyLevel);
		//Allocate texture in this array, load data to 0 mip-level and generate other mip levels.
		//return true, if allocation successful, outwise false
		bool allocateTexture(HSparseTexture& hTexture, const void* pData); 
		//Allocate texture in this array
		//return true, if allocation successful, outwise false
		bool allocateTexture(HSparseTexture& hTexture); 
		bool createMipLevel(HSparseTexture& hTexture, GLint iMipLevel);
		void writeMipLevelData(HSparseTexture& hTexture, GLint iMipLevel, const void* pData, GLint x, GLint y, GLint w, GLint h);
		GLSparseTexture2DArrayMemoryDeallocator beginMemoryDeallocator(HSparseTexture& hTexture, GLint iMipLevel);
		bool freeMipLevel(GLSparseTexture2DArrayMemoryDeallocator& allocator);
		GLSparseTexture2DArrayMemoryAllocator beginMemoryAllocator(HSparseTexture& hTexture, GLint iMipLevel);
		bool allocateMipLevel(GLSparseTexture2DArrayMemoryAllocator& allocator);
		bool writeMipLevel(HSparseTexture& hTexture, GLint iMipLevel, const void* pData); 
		bool freeMipLevel(HSparseTexture& hTexture, GLint iMipLevel);
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
		GLint _iMipLevels = -1; 
		GLint _iMipLevelsAllocated = -1;
		GLint _iWidth = 0, _iHeight = 0; 
		GLuint _glTextureId = -1;
		GLTextureFormat _glFormat;
		GLint _iPageSizeX = 0;
		GLint _iPageSizeY = 0; 
		glm::ivec2* _pMipLevels = nullptr; 
		SparseTextureLayer* _pTextures = nullptr;
		GLSparseTexture2DArrayLowMipLevelsMemoryAllocator _lowMipLevelsAllocator; 
	};

	class GLTexture2DArrayShadow;

	struct SparseShadow2DTextureLayer
	{
		bool bFree = true;
		GLTexture2DArrayShadow* _pParrent = nullptr;
		GLint iLayer = -1;
	};

	struct HTextureArray2Shadow
	{
		SparseShadow2DTextureLayer* _pTexture = nullptr;
		GLTexture2DArrayShadow* _pParrent = nullptr; 
	};

	class GLTexture2DArrayShadow : public TextureBase
	{
	public:
		GLTexture2DArrayShadow(int iWidth, int iLayerCount);
		HTextureArray2Shadow alocateShadowMap();
		bool free(HTextureArray2Shadow& hTexture);
		~GLTexture2DArrayShadow();
	private:
		CL::List<SparseShadow2DTextureLayer*> _freeTextures;
		SparseShadow2DTextureLayer* _pTextures = nullptr;
		GLTextureFormat _glFormat;
		GLint _iLayerCount = -1;
		GLint _iWidth;
	//	GLint _iPageSizeX = 0, _iPageSizeY = 0;
	};
};