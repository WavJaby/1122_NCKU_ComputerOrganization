#include <stdlib.h>
#include <stdio.h>

typedef struct StringStream {
    char* data;
    size_t offset;
    size_t len;
} StringStream;

#define STRING_STREAM_BUFF_SIZE 64

#define ssprintf(strStream, ...)                                                              \
    {                                                                                         \
        if ((strStream)->len - (strStream)->offset < STRING_STREAM_BUFF_SIZE)                 \
            (strStream)->data = realloc((strStream)->data, (strStream)->len += 100);          \
        (strStream)->offset += sprintf((strStream)->data + (strStream)->offset, __VA_ARGS__); \
    }

StringStream* newStreamStream() {
    StringStream* ss = malloc(sizeof(StringStream));
    ss->len = STRING_STREAM_BUFF_SIZE;
    ss->data = malloc(ss->len);
    ss->offset = 0;
    return ss;
}

void freeStreamStream(StringStream* ss) {
    free(ss->data);
    free(ss);
}