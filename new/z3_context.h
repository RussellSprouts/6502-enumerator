#pragma once

#include "z3++.h"

// Define a thread_local context so that we can easily construct
// expressions without passing a context.
thread_local z3::context z3_ctx;
