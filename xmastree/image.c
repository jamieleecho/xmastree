#include <cgfx.h>
#include <fcntl.h>
#include <os.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "app.h"
#include "image.h"


static const char *my_app_name;
static char buffer[APP_PATH_MAX];
static int pid;


void image_init(const char *app_name) {
    my_app_name = app_name;
    int err = _os_getpid(&pid);
    if (err) {
        exit(err);
    }
    image_free_all_buffers();
}


int image_load_image(const char *path, int buffer_number) {
    Flush();
    int file = open(path, FAP_READ);
    int jj, err, kk = 0;
    if (file < 0) {
        return errno;
    }

    buffer[0] = 0x1b;
    buffer[1] = 0x2b;
    buffer[2] = (char)pid;
    buffer[3] = (char)buffer_number;
    if (write(OUTPATH, buffer, 4) < 0) {
        err = errno;
        close(file);
        return err;
    }

    do {
        jj = read(file, buffer, sizeof(buffer));
        if (jj == 0) {
            close(file);
            return 0;
        } else if (jj < 0) {
            err = errno;
            close(file);
            return err;
        } else {
            kk = kk + jj;
            if (write(OUTPATH, buffer, jj) < 0) {
                err = errno;
                close(file);
                return err;
            }
        }
    } while(TRUE);

    return 0;
}


int image_load_image_resource(const char *name, int buffer_number) {
    strcpy(buffer, "/dd/SYS/");
    strncat(buffer, my_app_name, sizeof(buffer));
    strncat(buffer, "/", sizeof(buffer));
    strncat(buffer, name, sizeof(buffer));
    buffer[APP_PATH_MAX - 1] = 0;
    return image_load_image(buffer, buffer_number);
}


int image_draw_image(int buffer_number, int x, int y) {
    return _cgfx_putblk(OUTPATH, pid, buffer_number, x, y);
}


int image_free_buffer(int buffer_number) {
    return _cgfx_kilbuf(OUTPATH, pid, buffer_number);
}


int image_free_all_buffers(void) {
    return _cgfx_kilbuf(OUTPATH, pid, 0);
}
