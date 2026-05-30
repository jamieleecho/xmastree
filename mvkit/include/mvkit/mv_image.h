#ifndef _MVKIT_MV_IMAGE_H
#define _MVKIT_MV_IMAGE_H

#include <mvkit/mv_defs.h>   /* error_code */

extern void mv_image_init(const char *app_name);
extern error_code mv_image_load(const char *path, int buffer_number);
extern error_code mv_image_load_resource(const char *name, int buffer_number);
extern error_code mv_image_draw(int buffer_number, int x, int y);
extern error_code mv_image_free_buffer(int buffer_number);
extern error_code mv_image_free_all_buffers(void);

#endif /* _MVKIT_MV_IMAGE_H */
