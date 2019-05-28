/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#if defined(_MSC_VER)
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <unabto/unabto_env_base.h>
#include <modules/log/unabto_basename.h>


#include <string.h>

/** @return the filename part (without preceeding path) of a file pathname.
 * @param  path   pathname to a file.  */
const char* unabto_basename(const char* path)
{
    const char *p;
    char ch;

    p = path + strlen(path);
    while (p > path) {
        ch = *(p - 1);
        if (ch == '/' || ch == '\\' || ch == ':')
            break;
        --p;
    }
    return p;
}
