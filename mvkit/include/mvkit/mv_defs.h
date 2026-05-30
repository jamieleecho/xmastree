#ifndef _MVKIT_MV_DEFS_H
#define _MVKIT_MV_DEFS_H

#include <os.h>   /* error_code */

/* OS-9 standard input/output path numbers, used for all cgfx I/O. */
#define MV_INPATH  0
#define MV_OUTPATH 1

/* Maximum length (including the NUL) of an MVKit file-path buffer. */
#define MV_PATH_MAX 44

/* Flush buffered graphics output. Implemented in libcgfx (cbuffer.as, as the
   exported _Flush symbol); declared here so MVKit modules and apps need not
   each forward-declare it. */
error_code Flush(void);

#endif /* _MVKIT_MV_DEFS_H */
