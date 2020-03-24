#pragma once

#include "ArcGlobals.h"

#define KILOBYTES(n) (n * 1024)
#define MEGABYTES(n) ((u64)KILOBYTES(n) * 1024)
#define GIGABYTES(n) ((u64)MEGABYTES(n) * 1024)

#define VERTEX_BUFFER_SIZE MEGABYTES(128)
#define INDEX_BUFFER_SIZE MEGABYTES(64)
