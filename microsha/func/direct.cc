#include "config.cc"
#define DIRPROCEXIT 2

int main(int argc, char **argv) {

    if(argc < 2) {
        fprintf(stderr, "Too few arguments\n");
        return 1;
    }

    int i;
    vector<string> args;

    for(i = 1; i < argc; ++i) {
        args.push_back(string(argv[i]));
    }

    int input = 0, output = 1, flag_in = 0, flag_out = 0;
    vector<int> positions;
    vector<char *> cmd;
    char symbol;

    // введена пустая строка
    if(args[0] == "") {
        return 0;
    }

    // positions = [input, output] = [< , >], stdin = 0, stdout = 1
    if(direct_find(args, &positions) == -1) {
        return 0;
    }

    if(positions.size() != 2) {
        fprintf(stderr, "direct_process: positions error\n");
        exit(EXIT_FAILURE);
    }

    if(positions[0] != 0) {
        symbol = is_meta(args[positions[0] + 1]);

        if(symbol != 0) {
            fprintf(stderr, "misha: syntax error '%c'\n", symbol);
            return 0;
        }

        input = open(args[positions[0] + 1].c_str(), O_RDWR | O_CREAT, 0666);
        flag_in = 1;
    }

    if(positions[1] != 0) {
        symbol = is_meta(args[positions[1] + 1]);

        if(symbol != 0) {
            fprintf(stderr, "misha: syntax error '%c'\n", symbol);
            return 0;
        }

        output = open(args[positions[1] + 1].c_str(), O_RDWR | O_CREAT, 0666);
        flag_out = 1;
    }

    if(flag_in == 1) {
        args = direct_delete(args, "<");
    }

    if(flag_out == 1) {
        args = direct_delete(args, ">");
    }

    cmd = args_to_c(args);

    pid_t pid = fork();

    if(pid > 0) {
        // parent
        int status;
        wait(&status);
        if(WIFEXITED(status)) {
            int status_code = WEXITSTATUS(status);
            if(status_code == DIRPROCEXIT) {
                fprintf(stderr, "%s: command not found\n", cmd[0]);
                return 0;
            }
        }
	}

	if(pid == 0) {
        // child

        if(input != 0) {
            dup2(input, 0);
        }
        if(output != 1) {
            dup2(output, 1);
        }
        execvp(cmd[0], &cmd[0]);
        fprintf(stderr, "%s: command not found\n", cmd[0]);
        return 1;
	}

    if(pid < 0) {
        fprintf(stderr, "fork() error\n");
        return 1;
    }

    if(input != 0) {
            close(input);
        }
        if(output != 1) {
            close(output);
        }

    return 0;
}
