#pragma once
#include <juce_core/juce_core.h>

class FileLogger
{
public:
    FileLogger();
    ~FileLogger();

    void logMessage(const juce::String &message);

    void createLogger(const juce::String &logName);

private:
    juce::String generateLogFileDirectory();

    std::shared_ptr<juce::FileLogger> m_fileLogger;
    juce::String m_logName;
};

std::shared_ptr<FileLogger> generateFileLogger(const juce::String &logName);
