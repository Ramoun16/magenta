// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <magenta/compiler.h>
#include <magenta/types.h>
#include <runtime/mutex.h>
#include <stddef.h>
#include <stdint.h>

__BEGIN_CDECLS

typedef void (*mxr_thread_entry_t)(void*);

typedef struct {
    mx_handle_t handle;
    mxr_thread_entry_t entry;
    void* arg;

    uint64_t magic;

    mxr_mutex_t state_lock;
    int state;
} mxr_thread_t;

#pragma GCC visibility push(hidden)

// TODO(kulakowski) Document the possible mx_status_t values from these.

// Create a thread, filling in the given mxr_thread_t to describe it.
// The return value is that of mx_thread_create.
// On failure, the mxr_thread_t is clobbered and cannot be passed to
// any functions except mxr_thread_create or mxr_thread_adopt.
mx_status_t mxr_thread_create(mx_handle_t proc_self, const char* name,
                              mxr_thread_t* thread);

// Fill in the given mxr_thread_t to describe a thread given its handle.
// This takes ownership of the given thread handle.
mx_status_t mxr_thread_adopt(mx_handle_t handle, mxr_thread_t* thread);

// Start the thread with the given stack, entrypoint, and
// argument. stack_addr is taken to be the low address of the stack
// mapping, and should be page aligned. The size of the stack should
// be a multiple of PAGE_SIZE. When started, the thread will call
// entry(arg).
mx_status_t mxr_thread_start(mxr_thread_t* thread, uintptr_t stack_addr, size_t stack_size, mxr_thread_entry_t entry, void* arg);

// Once started, threads can be either joined or detached. If a thread
// is joined or detached more than once, ERR_INVALID_ARGS is
// returned. Some of the resources allocated to a thread are not
// collected until it returns and it is either joined or detached.

// If a thread is joined, the caller of mxr_thread_join blocks until
// the other thread is finished running.
mx_status_t mxr_thread_join(mxr_thread_t* thread);

// If a thread is detached, instead of waiting to be joined, it will
// clean up after itself, and the return value of the thread's
// entrypoint is ignored.
mx_status_t mxr_thread_detach(mxr_thread_t* thread);

// Exit from the thread. Equivalent to returning from that thread's
// entrypoint.
_Noreturn void mxr_thread_exit(mxr_thread_t* thread);

// Destroy a created but unstarted thread structure.
// This returns failure if the thread's handle was invalid.
// Regardless, the mxr_thread_t is destroyed.
mx_status_t mxr_thread_destroy(mxr_thread_t* thread);

// Get the mx_handle_t corresponding to the given thread.
// WARNING:
// This is intended for debuggers and so on. Holding this wrong could
// break internal invariants of mxr_thread_t. It is inherently
// racy. You probably don't need this.
mx_handle_t mxr_thread_get_handle(mxr_thread_t* thread);

#pragma GCC visibility pop

__END_CDECLS
