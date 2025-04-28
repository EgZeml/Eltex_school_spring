#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// Дополнительная программа, чтобы считать и вывести данные
int main() {
    int fd = open("output.dat", O_RDONLY);
    ssize_t r;
    int read_number;
    while ((r = read(fd, &read_number, sizeof(read_number))) == sizeof(read_number)) {
        printf("%d ", read_number);
    }

    if (r < 0) {
        perror("file read");
    }

    printf("\n");

    return EXIT_SUCCESS;
}