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


    statusLabel.setText("Status: Loading...", juce::dontSendNotification);
    addAndMakeVisible(statusLabel);

    localIpAndPortLabel.setText("Local IP and Port: Loading...",
                                juce::dontSendNotification);
    addAndMakeVisible(localIpAndPortLabel);

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
    m_startConnectionComponent.setBounds(area);
    m_inConnectionComponent.setBounds(area);
    ipEditor.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
    portEditor.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
    statusLabel.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
    localIpAndPortLabel.setBounds(
        area.removeFromTop(textFieldHeight).reduced(0, 5));
}

void ConnectDAWsComponent::buttonClickedCallback(juce::Button *button,
                                                 bool success)
{
    if (button->getButtonText() == "Connect" && success)
    {
        m_isConnected = true;
        m_startConnectionComponent.setVisible(false);
        m_inConnectionComponent.setVisible(true);
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
