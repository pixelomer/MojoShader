/**
* MojoShader; generate shader programs from bytecode of compiled
 *  Direct3D shaders.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 */

#define __MOJOSHADER_INTERNAL__ 1
#include "mojoshader_internal.h"

struct MOJOSHADER_headlessContext
{
    const char* profile;

    MOJOSHADER_malloc malloc_fn;
    MOJOSHADER_free free_fn;
    void *malloc_data;
};

struct MOJOSHADER_headlessShaderData
{
    const MOJOSHADER_parseData *parseData;
    uint16_t tag;
    uint32_t refcount;
    uint32_t samplerSlots;
    int32_t uniformBufferSize;
};

/* Error state... */

static char error_buffer[1024] = { '\0' };

static void set_error(const char *str)
{
    snprintf(error_buffer, sizeof (error_buffer), "%s", str);
} // set_error

static inline void out_of_memory(void)
{
    set_error("out of memory");
} // out_of_memory

/* Internals */

/* Public API */

MOJOSHADER_headlessContext *MOJOSHADER_headlessCreateContext(
    MOJOSHADER_malloc m,
    MOJOSHADER_free f,
    void *malloc_d
) {
    MOJOSHADER_headlessContext* resultCtx;

    if (m == NULL) m = MOJOSHADER_internal_malloc;
    if (f == NULL) f = MOJOSHADER_internal_free;

    resultCtx = (MOJOSHADER_headlessContext*) m(sizeof(MOJOSHADER_headlessContext), malloc_d);
    if (resultCtx == NULL)
    {
        out_of_memory();
        goto init_fail;
    } // if

    memset(resultCtx, '\0', sizeof(MOJOSHADER_headlessContext));

#ifdef __APPLE__
    resultCtx->profile = "metal";
#else
    resultCtx->profile = "spirv";
#endif

    resultCtx->malloc_fn = m;
    resultCtx->free_fn = f;
    resultCtx->malloc_data = malloc_d;

    return resultCtx;

    init_fail:
        if (resultCtx != NULL)
            f(resultCtx, malloc_d);
    return NULL;
} // MOJOSHADER_headlessCreateContext

void MOJOSHADER_headlessDestroyContext(
    MOJOSHADER_headlessContext *ctx
) {
    ctx->free_fn(ctx, ctx->malloc_data);
} // MOJOSHADER_sdlDestroyContext

static uint16_t shaderTagCounter = 1;

MOJOSHADER_headlessShaderData *MOJOSHADER_headlessCompileShader(
    MOJOSHADER_headlessContext *ctx,
    const char *mainfn,
    const unsigned char *tokenbuf,
    const unsigned int bufsize,
    const MOJOSHADER_swizzle *swiz,
    const unsigned int swizcount,
    const MOJOSHADER_samplerMap *smap,
    const unsigned int smapcount
) {
    MOJOSHADER_headlessShaderData *shader = NULL;
    int maxSamplerIndex = 0;
    int i;

    const MOJOSHADER_parseData *pd = MOJOSHADER_parse(
        ctx->profile, mainfn,
        tokenbuf, bufsize,
        swiz, swizcount,
        smap, smapcount,
        ctx->malloc_fn,
        ctx->free_fn,
        ctx->malloc_data
    );

    if (pd->error_count > 0)
    {
        set_error(pd->errors[0].error);
        goto parse_shader_fail;
    } // if

    shader = (MOJOSHADER_headlessShaderData*) ctx->malloc_fn(sizeof(MOJOSHADER_headlessShaderData), ctx->malloc_data);
    if (shader == NULL)
    {
        out_of_memory();
        goto parse_shader_fail;
    } // if

    shader->parseData = pd;
    shader->refcount = 1;
    shader->tag = shaderTagCounter++;

    /* XNA allows empty shader slots in the middle, so we have to find the actual max binding index */
    for (i = 0; i < pd->sampler_count; i += 1)
    {
        if (pd->samplers[i].index > maxSamplerIndex)
        {
            maxSamplerIndex = pd->samplers[i].index;
        }
    }

    shader->samplerSlots = (uint32_t) maxSamplerIndex + 1;

    shader->uniformBufferSize = 0;
    for (i = 0; i < pd->uniform_count; i++)
    {
        shader->uniformBufferSize += Max(pd->uniforms[i].array_count, 1);
    } // for
    shader->uniformBufferSize *= 16; // Yes, even the bool registers are this size

    return shader;

    parse_shader_fail:
        MOJOSHADER_freeParseData(pd);
    if (shader != NULL)
        ctx->free_fn(shader, ctx->malloc_data);
    return NULL;
} // MOJOSHADER_headlessCompileShader

const MOJOSHADER_parseData *MOJOSHADER_headlessGetShaderParseData(
    MOJOSHADER_headlessShaderData *shader
) {
    return (shader != NULL) ? shader->parseData : NULL;
} // MOJOSHADER_headlessGetShaderParseData