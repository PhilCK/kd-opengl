#include <karbon/drive.h>
#include <karbon/app.h>

#define COMMON_IMPL
#include <common.h>

#include <GL/gl3w.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <string.h>


/* ----------------------------------------------------------- Application -- */


struct ogl_clear_color {
        float timer;
} clear_color;


void
setup()
{
        memset(&clear_color, 0, sizeof(clear_color));

        cmn_setup();

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

        printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION),
                glGetString(GL_SHADING_LANGUAGE_VERSION));

        GL_ERR("GL - Setup");
}


void
think()
{
        cmn_process_events();
        
        GL_ERR("GL - New Frame");

        /* project tick */
        clear_color.timer += 0.16f;
        
        float r = (2.f + sinf(clear_color.timer * 0.15f)) / 2.f;
        float g = (2.f + cosf(clear_color.timer * 0.20f)) / 2.f;
        float b = (2.f + sinf(clear_color.timer * 0.10f)) / 2.f;
        float a = 1.f;

        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT);

        GL_ERR("GL - End Frame");
}


/* ----------------------------------------------- Application Description -- */


KD_APP_NAME("OpenGL Clear Color")
KD_APP_DESC("Clear a screen to a color")
KD_APP_GRAPHICS_API("OpenGL")
KD_APP_STARTUP_FN(setup)
KD_APP_TICK_FN(think)
