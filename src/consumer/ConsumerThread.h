#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp> 
#include "AudioBuffer.h"


class ConsumerThread : public juce::Thread
{
public:
  ConsumberThread();

  void run() override;

  void connectToVirtualStudio();

  void sendHandshake(const std::string& ip, const int port);

  void stopThreadSafely();

  void startListenerThread();

  void receiveHandsake();

  void sendHandsake();

  void 

private:
  std::unique_ptr<juce::Thread> m_listenerThread;
  AudioBufferFIFO& m_inputRingBuffer;
  juce::AudioBuffer<float> m_inputBuffer;
};
