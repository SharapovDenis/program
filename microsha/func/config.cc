#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <algorithm>
#include <pwd.h>

using namespace std;

int misha_readline(vector<string> *tokens) {

    /*

        Считывает строку из стандартного ввода.

    */


    string s = "";
    char c = ' ';

    while(c != '\n') {
        c = getchar();
        if(c == EOF) {
            return -1;
        }
        while(c != ' ' && c != '\n' && c != '\t') {
            s = s + c;
            c = getchar();
            if(c == EOF) {
                return -1;
            }
        }
        if(s != "") {
            tokens->push_back(s);
            s = "";
        }
    }

    if(tokens->size() == 0) {
        tokens->push_back("");
        return tokens->size();
    }

    // the last char
    c = tokens->back().back();

    if(c == '>' || c == '<') {
        fprintf(stderr, "misha: syntax error near unexpected token `newline'\n");
        tokens->clear();
        tokens->push_back("");
        return tokens->size();
    }

	return tokens->size();
}

vector<vector<string> > misha_parse(vector<string> line) {

    /*

        Разбивает строку на токены с разделителем | .

    */

    vector<vector<string> > cmd;
    vector<string> tokens;
    long unsigned int i;

    for(i = 0; i < line.size(); ++i) {
        while (i < line.size() && line[i] != "|") {
            tokens.push_back(line[i]);
            ++i;
        }
        cmd.push_back(tokens);
        tokens.clear();
    }

    if(cmd.size() < 1) {
        tokens.push_back("");
        cmd.push_back(tokens);
        return cmd;
    }

    return cmd;
}

vector<string> split(string line, char to_split) {

    /*

        Разделяет строку на слова с разделителем to_split.

    */

	long unsigned int i = 0;
	vector<string> ret;
	string s;

	while (i < line.size()) {
    	while (i < line.size() && line[i] != to_split) {
        	s = s + line[i];
            i++;
		}
    	if (s != "") {
            ret.push_back(s);
            s = "";
    	}
        i++;
	}
	return ret;
}

string join(vector<string> args, string s, int key) {

    /*

        Составляет из слов предложение с разделитем s.

    */

    string ret = s + args[0];
    long unsigned int i;

    if(key == 0) {
        ret = args[0];
    }

    if(key == 1) {
        ret = s + args[0];
    }

	for (i = 1; i < args.size(); i++) {
        ret = ret + s + args[i];
	}
	return ret;
}

vector<char *> args_to_c(vector<string> &args) {

    /*

        Переводит вектор string в вектор char * .
        В конец добавляется nullptr для дальнейшего запуска.

    */

    vector<char *> c_args;
    long unsigned int i;

	for (i = 0; i < args.size(); ++i) {
        c_args.push_back((char *)(args[i].c_str())); 
    }
	c_args.push_back(nullptr);
	return c_args;
}

int is_meta(string symbol){

    if(symbol == "*") {
        return '*';
    }
    if(symbol == "?") {
        return '?';
    }
    if(symbol == "\n") {
        return '\n';
    }
    return 0;
}

vector<string> direct_delete(vector<string> args, string symbol) {

    /*

        Удаляет первое вхождение слова и следующее за ним слово.

    */

    long unsigned int i;

    for(i = 0; i < args.size(); ++i) {
        if(args[i] == symbol) {
            args.erase(args.begin() + i, args.begin() + i + 2);
            return args;
        }
    }
    return args;
}

int direct_find(vector<string> args, vector<int> *positions) {

    /*

        Находит позиции расположения метасимволов > и < .

    */

    int input = 0, output = 0;
    int flag_in = 0, flag_out = 0;
    long unsigned int i;

    for(i = 0; i < args.size(); ++i) {
        if(args[i] == ">") {
            if(flag_out != 0) {
                fprintf(stderr, "misha: syntax error '>'\n");
                return -1;
            }
            output = i;
            flag_out = 1;
        }
        if(args[i] == "<") {
            if(flag_in != 0) {
                fprintf(stderr, "misha: syntax error '<'\n");
                return -1;
            }
            input = i;
            flag_in = 1;
        }
    }
    positions->push_back(input);
    positions->push_back(output);
    return 0;
}

int direct_process(vector<string> args) {

    /*

        Исполняет подготовленный вектор с переводом потоков,
        если необходимо.

    */

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

    return 0;
}

int conveer(vector<vector<string>> args) {

    /*

        Запускает конвейер.

    */

    int fd[2][2], status;
    long unsigned int i;
    pid_t proc_id[args.size()] = {-5};

    fd[0][0] = 0;
    fd[0][1] = 1;
    fd[1][0] = 0;
    fd[1][1] = 1;

    for(i = 0; i < args.size(); ++i) {

        if(i > 1) {
            close(fd[0][0]);
            close(fd[0][1]);
        }

        fd[0][0] = fd[1][0];
        fd[0][1] = fd[1][1];

        if(i != args.size() - 1){
            pipe(fd[1]);
        }
        
        proc_id[i] = fork();

        if(proc_id[i] == 0) {
            if(i) {
                close(fd[0][1]);
                dup2(fd[0][0], 0);
            }

            if(i != args.size() - 1) {
                close(fd[1][0]);
                dup2(fd[1][1], 1);
            }  

            vector<char *> cmd = args_to_c(args[i]);
            execvp(cmd[0], &cmd[0]);
            fprintf(stderr, "%s: command not found\n", cmd[0]);
            return 1;
        }

    }

    if(args.size() > 1) {
        close(fd[0][0]);
        close(fd[0][1]);
    }

    for(i = 0; i < args.size(); ++i) {
        waitpid(proc_id[i], &status, 0);
    }
    
    return 0;
}

int pattern_check(string line) {

    /*

        Проверяет вхождение шаблонов * и ? .

    */

    long unsigned int i;

    for (i = 0; i < line.size(); i++) {
        if (line[i] == '*' || line[i] == '?') {
            return 1;
        }
    }
    return 0;
}

int pattern_find(vector<string> args) {

    /*

        Находит первое вхождение шаблонов * и ? .

    */

    long unsigned int i;

    for (i = 0; i < args.size(); i++) {
        if(pattern_check(args[i])) {
            return i;
        }
	}
    return -1;
}

int misha_cd(vector<string> args) {

    /*

        Смена директории.

    */

    const char *home = "/home";

    if(args.size() < 2) {
        if(chdir(home) != 0) {
            perror("misha_cd");
        }
    } else {
        if(chdir(args[1].c_str()) != 0) {
            perror("misha_cd");
        }
    }
  return 0;
}

int misha_pwd(vector<string>) {

    /*

        Печать директории в стандартный вывод.

    */

    char cwd[PATH_MAX];

    if(getcwd(cwd, sizeof(cwd)) != nullptr) {
       printf("%s\n", cwd);
    } else {
       perror("getcwd() error");
       return 1;
    }
   return 0;
}

int misha_launch(vector<string> args) {

    /*

        Запуск программ в дочернем процессе.

    */

    vector<char *> cmd;
    cmd = args_to_c(args);

    pid_t pid = fork();

    if(pid > 0) {
        // parent
        int status;
        wait(&status);
	}

	if(pid == 0) {
        // child

        execvp(cmd[0], &cmd[0]);
        fprintf(stderr, "%s: command not found\n", cmd[0]);
        return 1;
	}

    if(pid < 0) {
        fprintf(stderr, "fork() error\n");
        return 1;
    }

    return 0;
}

int lauch_binaries(vector<string> args, string location, string cmd) {

    /*

        Запуск бинарных файлов из папки ./bin .

    */

    vector<string> new_args;
    string dir_name = location + "/bin";
    string file_name = dir_name + "/" + cmd;
    long unsigned int i;

    DIR *dir = opendir(dir_name.c_str());

    if(dir == nullptr) {
        fprintf(stderr, "misha: can not read '%s'\n", dir_name.c_str());
        exit(EXIT_FAILURE);
    }

    new_args.push_back(file_name);

	for (i = 0; i < args.size(); ++i) {
        new_args.push_back(args[i]); 
    }

    closedir(dir);

    return misha_launch(new_args);
}

int _execute(vector<string> args, string location) {

    /*

        Подготовка к исполнению программ.

    */

    if(args[0] == "") {
        // введена пустая строка
        return 0;
    }
    if(args[0] == "cd") {
        misha_cd(args);
        return 0;
    }
    if(args[0] == "pwd") {
        lauch_binaries(args, location, "pwd");
        return 0;
    }
    if(args[0] == "echo") {
        lauch_binaries(args, location, "echo");
        return 0;
    }
    if(args[0] == "time") {
        lauch_binaries(args, location, "time");
        return 0;
    }
    if(pattern_find(args) != -1) {
        lauch_binaries(args, location, "pattern");
        return 0;
    }
    return lauch_binaries(args, location, "direct");
}

int misha_execute(vector<string> line, string location) {

    /*

        Выбор исполнения строки: через обыкновенный запуск или
        посредством конвейера.

    */

    vector<vector<string>> args = misha_parse(line);

    if(args.size() > 1) {
        return lauch_binaries(line, location, "conv");
    }

    return _execute(args[0], location);
}

void check_user() {
	
    /*

        Вывод в стандартный вывод информации о пользователе и
        текущей директории.

    */

    struct passwd *userinfo;
    uid_t userid;

    char wd[PATH_MAX];

    userid = getuid();
    userinfo = getpwuid(userid);

    if(getcwd(wd, sizeof(wd)) != nullptr) {
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

void find_self(string *start_location) {

    /*

        Поиск текущей директории. Необходима для поиска 
        папки ./bin .

    */

    char wd[PATH_MAX];

    if(getcwd(wd, sizeof(wd)) != nullptr) {
        *start_location = string(wd);
    } else {
        fprintf(stderr, "can not read directory!\n");
        exit(1);
    }
}
