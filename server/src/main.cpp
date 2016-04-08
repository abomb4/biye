#include <unistd.h>

#include <csignal>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include "serverlistener.h"
#include "config.h"
#include "mysqlconnector.h"

#include "usermanager.h"

using std::cout;
using std::endl;
using std::cerr;

static char pid_file[256];
static int SERVER_LISTENER_PORT = 0;

void startService() {
    std::ofstream of_pid;
    of_pid.open(pid_file);
    of_pid << getpid();
    of_pid.close();

    cout << "start" << endl;
    // start listener
    ServerListener s = ServerListener::getInstance();
    s.startListener();

    cout << "end" << endl;
    unlink(pid_file);
}

void usage() {
    std::cout << "Usage: \n\tfxserver start|stop" << std::endl;
}

// handle SIGTERM and SIGINT signal, quit service
void handleSigterm(int signum) {
    if (signum == SIGTERM)
        cout << "Catch SIGTERM signal, quit..." << endl;
    else if (signum == SIGINT)
        cout << "Catch SIGINT signal, quit..." << endl;
    ServerListener s = ServerListener::getInstance();
    s.clear();
    unlink(pid_file);
    MysqlConnector c = MysqlConnector::getInstance();
    exit(0);
}

int main(int argc, char *argv[])
{
    // pid file path XIE SI
    sprintf(pid_file, "/tmp/fxserver.pid");

    // deal with parameter
    if (argc != 2) { // 参数数量为2
        usage();
        exit(0);
    }

    // handle SIGTERM signal
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = handleSigterm;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);


    // check pid file status
    std::fstream fs_pid;
    fs_pid.open (pid_file, std::fstream::in);
    int pid_from_file = 0;
    fs_pid >> pid_from_file;
    bool pid_existed = pid_from_file != 0;
    fs_pid.close();

    if (strcasecmp(argv[1], "start") == 0) { // start service
        if (pid_existed) { // pid file exists
            std::cerr << "File '" << pid_file
                      << "' exists, service may running("
                      << pid_from_file << "). run 'fxserver stop' to stop service."
                      << std::endl;
            exit(1);
        }

        // load config
        Config::load("/home/x4x/Workspace/biye/server/src/default.cfg");

        // fork to background
        // pid_t pid = fork();
        int pid = 0;
        if (pid < 0) {
            std::cerr << "FORK FAILED!" << std::endl;
        } else if (pid == 0) { // child process
            // init mysql connector
            MysqlConnector c = MysqlConnector::getInstance();

            startService(); // SERVICE START !
        } else { // parent process
            std::cout << "Service has been started. PID: " << pid << std::endl;
        }

    } else if (strcasecmp(argv[1], "stop") == 0) {
        // TODO read pid file, send SIGTERM to pid
        if (pid_existed) {
            int r = 0;
            r = kill(pid_from_file, SIGTERM);
            unlink(pid_file);
            if (r != 0) { // kill faild
                std::cerr << "Kill process " << pid_from_file << " failed." << std::endl;
                exit(2);
            }
        } else {
            std::cout << "Service is not running." << std::endl;
        }
    } else {
        usage();
    }
    return 0;
}
