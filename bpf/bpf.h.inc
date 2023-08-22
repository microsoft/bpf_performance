// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#pragma once

#cmakedefine PLATFORM_WINDOWS
#cmakedefine PLATFORM_LINUX
#cmakedefine PLATFORM_MACOS

#if defined(PLATFORM_WINDOWS)
#include <bpf_helpers.h>
#include <bpf_endian.h>
#define BPF_F_NO_PREALLOC 0
#elif defined(PLATFORM_LINUX)
#include <linux/bpf.h>
#include <linux/types.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#undef bpf_printk
#define bpf_printk(fmt, ...)
#endif
