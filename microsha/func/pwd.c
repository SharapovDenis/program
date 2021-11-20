#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>

int main(int argc, char** argv) {

    if(argc > 2) {
        fprintf(stderr, "%s: too many arguments!", argv[0]);
    }

    char cwd[PATH_MAX];

    if(getcwd(cwd, sizeof(cwd)) != NULL) {
        fprintf(stdout, "%s\n", cwd);
    } else {
       perror("getcwd() error");
       return 1;
    }

   return 0;
}
