#include "ConnectDAWsComponents/mainComponent.h"

MainComponent::MainComponent()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint(juce::Graphics &g)
{
}

void MainComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

    // In this example, we'll set the size of our child component to be the same
    // as our own size.
    auto area = getLocalBounds();
}
