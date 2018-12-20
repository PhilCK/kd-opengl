#include <karbon/drive.h>
#include <karbon/app.h>

#define COMMON_IMPL
#include <common.h>

#include <GL/gl3w.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <string.h>


/* this OpenGL code is modified from https://open.gl/drawing */


/* ----------------------------------------------------------- Application -- */


struct ogl_triangle {
        GLuint vao, vbo, pro;
} tri;


void
setup()
{
        memset(&tri, 0, sizeof(tri));

        cmn_setup();

        if(GL_DEBUG_HELPERS && glPushDebugGroup) {
                glPushDebugGroup(
                        GL_DEBUG_SOURCE_APPLICATION,
                        GL_DEBUG_TYPE_PUSH_GROUP,
                        -1,
                        "Triangle Setup");
        }

        /* vao */
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        tri.vao = vao;
        
        if(GL_DEBUG_HELPERS && glObjectLabel) {
                glObjectLabel(GL_VERTEX_ARRAY, vao, -1, "Triangle::VAO");
        }

        /* vbo */
        GLfloat verts[] = {
                0.0f,  0.5f,
                0.5f, -0.5f,
                -0.5f, -0.5f
        };

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        tri.vbo = vbo;

        if(GL_DEBUG_HELPERS && glObjectLabel) {
                glObjectLabel(GL_BUFFER, vbo, -1, "Triangle::VBO");
        }

        /* shd */
        const GLchar *vs_src = ""
                "#version 130\n"
                "in vec2 position;\n"
                "void main() {\n"
                        "gl_Position = vec4(position, 0.0, 1.0);\n"
                "}";

        GLuint vs_shd = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs_shd, 1, &vs_src, NULL);
        glCompileShader(vs_shd);
        GLint vs_status;
        glGetShaderiv(vs_shd, GL_COMPILE_STATUS, &vs_status);
        if(vs_status == GL_FALSE) {
                char buffer[1024];
                glGetShaderInfoLog(vs_shd, sizeof(buffer), NULL, buffer);
                printf("GL Shd Err: %s\n", buffer);
                assert(!"Failed to build vs shd");
        }

        const GLchar *fs_src = ""
                "#version 130\n"
                "out vec4 outColor;\n"
                "void main() {\n"
                        "outColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
                "}\n";

        GLuint fs_shd = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs_shd, 1, &fs_src, NULL);
        glCompileShader(fs_shd);
        GLint fs_status;
        glGetShaderiv(fs_shd, GL_COMPILE_STATUS, &fs_status);
        if(fs_status == GL_FALSE) {
                char buffer[1024];
                glGetShaderInfoLog(vs_shd, sizeof(buffer), NULL, buffer);
                printf("GL Shd Err: %s\n", buffer);
                assert(!"Failed to build fs shd");
        }

        GLuint pro = glCreateProgram();
        glAttachShader(pro, vs_shd);
        glAttachShader(pro, fs_shd);
        glBindFragDataLocation(pro, 0, "outColor");
        glLinkProgram(pro);
        GLint pro_status;
        glGetProgramiv(pro, GL_LINK_STATUS, &pro_status);
        if(pro_status == GL_FALSE) {
                char buffer[1024];
                glGetProgramInfoLog(pro, sizeof(buffer), NULL, buffer);
                printf("GL Pro Err: %s\n", buffer);
                assert(!"Failed to link");
        }

        glDeleteShader(vs_shd);
        glDeleteShader(fs_shd);

        tri.pro = pro;

        if(GL_DEBUG_HELPERS && glObjectLabel) {
                glObjectLabel(GL_PROGRAM, pro, -1, "Fill::White");
        }

        GL_ERR("Setup")

        if(GL_DEBUG_HELPERS && glPopDebugGroup) {
                glPopDebugGroup();
        }
}


void
shutdown()
{
        glDeleteProgram(tri.pro);
        glDeleteBuffers(1, &tri.vbo);
        glDeleteVertexArrays(1, &tri.vao);
}


void
think()
{
        cmn_process_events();

        GL_ERR("New Frame");

        /* render */
        if(GL_DEBUG_HELPERS && glPushDebugGroup) {
                glPushDebugGroup(
                        GL_DEBUG_SOURCE_APPLICATION,
                        GL_DEBUG_TYPE_PUSH_GROUP,
                        -1,
                        "Triangle Render");
        }

        /* clear */
        glViewport(0, 0, 600, 600);
        glClearColor(0.2, 0.15, 0.15, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_ERR("Clear")

        /* setup */
        glBindVertexArray(tri.vao);
        GL_ERR("Setup - VAO")
        glUseProgram(tri.pro);
        GL_ERR("Setup - use program")
        glBindBuffer(GL_ARRAY_BUFFER, tri.vbo);
        GL_ERR("Setup")

        /* input */
        GLint posAttrib = glGetAttribLocation(tri.pro, "position");
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
        GL_ERR("Input")

        /* draw */
        glDrawArrays(GL_TRIANGLES, 0, 3);
        GL_ERR("Draw")

        if(GL_DEBUG_HELPERS && glPopDebugGroup) {
                glPopDebugGroup();
        }

        GL_ERR("End Frame");
}


/* ----------------------------------------------- Application Description -- */


KD_APP_NAME("OpenGL Triangle")
KD_APP_DESC("Rendering a triangle with OpenGL")
KD_APP_GRAPHICS_API("OpenGL")
KD_APP_STARTUP_FN(setup)
KD_APP_TICK_FN(think)
KD_APP_SHUTDOWN_FN(shutdown)
