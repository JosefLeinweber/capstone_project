#include "YourPluginName/NetworkingThread.h"

NetworkThread::NetworkThread(AudioBufferFIFO& audioBufferFIFOArg) : juce::Thread("Network Thread"), audioBufferFIFO(audioBufferFIFOArg) {};

void NetworkThread::run() {
  boost::asio::io_context io_context;
  boost::asio::ip::udp::socket socket(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 8001));

  try { 
    std::array<char, 128> recv_buf;
    boost::asio::ip::udp::endpoint remote_endpoint;
    socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);

    OutputDebugString(recv_buf.data());

    juce::AudioBuffer<float> tempBuffer(2, 256);
    tempBuffer.clear();

    for (;;) {
      
      audioBufferFIFO.readFromBuffer(tempBuffer);

      const float* data = tempBuffer.getReadPointer(0);
      // float* data2 = tempBuffer.getReadPointer(1);
      std::size_t length = tempBuffer.getNumSamples() * sizeof(float) * tempBuffer.getNumChannels();

      boost::system::error_code ignored_error;

      // const int numChannels = tempBuffer.getNumChannels();
      // const int numSamples = tempBuffer.getNumSamples();

      // for (int sample = 0; sample < numSamples; ++sample) {
      //   for (int channel = 0; channel < numChannels; ++channel) {
      //     float sampleValue = tempBuffer.getSample(channel, sample);

      //     OutputDebugString(std::to_string(sampleValue).c_str());
      //     // Convert the float sample to the appropriate byte representation (e.g., endianness)
      //     // ... (place your conversion logic here)        }
      //   }

      // }

      socket.send_to(boost::asio::buffer(data, length), remote_endpoint, 0, ignored_error);
    }

  } 
  catch (std::exception& e) {
      juce::ignoreUnused(e);
  }
};

void NetworkThread::stopThreadSafely(){
  signalThreadShouldExit();
  waitForThreadToExit(1000);
};