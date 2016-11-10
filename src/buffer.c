#include "buffer.h"
#include "stralloc.h"
#include "svalue.h"
#include "pike_error.h"
#include "bignum.h"

PMOD_EXPORT void buffer_free(struct byte_buffer *b) {
    if (!b->dst) return;
    free(buffer_ptr(b));
    b->length = 0;
    b->left = 0;
    b->dst = NULL;
}

PMOD_EXPORT int buffer_make_space_nothrow(struct byte_buffer *b, size_t len) {
    size_t new_length = b->length;
    const size_t content_length = buffer_content_length(b);
    size_t needed = content_length;
    void *ptr;

#ifdef PIKE_DEBUG
    if (b->left >= len) Pike_fatal("Misuse of buffer_make_space.\n");
#endif
    if (DO_SIZE_T_ADD_OVERFLOW(len, content_length, &needed)) goto OUT_OF_MEMORY_FATAL;

    if (new_length < 16) new_length = 16;

    while (new_length < needed) {
        if (DO_SIZE_T_MUL_OVERFLOW(new_length, 2, &new_length)) goto OUT_OF_MEMORY_FATAL;
    }

    /* fprintf(stderr, "BUF { left=%llu; length=%llu; dst=%p; }\n", b->left, b->length, b->dst); */

    ptr = realloc(buffer_ptr(b), new_length);

    if (UNLIKELY(!ptr)) return 0;

    /* fprintf(stderr, "realloc(%p, %llu) = %p\n", buffer_ptr(b), new_length, ptr); */
    /* fprintf(stderr, "content_length: %llu\n", content_length); */

    b->dst = (char*)ptr + content_length;
    b->length = new_length;
    b->left = new_length - content_length;
    return 1;
OUT_OF_MEMORY_FATAL:
    Pike_fatal(msg_out_of_mem_2, len);
}

PMOD_EXPORT void buffer_make_space(struct byte_buffer *b, size_t len) {
    if (UNLIKELY(!buffer_make_space_nothrow(b, len))) Pike_error(msg_out_of_mem_2, len);
}

PMOD_EXPORT void* buffer_finish(struct byte_buffer *b) {
    void *ret = buffer_ptr(b);

    if (buffer_content_length(b) < buffer_length(b)) {
        ret = realloc(ret, buffer_content_length(b));
        /* we cannot shrink, that does not matter */
        if (!ret) ret = buffer_ptr(b);
    }

    b->left = 0;
    b->length = 0;
    b->dst = NULL;

    return ret;
}

PMOD_EXPORT struct pike_string *buffer_finish_pike_string(struct byte_buffer *b) {
    struct pike_string *ret;
    size_t len = buffer_content_length(b);

    if (!len) {
        buffer_free(b);
        ret = empty_pike_string;
        add_ref(ret);
        return ret;
    }

    return make_shared_malloc_string(buffer_finish(b), len, eightbit);
}

