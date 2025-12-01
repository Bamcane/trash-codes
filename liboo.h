#ifndef __LIBOO_H__
#define __LIBOO_H__
#ifndef __cplusplus

#include <stdbool.h>

typedef enum
{
    LIBOOERROR_NONE = 0,
    LIBOOERROR_DOUBLE_FREE
}LibOOError;

typedef void (*LIBOOERROR_CALLBACK)(const char *pErrorString);

static LIBOOERROR_CALLBACK g_pfnLibOOErrorCallback = 0;

void SetLibOOErrorCallBack(LIBOOERROR_CALLBACK pfnCallback)
{
    if(pfnCallback)
        g_pfnLibOOErrorCallback = pfnCallback;
}

const char *GetLibOOErrorString(LibOOError Error)
{
    switch (Error)
    {
        case LIBOOERROR_DOUBLE_FREE: return "double free";
        default: return "";
    }
}

void LibOO_CallError(LibOOError Error)
{
    if(g_pfnLibOOErrorCallback)
        g_pfnLibOOErrorCallback(GetLibOOErrorString(Error));
}

#define class(ClassName) \
    typedef struct ClassName ClassName; \
    struct ClassName
#define new_ex(ClassName, suffix, ...) (ClassName##_new_##suffix(__VA_ARGS__))
#define new(ClassName) new_ex(ClassName, default)
#define delete(pObject) \
    do \
    { \
        if(pObject) \
        { \
            free(pObject); \
            pObject = 0; \
        } \
        else LibOO_CallError(LIBOOERROR_DOUBLE_FREE); \
    }while(0)

#define ClassInitEx(type, params, suffix) \
    static inline type* type##_new_##suffix params { \
        type* this = malloc(sizeof(type)); \
        if (!this) return NULL; \
        do

#define ClassInit(type) ClassInitEx(type, (void), default)

#define EndClassInit() \
        while(0); \
        return this; \
    }

#define ClassFunctionEx(type, method, params) \
    type (*method)(type *this, params)
#define ClassFunction(type, method) \
    type (*method)(type *this)

#define InitFunction(type, method) \
    this->method = type##_##method

#define NewFunctionEx(type, method, ...) \
    type##_##method(type *this, __VA_ARGS__)
#define NewFunction(type, method) \
    type##_##method(type *this)

#define CallFunctionEx(object, type, method, ...) \
    type##_##method(object, __VA_ARGS__)
#define CallFunction(object, type, method) \
    type##_##method(object)

#endif
#endif // __LIBOO_H__