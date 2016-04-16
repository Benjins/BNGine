#ifndef GL_EXT_INIT_H
#define GL_EXT_INIT_H

#pragma once

#include "GLWrap.h"

void InitGlExts();

//These will be declared extern in all translation units except GLExtInit.cpp
#if defined(_GL_EXT_INIT_CPP)
#define EXT_EXTERN
#else
#define EXT_EXTERN extern
#endif

EXT_EXTERN PFNGLCREATESHADERPROC glCreateShader;
EXT_EXTERN PFNGLSHADERSOURCEPROC glShaderSource;
EXT_EXTERN PFNGLCOMPILESHADERPROC glCompileShader;
EXT_EXTERN PFNGLGETSHADERIVPROC glGetShaderiv;
EXT_EXTERN PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
EXT_EXTERN PFNGLATTACHSHADERPROC glAttachShader;
EXT_EXTERN PFNGLGETPROGRAMIVPROC glGetProgramiv;
EXT_EXTERN PFNGLLINKPROGRAMPROC glLinkProgram;
EXT_EXTERN PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
EXT_EXTERN PFNGLVALIDATEPROGRAMPROC glValidateProgram;
EXT_EXTERN PFNGLCREATEPROGRAMPROC glCreateProgram;
EXT_EXTERN PFNGLUSEPROGRAMPROC glUseProgram;
EXT_EXTERN PFNGLGENBUFFERSPROC glGenBuffers;
EXT_EXTERN PFNGLBINDBUFFERPROC glBindBuffer;
EXT_EXTERN PFNGLBUFFERDATAPROC glBufferData;
EXT_EXTERN PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
EXT_EXTERN PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
EXT_EXTERN PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
EXT_EXTERN PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
EXT_EXTERN PFNGLUNIFORM1FPROC glUniform1f;
EXT_EXTERN PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
EXT_EXTERN PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
#if defined(_WIN32)
EXT_EXTERN PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif
EXT_EXTERN PFNGLUNIFORM1IPROC glUniform1i;

#undef EXT_EXTERN

#endif
