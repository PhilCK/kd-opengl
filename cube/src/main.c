#include <karbon/drive.h>
#include <karbon/app.h>
#include <karbon/math.h>

#define COMMON_IMPL
#include <common.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#define STB_IMAGE_IMPLEMENTATION 
#include <stb_image.h>
#pragma GCC diagnostic pop

#include <GL/gl3w.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>


/* this OpenGL code is modified from https://open.gl/depthstencils */


/* ----------------------------------------------------------- Application -- */


struct ogl_cube {
        GLuint vao, tex1, tex2, vbo, pro;
} cube;


void
setup()
{
        memset(&cube, 0, sizeof(cube));
        cmn_setup();

        if(GL_DEBUG_HELPERS && glPushDebugGroup) {
                glPushDebugGroup(
                        GL_DEBUG_SOURCE_APPLICATION,
                        GL_DEBUG_TYPE_PUSH_GROUP,
                        -1,
                        "Cube Setup");
        }

        /* vao */
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        cube.vao = vao;
        
        if(GL_DEBUG_HELPERS && glObjectLabel) {
                glObjectLabel(GL_VERTEX_ARRAY, vao, -1, "Cube::VAO");
        }
        
        /* vbo */
        /* pos v2 - color v3 - tc v2 */
        GLfloat verts[] = {
                -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
                0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
                0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

                -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
                0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
                0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

                -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
                -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
                -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

                0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
                0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
                0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

                -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
                0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
                -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
                -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,

                -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
                0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
                -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
                -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f,

                -1.0f, -1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                1.0f, -1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                1.0f,  1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
                1.0f,  1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
                -1.0f,  1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
        };

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        cube.vbo = vbo;

        if(GL_DEBUG_HELPERS && glObjectLabel) {
                glObjectLabel(GL_BUFFER, vbo, -1, "Cube::VBO::f3f3f2");
        }

        /* textures */
        int w, h, c;
        char t1[2048] = {0};
        kd_ctx_get_exe_dir(t1, 0);
        strcat(t1, "assets/sample.png");

        unsigned char* img1 = stbi_load(&t1[0], &w, &h, &c, 0);

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

                cube.tex1 = tex1;

                stbi_image_free(img1);
                if (GL_DEBUG_HELPERS && glObjectLabel) {
                        glObjectLabel(GL_TEXTURE, tex1, -1, "Cube::Sample1");
                }
        }

        char t2[2048] = {0};
        kd_ctx_get_exe_dir(t2, 0);
        strcat(t2, "assets/sample2.png");
        unsigned char* img2 = stbi_load(&t2[0], &w, &h, &c, 0);

        if (img2) {
                printf("Loaded Image 2: %dx%d:%d\n", w, h, c);

                GLuint tex2;
                glGenTextures(1, &tex2);
                glActiveTexture(GL_TEXTURE0);
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

                cube.tex2 = tex2;

                stbi_image_free(img2);
                if (GL_DEBUG_HELPERS && glObjectLabel) {
                        glObjectLabel(GL_TEXTURE, tex2, -1, "Cube::Sample2");
                }
        }

        /* shd */
        const GLchar *vs_src = ""
                "#version 130\n"
                "in vec3 position;\n"
                "in vec3 color;\n"
                "in vec2 texcoord;\n"
                "out vec3 Color;\n"
                "out vec2 Texcoord;\n"
                "uniform vec3 overrideColor;\n"
                "uniform mat4 model;\n"
                "uniform mat4 view;\n"
                "uniform mat4 proj;\n"
                "void main() {\n"
                        "Texcoord = texcoord;\n"
                        "Color = overrideColor * color;\n"
                        "gl_Position = proj * view * model * vec4(position, 1.0);\n"
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
                "in vec2 Texcoord;\n"
                "in vec3 Color;\n"
                "uniform sampler2D texKitten;\n"
                "uniform sampler2D texPuppy;\n"
                "out vec4 outColor;\n"
                "void main() {\n"
                        "outColor = vec4(Color, 1.0) * mix(texture(texKitten, Texcoord), texture(texPuppy, Texcoord), 0.5);\n"
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

        cube.pro = pro;

        if(GL_DEBUG_HELPERS && glObjectLabel) {
                glObjectLabel(GL_PROGRAM, pro, -1, "Cube::Fill");
        }

        GL_ERR("Setup")

        if(GL_DEBUG_HELPERS && glPopDebugGroup) {
                glPopDebugGroup();
        }
}


void
shutdown()
{
        /* texture may have failed to load */
        if(cube.tex1) {
                glDeleteTextures(1, &cube.tex1);
        }

        if (cube.tex2) {
                glDeleteTextures(1, &cube.tex2);
        }

        glDeleteProgram(cube.pro);
        glDeleteBuffers(1, &cube.vbo);
        glDeleteVertexArrays(1, &cube.vao);
}


void
think()
{
        cmn_process_events();

        struct kd_window_desc win_desc;
        win_desc.type_id = KD_STRUCT_WINDOW_DESC;
        kd_window_get(&win_desc);
        
        GL_ERR("New frame");

        /* render */
        if(GL_DEBUG_HELPERS && glPushDebugGroup) {
                glPushDebugGroup(
                        GL_DEBUG_SOURCE_APPLICATION,
                        GL_DEBUG_TYPE_PUSH_GROUP,
                        -1,
                        "Cube Render");
        }

        /* clear */
        glClearColor(0.2, 0.15, 0.15, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GL_ERR("Clear")
        
        /* setup */
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        glBindVertexArray(cube.vao);
        GL_ERR("Setup - VAO")
        glUseProgram(cube.pro);
        GL_ERR("Setup - use program")
        glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
        GL_ERR("Setup - vbo")

        /* draw cube */
        GLint color = glGetUniformLocation(cube.pro, "overrideColor");
        glUniform3f(color, 1.0f, 1.0f, 1.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cube.tex1);
        GLint samp1 = glGetUniformLocation(cube.pro, "texKitten");
        glUniform1i(samp1, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, cube.tex2);
        GLint samp2 = glGetUniformLocation(cube.pro, "texPuppy");
        glUniform1i(samp2, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GL_ERR("Bind textures")

        float model[16];
        kdm_mat4_id(model);
        GLint model_i = glGetUniformLocation(cube.pro, "model");
        glUniformMatrix4fv(model_i, 1, GL_FALSE, model);

        float view[16];
        float eye[] = {2.5f, 2.5f, 1.5f};
        float at[] = {0.f, 0.f, 0.f};
        float up[] = {0.f, 0.f, 1.f};
        kdm_mat4_lookat(eye, at, up, view);
        GLint view_i = glGetUniformLocation(cube.pro, "view");
        glUniformMatrix4fv(view_i, 1, GL_FALSE, view);

        float proj[16];
        float ratio = (float)win_desc.width / (float)win_desc.height;
        float fov = KDM_TAU * 0.125f;
        kdm_mat4_perspective_projection(ratio, 0.1f, 100.f, fov, proj);
        GLint proj_i = glGetUniformLocation(cube.pro, "proj");
        glUniformMatrix4fv(proj_i, 1, GL_FALSE, proj);
        GL_ERR("Update Mats")

        /* input */
        GLsizei jmp = 8 * sizeof(GLfloat);
        void *off = 0;

        GLint pos_a = glGetAttribLocation(cube.pro, "position");
        if(pos_a > -1) {
                glEnableVertexAttribArray(pos_a);
                glVertexAttribPointer(pos_a, 3, GL_FLOAT, GL_FALSE, jmp, off);
                GL_ERR("Input - POS")
        }
        off = (void*)(3 * sizeof(GLfloat));

        GLint col_a = glGetAttribLocation(cube.pro, "color");
        if(col_a > -1) {
                glEnableVertexAttribArray(col_a);
                glVertexAttribPointer(col_a, 3, GL_FLOAT, GL_FALSE, jmp, off);
                GL_ERR("Input - COL3")
        }
        off = (void*)(6 * sizeof(GLfloat));

        GLint tex_a = glGetAttribLocation(cube.pro, "texcoord");
        if(tex_a > -1) {
                glEnableVertexAttribArray(tex_a);
                glVertexAttribPointer(tex_a, 2, GL_FLOAT, GL_FALSE, jmp, off);
                GL_ERR("Input - TC")
        }
        GL_ERR("Vertex Attrs")

        glDrawArrays(GL_TRIANGLES, 0, 36);
        GL_ERR("Draw")

        if(GL_DEBUG_HELPERS && glPopDebugGroup) {
                glPopDebugGroup();
        }

        /* draw floor */
        if(GL_DEBUG_HELPERS && glPushDebugGroup) {
                glPushDebugGroup(
                        GL_DEBUG_SOURCE_APPLICATION,
                        GL_DEBUG_TYPE_PUSH_GROUP,
                        -1,
                        "Cube Floor");
        }

        glEnable(GL_STENCIL_TEST);

        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xFF);
        glDepthMask(GL_FALSE);
        glClear(GL_STENCIL_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 36, 6);

        if(GL_DEBUG_HELPERS && glPopDebugGroup) {
                glPopDebugGroup();
        }

        /* drw reflection */
        if(GL_DEBUG_HELPERS && glPushDebugGroup) {
                glPushDebugGroup(
                        GL_DEBUG_SOURCE_APPLICATION,
                        GL_DEBUG_TYPE_PUSH_GROUP,
                        -1,
                        "Cube Reflection");
        }

        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glDepthMask(GL_TRUE);

        model[14] = -1;
        glUniformMatrix4fv(model_i, 1, GL_FALSE, model);
        glUniform3f(color, 0.3f, 0.3f, 0.3f);
        
        glDrawArrays(GL_TRIANGLES, 0, 36);

        if(GL_DEBUG_HELPERS && glPopDebugGroup) {
                glPopDebugGroup();
        }

        GL_ERR("End Frame");
}


/* ----------------------------------------------- Application Description -- */


KD_APP_NAME("OpenGL Cube")
KD_APP_DESC("Render a spinning cube")
KD_APP_GRAPHICS_API("OpenGL")
KD_APP_STARTUP_FN(setup)
KD_APP_TICK_FN(think)
KD_APP_SHUTDOWN_FN(shutdown)
