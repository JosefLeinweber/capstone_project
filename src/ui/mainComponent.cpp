#include "ConnectDAWsComponents/mainComponent.h"

MainComponent::MainComponent(std::shared_ptr<Messenger> &guiMessenger,
                             std::shared_ptr<Messenger> &cmtMessenger)
    : m_guiMessenger(guiMessenger), m_cmtMessenger(cmtMessenger)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    initGUIMessenger();
    constexpr auto HEIGHT = 500;
    constexpr auto WIDTH = 500;

    // Set up IP text editor
    ipLabel.attachToComponent(&ipEditor, true);
    ipEditor.setMultiLine(false);
    ipEditor.setTextToShowWhenEmpty("Enter IP address", juce::Colours::grey);
    addAndMakeVisible(ipEditor);

    // Set up Port text editor
    portLabel.attachToComponent(&portEditor, true);
    portEditor.setMultiLine(false);
    portEditor.setInputFilter(
        new juce::TextEditor::LengthAndCharacterRestriction(5, "0123456789"),
        true);
    portEditor.setTextToShowWhenEmpty("Enter Port", juce::Colours::grey);
    addAndMakeVisible(portEditor);

    // Set up Submit button
    sendButton.setButtonText("Connect");
    sendButton.addListener(this);
    addAndMakeVisible(sendButton);

    statusLabel.setText("Status: Loading...", juce::dontSendNotification);
    addAndMakeVisible(statusLabel);

    localIpAndPortLabel.setText("Local IP and Port: Loading...",
                                juce::dontSendNotification);
    addAndMakeVisible(localIpAndPortLabel);
    addAndMakeVisible(stopButton);
    stopButton.addListener(this);
    stopButton.setButtonText("Stop Connection");


    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setSize(WIDTH, HEIGHT);
}

MainComponent::~MainComponent()
{
    sendButton.removeListener(this);
    stopButton.removeListener(this);
}

void MainComponent::paint(juce::Graphics &g)
{
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
}

void MainComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

    // In this example, we'll set the size of our child component to be the same
    // as our own size.
    auto area = getLocalBounds();
    auto textFieldHeight = 30;

    ipEditor.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
    portEditor.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
    sendButton.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
    statusLabel.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
    localIpAndPortLabel.setBounds(
        area.removeFromTop(textFieldHeight).reduced(0, 5));
    stopButton.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
}

void MainComponent::buttonClicked(juce::Button *button)
{
    if (button == &sendButton)
    {
        std::string ip = ipEditor.getText().toStdString();
        int port = portEditor.getText().getIntValue();
        MessageToCMT *message = new MessageToCMT(ip, port);
        m_cmtMessenger->postMessage(message);
    }
    else if (button == &stopButton)
    {
        MessageToCMT *message = new MessageToCMT("stop", 1000);
        m_cmtMessenger->postMessage(message);
    }
}

void MainComponent::handleMessage(const juce::Message &message)
{
    if (auto *m = dynamic_cast<const MessageToGUI *>(&message))
    {
        if (m->m_messageType == "status")
        {
            statusLabel.setText(m->m_message, juce::dontSendNotification);
        }
        else if (m->m_messageType == "localIpAndPort")
        {
            localIpAndPortLabel.setText(m->m_message,
                                        juce::dontSendNotification);
        }
        else
        {
            std::cout << "MainComponent::handleMessage | Received "
                         "unknown message type from CMT"
                      << std::endl;
        }
    }
    else
    {
        std::cout << "MainComponent::handleMessage | Received "
                     "unknown message from CMT"
                  << std::endl;
    }
}

void MainComponent::initGUIMessenger()
{
    m_guiMessenger = std::make_shared<Messenger>(
        std::bind(&MainComponent::handleMessage, this, std::placeholders::_1));
}
