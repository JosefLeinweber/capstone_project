#pragma once


#include <juce_gui_basics/juce_gui_basics.h>

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent();

    void paint(juce::Graphics &) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
