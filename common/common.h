#ifndef COMMON_INCLUDED
#define COMMON_INCLUDED


#define GL_ERR(msg) \
do { \
        GLuint err = glGetError(); \
        if(err) { \
                printf("GL Err: %d - %s\n", err, msg); \
                assert(!msg); \
        } \
} while(0); 


#define GL_DEBUG_HELPERS 1


void
cmd_setup();


void
cmn_process_events();


#endif


/* ================================== IMPL ================================== */


#ifdef COMMON_IMPL
#ifndef COMMON_IMPL_INCLUDED
#define COMMON_IMPL_INCLUDED


#include <karbon/drive.h>
#include <GL/gl3w.h>
#include <assert.h>
#include <stdio.h>


void
cmn_setup()
{
        kd_result ok = KD_RESULT_OK;

        ok = kd_gl_make_current();
        assert(ok == KD_RESULT_OK && "Failed to make current");

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
  
}


void
cmn_process_events()
{
        kd_result ok = KD_RESULT_OK;

        /* check events */
        uint64_t events = 0;
        ok = kd_events_get(&events);
        assert(ok == KD_RESULT_OK && "Failed to get events");

        /* screen was resized */
        if(events & KD_EVENT_VIEWPORT_RESIZE) {
                /*
                struct kd_window_desc win_desc;
                win_desc.type_id = KD_STRUCT_WINDOW_DESC;
                kd_window_get(&win_desc);

                glViewport(0, 0, win_desc.width, win_desc.height);
                */
        }

        /* a key was pushed */
        if(events & KD_EVENT_INPUT_KB) {
                int app_idx, app_count;
                ok = kd_ctx_application_index_get(&app_idx, &app_count);
                assert(ok == KD_RESULT_OK && "Failed to get index");

                int next_idx = (app_idx + 1) % app_count;
                printf(
                        "Curr IDX(%d) of (%d), Next IDX(%d)",
                        app_idx,
                        app_count,
                        next_idx);

                ok = kd_ctx_application_index_set(next_idx);
                assert(ok == KD_RESULT_OK && "Failed to set index");
        }
}



#endif
#endif

