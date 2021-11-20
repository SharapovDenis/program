#include "config.cc"
#include <sys/time.h>
#include <sys/resource.h>

void printt(const char *s, struct timeval *tv) {
    
    fprintf(stderr, "%s: %9ld.%01ld\n", s, tv->tv_sec, tv->tv_usec/100000);
}

int main(int argc, char** argv) {

    if (argc <= 1){
	    exit(0);
    }

	int status, i;
	pid_t pid;
	struct timeval before, after;
	struct rusage ru;

    size_t pos;
    string location = string(argv[0]);

    pos = location.find("/bin/time");
    location.erase(pos);

    vector<string> args;

    for(i = 2; i < argc; ++i) {
        args.push_back(string(argv[i]));
    }

	gettimeofday(&before, 0);

	pid = fork();

	if (pid < 0) {
		perror("time");
		exit(1);
	}
	if (pid == 0) {
		return _execute(args, location);
	}

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	while (wait3(&status, 0, &ru) != pid);

	gettimeofday(&after, 0);

	if ((status&0377) != 0) {
        fprintf(stderr, "\nCommand terminated abnormally.\n");
    }
    
	after.tv_sec -= before.tv_sec;
	after.tv_usec -= before.tv_usec;

	if (after.tv_usec < 0) {
        after.tv_sec--, after.tv_usec += 1000000;
    }
    
    fprintf(stderr, "\n");
	printt("real", &after);
	printt("user", &ru.ru_utime);
	printt("sys ", &ru.ru_stime);
	fprintf(stderr, "\n");
	exit (status>>8);
}
