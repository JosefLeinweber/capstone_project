#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp>
#include "AudioBuffer.h"
#include "Host.h"


class ConsumerThread : public juce::Thread
{
public:
  ConsumerThread(
    AudioBufferFIFO& inputRingBuffer,
    std::atomic<bool>& isConsumerConnected,
    addressData& hostAddress
  );

  void run() override;

  void setupHost();

  void startRecievingAudio();

  bool validateConnection();

  std::unique_ptr<Host>& getHost() { return m_host; }

private:
  addressData m_hostAddress;
  addressData m_remoteAddress;
  std::unique_ptr<Host> m_host;

  AudioBufferFIFO& m_inputRingBuffer;
  juce::AudioBuffer<float> m_inputBuffer;
  std::atomic<bool>& m_isConsumerConnected;
};
