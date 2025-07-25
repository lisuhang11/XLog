#include "log.h"
#include <thread>

using namespace xlog;
void worker(int id) {
    for (int i = 0; i < 5; ++i) {
        LOG_INFO << "worker " << id << " count " << i;
    }
}

int main() {
    xlog::Logger::getInstance().init("./logs", "app");
    xlog::Logger::getInstance().set_level(xlog::LogLevel::DEBUG);

    LOG_DEBUG << "debug test";
    LOG_INFO << "program start";
    LOG_WARN << "this is warning";
    LOG_ERROR << "something wrong";
    LOG_FATAL << "fatal error";

    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    t1.join();
    t2.join();
    return 0;
}