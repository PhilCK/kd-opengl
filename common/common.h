#ifndef COMMON_INCLUDED
#define COMMON_INCLUDED


typedef unsigned int GLenum;
typedef unsigned int GLuint;


#define GL_ERR(msg)                                           \
do {                                                          \
        GLuint err = glGetError();                            \
        if(err) {                                             \
                assert(sizeof(msg) < 256 && "GL msg too big");\
                char buf[512] = {0};                          \
                sprintf(buf, "GL ERR: %d - %s\n", err, msg);  \
                kd_log(KD_LOG_ERROR, buf);                    \
                assert(!msg);                                 \
        }                                                     \
} while(0);                                                   \


#define GL_DEBUG_HELPERS 1


void
cmn_setup();


uint64_t
cmn_process_events();


void
cmn_push_debug_group(
        const char *group_name);


void
cmn_pop_debug_group();


void
cmn_label_object(
        GLenum obj,
        GLuint obj_id,
        const char *name);
        


#endif


/* ================================== IMPL ================================== */


#ifdef COMMON_IMPL
#ifndef COMMON_IMPL_INCLUDED
#define COMMON_IMPL_INCLUDED


#include <karbon/drive.h>
#include <GL/gl3w.h>
#include <assert.h>
#include <stdio.h>


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined(_MSC_VER )
#pragma warning(push)
#pragma warning(disable: 4100)
#endif

#define STB_IMAGE_IMPLEMENTATION 
#include <stb_image.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#elif defined(_MSC_VER )
#pragma warning(pop)
#endif


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
        char buf[512] = {0};
        sprintf(
                buf,
                "OpenGL %s, GLSL %s, OGL %s\n",
                glGetString(GL_VERSION),
                glGetString(GL_SHADING_LANGUAGE_VERSION),
                glGetString(GL_VERSION));

        ok = kd_log(KD_LOG_INFO, buf);
        assert(ok == KD_RESULT_OK && "Failed to log");

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
                assert(ok == KD_RESULT_OK && "Failed to get kb desc");
                assert(kb_desc.kb_count && "No kb found");

                if(kb_desc.kb_state[0][KD_KB_ESC] & KD_KEY_UP_EVENT) {
                        ok = kd_log(KD_LOG_INFO, "Quit");
                        assert(ok == KD_RESULT_OK && "Failed to log");
                        kd_ctx_close();
                }
                else if(kb_desc.kb_state[0][KD_KB_ANY] & KD_KEY_UP_EVENT) {

                        int app_idx, app_cnt;
                        ok = kd_ctx_application_index_get(&app_idx, &app_cnt);
                        assert(ok == KD_RESULT_OK && "Failed to get index");

                        int next_idx = (app_idx + 1) % app_cnt;

                        char buf[512] = {0};
                        sprintf(
                                buf,
                                "Curr IDX(%d) of (%d), next IDX(%d)\n",
                                app_idx,
                                app_cnt,
                                next_idx);

                        ok = kd_log(KD_LOG_INFO, buf);
                        assert(ok == KD_RESULT_OK && "Failed to log");

                        ok = kd_ctx_application_index_set(next_idx);
                        assert(ok == KD_RESULT_OK && "Failed to set index");
                }
        }

        GL_ERR("GL - Common Setup")

        return events;
}


void
cmn_push_debug_group(
        const char *group_name)
{
        if(GL_DEBUG_HELPERS && glPushDebugGroup) {
                glPushDebugGroup(
                        GL_DEBUG_SOURCE_APPLICATION,
                        GL_DEBUG_TYPE_PUSH_GROUP,
                        -1,
                        group_name);
        }

}


void
cmn_pop_debug_group()
{
        if(GL_DEBUG_HELPERS && glPopDebugGroup) {
                glPopDebugGroup();
        }
}


void
cmn_label_object(
        GLenum obj,
        GLuint obj_id,
        const char *name)
{
        if(GL_DEBUG_HELPERS && glObjectLabel) {
                glObjectLabel(obj, obj_id, -1, name);
        }
}



#endif
#endif

