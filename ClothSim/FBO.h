#pragma once
#include <gl/glew.h>
#include <iostream>
#include "VBO.h"
class FBO
{
protected:
	GLuint m_fbo;
	GLuint m_texture;
	GLenum m_texType;
	GLuint m_vao;
	GLuint m_vbo;
	GLuint m_rbo;
	GLuint m_width;
	GLuint m_height;

public:
	FBO() {}
	FBO(unsigned int width, unsigned int height) : m_width(width), m_height(height)
	{
		glGenFramebuffers(1, &m_fbo);
	}
	void GenTexture()
	{
		glGenTextures(1, &m_texture);
	}
	void BindTex2D()
	{
		glBindTexture(GL_TEXTURE_2D, m_texture);
	}
	void AttachColor()
	{
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		SetTexParameter(GL_NEAREST, GL_CLAMP_TO_EDGE);

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	}
	void Attach16F()
	{
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);

		SetTexParameter(GL_NEAREST, GL_CLAMP_TO_EDGE);

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	}
	void AttachDepth()
	{
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		SetTexParameter(GL_NEAREST, GL_CLAMP_TO_BORDER);
		float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_texture, 0);
	}
	void AttachRGB32UI()
	{
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32UI, m_width, m_height, 0,
			GL_RGB_INTEGER, GL_UNSIGNED_INT, NULL);

		SetTexParameter(GL_NEAREST, GL_CLAMP_TO_EDGE);

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
	}
	void SetTexParameter(GLenum scaling, GLenum fitting)
	{
		glBindTexture(GL_TEXTURE_2D, m_texture);
		// scaling
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scaling);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scaling);

		// fitting
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, fitting);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, fitting);
	}

	void DisableColor()
	{
		Bind();
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	void GenRBO()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glGenRenderbuffers(1, &m_rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
	}
	void GenRectVAO()
	{
		float rectangleVertices[24] =
		{
			// Coords    // texCoords
			 1.0f, -1.0f,  1.0f, 0.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			-1.0f,  1.0f,  0.0f, 1.0f,

			 1.0f,  1.0f,  1.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f,  0.0f, 1.0f
		};

		glGenVertexArrays(1, &m_vao);
		glGenBuffers(1, &m_vbo);
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	}
	void GenFramebuffer()
	{
		glGenFramebuffers(1, &m_fbo);
	}
	void Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	}
	void Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void Viewport()
	{
		glViewport(0, 0, m_width, m_height);
	}
	void DrawRect()
	{
		glBindVertexArray(m_vao);
		glDisable(GL_DEPTH_TEST);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	/*
	void DrawRect(Shader shader, int unit)
	{
		shader.Activate();
		glBindVertexArray(vao);
		glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, texture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	*/
	void BindVAO()
	{
		glBindVertexArray(m_vao);
	}

	GLuint GetHeight() const
	{
		return m_height;
	}
	GLuint GetWidth() const
	{
		return m_width;
	}
	GLuint GetID() const
	{
		return m_fbo;
	}
	GLuint GetTexID() const
	{
		return m_texture;
	}
	void SetWidth(GLuint width) { m_width = width; }
	void SetHeight(GLuint height) { m_height = height; }
	void SetTexType(GLenum TexType) { m_texType = TexType; }

	std::string FramebufferStatusEnum(GLenum status)
	{
		switch (status)
		{
		case 36053:
			return std::string(" - Complete");
		case 36054:
			return std::string(" - Incomplete attachment");
		case 36057:
			return std::string(" - Incomplete dimensions");
		case 36055:
			return std::string(" - Missing attachment");
		case 36061:
			return std::string(" - Unsupported");
		default:
			return std::string(" - Unknown");
		}
	}

	std::string AttachmentEnum(GLenum attachment)
	{
		switch (attachment)
		{
		case 6402:
			return std::string(" - DEPTH_COMPONENT");
		case 36064:
			return std::string(" - COLOR_ATTACHMENT0");
		default:
			return std::string(" - Unknown");
		}
	}
};

