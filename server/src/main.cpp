///
/// main: do
///     1 create pid file
///     2 load config
///     3 init mysql
///     4 inti spdlog
///     5 start listener
///
#include <unistd.h>

#include <csignal>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <spdlog/spdlog.h>

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

    auto logger = spdlog::get("main");

    logger->info("Call ServerListener.startListener(), START SERVICE!");
    // start listener
    ServerListener *s = ServerListener::getInstance();
    s->startListener();

    logger->info("Call ServerListener.startListener() stop service.");
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
    auto logger = spdlog::get("main");
    if (logger != nullptr) {
        logger->info("Catch signal {}, quit...", signum);
    }
    ServerListener *s = ServerListener::getInstance();
    s->clear();
    unlink(pid_file);
    logger->info("Quited.", signum);
    exit(0);
}
// handle SIGPIPE, dont quit
void handle_pipe(int sig)
{
//不做任何处理即可
}

// regist Loggers
void registLoggers() {
    try {
        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(std::make_shared<spdlog::sinks::stdout_sink_st>());
        sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_st>(
                            Config::get("log.file.basename"),
                            Config::get("log.file.extension"), 23, 59));
        string loglevel_s = Config::get("log.level");
        spdlog::level::level_enum loglevel = spdlog::level::info;
        if (loglevel_s == "warn") loglevel = spdlog::level::warn;
        else if (loglevel_s == "error") loglevel = spdlog::level::err;
        else if (loglevel_s == "info") loglevel = spdlog::level::info;
        else if (loglevel_s == "debug") loglevel = spdlog::level::debug;
        spdlog::set_level(loglevel);
        // if new logger required, must add to here!
        const char *loglist[] = {
            "main",
            "ClientConnection",
            "ServerListener",
            "LinearMemoryPool",
            "BlockedMemoryPool",
            "UserSession",
            "FxServer",
            ""
        };
        for (int i = 0; strcmp("", loglist[i]) != 0; i++) {
            auto combined_logger = std::make_shared<spdlog::logger>(loglist[i], begin(sinks), end(sinks));
            combined_logger->set_level(loglevel);
            spdlog::register_logger(combined_logger);
        }
    } catch (const spdlog::spdlog_ex& ex) {
        cerr << "Log failed: " << ex.what() << endl;
    }
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

    // SIGPIPE
    action.sa_handler = handle_pipe;
    // sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGPIPE, &action, NULL);


    // check pid file status
    std::fstream fs_pid;
    fs_pid.open (pid_file, std::fstream::in);
    int pid_from_file = 0;
    fs_pid >> pid_from_file;
    bool pid_existed = pid_from_file != 0;
    fs_pid.close();

    if (strcasecmp(argv[1], "start") == 0) { // start service
        //if (pid_existed) { // pid file exists
        if (false) {
            std::cerr << "File '" << pid_file
                      << "' exists, service may running("
                      << pid_from_file << "). run 'fxserver stop' to stop service."
                      << std::endl;
            exit(1);
        }

        // load config
        Config::load("/home/x4x/Workspace/biye/server/src/default.cfg");
        // regist loggers
        registLoggers();

        // fork to background
        // pid_t pid = fork();
        int pid = 0;
        if (pid < 0) {
            std::cerr << "FORK FAILED!" << std::endl;
        } else if (pid == 0) { // child process
            // init mysql connector
            MysqlConnector::getInstance();

            startService(); // SERVICE START !
        } else { // parent process
            spdlog::get("main")->info("Service has been started. PID: {}", pid);
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
