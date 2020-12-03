/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2020 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#pragma once

#include <set>
#include <string>
#include <mango/core/configure.hpp>

// -----------------------------------------------------------------------
// OpenGL API
// -----------------------------------------------------------------------

//#define MANGO_OPENGL_DISABLE_PLATFORM_API
//#define MANGO_OPENGL_LEGACY_PROFILE
//#define MANGO_OPENGL_CORE_PROFILE

#if defined(MANGO_PLATFORM_WINDOWS)

    // -----------------------------------------------------------------------
    // WGL
    // -----------------------------------------------------------------------

    #define MANGO_OPENGL_CONTEXT_WGL

    #define GLEXT_PROC(proc, name) extern proc name

    #ifdef MANGO_OPENGL_CORE_PROFILE
        #include <mango/opengl/khronos/GL/glcorearb.h>
        #include <mango/opengl/func/glcorearb.hpp>
    #else
        #include <GL/gl.h>
        #include <mango/opengl/khronos/GL/glext.h>
        #include <mango/opengl/func/glext.hpp>
    #endif

    #ifndef MANGO_OPENGL_DISABLE_PLATFORM_API
        #include <mango/opengl/khronos/GL/wgl.h>
        #include <mango/opengl/khronos/GL/wglext.h>
        #include <mango/opengl/func/wglext.hpp>
    #endif

    #undef GLEXT_PROC

#elif defined(MANGO_PLATFORM_OSX)

    // -----------------------------------------------------------------------
    // Cocoa
    // -----------------------------------------------------------------------

    #define MANGO_OPENGL_CONTEXT_COCOA

    #define GL_SILENCE_DEPRECATION /* macOS 10.14 deprecated OpenGL API */

    #if defined(MANGO_OPENGL_LEGACY_PROFILE)
        #include <OpenGL/gl.h>
        #include <OpenGL/glext.h>
    #else
        #include <OpenGL/gl3.h>
        #include <OpenGL/gl3ext.h>

        #define GL_GLEXT_PROTOTYPES
        #include <mango/opengl/khronos/GL/glext.h>
    #endif

#elif defined(MANGO_PLATFORM_IOS)

    // -----------------------------------------------------------------------
    // EGL
    // -----------------------------------------------------------------------

    #define MANGO_OPENGL_CONTEXT_EGL

    //#include <OpenGLES/ES1/gl.h>
    //#include <OpenGLES/ES1/glext.h>

    // TODO: EGL context

#elif defined(MANGO_PLATFORM_ANDROID)

    // -----------------------------------------------------------------------
    // EGL
    // -----------------------------------------------------------------------

    #define MANGO_OPENGL_CONTEXT_EGL

    //#include <GLES/gl.h>
    //#include <GLES/glext.h>
    //#include <GLES2/gl2.h>
    #include <GLES3/gl3.h>

    // TODO: EGL context

#elif defined(MANGO_PLATFORM_UNIX)

    // -----------------------------------------------------------------------
    // GLX
    // -----------------------------------------------------------------------

    #define MANGO_OPENGL_CONTEXT_GLX

    #ifdef MANGO_OPENGL_CORE_PROFILE
        #define GL_GLEXT_PROTOTYPES
        #include <mango/opengl/khronos/GL/glcorearb.h>
    #else
        #define GL_GLEXT_PROTOTYPES
        #include <GL/gl.h>
        #include <GL/glext.h>
    #endif

    #ifndef MANGO_OPENGL_DISABLE_PLATFORM_API
        #define GLX_GLXEXT_PROTOTYPES
        #include <GL/glx.h>
        #include <GL/glxext.h>
        #if defined (Status)
        #undef Status
            typedef int Status;
        #endif
    #endif

#else

    //#error "Unsupported OpenGL implementation."

#endif

#include <mango/image/compression.hpp>
#include <mango/window/window.hpp>

namespace mango {

    // -------------------------------------------------------------------
    // OpenGLContext
    // -------------------------------------------------------------------

    class OpenGLContext : public Window
    {
    protected:
        struct OpenGLContextHandle* m_context;
        std::set<std::string> m_extensions;

        void initExtensionMask();

    public:
        struct Config
        {
            u32 version  = 0;

            u32 red      = 8;
            u32 green    = 8;
            u32 blue     = 8;
            u32 alpha    = 8;
            u32 depth    = 24;
            u32 stencil  = 8;
            u32 samples  = 1;
        };

        struct InternalFormat
        {
            GLenum iformat;
            Format format;
            bool srgb;
            const char* name;
        };

        OpenGLContext(int width, int height, u32 flags = 0, const Config* config = nullptr, OpenGLContext* shared = nullptr);
        ~OpenGLContext();

        bool isExtension(const std::string& name) const;
        bool isGLES() const;
        int getVersion() const;
        bool isCompressedTextureSupported(TextureCompression compression) const;
        const InternalFormat* getInternalFormat(GLenum internalFormat) const;

        void makeCurrent();
        void swapBuffers();
        void swapInterval(int interval);
        void toggleFullscreen();
        bool isFullscreen() const;
        int32x2 getWindowSize() const override;
    };

    // -------------------------------------------------------------------
    // OpenGLFramebuffer
    // -------------------------------------------------------------------

    class OpenGLFramebuffer : public OpenGLContext
    {
    protected:
        Surface m_surface;

        GLuint m_texture = 0;
        GLuint m_buffer = 0;

        GLuint m_vao = 0;
        GLuint m_vbo = 0;
        GLuint m_ibo = 0;

        struct Program
        {
            GLuint program = 0;
            GLint transform = -1;
            GLint texture = -1;
            GLint scale = -1;
            GLint position = -1;
        };

        Program m_bilinear;
        Program m_bicubic;

    public:
        enum Filter
        {
            FILTER_NEAREST,
            FILTER_BILINEAR,
            FILTER_BICUBIC
        };

        OpenGLFramebuffer(int width, int height);
        ~OpenGLFramebuffer();

        Surface lock();
        void unlock();
        void present(Filter filter = FILTER_NEAREST);
    };

    // -------------------------------------------------------------------
    // glext
    // -------------------------------------------------------------------

    struct glExtensionMask
    {
#define GL_EXTENSION(Name) u32 Name : 1;
#include <mango/opengl/func/glext.hpp>
#undef GL_EXTENSION
    };

    extern glExtensionMask glext;

    // -------------------------------------------------------------------
    // core
    // -------------------------------------------------------------------

    struct coreExtensionMask
    {
#define CORE_EXTENSION(Version, Name) u32 Name : 1;
#include <mango/opengl/func/glcorearb.hpp>
#undef CORE_EXTENSION

        // custom extension flags
        u32 texture_compression_dxt1 : 1;
        u32 texture_compression_dxt3 : 1;
        u32 texture_compression_dxt5 : 1;
        u32 texture_compression_etc2 : 1;
        u32 texture_compression_eac : 1;
        u32 texture_compression_latc : 1;
        u32 texture_compression_atc : 1;
        u32 texture_compression_astc : 1;
    };

    extern coreExtensionMask core;

    // -------------------------------------------------------------------
    // wglext
    // -------------------------------------------------------------------

#ifndef MANGO_OPENGL_DISABLE_PLATFORM_API
#ifdef MANGO_OPENGL_CONTEXT_WGL

    struct wglExtensionMask
    {
#define WGL_EXTENSION(Name) u32 Name : 1;
#include <mango/opengl/func/wglext.hpp>
#undef WGL_EXTENSION
    };

    extern wglExtensionMask wglext;

#endif // MANGO_OPENGL_CONTEXT_WGL
#endif // MANGO_OPENGL_DISABLE_PLATFORM_API

    // -------------------------------------------------------------------
    // glxext
    // -------------------------------------------------------------------

#ifndef MANGO_OPENGL_DISABLE_PLATFORM_API
#ifdef MANGO_OPENGL_CONTEXT_GLX

    struct glxExtensionMask
    {
#define GLX_EXTENSION(Name) u32 Name : 1;
#include <mango/opengl/func/glxext.hpp>
#undef GLX_EXTENSION
    };

    extern glxExtensionMask glxext;

#endif // MANGO_OPENGL_CONTEXT_GLX
#endif // MANGO_OPENGL_DISABLE_PLATFORM_API

} // namespace mango
