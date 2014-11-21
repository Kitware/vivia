/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
