/*
 * Copyright © 2018, VideoLAN and dav1d authors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Windows pthread compatibility layer
 * Minimal implementation for dav1d decoder threading support
 */

#include "config.h"

#include <windows.h>
#include <process.h>
#include <errno.h>

#include "common/attributes.h"

typedef struct {
    void *(*func)(void *);
    void *arg;
} dav1d_thread_wrapper_arg;

static unsigned __stdcall dav1d_thread_wrapper(void *arg) {
    dav1d_thread_wrapper_arg *t = arg;
    void *(*func)(void *) = t->func;
    void *func_arg = t->arg;

    free(t);
    func(func_arg);
    return 0;
}

int dav1d_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                        void *(*func)(void *), void *arg)
{
    dav1d_thread_wrapper_arg *wrapper_arg = malloc(sizeof(dav1d_thread_wrapper_arg));
    if (!wrapper_arg)
        return ENOMEM;

    wrapper_arg->func = func;
    wrapper_arg->arg = arg;

    *thread = (pthread_t) _beginthreadex(NULL, 0, dav1d_thread_wrapper,
                                         wrapper_arg, 0, NULL);
    if (!*thread) {
        free(wrapper_arg);
        return errno;
    }

    return 0;
}

int dav1d_pthread_join(pthread_t thread, void **res) {
    DWORD ret = WaitForSingleObject((HANDLE) thread, INFINITE);
    if (ret != WAIT_OBJECT_0) {
        if (ret == WAIT_FAILED)
            return EINVAL;
        else
            return EDEADLK;
    }
    CloseHandle((HANDLE) thread);
    if (res)
        *res = NULL;
    return 0;
}

int dav1d_pthread_once(pthread_once_t *once_control,
                      void (*init_routine)(void))
{
    BOOL pending = FALSE;
    if (!InitOnceBeginInitialize(once_control, 0, &pending, NULL))
        return EINVAL;

    if (pending)
        init_routine();

    InitOnceComplete(once_control, 0, NULL);
    return 0;
}
