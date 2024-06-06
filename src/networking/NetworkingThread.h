#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp> 
#include "AudioBuffer.h"


class NetworkThread : public juce::Thread
{
public:
  NetworkThread(AudioBufferFIFO& audioBufferFIFOArg);

  void run() override;

  void stopThreadSafely();

private:
  AudioBufferFIFO& audioBufferFIFO;
};
