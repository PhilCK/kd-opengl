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
        GLuint sce_tex1;
        GLuint sce_gbuff_pro, sce_light_pro;
        GLuint sce_fbo;
        GLuint sce_fbo_out[5];
        GLuint sce_fbo_depth;

        /* Render Decal */
        GLuint dec_vbo;
        GLuint dec_pro;
        GLuint dec_tex1;

        /* Fullscreen Blit */
        GLuint fsb_vbo;
        GLuint fsb_pro;

        float time;
} glctx;


GLuint
gl_create_program(const char * name, const char *vs_src, const char *fs_src) {
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

                return 0;
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

                return 0;
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

                return 0;
        }

        char msg[1024] = {0};
        sprintf(msg, "Loaded program %s", name);
        kd_log(KD_LOG_INFO, msg);

        cmn_label_object(GL_PROGRAM, pro, name);

        glDeleteShader(vs_shd);
        glDeleteShader(fs_shd);

        return pro;
}


GLuint
gl_create_texture(const char *file) {
        int w, h, c;
        char t1[2048] = {0};
        kd_ctx_get_exe_dir(t1, 0);
        strcat(t1, "assets/");
        strcat(t1, file);
        GLuint tex = 0;

        unsigned char* img = stbi_load(&t1[0], &w, &h, &c, 0);

        if(!img) {
                char msg[2048] = {0};
                sprintf(msg, "Failed to load %s", t1);
                kd_log(KD_LOG_WARNING, msg);
                assert(!"Failed to load texture!");
                return 0;
        }

        char buf[2048] = {0};
        sprintf(buf, "Loaded image: %s %dx%d:%d", file, w, h, c);
        kd_log(KD_LOG_INFO, buf);

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGB,
                w,
                h,
                0,
                c == 4 ? GL_RGBA : GL_RGB,
                GL_UNSIGNED_BYTE,
                img);

        stbi_image_free(img);

        char tag[512] = {0};
        sprintf(tag, "%s - %dx%d:%d", file, w, h, c);
        cmn_label_object(GL_TEXTURE, tex, tag);

        return tex;
}


void
setup()
{
        kd_log(KD_LOG_INFO, "Decal Startup");
        int i;

        struct kd_window_desc win_desc;
        win_desc.type_id = KD_STRUCT_WINDOW_DESC;
        kd_window_get(&win_desc);

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

        glctx.sce_gbuff_pro = gl_create_program("Fill GBuf", gbuf_vs, gbuf_fs);


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

        glctx.sce_light_pro = gl_create_program("Dir Light", dir_vs, dir_fs);
        glctx.sce_tex1 = gl_create_texture("sample.png");

        GL_ERR("GBuf rsrc");

        const char *gnames[] = {
                "GBuf:Positions",
                "GBuf:Diffuse",
                "GBuf:Normals",
                "GBuf:TexC",
                "GBuf:Final",
        };

        int count = sizeof(glctx.sce_fbo_out) / sizeof(glctx.sce_fbo_out[0]);

        glGenTextures(count, glctx.sce_fbo_out);

        for(i = 0; i < count; ++i) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, glctx.sce_fbo_out[i]);
                glTexImage2D(
                        GL_TEXTURE_2D,
                        0,
                        GL_RGBA32F,
                        win_desc.width,
                        win_desc.height,
                        0,
                        GL_RGBA,
                        GL_FLOAT,
                        0);

                glTexParameteri(
                        GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_S,
                        GL_CLAMP_TO_EDGE);

                glTexParameteri(
                        GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_T,
                        GL_CLAMP_TO_EDGE);

                glTexParameteri(
                        GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR);

                glTexParameteri(
                        GL_TEXTURE_2D,
                        GL_TEXTURE_MAG_FILTER,
                        GL_LINEAR);

                cmn_label_object(GL_TEXTURE, glctx.sce_fbo_out[i], gnames[i]);
        }

        glGenTextures(1, &glctx.sce_fbo_depth);
        glBindTexture(GL_TEXTURE_2D, glctx.sce_fbo_depth);

        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_DEPTH_COMPONENT32F,
                win_desc.width,
                win_desc.height,
                0,
                GL_DEPTH_COMPONENT,
                GL_FLOAT,
                0);

        glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_WRAP_S,
                GL_CLAMP_TO_EDGE);

        glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_WRAP_T,
                GL_CLAMP_TO_EDGE);

        glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_MIN_FILTER,
                GL_LINEAR);

        glTexParameteri(
                GL_TEXTURE_2D,
                GL_TEXTURE_MAG_FILTER,
                GL_LINEAR);

        cmn_label_object(GL_TEXTURE, glctx.sce_fbo_depth, "GBuf:Depth");

        GL_ERR("Create FBO Targets");

        glGenFramebuffers(1, &glctx.sce_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, glctx.sce_fbo);

        GLenum dbufs[5];
        assert(count == 5);

        for(i = 0; i < count; ++i) {
                glFramebufferTexture2D(
                        GL_FRAMEBUFFER,
                        GL_COLOR_ATTACHMENT0 + i,
                        GL_TEXTURE_2D,
                        glctx.sce_fbo_out[i],
                        0);

                dbufs[i] = GL_COLOR_ATTACHMENT0 + i;
        }

        glDrawBuffers(count, dbufs);

        glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT,
                GL_TEXTURE_2D,
                glctx.sce_fbo_depth,
                0);

        GL_ERR("Marco");

        GLenum state = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        assert(state == GL_FRAMEBUFFER_COMPLETE);

        cmn_label_object(GL_FRAMEBUFFER, glctx.sce_fbo, "GBuf:FBO");

        GL_ERR("Fbo create");

        GLfloat cube[] = {
                /* pos v3 - tex v2 - norma v3 */
                +1.f, -1.f, +1.f, /**/ 1.f, 0.f, /**/ +0.f, -1.f, +0.f,
                -1.f, -1.f, +1.f, /**/ 1.f, 1.f, /**/ +0.f, -1.f, +0.f,
                -1.f, -1.f, -1.f, /**/ 0.f, 0.f, /**/ +0.f, -1.f, +0.f,
        
                -1.f, +1.f, -1.f, /**/ 0.f, 0.f, /**/ +0.f, +1.f, +0.f,
                -1.f, +1.f, +1.f, /**/ 1.f, 1.f, /**/ +0.f, +1.f, +0.f,
                +1.f, +1.f, +1.f, /**/ 0.f, 1.f, /**/ +0.f, +1.f, +0.f,

                +1.f, +1.f, -1.f, /**/ 1.f, 0.f, /**/ +1.f, +0.f, +0.f,
                +1.f, +1.f, +1.f, /**/ 1.f, 1.f, /**/ +1.f, +0.f, +0.f,
                +1.f, -1.f, -1.f, /**/ 0.f, 1.f, /**/ +1.f, +0.f, +0.f,

                +1.f, +1.f, +1.f, /**/ 1.f, 0.f, /**/ +0.f, +0.f, +1.f,
                -1.f, +1.f, +1.f, /**/ 1.f, 1.f, /**/ +0.f, +0.f, +1.f,
                -1.f, -1.f, +1.f, /**/ 0.f, 1.f, /**/ +0.f, +0.f, +1.f,

                -1.f, -1.f, +1.f, /**/ 0.f, 0.f, /**/ -1.f, +0.f, +0.f,
                -1.f, +1.f, +1.f, /**/ 1.f, 0.f, /**/ -1.f, +0.f, +0.f,
                -1.f, +1.f, -1.f, /**/ 1.f, 1.f, /**/ -1.f, +0.f, +0.f,

                +1.f, -1.f, -1.f, /**/ 1.f, 0.f, /**/ +0.f, +0.f, -1.f,
                -1.f, -1.f, -1.f, /**/ 1.f, 1.f, /**/ +0.f, +0.f, -1.f,
                -1.f, +1.f, -1.f, /**/ 0.f, 1.f, /**/ +0.f, +0.f, -1.f,

                +1.f, -1.f, -1.f, /**/ 0.f, 0.f, /**/ +0.f, -1.f, +0.f,
                +1.f, -1.f, +1.f, /**/ 1.f, 0.f, /**/ +0.f, -1.f, +0.f,
                -1.f, -1.f, -1.f, /**/ 0.f, 1.f, /**/ +0.f, -1.f, +0.f,

                +1.f, +1.f, -1.f, /**/ 0.f, 0.f, /**/ +0.f, +1.f, +0.f,
                -1.f, +1.f, -1.f, /**/ 1.f, 0.f, /**/ +0.f, +1.f, +0.f,
                +1.f, +1.f, +1.f, /**/ 0.f, 1.f, /**/ +0.f, +1.f, +0.f,

                +1.f, -1.f, -1.f, /**/ 0.f, 0.f, /**/ +1.f, +0.f, +0.f,
                +1.f, +1.f, -1.f, /**/ 1.f, 0.f, /**/ +1.f, +0.f, +0.f,
                +1.f, -1.f, +1.f, /**/ 0.f, 1.f, /**/ +1.f, +0.f, +0.f,

                +1.f, -1.f, +1.f, /**/ 0.f, 0.f, /**/ +0.f, +0.f, +1.f,
                +1.f, +1.f, +1.f, /**/ 1.f, 0.f, /**/ +0.f, +0.f, +1.f,
                -1.f, -1.f, +1.f, /**/ 0.f, 1.f, /**/ +0.f, +0.f, +1.f,

                -1.f, -1.f, -1.f, /**/ 0.f, 1.f, /**/ -1.f, +0.f, +0.f,
                -1.f, -1.f, +1.f, /**/ 0.f, 0.f, /**/ -1.f, +0.f, +0.f,
                -1.f, +1.f, -1.f, /**/ 1.f, 1.f, /**/ -1.f, +0.f, +0.f,

                +1.f, +1.f, -1.f, /**/ 0.f, 0.f, /**/ +0.f, +0.f, -1.f,
                +1.f, -1.f, -1.f, /**/ 1.f, 0.f, /**/ +0.f, +0.f, -1.f,
                -1.f, +1.f, -1.f, /**/ 0.f, 1.f, /**/ +0.f, +0.f, -1.f,
        };

        glGenBuffers(1, &glctx.sce_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, glctx.sce_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

        cmn_label_object(GL_BUFFER, glctx.sce_vbo, "Scene Cube");

        GL_ERR("Create Scene VBO");

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

        glctx.dec_pro = gl_create_program("Decal", dec_vs, dec_fs);
        glctx.dec_tex1 = gl_create_texture("sample2.png");

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

        glctx.fsb_pro = gl_create_program("Blit", blit_vs, blit_fs);

        float fs_tri[] = {
                /* x,y,z - s,t */
                -1.f, +3.f, 0.f, 0.f, 2.f,
                -1.f, -1.f, 0.f, 0.f, 0.f,
                +3.f, -1.f, 0.f, 2.f, 0.f,
        };

        glGenBuffers(1, &glctx.fsb_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, glctx.fsb_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fs_tri), fs_tri, GL_STATIC_DRAW);

        cmn_label_object(GL_BUFFER, glctx.fsb_vbo, "FSTri:f3f2");

        cmn_pop_debug_group();
}


void
shutdown()
{
          /* todo cleanup */
}


void
render(int steps) {
        kd_result ok = KD_RESULT_OK;

        struct kd_window_desc win_desc;
        win_desc.type_id = KD_STRUCT_WINDOW_DESC;
        ok = kd_window_get(&win_desc);

        float world[16];
        ok = kdm_mat4_id(world);

        float view[16];
        float at[] = {0,0,0};
        float up[] = {0,1,0};
        float eye[] = {10, 10, 10};
        ok = kdm_mat4_lookat(eye, at, up, view);

        float proj[16];
        float ratio = (float)(win_desc.width) / (float)win_desc.height;
        float fov = KDM_TAU * 0.125f;
        kdm_mat4_perspective_projection(ratio, 0.1f, 100.f, fov, proj);

        float view_proj[16];
        kdm_mat4_multiply(view, proj, view_proj);

        float wvp[16];
        kdm_mat4_multiply(world, view_proj, wvp);

        /* setup */
        glBindVertexArray(glctx.vao);

        /* fill gbuffer */
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, glctx.sce_fbo);
        glDrawBuffers(5, glctx.sce_fbo_out);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(glctx.sce_gbuff_pro);
        glBindBuffer(GL_ARRAY_BUFFER, glctx.sce_vbo);

        GLsizei jmp = 8 * sizeof(GLfloat);
        void *off = 0;

        GLint pos_idx = glGetAttribLocation(glctx.sce_gbuff_pro, "Position");
        if(pos_idx > -1) {
                glEnableVertexAttribArray(pos_idx);
                glVertexAttribPointer(pos_idx, 3, GL_FLOAT, GL_FALSE, jmp, off);
        }

        off = (void*)(3 * sizeof(GLfloat));




        /* decal renderpass */

        /* dir light renderpass */

        /* final renderpass */
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
