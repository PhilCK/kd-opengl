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
                        "Tex Quad Setup");
        }

        /* vao */
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        tex.vao = vao;
        
        if(GL_DEBUG_HELPERS && glObjectLabel) {
                glObjectLabel(GL_VERTEX_ARRAY, vao, -1, "TexQuad::VAO");
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
                glObjectLabel(GL_BUFFER, ebo, -1, "TexQuad::EBO::ui");
        }
        
        /* vbo */
        /* pos v2 - color v3 - tc v2 */
        GLfloat verts[] = {
                -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
                -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        };

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        tex.vbo = vbo;

        if(GL_DEBUG_HELPERS && glObjectLabel) {
                glObjectLabel(GL_BUFFER, vbo, -1, "TexQuad::VBO::v2v3v2");
        }

        /* textures */
        int w, h, c;
        char t[2048] = {0};
        kd_ctx_get_exe_dir(&t, 0);
        strcat(t, "assets/sample.png");

        unsigned char* img1 = stbi_load(&t[0], &w, &h, &c, 0);

        if(img1) {
                printf("Loaded Image 1: %dx%d:%d\n", w, h, c);

                GLuint tex1;
                glGenTextures(1, &tex1);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, tex1);

                glTexImage2D(
                        GL_TEXTURE_2D,
                        0,
                        GL_RGB,
                        w,
                        h,
                        0,
                        c == 4 ? GL_RGBA : GL_RGB,
                        GL_UNSIGNED_BYTE,
                        img1);

                tex.tex1 = tex1;

                stbi_image_free(img1);
                if (GL_DEBUG_HELPERS && glObjectLabel) {
                        glObjectLabel(GL_TEXTURE, tex1, -1, "TexQuad::Sample1");
                }
        }

        memset(t, 0, sizeof(t));
        kd_ctx_get_exe_dir(&t, 0);
        strcat(t, "assets/sample2.png");

        unsigned char *img2 = stbi_load(t, &w, &h, &c, 0);

        if(img2) {
                printf("Loaded Image 2: %dx%d:%d\n", w, h, c);

                GLuint tex2;
                glGenTextures(1, &tex2);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, tex2);

                glTexImage2D(
                        GL_TEXTURE_2D,
                        0,
                        GL_RGB,
                        w,
                        h,
                        0,
                        c == 4 ? GL_RGBA : GL_RGB,
                        GL_UNSIGNED_BYTE,
                        img2);

                if (GL_DEBUG_HELPERS && glObjectLabel) {
                        glObjectLabel(GL_TEXTURE, tex2, -1, "TexQuad::Sample2");
                }

                tex.tex2 = tex2;
                stbi_image_free(img2);
        }
        
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
                        "outColor = mix("
                                "texture(texKitten, Texcoord),"
                                "texture(texPuppy, Texcoord), 0.5);\n"
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
                glObjectLabel(GL_PROGRAM, pro, -1, "TexQuad::Fill");
        }

        GL_ERR("Setup")

        if(GL_DEBUG_HELPERS && glPopDebugGroup) {
                glPopDebugGroup();
        }
}


void
shutdown() {
        if(tex.tex1) {
                glDeleteTextures(1, &tex.tex1);
        }

        if(tex.tex2) {
                glDeleteTextures(1, &tex.tex2);
        }

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
                        "Tex Quad Render");
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

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex.tex1);
        glUniform1i(glGetUniformLocation(tex.pro, "texKitten"), 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex.tex2);
        glUniform1i(glGetUniformLocation(tex.pro, "texPuppy"), 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        /* input */
        GLsizei jmp = 7 * sizeof(GLfloat);
        void *off = 0;

        GLint pos_a = glGetAttribLocation(tex.pro, "position");
        if(pos_a > -1) {
                glEnableVertexAttribArray(pos_a);
                glVertexAttribPointer(pos_a, 2, GL_FLOAT, GL_FALSE, jmp, off);
                GL_ERR("Input - POS")

        }
        off = (void*)(2 * sizeof(GLfloat));

        GLint col_a = glGetAttribLocation(tex.pro, "color");
        if(col_a > -1) {
                glEnableVertexAttribArray(col_a);
                glVertexAttribPointer(col_a, 3, GL_FLOAT, GL_FALSE, jmp, off);
                GL_ERR("Input - COL2")
        }
        off = (void*)(5 * sizeof(GLfloat));

        GLint tex_a = glGetAttribLocation(tex.pro, "texcoord");
        if(tex_a > -1) {
                glEnableVertexAttribArray(tex_a);
                glVertexAttribPointer(tex_a, 2, GL_FLOAT, GL_FALSE, jmp, off);
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
