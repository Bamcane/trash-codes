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

#define LIBOO_ARG_N(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, N, ...) N
#define LIBOO_NARGS_(...) LIBOO_ARG_N(__VA_ARGS__)
#define LIBOO_NARGS(...) LIBOO_NARGS_(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define LIBOO_IMPL_FUNCTION(...) LIBOO_NARGS_(__VA_ARGS__, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, default, 0)

#define LIBOO_CONCAT_IMPL(x, y) x##y
#define LIBOO_CONCAT(x, y) LIBOO_CONCAT_IMPL(x, y)

// so that we can highlight the method in the IDE.
#define __new_impl(ClassName, method, ...) method(__VA_ARGS__)
#define __new_default(ClassName) __new_impl(ClassName, ClassName##_new_default)

#define class(ClassName) \
    typedef struct ClassName ClassName; \
    struct ClassName
#define new(...) LIBOO_CONCAT(__new_, LIBOO_IMPL_FUNCTION(__VA_ARGS__))(__VA_ARGS__)
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

#define __ClassInit_impl(ClassName, suffix, ...) \
    ClassName* ClassName##_new_##suffix(__VA_ARGS__) { \
            ClassName* this = malloc(sizeof(ClassName)); \
            if (!this) return NULL; \
            do

#define __ClassInit_default(ClassName) __ClassInit_impl(ClassName, default)

#define ClassInit(...) LIBOO_CONCAT(__ClassInit_, LIBOO_IMPL_FUNCTION(__VA_ARGS__))(__VA_ARGS__)

#define ClassBase(ClassName) ClassName classbase

#define EndClassInit() \
        while(0); \
        return this; \
    }

#define LIBOO_METHOD_FUNCTION(...) LIBOO_NARGS_(__VA_ARGS__, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, default, default, 0)

#define __ClassMethod_impl(type, method, ...) \
    type##_##method(type *this, __VA_ARGS__)

#define __ClassMethod_default(type, method, ...) \
    type##_##method(type *this)

#define ClassMethod(...) LIBOO_CONCAT(__ClassMethod_, LIBOO_METHOD_FUNCTION(__VA_ARGS__))(__VA_ARGS__)

// so that we can highlight the method in the IDE.
#define __CallMethod_impl(object, method, ...) method(object, __VA_ARGS__)
#define __CallMethod_default(object, method, ...) method(object)

#define LIBOO_CALL_METHOD_FUNCTION(...) LIBOO_NARGS_(__VA_ARGS__, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, impl, default, default, default, 0)

#define CallMethod(...) LIBOO_CONCAT(__CallMethod_, LIBOO_CALL_METHOD_FUNCTION(__VA_ARGS__))(__VA_ARGS__) \

#endif
#endif // __LIBOO_H__