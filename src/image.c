#include "../include/image.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int image_fd;

int image_open(char *filename, int truncate) {
    // Open the image file with read/write permissions and create if it doesn't exist
    int flags = O_RDWR | O_CREAT;
    if (truncate) {
        flags |= O_TRUNC;
    }

    // Open the image file and return the file descriptor
    image_fd = open(filename, flags, 0600);
    return image_fd;
}

int image_close(void) {
    return close(image_fd);
}