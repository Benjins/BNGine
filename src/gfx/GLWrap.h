#ifndef GL_WRAP_H
#define GL_WRAP_H

#pragma once

#if defined(_WIN32)
#include <Windows.h>
#include <gl/GL.h>
#include <gl/glext.h>
#elif defined(__APPLE__)
#include <OpenGl/gl.h>
#include <OpenGL/glext.h>
#else
#include<GL/gl.h>
#include<GL/glx.h>
#endif

static_assert(sizeof(GLuint) == sizeof(GLuint), "Opengl headers not found");

#endif
