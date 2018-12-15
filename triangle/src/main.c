#include <karbon/drive.h>
#include <GL/gl3w.h>
#include <assert.h>
#include <math.h>


/* this Open GL code is modified from https://open.gl/drawing */


struct ogl_triangle {
        GLuint vao, vbo, pro;
} tri;


KD_API KD_EXPORT void
kd_setup() {
        gl3wInit();
        
        /* vao */
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        tri.vao = vao;

        /* vbo */
        GLfloat verts[] = {
                0.0f,  0.5f,
                0.5f, -0.5f,
                -0.5f, -0.5f
        };

        GLuint vbo;
        glBindBuffer(GL_ARRAY_BUFFER, &vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        tri.vbo = vbo;

        /* shd */
        const GLchar *vs_src = ""
                "#version 150 core\n"
                "in vec2 position;\n"
                "void main() {\n"
                        "gl_Position = vec4(position, 0.0, 1.0);\n"
                "}";

        GLuint vs_shd = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs_shd, 1, &vs_src, NULL);
        glCompileShader(vs_shd);

        const GLchar *fs_src = ""
                "#version 150 core\n"
                "out vec4 outColor;\n"
                "void main() {\n"
                        "outColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
                "}\n";

        GLuint fs_shd = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs_shd, 1, &fs_src, NULL);
        glCompileShader(fs_shd);

        GLuint pro = glCreateProgram();
        glAttachShader(pro, vs_shd);
        glAttachShader(pro, fs_shd);
        glBindFragDataLocation(pro, 0, "outColor");
        glLinkProgram(pro);

        glDeleteShader(vs_shd);
        glDeleteShader(fs_shd);

        tri.pro = pro;
}


KD_API KD_EXPORT void
kd_shutdown() {
        glDeleteProgram(tri.pro);
        glDeleteBuffers(1, &tri.vbo);
        glDeleteVertexArrays(1, &tri.vao);
}


KD_API KD_EXPORT void
kd_think() {
        struct kd_window_desc win_desc = {0};
        win_desc.type_id = KD_STRUCT_WINDOW_DESC;
        kd_result ok = kd_window_get(&win_desc);
        //assert(ok == KD_RESULT_OK);

        /* clear */
        glClearColor(0.2, 0.15, 0.15, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* setup */
        glBindVertexArray(tri.vao);
        glUseProgram(tri.pro);

        /* input */
        GLint posAttrib = glGetAttribLocation(tri.pro, "position");
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

        /* draw */
        glDrawArrays(GL_TRIANGLES, 0, 3);
}


KD_API KD_EXPORT int
kd_project_entry() {
        kd_load(0);
        return 1; /* 1 for good, 0 for fail */
}
