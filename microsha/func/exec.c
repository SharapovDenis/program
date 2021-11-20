#include <unistd.h>

int main() {
    char *args[2];
    args[0] = "./pattern";
    args[1] = NULL;
    execv(args[0], args);
}
