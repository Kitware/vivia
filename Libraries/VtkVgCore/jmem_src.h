// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

/* jmem_src.h: memory source manager for JFIF library
*/

#include <vtk_jpeg.h>

typedef struct {
  struct jpeg_source_mgr pub;   /* public fields */

  JOCTET * buffer;              /* image buffer */
  unsigned int buffer_size;     /* image buffer size */
} mem_source_mgr;

typedef mem_source_mgr * mem_src_ptr;

extern GLOBAL(void)
jpeg_memory_src (j_decompress_ptr cinfo, const unsigned char *jfif_buffer,
                 int buf_size);
