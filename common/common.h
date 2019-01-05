#ifndef COMMON_INCLUDED
#define COMMON_INCLUDED


#define GL_ERR(msg)                                     \
do {                                                    \
        GLuint err = glGetError();                      \
        if(err) {                                       \
                printf("GL Err: %d - %s\n", err, msg);  \
                assert(!msg);                           \
        }                                               \
} while(0);                                             \


#define GL_DEBUG_HELPERS 1


void
cmn_setup();


uint64_t
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

        if (!gl3wIsSupported(3, 2)) {
                assert(!"OGL 3 2 not supported");
        } 
}


uint64_t
cmn_process_events()
{
        kd_result ok = KD_RESULT_OK;

        /* check events */
        uint64_t events = 0;
        ok = kd_events_get(&events);
        assert(ok == KD_RESULT_OK && "Failed to get events");

        #ifndef NDEBUG
        /* clear gl errors */
        while(glGetError()){}
        #endif

        /* screen was resized */
        if(events & KD_EVENT_VIEWPORT_RESIZE) {
                struct kd_window_desc win_desc;
                win_desc.type_id = KD_STRUCT_WINDOW_DESC;
                kd_window_get(&win_desc);

                glViewport(0, 0, win_desc.width, win_desc.height);
        }

        /* a key was pushed */
        if(events & KD_EVENT_INPUT_KB) {
                struct kd_keyboard_desc kb_desc;
                ok = kd_input_get_keyboards(&kb_desc);
                printf("res %d == %d\n", ok, KD_RESULT_OK);
                assert(ok == KD_RESULT_OK && "Failed to get kb desc");
                assert(kb_desc.kb_count && "No kb found");

                if(kb_desc.kb_state[0][KD_KB_ANY] & KD_KEY_UP_EVENT) {

                        int app_idx, app_cnt;
                        ok = kd_ctx_application_index_get(&app_idx, &app_cnt);
                        assert(ok == KD_RESULT_OK && "Failed to get index");

                        int next_idx = (app_idx + 1) % app_cnt;
                        printf(
                                "Curr IDX(%d) of (%d), Next IDX(%d)",
                                app_idx,
                                app_cnt,
                                next_idx);

                        ok = kd_ctx_application_index_set(next_idx);
                        assert(ok == KD_RESULT_OK && "Failed to set index");
                }
        }

        GL_ERR("GL - Common Setup")

        return events;
}



#endif
#endif

