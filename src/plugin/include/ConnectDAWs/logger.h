#pragma once
#include <juce_core/juce_core.h>

class FileLogger
{
public:
    FileLogger(const juce::String &logName);
    ~FileLogger();

    void logMessage(const juce::String &message);

private:
    juce::String generateLogFileDirectory();
    std::unique_ptr<juce::FileLogger> createLogger(const juce::String &logName);

    std::unique_ptr<juce::FileLogger> m_fileLogger;
    juce::String m_logName;
};

std::unique_ptr<FileLogger> getFileLogger();
