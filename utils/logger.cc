#include "logger.h"

INITIALIZE_EASYLOGGINGPP

//void Logger::Initialize(int argc, char *argv[]) {
    //START_EASYLOGGINGPP(argc, argv);

void Logger::Initialize(int loggerLevel) {

    //el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format, 
    el::Loggers::reconfigureLogger("performance", el::ConfigurationType::Format, 
            "%datetime %logger %level : %msg");

    el::Configurations defaultConf;
    defaultConf.setToDefault();

    std::string enableDebug = "false";

    //if ( VLOG_IS_ON(1) ){
    if ( loggerLevel >= 1 ){
        enableDebug = "true";
    }
    defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, enableDebug);

    //if ( VLOG_IS_ON(5) ) {
    if ( loggerLevel >= 5 ) {
        defaultConf.set(el::Level::Global, el::ConfigurationType::Format, 
                "%datetime %logger %level [%fbase:%line %func]: %msg");
    } else if ( VLOG_IS_ON(1) ) {
        defaultConf.set(el::Level::Global, el::ConfigurationType::Format, 
                "%datetime %logger %level [%fbase:%line %func]: %msg");
    } else {
        defaultConf.set(el::Level::Global, el::ConfigurationType::Format, 
                "%datetime %logger %level [%fbase:%line]: %msg");
    }

    el::Loggers::reconfigureLogger("default", defaultConf);

    el::Loggers::addFlag(el::LoggingFlag::NewLineForContainer);
    el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);
    el::Loggers::addFlag(el::LoggingFlag::LogDetailedCrashReason);
    //el::Loggers::setVerboseLevel(5);
}


    //TIMED_FUNC(timerOpenOFD);
    //TIMED_SCOPE(timerOpenPage, "Open OFD Page.");

