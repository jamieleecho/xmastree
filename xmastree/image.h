#ifndef _IMAGE_H
#define _IMAGE_H

extern void image_init(const char *app_name);
extern error_code image_load_image(const char *path, int buffer_number);
extern error_code image_load_image_resource(const char *name, int buffer_number);
extern error_code image_draw_image(int buffer_number, int x, int y);
extern error_code image_free_buffer(int buffer_number);
extern error_code image_free_all_buffers(void);

#endif /* _IMAGE_H */
