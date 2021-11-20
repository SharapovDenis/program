#include "config.cc"

int main(int argc, char **argv) {

    if(argc < 2) {
        fprintf(stderr, "Too few arguments\n");
        return 1;
    }

    size_t pos;
    string location = string(argv[0]);
    
    pos = location.find("/bin/conv");
    location.erase(pos);

    int i, fd[2][2], status;
    vector<string> line;
    vector<vector<string>> args;

    for(i = 1; i < argc; ++i) {
        line.push_back(string(argv[i]));
    }

    args = misha_parse(line);

    if(args[0][0] == "") {
        return 0;
    }

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

            return _execute(args[i], location);
        }

    }

    if(args.size() > 1) {
        close(fd[0][0]);
        close(fd[0][1]);
    }

    for(i = 0; i < args.size(); ++i) {
        int status;
        waitpid(proc_id[i], &status, 0);
    }
    
    return 0;
}
