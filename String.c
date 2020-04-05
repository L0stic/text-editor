#include "String.h"

#define DIV_WITH_ROUND_UP(op1, op2) (op1 / op2 + ((op1 % op2) ? 1 : 0))

static int ResizeString(String* str) {
    assert(str->len >= str->size);

    char* tmpData = NULL;

    str->size = DIV_WITH_ROUND_UP(str->len, BASE_STRING_SIZE);
    str->size *= BASE_STRING_SIZE;

    tmpData = realloc(str->data, str->size * sizeof(char));

    if (tmpData) {
        str->data = tmpData;
        return 0;
    }
    return -1;
}

int ReserveSize(String* str, size_t size) {
    assert(str && !str->data && size > 0);

    str->size = DIV_WITH_ROUND_UP(size, BASE_STRING_SIZE);
    str->size *= BASE_STRING_SIZE;

    str->data = malloc(sizeof(char) * str->size);
    if (!str->data) { return -1; }

    return 0;
}

static int SetString(String* str, const char* src) {
    assert(str && src);

    str->len = strlen(src);
    str->size = DIV_WITH_ROUND_UP(str->len, BASE_STRING_SIZE);
    str->size *= BASE_STRING_SIZE;

    str->data = malloc(str->size * sizeof(char));

    if (str->data) {
        strncpy(str->data, src, str->len);
        return 0;
    }
    return -1;
}

// обработать ошибки
String* CreateString(const char* src) {
    String* str = calloc(1, sizeof(String));
    if (!str) { return NULL; }

    if (src) { SetString(str, src); }

    return str;
}

void DestroyString(String** ppStr) {
    assert(ppStr && *ppStr);

    if ((*ppStr)->data) { free((*ppStr)->data); }

    free(*ppStr);
    *ppStr = NULL;
}

int AddString(String* str, const char* src) {
    assert(str && src);

    if (str->data) {
        size_t oldLen = str->len;

        str->len += strlen(src);

        if (oldLen < str->len) {
            if (str->len > str->size && ResizeString(str)) { return -1; }

            strncpy(str->data + oldLen, src, str->len - oldLen);
        }
        return 0;
    }

    return SetString(str, src);
}

size_t PrintString(const String* str) {
    assert(str && str->data);

    for (size_t i = 0; i < str->len; ++i) {
        fputchar(str->data[i]);
    }

    return str->len;
}
