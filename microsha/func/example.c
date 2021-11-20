#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <signal.h>
#define TOK_DELIM " |\t\n\r"
#define TOK_BUFSIZE 64

typedef struct vector {
    char** ptr;
    int size;
} vector;

char *misha_read_line() {
    /* buffer (line) should be freed 
        even if getline() failed */

    char *line = NULL;
    ssize_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

vector misha_parse(char *line) {
    /*tokens should be freed*/
    int bufsize = TOK_BUFSIZE;
    int position = 0;
    vector tokens;
    tokens.size = 0;
    tokens.ptr = (char**) malloc(bufsize * sizeof (char*));
    char *token;

    if(!tokens.ptr) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while(token != NULL) {
        tokens.ptr[position] = token;
        printf("%s\n", tokens.ptr[position]);
        ++position;

        if(position >= bufsize) {
            bufsize += TOK_BUFSIZE;
            tokens.ptr = realloc(tokens.ptr, bufsize * sizeof(char*));
            if(!tokens.ptr) {
                fprintf(stderr, "Memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIM);
    }
    tokens.ptr[position] = NULL;
    tokens.size = position;
    return tokens;
}

// ------------------------------------------------------------------ 1

int misha_cd(char **args) {
    if (args[1] == NULL) {
    fprintf(stderr, "misha: ожидается аргумент для \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("misha_cd");
    }
  }
  return 0;
}

int misha_pwd(char **args) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("%s\n", cwd);
    } else {
       perror("getcwd() error");
       return 1;
    }
   return 0;
}

char *builtin_str[] = {
  "cd",
  "pwd"
};

int misha_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int (*builtin_func[]) (char **) = {
  &misha_cd,
  &misha_pwd
};

// ------------------------------------------------------------------ 1

int misha_launch(char **args) {
    
    pid_t pid = fork();

    if (pid > 0) {
        // parent
        int status;
    	pid_t waitid = wait(&status);
    	printf("wait ended with status %d\n", status);
    	return 0;
    }

    if (pid == 0) {
        // child
        printf("Child proccess has started\n");
        execvp(args[0], args);
        printf("%s: command not found\n", args[0]);
    }

    if (pid < 0) {
        fprintf(stderr, "No child process is created\n");
        return 1;
    }
}


int misha_execute(char **args) {
    int i;
    if(args[0] == NULL) {
        // введена пустая строка
        return 0;
    }
    // built-in functions
    for (i = 0; i < misha_num_builtins(); ++i) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return misha_launch(args);
}

void check_user() {

/* Тут надо вынести печать директории в отдельную функцию 
	или не нужно, т.к пользователь может поменяться */
	
    struct passwd *userinfo;
    uid_t userid;

    char wd[PATH_MAX];

    userid = getuid();
    userinfo = getpwuid(userid);

    if(getcwd(wd, sizeof(wd)) != NULL) {
        if(userinfo->pw_uid == 0) {
            printf("%s:%s# ", userinfo->pw_name, wd);
        }
        else {
            printf("%s:%s$ ", userinfo->pw_name, wd);
        }
    }
    else {
       perror("getcwd() error");
    }
}

// ------------------------------------------------------------------ 2

// Функции, которые стоит написать 

void signals(int sign_no) {
    // обработчик ^C
}

// ------------------------------------------------------------------ 2

int main() {
    char *line;
    vector args;
    int status;

    do {
        check_user();
        line = misha_read_line();
        args = misha_parse(line);
        printf("size=%d\n", args.size);
        status = misha_execute(args.ptr);
        free(args.ptr);
        free(line);
    } while(status == 0);
}
