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

    addAndMakeVisible(m_errorComponent);
    m_errorComponent.setVisible(false);
    addAndMakeVisible(m_startConnectionComponent);
    addAndMakeVisible(m_inConnectionComponent);
    updateComponentVisibility(m_isConnected);
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
    m_errorComponent.setBounds(area);
    m_startConnectionComponent.setBounds(area);
    m_inConnectionComponent.setBounds(area);
}

void ConnectDAWsComponent::buttonClickedCallback(juce::Button *button,
                                                 bool success)
{
    if (button->getButtonText() == "Connect" && success)
    {
        //TODO: change port to be loaded from plugin config or something
        // auto *message = new StatusMessage("start", "something");
        // m_cmtMessenger->postMessage(message);
        sendAddressMessageToCMT(m_startConnectionComponent.getIP(), 7000);
        sendStatusMessageToCMT("start", "Try to connecto to remote...");
    }
    else if (button->getButtonText() == "Cancel" && success)
    {
        sendStatusMessageToCMT("stop", "Stop connection attempt");
    }
    else if (button->getButtonText() == "Continue" && success)
    {
        m_error = false;
        m_cmtMessenger->postMessage(
            new StatusMessage("ready", "Ready to connect"));
    }
    else if (button->getButtonText() == "Stop" && success)
    {
        sendStatusMessageToCMT("stop", "Stop connection attempt");
    }
    updateComponentVisibility(m_isConnected);
}

void ConnectDAWsComponent::updateComponentVisibility(bool isConnected)
{
    if (m_error)
    {
        m_errorComponent.setVisible(true);
        m_startConnectionComponent.setVisible(false);
        m_inConnectionComponent.setVisible(false);
    }
    else if (isConnected)
    {
        m_startConnectionComponent.setVisible(false);
        m_inConnectionComponent.setVisible(true);
    }
    else
    {
        m_startConnectionComponent.setVisible(true);
        m_inConnectionComponent.setVisible(false);
    }
}


void ConnectDAWsComponent::handleMessage(const juce::Message &message)
{
    if (auto *statusMessage = dynamic_cast<const StatusMessage *>(&message))
    {
        if (statusMessage->m_messageType == "status")
        {
            m_statusLabel.setText(statusMessage->m_message,
                                  juce::dontSendNotification);
            if (statusMessage->m_message == "Ready to connect")
            {
                m_isConnected = false;
            }
            else if (statusMessage->m_message == "Started stream")
            {
                m_inConnectionComponent.setButtonText("Stop");
                m_isConnected = true;
            }
            else if (statusMessage->m_message == "Stoped stream")
            {
                m_inConnectionComponent.setButtonText("Cancel");
                m_isConnected = false;
            }
            else if (statusMessage->m_message == "Failed to connect")
            {
                m_statusLabel.setText(
                    "Could not connect to remote with that IP",
                    juce::dontSendNotification);
                m_error = true;
                m_isConnected = false;
            }
            else if (statusMessage->m_message == "Failed to start stream")
            {
                m_statusLabel.setText(
                    "Could not start stream, check remote configuration",
                    juce::dontSendNotification);
                m_error = true;
                m_isConnected = false;
            }
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
    updateComponentVisibility(m_isConnected);
    repaint();
    m_forceResizeCallback();
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
