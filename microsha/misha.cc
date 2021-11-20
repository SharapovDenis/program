#include "func/config.cc"
using namespace std;

void sighandler(int signum) {
    if(signum == SIGINT) {
        printf("\n");
        check_user();
        fflush(stdout);
    }
}

int main() {
    vector<string> line;
    vector<vector<string>> args;
    string misha_location;
    int status = 1, flag = -1;
    
    find_self(&misha_location);

    // ^C contol
    signal(SIGINT, sighandler);

    do {
        check_user();
        flag = misha_readline(&line);
        if(flag == -1) {
            line.clear();
            return 0;
        }
        args = misha_parse(line);
        status = misha_execute(line, misha_location);
        line.clear();
    } while(status == 0);
}
