#include "logger.h"

FileLogger::FileLogger()
{
}

FileLogger::~FileLogger()
{
}

juce::String FileLogger::generateLogFileDirectory()
{
    juce::File logDirectory(
        juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("ConnectDAWs"));
    return logDirectory.getFullPathName();
}


void FileLogger::createLogger(const juce::String &logName)
{
    m_fileLogger = std::shared_ptr<juce::FileLogger>(
        juce::FileLogger::createDefaultAppLogger(
            generateLogFileDirectory(),
            logName,
            "ConnectDAWs File Logger | only the ConnectionManagerThread will "
            "write to this file to avoid blocking calls inside the audio "
            "thread.",
            128 * 1024));
}

void FileLogger::logMessage(const juce::String &message)
{


    if (m_fileLogger)
    {
        m_fileLogger->logMessage(message);
    }
    else
    {
        std::cout << "FileLogger | Logger not initialized!" << std::endl;
    }
}

std::shared_ptr<FileLogger> generateFileLogger(const juce::String &logName)
{

    std::shared_ptr<FileLogger> fileLoggerPtr = std::make_shared<FileLogger>();
    fileLoggerPtr->createLogger(logName);
    return fileLoggerPtr;
}
