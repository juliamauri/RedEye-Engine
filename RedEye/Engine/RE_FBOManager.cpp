#include <EASTL/vector.h>
#include <EASTL/map.h>

#include "RE_FBOManager.h"

#include "Application.h"
#include "RE_GLCache.h"
#include <GL/glew.h>

eastl::map<uint, RE_FBO> fbos;

void RE_FBO::LoadDeferredTextures()
{
	// position
	uint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	texturesID.push_back(tex);

	// normal
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex, 0);
	texturesID.push_back(tex);

	// Albedo
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tex, 0);
	texturesID.push_back(tex);

	// Specular + Shininess + Alpha
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tex, 0);
	texturesID.push_back(tex);

	// Result
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, tex, 0);
	texturesID.push_back(tex);

	// Bind Attachments
	uint attachments[5] =
	{
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4
	};
	glDrawBuffers(5, attachments);

	glBindTexture(GL_TEXTURE_2D, 0);
}


int RE_FBOManager::CreateFBO(uint width, uint height, uint texturesSize, bool depth, bool stencil)
{
	int ret = -1;

	RE_FBO newFbo;
	newFbo.type = RE_FBO::Type::DEFAULT;
	newFbo.width = width;
	newFbo.height = height;

	glGenFramebuffers(1, &newFbo.ID);
	ChangeFBOBind(newFbo.ID);

	for (uint i = 0; i < texturesSize; i++)
	{
		uint tex = 0;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexImage2D(GL_TEXTURE_2D,
			0,
			GL_RGBA,
			width, height,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex, 0);

		newFbo.texturesID.push_back(tex);
	}

	// Depth Texture
	glGenTextures(1, &newFbo.depthBufferTexture);
	glBindTexture(GL_TEXTURE_2D, newFbo.depthBufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, newFbo.width, newFbo.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, newFbo.depthBufferTexture, 0);

	if (depth && stencil)
	{
		glGenRenderbuffers(1, &newFbo.depthstencilBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, newFbo.depthstencilBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newFbo.depthstencilBuffer);
	}
	else
	{
		if (depth)
		{
			glGenRenderbuffers(1, &newFbo.depthBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, newFbo.depthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, newFbo.depthBuffer);
		}

		if (stencil)
		{
			glGenRenderbuffers(1, &newFbo.stencilBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, newFbo.stencilBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newFbo.stencilBuffer);
		}
	}

	eastl_size_t drawFubbersize = newFbo.texturesID.size();
	GLenum* DrawBuffers = new GLenum[drawFubbersize];
	for (eastl_size_t i = 0; i < drawFubbersize; i++) DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
	glDrawBuffers(static_cast<GLenum>(drawFubbersize), DrawBuffers);
	
	fbos.insert(eastl::pair<uint, RE_FBO>(newFbo.ID, newFbo));
	int error = 0;
	if ((error = glCheckFramebufferStatus(GL_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE)
		ret = newFbo.ID;
	else
	{
		ClearFBO(newFbo.ID);

		switch (error) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			RE_LOG_ERROR("Not all framebuffer attachment points are framebuffer attachment complete..");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			RE_LOG_ERROR("Not all Not all attached images have the same width and height.");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			RE_LOG_ERROR("No images are attached to the framebuffer.");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			RE_LOG_ERROR("The combination of internal formats of the attached images violates an implementation-dependent set of restrictions.");
			break;
		}
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	ChangeFBOBind(0);

	return ret;
}

int RE_FBOManager::CreateDeferredFBO(uint width, uint height)
{
	int ret = -1;

	RE_FBO newFbo;
	newFbo.type = RE_FBO::Type::DEFERRED;
	newFbo.width = width;
	newFbo.height = height;
	glGenFramebuffers(1, &newFbo.ID);
	ChangeFBOBind(newFbo.ID);

	newFbo.LoadDeferredTextures();

	// Depth Texture
	glGenTextures(1, &newFbo.depthBufferTexture);
	glBindTexture(GL_TEXTURE_2D, newFbo.depthBufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, newFbo.width, newFbo.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, newFbo.depthBufferTexture, 0);

	// Depth/stencil Buffer
	glGenRenderbuffers(1, &newFbo.depthstencilBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, newFbo.depthstencilBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newFbo.depthstencilBuffer);

	// Check status
	fbos.insert(eastl::pair<uint, RE_FBO>(newFbo.ID, newFbo));
	int error = 0;
	if ((error = glCheckFramebufferStatus(GL_FRAMEBUFFER)) == GL_FRAMEBUFFER_COMPLETE)
		ret = newFbo.ID;
	else
	{
		ClearFBO(newFbo.ID);

		switch (error) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			RE_LOG_ERROR("Not all framebuffer attachment points are framebuffer attachment complete..");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			RE_LOG_ERROR("Not all Not all attached images have the same width and height.");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			RE_LOG_ERROR("No images are attached to the framebuffer.");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			RE_LOG_ERROR("The combination of internal formats of the attached images violates an implementation-dependent set of restrictions.");
			break;
		}
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	ChangeFBOBind(0);

	return ret;
}

void RE_FBOManager::ChangeFBOSize(uint ID, uint width, uint height)
{
	RE_FBO& toChange = fbos.at(ID);

	ChangeFBOBind(ID);
	toChange.width = width;
	toChange.height = height;

	eastl_size_t texturesNum = toChange.texturesID.size();
	bool stencil = false;
	bool depth = false;
	if (stencil = (toChange.stencilBuffer != 0)) glDeleteRenderbuffers(1, &toChange.stencilBuffer);
	if (depth = (toChange.depthBuffer != 0)) glDeleteRenderbuffers(1, &toChange.depthBuffer);
	if (toChange.depthstencilBuffer != 0) {
		glDeleteRenderbuffers(1, &toChange.depthstencilBuffer);
		depth = stencil = true;
	}
	for (auto c : toChange.texturesID) glDeleteTextures(1, &c);
	toChange.texturesID.clear();
	glDeleteTextures(1, &toChange.depthBufferTexture);

	if (toChange.type == RE_FBO::Type::DEFERRED)
	{
		toChange.LoadDeferredTextures();
	}
	else
	{
		for (eastl_size_t i = 0; i < texturesNum; i++) {
			uint32_t tex = 0;
			glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);

			glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RGBA,
				width, height,
				0,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				NULL);


			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i), GL_TEXTURE_2D, tex, 0);

			toChange.texturesID.push_back(tex);
		}
	}

	// Depth Texture
	glGenTextures(1, &toChange.depthBufferTexture);
	glBindTexture(GL_TEXTURE_2D, toChange.depthBufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, toChange.width, toChange.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, toChange.depthBufferTexture, 0);

	if (depth && stencil) {
		glGenRenderbuffers(1, &toChange.depthstencilBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, toChange.depthstencilBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, toChange.depthstencilBuffer);
	}
	else {
		if (depth) {
			glGenRenderbuffers(1, &toChange.depthBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, toChange.depthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, toChange.depthBuffer);
		}

		if (stencil) {
			glGenRenderbuffers(1, &toChange.stencilBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, toChange.stencilBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, toChange.stencilBuffer);
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	ChangeFBOBind(0);
}

void RE_FBOManager::ClearFBOBuffers(uint ID, const float color[4])
{
	RE_FBO fbo = fbos.at(ID);
	GLbitfield mask = GL_COLOR_BUFFER_BIT;

	if (fbo.depthBuffer != 0 || fbo.depthstencilBuffer != 0)
	{
		mask |= GL_DEPTH_BUFFER_BIT;
	}

	if (fbo.stencilBuffer != 0 || fbo.depthstencilBuffer != 0)
	{
		glClearStencil(0);
		mask |= GL_STENCIL_BUFFER_BIT;
	}

	glClearColor(color[0], color[1], color[2], color[3]);
	glClear(mask);
}

void RE_FBOManager::ClearFBO(uint ID)
{
	RE_FBO toDelete = fbos.at(ID);

	if (toDelete.stencilBuffer != 0) glDeleteRenderbuffers(1, &toDelete.stencilBuffer);
	if (toDelete.depthBuffer != 0) glDeleteRenderbuffers(1, &toDelete.depthBuffer);
	for (auto c : toDelete.texturesID) glDeleteTextures(1, &c);
	glDeleteFramebuffers(1, &toDelete.ID);

	fbos.erase(ID);
}

uint RE_FBOManager::GetWidth(uint ID) { return fbos.at(ID).width; }
uint RE_FBOManager::GetHeight(uint ID) { return fbos.at(ID).height; }

void RE_FBOManager::GetWidthAndHeight(uint ID, uint32_t& width, uint32_t& height)
{
	auto& fbo = fbos.at(ID);
	width = fbo.width;
	height = fbo.height;
}

uint RE_FBOManager::GetDepthTexture(uint ID)
{
	RE_FBO fbo = fbos.at(ID);
	RE_GLCache::ChangeTextureBind(fbo.depthBufferTexture);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 0, 0, fbo.width, fbo.height, 0);
	RE_GLCache::ChangeTextureBind(0);
	return fbo.depthBufferTexture;
}

uint32_t RE_FBOManager::GetTextureID(uint ID, uint texAttachment)
{
	auto& fbo = fbos.at(ID);
	return (fbo.texturesID.size() < texAttachment) ? 0 : fbo.texturesID[texAttachment];
}

void RE_FBOManager::ChangeFBOBind(uint fID, uint width, uint height)
{
	static uint currentFBOID = 0;
	if (currentFBOID != fID) glBindFramebuffer(GL_FRAMEBUFFER, currentFBOID = fID);
	if(width != 0 && height != 0) glViewport(0, 0, width, height);
}

void RE_FBOManager::ClearAll()
{
	for (eastl::pair<uint, RE_FBO> fbo : fbos)
	{
		const RE_FBO current = fbo.second;
		if (current.stencilBuffer != 0) glDeleteRenderbuffers(1, &current.stencilBuffer);
		if (current.depthBuffer != 0) glDeleteRenderbuffers(1, &current.depthBuffer);
		if (current.depthstencilBuffer != 0) glDeleteRenderbuffers(1, &current.depthstencilBuffer);
		for (auto c : current.texturesID) glDeleteTextures(1, &c);
		glDeleteFramebuffers(1, &current.ID);
	}
	fbos.clear();
}