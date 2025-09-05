#include "Logger.h"

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}
