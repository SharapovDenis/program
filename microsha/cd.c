#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#ifndef MAX_BUF
#define MAX_BUF 200
#endif

int main(void) {
    char path[MAX_BUF];

    errno = 0;
    char *buf = getcwd(NULL, 0);
    if (buf == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
    printf("Current working directory: %s\n", buf);
    free(buf);

    exit(EXIT_SUCCESS);
}
