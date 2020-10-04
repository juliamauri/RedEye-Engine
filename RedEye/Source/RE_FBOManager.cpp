#include "RE_FBOManager.h"

#include "OutputLog.h"

#include "Glew/include/glew.h"

RE_FBOManager::RE_FBOManager() { }

RE_FBOManager::~RE_FBOManager() {
	for (auto fbo : fbos) {
		if (fbo.second.stencilBuffer != 0) glDeleteRenderbuffers(1, &fbo.second.stencilBuffer);
		if (fbo.second.depthBuffer != 0) glDeleteRenderbuffers(1, &fbo.second.depthBuffer);
		if (fbo.second.depthstencilBuffer != 0) glDeleteRenderbuffers(1, &fbo.second.depthstencilBuffer);
		for(auto c : fbo.second.texturesID) glDeleteTextures(1, &c);
		glDeleteFramebuffers(1, &fbo.second.ID);
	}
}

int RE_FBOManager::CreateFBO(unsigned int width, unsigned int height, unsigned int texturesSize, bool depth, bool stencil)
{
	int ret = -1;

	RE_FBO newFbo;
	newFbo.type = RE_FBO::FBO_Type::DEFAULT;
	newFbo.width = width;
	newFbo.height = height;

	glGenFramebuffers(1, &newFbo.ID);
	ChangeFBOBind(newFbo.ID);

	for (unsigned int i = 0; i < texturesSize; i++) {
		unsigned int tex = 0;
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

	if (depth && stencil) {
		glGenRenderbuffers(1, &newFbo.depthstencilBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, newFbo.depthstencilBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newFbo.depthstencilBuffer);
	}
	else {
		if (depth) {
			glGenRenderbuffers(1, &newFbo.depthBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, newFbo.depthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, newFbo.depthBuffer);
		}

		if (stencil) {
			glGenRenderbuffers(1, &newFbo.stencilBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, newFbo.stencilBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newFbo.stencilBuffer);
		}
	}

	unsigned int drawFubbersize = newFbo.texturesID.size();
	GLenum* DrawBuffers = new GLenum[drawFubbersize];
	for (unsigned int i = 0; i < drawFubbersize; i++) DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
	glDrawBuffers(drawFubbersize, DrawBuffers);

	fbos.insert(eastl::pair<unsigned int, RE_FBO>(newFbo.ID, newFbo));

	if (glCheckFramebufferStatus(newFbo.ID) != GL_FRAMEBUFFER_COMPLETE)
		ret = newFbo.ID;
	else {
		ClearFBO(newFbo.ID);
		LOG_ERROR("FBO can't be created.");
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	ChangeFBOBind(0);

	return ret;
}

int RE_FBOManager::CreateDeferredFBO(unsigned int width, unsigned int height)
{
	int ret = -1;

	RE_FBO newFbo;
	newFbo.type = RE_FBO::FBO_Type::DEFERRED;
	newFbo.width = width;
	newFbo.height = height;
	glGenFramebuffers(1, &newFbo.ID);
	ChangeFBOBind(newFbo.ID);
	LoadDeferredTextures(newFbo);

	// Depth Buffer
	glGenRenderbuffers(1, &newFbo.depthstencilBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, newFbo.depthstencilBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newFbo.depthstencilBuffer);

	// Check status
	fbos.insert(eastl::pair<unsigned int, RE_FBO>(newFbo.ID, newFbo));
	if (glCheckFramebufferStatus(newFbo.ID) != GL_FRAMEBUFFER_COMPLETE)
		ret = newFbo.ID;
	else {
		ClearFBO(newFbo.ID);
		LOG_ERROR("FBO can't be created.");
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	ChangeFBOBind(0);

	return ret;
}

void RE_FBOManager::ChangeFBOSize(unsigned int ID, unsigned int width, unsigned int height)
{
	RE_FBO& toChange = fbos.at(ID);

	ChangeFBOBind(ID);
	toChange.width = width;
	toChange.height = height;

	unsigned int texturesNum = toChange.texturesID.size();
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

	if (toChange.type == RE_FBO::FBO_Type::DEFERRED)
	{
		LoadDeferredTextures(toChange);
	}
	else
	{
		for (unsigned int i = 0; i < texturesNum; i++) {
			unsigned int tex = 0;
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

			toChange.texturesID.push_back(tex);
		}
	}

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

void RE_FBOManager::ClearFBO(unsigned int ID)
{
	RE_FBO toDelete = fbos.at(ID);

	if (toDelete.stencilBuffer != 0) glDeleteRenderbuffers(1, &toDelete.stencilBuffer);
	if (toDelete.depthBuffer != 0) glDeleteRenderbuffers(1, &toDelete.depthBuffer);
	for (auto c : toDelete.texturesID) glDeleteTextures(1, &c);
	glDeleteFramebuffers(1, &toDelete.ID);

	fbos.erase(ID);
}

unsigned int RE_FBOManager::GetWidth(unsigned int ID) const
{
	return fbos.at(ID).width;
}

unsigned int RE_FBOManager::GetHeight(unsigned int ID) const
{
	return fbos.at(ID).height;
}

unsigned int RE_FBOManager::GetTextureID(unsigned int ID, unsigned int texAttachment)const
{
	return (fbos.at(ID).texturesID.size() < texAttachment) ? 0 : fbos.at(ID).texturesID[texAttachment];
}

void RE_FBOManager::ChangeFBOBind(unsigned int fID, unsigned int width, unsigned int height)
{
	static unsigned int currentFBOID = 0;
	if (currentFBOID != fID) glBindFramebuffer(GL_FRAMEBUFFER, currentFBOID = fID);
	if(width != 0 && height != 0) glViewport(0, 0, width, height);
}

void RE_FBOManager::LoadDeferredTextures(RE_FBO& fbo)
{
	// position
	unsigned int tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, fbo.width, fbo.height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	fbo.texturesID.push_back(tex);

	// normal
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, fbo.width, fbo.height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex, 0);
	fbo.texturesID.push_back(tex);

	// Albedo
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, fbo.width, fbo.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tex, 0);
	fbo.texturesID.push_back(tex);

	// Specular
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, fbo.width, fbo.height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tex, 0);
	fbo.texturesID.push_back(tex);

	// Result
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, fbo.width, fbo.height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, tex, 0);
	fbo.texturesID.push_back(tex);

	// Bind Attachments
	unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, attachments);
}
