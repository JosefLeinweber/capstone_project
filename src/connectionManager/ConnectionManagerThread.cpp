#include "ConnectionManagerThread.h"

ConnectionManagerThread::ConnectionManagerThread(AudioBufferFIFO& inputRingBuffer, AudioBufferFIFO& outputRingBuffer) : 
    juce::Thread("Network Thread"), m_inputRingBuffer(inputRingBuffer), m_outputRingBuffer(outputRingBuffer) {};

void ConnectionManagerThread::run() {
  while(!threadShouldExit()){
    setupHost();

    asyncWaitForConnection();
    
    while (!m_isConnected /** || or a event from gui happens */) {
      threadSleep(100);
    }

    if (m_isConnected) {
      sendHandshake();
      receiveHandshake();
    } else{
      recieveHandshake();
      sendHandshake();
    }

    startUpProviderAndConsumerThreads();

    waitForClosingRequest();

    closeProviderAndConsumerThreads(); 
  }
};

void ConnectionManagerThread::setupSocket() {
  m_socket = std::make_unique<boost::asio::ip::udp::socket>(m_io_context);
};

void ConnectionManagerThread::stopThreadSafely(){
  signalThreadShouldExit();
  waitForThreadToExit(1000);
};

void NetworkingThread::connectToVirtualStudio() {
  m_isProviderConnected = false;
  m_isConsumerConnected = false;
  auto providerThread = std::make_unique<ProviderThread>(m_outputRingBuffer, m_isProviderConnected);
  auto consumerThread = std::make_unique<ConsumerThread>(m_inputRingBuffer, m_isConsumerConnected);
  providerThread->startRealtimeThread();
  consumerThread->startRealtimeThread();

  //time now
  auto startTime = std::chrono::high_resolution_clock::now();

  while (!m_isProviderConnected || !m_isConsumerConnected) {
    // 4. wait for the provider and consumer threads to connect

    // 5. timeout if the threads do not connect

      // 5.1 signal the provider and consumer threads to stop
      // -> return false;
    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - startTime).count() > 5) {
      providerThread->stopThread();
      consumerThread->stopThread();
      return false;
    }


    threadSleep(100);
  }

  return true;

}

void ConnectionManagerThread::waitForClosingRequest() {
  while (!m_shouldClose) {
    // 6. wait for a closing request from the gui or the connected virtual studio
    threadSleep(100);
  }

  providerThread->stopRealtimeThread();
  consumerThread->stopRealtimeThread();
  
}