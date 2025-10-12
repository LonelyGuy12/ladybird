/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

struct _object;
typedef struct _object PyObject;

// Forward declarations for Python objects
struct _ts;  // Thread state
typedef struct _ts PyThreadState;

struct _is;  // Interpreter state  
typedef struct _is PyInterpreterState;