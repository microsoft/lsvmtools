/*
**==============================================================================
**
** LSVMTools 
** 
** MIT License
** 
** Copyright (c) Microsoft Corporation. All rights reserved.
** 
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
** 
** The above copyright notice and this permission notice shall be included in 
** all copies or substantial portions of the Software.
** 
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE
**
**==============================================================================
*/
#ifndef _log_h
#define _log_h

#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/inline.h>

#define LOGF(...) PutLog(FATAL, __VA_ARGS__)
#define LOGE(...) PutLog(ERROR, __VA_ARGS__)
#define LOGW(...) PutLog(WARNING, __VA_ARGS__)
#define LOGI(...) PutLog(INFO, __VA_ARGS__)
#define LOGD(...) PutLog(DEBUG, __VA_ARGS__)

typedef enum _LogLevel
{
    FATAL,
    ERROR,
    WARNING,
    INFO,
    DEBUG
}
LogLevel;

void SetLogLevel(
    LogLevel logLevel);

LogLevel GetLogLevel(void);

EFI_STATUS TruncLog(void);

EFI_STATUS PutLog(
    LogLevel logLevel,
    const CHAR16* format, 
    ...);

INLINE const char* LogLevelToStr(LogLevel logLevel)
{
    extern const char* __logLevelStrings[];
    return __logLevelStrings[(UINT32)logLevel];
}

int StrToLogLevel(
    const char* str, 
    LogLevel* logLevel);

#endif /* _log_h */
