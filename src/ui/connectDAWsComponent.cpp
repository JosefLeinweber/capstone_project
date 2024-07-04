#include "ConnectDAWsComponents/connectDAWsComponent.h"

ConnectDAWsComponent::ConnectDAWsComponent(
    std::shared_ptr<Messenger> &guiMessenger,
    std::shared_ptr<Messenger> &cmtMessenger,
    std::function<void()> forceResizeCallback)
    : m_guiMessenger(guiMessenger), m_cmtMessenger(cmtMessenger),
      m_forceResizeCallback(forceResizeCallback)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    initGUIMessenger();
    constexpr auto HEIGHT = 500;
    constexpr auto WIDTH = 500;

    addAndMakeVisible(m_statusLabel);
    m_statusLabel.setText("Status: loading...", juce::dontSendNotification);
    m_statusLabel.setColour(juce::TextEditor::textColourId,
                            juce::Colours::white);

    addAndMakeVisible(m_startConnectionComponent);
    addAndMakeVisible(m_inConnectionComponent);
    m_inConnectionComponent.setVisible(false);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setSize(WIDTH, HEIGHT);
}

ConnectDAWsComponent::~ConnectDAWsComponent()
{
}

void ConnectDAWsComponent::paint(juce::Graphics &g)
{
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
}

void ConnectDAWsComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

    // In this example, we'll set the size of our child component to be the same
    // as our own size.
    auto area = getLocalBounds();
    auto textFieldHeight = 30;
    m_statusLabel.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
    m_startConnectionComponent.setBounds(area);
    m_inConnectionComponent.setBounds(area);
}

void ConnectDAWsComponent::buttonClickedCallback(juce::Button *button,
                                                 bool success)
{
    if (button->getButtonText() == "Connect" && success)
    {
        m_isConnected = true;
        m_startConnectionComponent.setVisible(false);
        m_inConnectionComponent.setVisible(true);
        sendAddressMessageToCMT(m_startConnectionComponent.getIP(), 0);
        sendStatusMessageToCMT("start", "Try to connecto to remote...");
    }
    else if (button->getButtonText() == "Cancel" && success)
    {
        m_isConnected = false;
        m_inConnectionComponent.setVisible(false);
        m_startConnectionComponent.setVisible(true);
    }
    m_forceResizeCallback();
}


void ConnectDAWsComponent::handleMessage(const juce::Message &message)
{
    if (auto *statusMessage = dynamic_cast<const StatusMessage *>(&message))
    {
        if (statusMessage->m_messageType == "status")
        {
            m_statusLabel.setText(statusMessage->m_message,
                                  juce::dontSendNotification);
        }
        else
        {
            std::cout << "ConnectDAWsComponent::handleMessage | Received "
                         "unknown message type from CMT"
                      << std::endl;
        }
    }
    else
    {
        std::cout << "ConnectDAWsComponent::handleMessage | Received "
                     "unknown message from CMT"
                  << std::endl;
    }
}

void ConnectDAWsComponent::initGUIMessenger()
{
    m_guiMessenger = std::make_shared<Messenger>(
        std::bind(&ConnectDAWsComponent::handleMessage,
                  this,
                  std::placeholders::_1));
}

void ConnectDAWsComponent::sendStatusMessageToCMT(std::string type,
                                                  std::string message)
{
    m_cmtMessenger->postMessage(new StatusMessage(type, message));
}

void ConnectDAWsComponent::sendAddressMessageToCMT(std::string ip, int port)
{
    m_cmtMessenger->postMessage(new AddressMessage(ip, port));
}
