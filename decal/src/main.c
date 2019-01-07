#include <karbon/drive.h>
#include <karbon/app.h>
#include <karbon/math.h>
#include <stb_image.h>
#include <GL/gl3w.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#define COMMON_IMPL
#include <common.h>


/* ---------------------------------------------------------------- Config -- */


#define DECAL_TEST_SHOW_G_BUFFER 0



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


GLuint
gl_create_program(const char *vs_src, const char *fs_src) {
        GLuint vs_shd = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs_shd, 1, &vs_src, NULL);
        glCompileShader(vs_shd);
        GLint vs_status;
        glGetShaderiv(vs_shd, GL_COMPILE_STATUS, &vs_status);
        if(vs_status == GL_FALSE) {
                char buffer[1024];
                glGetShaderInfoLog(vs_shd, sizeof(buffer), NULL, buffer);
                kd_log(KD_LOG_FATAL, buffer);
                assert(!"Failed to build vs shd");
        }

        GLuint fs_shd = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs_shd, 1, &fs_src, NULL);
        glCompileShader(fs_shd);
        GLint fs_status;
        glGetShaderiv(fs_shd, GL_COMPILE_STATUS, &fs_status);
        if(fs_status == GL_FALSE) {
                char buffer[1024];
                glGetShaderInfoLog(vs_shd, sizeof(buffer), NULL, buffer);
                kd_log(KD_LOG_FATAL, buffer);
                assert(!"Failed to build vs shd");
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
                kd_log(KD_LOG_FATAL, buffer);
                assert(!"Failed to link");
        }

        glDeleteShader(vs_shd);
        glDeleteShader(fs_shd);

        return pro;
}


void
setup()
{
        kd_log(KD_LOG_INFO, "Decal Startup");

        memset(&glctx, 0, sizeof(glctx));
        cmn_setup();

        cmn_push_debug_group("Decal Setup");

        /* vao */
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glctx.vao = vao;

        cmn_label_object(GL_VERTEX_ARRAY, vao, "Decal::VAO");

        /* g buffer */
        const char gbuf_vs[] = ""
                "#version 330\n"

                "layout(location = 0) in vec3 Position;\n"
                "layout(location = 1) in vec2 TexCoord; \n"
                "layout(location = 2) in vec3 Normal;\n"

                "uniform mat4 gWVP;\n"
                "uniform mat4 gWorld;\n"

                "out vec2 TexCoord0;\n"
                "out vec3 Normal0;\n"
                "out vec3 WorldPos0;\n"

                "void main()\n"
                "{\n"
                " gl_Position = gWVP * vec4(Position, 1.0);\n"
                " TexCoord0 = TexCoord;\n"
                " Normal0 = (gWorld * vec4(Normal, 0.0)).xyz;\n"
                " WorldPos0 = (gWorld * vec4(Position, 1.0)).xyz;\n"
                "}\n";

        const char gbuf_fs[] = ""
                "#version 330\n"

                "in vec2 TexCoord0;   \n"
                "in vec3 Normal0;   \n"
                "in vec3 WorldPos0;  \n"

                "uniform sampler2D gColorMap;\n"

                "layout (location = 0) out vec3 WorldPosOut;\n"
                "layout (location = 1) out vec3 DiffuseOut;\n"
                "layout (location = 2) out vec3 NormalOut;\n"
                "layout (location = 3) out vec3 TexCoordOut;\n"

                "void main()  \n"
                "{\n"
                " WorldPosOut     = WorldPos0;\n"
                " DiffuseOut      = vec3(1, 0.3, 0); /*texture(gColorMap, TexCoord0).xyz;*/\n"
                " NormalOut       = normalize(Normal0);\n"
                " TexCoordOut     = vec3(TexCoord0, 0.0);\n"
                "}\n";

        glctx.sce_gbuff_pro = gl_create_program(gbuf_vs, gbuf_fs);


        /* dir light */
        const char dir_vs[] = ""
                "#version 330\n"
                "layout (location = 0) in vec2 Position; \n"
                "layout (location = 1) in vec2 TexC;\n"

                "out vec2 TexCoords;\n"

                "uniform mat4 gWVP;\n"
                "void main()\n"
                "{\n"
                "TexCoords = TexC;\n"
                "gl_Position = vec4(Position, 0.0, 1.0);\n"
                "}\n";

        const char dir_fs[] = ""
                "#version 330\n"

                "struct BaseLight\n"
                "{\n"
                "vec3 Color;\n"
                "float AmbientIntensity;\n"
                "float DiffuseIntensity;\n"
                "};\n"

                "struct DirectionalLight\n"
                "{\n"
                "BaseLight Base;\n"
                "vec3 Direction;\n"
                "};\n"

                "struct Attenuation\n"
                "{\n"
                "float Constant;\n"
                "float Linear;\n"
                "float Exp;\n"
                "};\n"

                "struct PointLight\n"
                "{\n"
                "BaseLight Base;\n"
                "vec3 Position;\n"
                "Attenuation Atten;\n"
                "};\n"

                "struct SpotLight\n"
                "{\n"
                "PointLight Base;\n"
                "vec3 Direction;\n"
                "float Cutoff;\n"
                "};\n"

                "uniform sampler2D gPositionMap;\n"
                "uniform sampler2D gColorMap;\n"
                "uniform sampler2D gNormalMap;\n"

                "uniform DirectionalLight gDirectionalLight;\n"
                "uniform PointLight gPointLight;\n"
                "uniform SpotLight gSpotLight;\n"
                "uniform vec3 gEyeWorldPos;\n"
                "uniform float gMatSpecularIntensity;\n"
                "uniform float gSpecularPower;\n"
                "uniform int gLightType;\n"
                "uniform vec2 gScreenSize;\n"

                "vec4 CalcLightInternal(BaseLight Light,\n"
                "vec3 LightDirection,\n"
                "vec3 WorldPos,\n"
                "vec3 Normal)\n"
                "{\n"
                "vec4 AmbientColor = vec4(Light.Color * Light.AmbientIntensity, 1.0);\n"
                "float DiffuseFactor = dot(Normal, -LightDirection);\n"

                "vec4 DiffuseColor  = vec4(0, 0, 0, 0);\n"
                "vec4 SpecularColor = vec4(0, 0, 0, 0);\n"

                "if (DiffuseFactor > 0.0) {\n"
                "DiffuseColor = vec4(Light.Color * Light.DiffuseIntensity * DiffuseFactor, 1.0);\n"

                "vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos);\n"
                "vec3 LightReflect = normalize(reflect(LightDirection, Normal));\n"
                "float SpecularFactor = dot(VertexToEye, LightReflect); \n"
                "if (SpecularFactor > 0.0) {\n"
                "SpecularFactor = pow(SpecularFactor, gSpecularPower);\n"
                "SpecularColor = vec4(Light.Color * gMatSpecularIntensity * SpecularFactor, 1.0);\n"
                "}\n"
                "}\n"

                "return (AmbientColor + DiffuseColor + SpecularColor);\n"
                "}\n"

                "vec4 CalcDirectionalLight(vec3 WorldPos, vec3 Normal)\n"
                "{\n"
                "return CalcLightInternal(gDirectionalLight.Base,\n"
                "gDirectionalLight.Direction,\n"
                "WorldPos,\n"
                "Normal);\n"
                "}\n"

                "vec4 CalcPointLight(vec3 WorldPos, vec3 Normal)\n"
                "{\n"
                "vec3 LightDirection = WorldPos - gPointLight.Position;\n"
                "float Distance = length(LightDirection);\n"
                "LightDirection = normalize(LightDirection);\n"

                "vec4 Color = CalcLightInternal(gPointLight.Base, LightDirection, WorldPos, Normal);\n"

                "float Attenuation =  gPointLight.Atten.Constant +\n"
                "gPointLight.Atten.Linear * Distance +\n"
                "gPointLight.Atten.Exp * Distance * Distance;\n"

                "Attenuation = max(1.0, Attenuation);\n"

                "return Color / Attenuation;\n"
                "}\n"

                "vec2 CalcTexCoord()\n"
                "{\n"
                "return gl_FragCoord.xy / gScreenSize;\n"
                "}\n"

                "in vec2 TexCoords;\n"
                "out vec4 FragColor;\n"

                "void main()\n"
                "{\n"
                "vec2 TexCoord = TexCoords; //CalcTexCoord();\n"
                "vec3 WorldPos = texture(gPositionMap, TexCoord).xyz;\n"
                "vec3 Color = texture(gColorMap, TexCoord).xyz;\n"
                "vec3 Normal = texture(gNormalMap, TexCoord).xyz;\n"
                "Normal = normalize(Normal);\n"
                "FragColor = (vec4(Color, 1.0) * 0.1) + vec4(Color, 1.0) * CalcDirectionalLight(WorldPos, Normal);\n"
                        /* "FragColor = vec4(Color, 1.0);\n"*/
                "}\n";

        glctx.sce_light_pro = gl_create_program(dir_vs, dir_fs);

        /* decals */
        const char dec_vs[] = ""
                "#version 430\n"

                "layout(location=0) in vec4 vs_in_position;\n"
                "layout(location=1) in vec3 vs_in_normal;\n"
                "layout(location=2) in vec2 vs_in_texcoord;\n"

                "uniform mat4 u_view;\n"
                "uniform mat4 u_proj;\n"
                "uniform mat4 u_model;\n"
                "uniform vec3 u_decal_size;\n"
                "uniform float u_far_clip;\n"

                "out vec4 pos_fs;\n"
                "out vec4 pos_w;\n"
                "out vec2 uv_fs;\n"

                "void main()\n"
                "{\n"
                " pos_w       = u_model * vs_in_position;\n"
                " pos_fs      = u_proj * u_view * pos_w;\n"
                " uv_fs       = vs_in_texcoord;\n"
                " gl_Position = pos_fs;\n"
                "}\n";

        const char dec_fs[] = ""
                "#version 430\n"
                "#extension GL_ARB_texture_rectangle : enable\n"

                "in vec4 pos_fs;\n"
                "in vec4 pos_w;\n"
                "in vec2 uv_fs;\n"

                "uniform mat4 u_model;\n"
                "uniform mat4 u_view;\n"
                "uniform mat4 u_proj;\n"

                "uniform sampler2D samp_depth;\n"
                "uniform sampler2D samp_diffuse;\n"

                "uniform mat4 u_inv_proj_view;\n"
                "uniform mat4 u_inv_model;\n"

                "vec4 reconstruct_pos(float z, vec2 uv_f)\n"
                "{\n"
                "  vec4 sPos = vec4(uv_f * 2.0 - 1.0, -z, 1.0);\n"
                "mat4 proj_view = u_view * u_proj;"
                "sPos = inverse(proj_view) * sPos;\n"
                //"sPos = u_inv_proj_view * sPos;\n"
                "return vec4((sPos.xyz / sPos.w), sPos.w);\n"
                "}\n"

                "layout (location = 0) out vec3 fs_out_diffuse;\n"

                "void main()  \n"
                "{\n"
                "vec2 screenPosition = pos_fs.xy / pos_fs.w;\n"

                "vec2 depthUV = screenPosition * vec2(0.5, -0.5);\n"
                "depthUV += vec2(1200.0, 720.0);\n"

                "float depth = texture2D(samp_depth, depthUV).r;\n"

                "vec4 worldPos = reconstruct_pos(depth, depthUV);\n"
                "vec4 localPos = inverse(u_model) * worldPos;\n"

                //"if(0.5 - abs(length(worldPos.xyz)) < 0) { discard; }"

                //"vec4 localPos = u_inv_model * worldPos;\n"

                "float dist = 0.5 - abs(localPos.y);\n"
                "float dist2 = 0.5 - abs(localPos.x);\n"

                //Position on the screen
                "vec2 screenPos = pos_fs.xy / pos_fs.w;\n"

                //Convert into a texture coordinate
                "vec2 texCoord = vec2("
                "(1 + screenPos.x) / 2 + (0.5 / 1200.0),"
                "(1 - screenPos.y) / 2 + (0.5 / 720.0)"
                ");"

                "if (dist > 0.0f && dist2 > 0)\n"
                "{\n"
                "vec2 uv = vec2(localPos.x, localPos.y) + 0.5;\n"
                "vec4 diffuseColor = texture2D(samp_diffuse, uv);\n"
                "fs_out_diffuse = diffuseColor.xyz;\n"
                "}\n"
                        "else {\n"
                        "fs_out_diffuse = vec4(1.0, 0, 0, 1).xyz;\n"
                        "}\n"
                "}\n";

        glctx.dec_pro = gl_create_program(dec_vs, dec_fs);

        /* blit */
        const char blit_vs[] = 
                "#version 400 core\n"
                "layout(location=0) in vec3 vs_in_position;\n"
                "layout(location=1) in vec2 vs_in_texcoord;\n"
                "out vec2 fs_in_texcoord;\n"
                "void main()\n"
                "{\n"
                "  gl_Position = vec4(vs_in_position, 1.0);\n"
                "  fs_in_texcoord = vs_in_texcoord;\n"
                "}\n";

        const char blit_fs[] =
                "#version 400 core\n"
                "in vec2 fs_in_texcoord;\n"
                "uniform sampler2D samp_diffuse_01;\n"
                "out vec4 fs_out_fragcolor;\n"
                "void main()\n"
                "{\n"
                " fs_out_fragcolor = texture(samp_diffuse_01, fs_in_texcoord);\n"
                "}\n";

        glctx.fsb_pro = gl_create_program(blit_vs, blit_fs);

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
