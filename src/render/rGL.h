#ifndef AT_GL_H
#define AT_GL_H

#ifndef DEDICATED

#ifdef WIN32
#include <windows.h>
#endif


#include <SDL3/SDL_opengl.h>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif



#else

typedef float GLfloat;
typedef unsigned char GLubyte;
typedef unsigned int GLuint;
typedef unsigned int GLenum;
#endif


#ifdef DEBUG
#ifndef DEDICATED
#define AA_GL_ERROR_CHECKING
#endif
#endif

#ifdef AA_GL_ERROR_CHECKING
//! for debugging purposes: checks for OpenGL errors and prints them to the console.
void sr_CheckGLError();
#else
inline void sr_CheckGLError(){}
#endif

#endif
