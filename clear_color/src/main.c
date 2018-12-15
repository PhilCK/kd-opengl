#include <karbon/drive.h>
#include <GL/gl3w.h>
#include <assert.h>
#include <math.h>


struct ogl_clear_color {
        float timer;
} clear_color;


KD_API KD_EXPORT void
kd_setup() {
        gl3wInit();
        clear_color.timer = 0.f;
}


KD_API KD_EXPORT void
kd_think() {
        /* project tick */
        clear_color.timer += 0.16f;

        float r = (2.f + sinf(clear_color.timer * 0.15f)) / 2.f;
        float g = (2.f + cosf(clear_color.timer * 0.20f)) / 2.f;
        float b = (2.f + sinf(clear_color.timer * 0.10f)) / 2.f;
        float a = 1.f;

        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT);
}


KD_API KD_EXPORT int
kd_project_entry()
{
        kd_load(0);
        return 1; /* 1 for good, 0 for fail */
}
