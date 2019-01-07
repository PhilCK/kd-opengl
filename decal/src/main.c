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


struct ogl_decal {
        GLuint vao;

        /* Render Scene */
        GLuint sce_vbo;
        GLuint sce_tex1, sce_tex2;
        GLuint sce_gbuff_pro;
        GLuint sce_light_pro;
        GLuint sce_fbo;

        /* Render Decal */
        GLuint dec_vbo;
        GLuint dec_pro;

        /* Fullscreen Blit */
        GLuint fsb_vbo;
        GLuint fsb_pro;

        float time;
} glctx;


void
setup()
{
        memset(&glctx, 0, sizeof(glctx));
        cmn_setup();

        cmn_push_debug_group("Decal Setup");

        /* vao */
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glctx.vao = vao;

        cmn_label_object(GL_VERTEX_ARRAY, vao, "Decal::VAO");

        cmn_pop_debug_group();
}


void
shutdown()
{
}


void
render(int steps) {
        struct kd_window_desc win_desc;
        win_desc.type_id = KD_STRUCT_WINDOW_DESC;
        kd_window_get(&win_desc);
}

void
think()
{
        cmn_process_events();
        glctx.time += 0.01f;

        struct kd_window_desc win_desc;
        win_desc.type_id = KD_STRUCT_WINDOW_DESC;
        kd_window_get(&win_desc);

        glBindVertexArray(glctx.vao);

        render(10);

        GL_ERR("End Frame");
}


/* ----------------------------------------------- Application Description -- */


KD_APP_NAME("OpenGL Decal Projection")
KD_APP_DESC("Render a quad with a decal projection")
KD_APP_GRAPHICS_API("OpenGL")
KD_APP_STARTUP_FN(setup)
KD_APP_TICK_FN(think)
KD_APP_SHUTDOWN_FN(shutdown)
