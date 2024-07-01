#include "ConnectDAWs/logger.h"

FileLogger::FileLogger(const juce::String &logName) : m_logName(logName)
{
    m_fileLogger = createLogger(logName);
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

std::unique_ptr<juce::FileLogger> FileLogger::createLogger(
    const juce::String &logName)
{
    std::unique_ptr<juce::FileLogger> fileLogger(
        juce::FileLogger::createDefaultAppLogger(
            generateLogFileDirectory(),
            logName,
            "ConnectDAWs File Logger | only the ConnectionManagerThread will "
            "write to this file to avoid blocking calls inside the audio "
            "thread.",
            128 * 1024));
    return fileLogger;
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
