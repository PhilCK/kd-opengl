#include <karbon/drive.h>
#include <karbon/app.h>
#include <GL/gl3w.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

/* this OpenGL code is modified from https://open.gl/textures */


/* ----------------------------------------------------------- Application -- */


struct ogl_texture {
        GLuint vao, ebo, tex1, tex2, vbo, pro;
} tex;


#define GL_ERR(msg) \
do { \
        GLuint err = glGetError(); \
        if(err) { \
                printf("GL Err: %d - %s\n", err, msg); \
                assert(0); \
        } \
} while(0); 


#define GL_DEBUG_HELPERS 1


void
setup() {
        kd_gl_make_current();

        if (gl3wInit()) {
                assert(!"FAILED TO INIT");
        }
        
        /* print out version */
        printf("OpenGL %s, GLSL %s, OGL %s\n",
            glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION),
            glGetString(GL_VERSION));

        if (!gl3wIsSupported(3, 0)) {
                assert(!"OGL 3 0 not supported");
        }

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
        tex.vao = vao;
        
        if(GL_DEBUG_HELPERS && glObjectLabel) {
                glObjectLabel(GL_VERTEX_ARRAY, vao, -1, "Quad::VAO");
        }

        /* ebo */
        GLuint idx[] = {
                0,1,2,
                2,3,0
        };

        GLuint ebo;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
        tex.ebo = ebo;

        if(GL_DEBUG_HELPERS && glObjectLabel) {
                glObjectLabel(GL_BUFFER, ebo, -1, "Quad::EBO");
        }
        
        /* vbo */
        GLfloat verts[] = {
                //  Position      Color             Texcoords
                -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
                 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
                 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
                -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
        };

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        tex.vbo = vbo;

        if(GL_DEBUG_HELPERS && glObjectLabel) {
                glObjectLabel(GL_BUFFER, vbo, -1, "Quad::VBO::v2v3v2");
        }

        /* textures */
        GLint tex1;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, &tex1);

        int w, h, c;
        unsigned char* img1 = stbi_load("sample.png", &w, &h, &c, STBI_rgb);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img1);
        tex.tex1 = tex1;

        /* shd */
        const GLchar *vs_src = ""
                "#version 130\n"
                "in vec2 position;\n"
                "in vec3 color;\n"
                "in vec2 texcoord;\n"
                "out vec3 Color;\n"
                "out vec2 Texcoord;\n"
                "void main() {\n"
                        "Color = color;\n"
                        "Texcoord = texcoord;\n"
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
                "in vec3 Color;\n"
                "in vec2 Texcoord;\n"
                "uniform sampler2D texKitten;\n"
                "uniform sampler2D texPuppy;\n"
                "out vec4 outColor;\n"
                "void main() {\n"
                        "outColor = vec4(1,0,0,1);\n"
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

        tex.pro = pro;

        if(GL_DEBUG_HELPERS && glObjectLabel) {
                glObjectLabel(GL_PROGRAM, pro, -1, "Fill::Textures");
        }

        GL_ERR("Setup")

        if(GL_DEBUG_HELPERS && glPopDebugGroup) {
                glPopDebugGroup();
        }
}


void
shutdown() {
        glDeleteProgram(tex.pro);
        glDeleteBuffers(1, &tex.vbo);
        glDeleteVertexArrays(1, &tex.vao);
}


void
think() {
        /* switch to next app */
        struct kd_keyboard_desc kb;
        kd_input_get_keyboards(&kb);
        if (kb.kb_state[0][KD_KB_ANY] & KD_KEY_DOWN_EVENT) {
                /* change state */
                int app_idx, app_count;
                kd_ctx_application_index_get(&app_idx, &app_count);

                int next_idx = (app_idx + 1) % app_count;
                printf(
                        "Curr IDX(%d) of (%d), Next IDX(%d)",
                        app_idx,
                        app_count,
                        next_idx);

                kd_ctx_application_index_set(next_idx);
        }

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
        glBindVertexArray(tex.vao);
        GL_ERR("Setup - VAO")
        glUseProgram(tex.pro);
        GL_ERR("Setup - use program")
        glBindBuffer(GL_ARRAY_BUFFER, tex.vbo);
        GL_ERR("Setup - vbo")
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tex.ebo);
        GL_ERR("Setup - ebo");

        /* input */
        GLint posAttrib = glGetAttribLocation(tex.pro, "position");
        if(posAttrib > -1) {
                glEnableVertexAttribArray(posAttrib);
                glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);
                GL_ERR("Input - POS")
        }

        GLint colAttrib = glGetAttribLocation(tex.pro, "color");
        if(colAttrib > -1) {
                glEnableVertexAttribArray(colAttrib);
                glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
                GL_ERR("Input - COL2")
        }

        GLint texAttrib = glGetAttribLocation(tex.pro, "texcoord");
        if(texAttrib > -1) {
                glEnableVertexAttribArray(texAttrib);
                glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
                GL_ERR("Input - TC")
        }

        /* draw */
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        GL_ERR("Draw")

        if(GL_DEBUG_HELPERS && glPopDebugGroup) {
                glPopDebugGroup();
        }
}


/* ----------------------------------------------- Application Description -- */


KD_APP_NAME("OpenGL Textures")
KD_APP_DESC("Rendering a quad with Textures in OpenGL")
KD_APP_GRAPHICS_API("OpenGL")
KD_APP_STARTUP_FN(setup)
KD_APP_TICK_FN(think)
KD_APP_SHUTDOWN_FN(shutdown)
