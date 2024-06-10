#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp> 
#include "AudioBuffer.h"


class ConnectionManagerThread : public juce::Thread
{
public:
  ConnectionManagerThread(AudioBufferFIFO& inputRingBuffer, AudioBufferFIFO& outputRingBuffer);

  void run() override;

  void connectToVirtualStudio();

  void sendHandshake(const std::string& ip, const int port);

  void stopThreadSafely();

  void receiveHandshake();

  void sendHandsake();

  void 

private:
  std::atomic<bool> m_isProviderConnected;
  std::atomic<bool> m_isConsumerConnected;
  std::unique_ptr<juce::Thread> m_listenerThread;
  AudioBufferFIFO& m_inputRingBuffer;
  AudioBufferFIFO& m_outputRingBuffer;
};
