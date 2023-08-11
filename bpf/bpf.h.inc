// Copyright (c) Microsoft Corporation
// SPDX-License-Identifier: MIT

#pragma once

#cmakedefine PLATFORM_WINDOWS
#cmakedefine PLATFORM_LINUX
#cmakedefine PLATFORM_MACOS

#if defined(PLATFORM_WINDOWS)
#include <bpf_helpers.h>
#elif defined(PLATFORM_LINUX)
#include <linux/bpf.h>
#include <linux/types.h>
#include <bpf/bpf_helpers.h>
#endif
