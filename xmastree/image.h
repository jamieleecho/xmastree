#ifndef _IMAGE_H
#define _IMAGE_H

extern void image_init(const char *app_name);
extern int image_load_image(const char *path, int buffer_number);
extern int image_load_image_from_sys(const char *name, int buffer_number);
extern int image_draw_image(int buffer_number, int x, int y);
extern int image_free_buffer(int buffer_number);
extern int image_free_all_buffers(void);

#endif /* _IMAGE_H */
