#ifndef _MVKIT_MV_IMAGE_H
#define _MVKIT_MV_IMAGE_H

#include <mvkit/mv_defs.h>   /* error_code */

/**
 * @file
 * Loading and drawing of OS-9 graphics-buffer images (the format produced by
 * the repo's png-to-os9-image tool). Images are loaded into numbered cgfx get/
 * put buffers, then blitted to the screen. All functions return an error_code
 * (0 on success) unless noted.
 */

/** Initialize the image subsystem for the calling process and associate it with
   `app_name` (used to locate resources under /dd/SYS/<app_name>/; the string is
   borrowed, not copied, so it must outlive use). Frees any existing buffers.
   Call once before any other mv_image function. Exits the process on failure to
   obtain the process id. */
extern void mv_image_init(const char *app_name);

/** Load the OS-9 image file at `path` into get/put buffer `buffer_number`. */
extern error_code mv_image_load(const char *path, int buffer_number);

/** Load image `name` from this app's resource directory (/dd/SYS/<app_name>/
   <name>, per mv_image_init) into `buffer_number`. */
extern error_code mv_image_load_resource(const char *name, int buffer_number);

/** Draw the image in `buffer_number` to the screen with its top-left at (x, y). */
extern error_code mv_image_draw(int buffer_number, int x, int y);

/** Free the single get/put buffer `buffer_number`. */
extern error_code mv_image_free_buffer(int buffer_number);

/** Free all of this process's get/put buffers. */
extern error_code mv_image_free_all_buffers(void);

#endif /* _MVKIT_MV_IMAGE_H */
