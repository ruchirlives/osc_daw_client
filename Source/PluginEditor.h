/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class GlobalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GlobalLookAndFeel()
        : base(juce::Colours::darkslategrey.darker(0.25f)),
          panel(base.brighter(0.1f)),
          accent(juce::Colour::fromRGB(90, 224, 255)),
          shadowColour(juce::Colours::black.withAlpha(0.35f))
    {
        setColour(juce::ResizableWindow::backgroundColourId, base);
        setColour(juce::TextButton::buttonColourId, panel.brighter(0.08f));
        setColour(juce::TextButton::buttonOnColourId, accent);
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::TextButton::textColourOnId, juce::Colours::white);

        setColour(juce::ComboBox::backgroundColourId, base.brighter(0.1f));
        setColour(juce::ComboBox::outlineColourId, juce::Colours::white.withAlpha(0.25f));
        setColour(juce::ComboBox::textColourId, juce::Colours::white);

        setColour(juce::Label::textColourId, juce::Colours::whitesmoke);

        setColour(juce::PopupMenu::backgroundColourId, base);
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, accent.withAlpha(0.35f));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);

        setColour(juce::Slider::thumbColourId, accent);
        setColour(juce::Slider::trackColourId, panel.brighter(0.2f));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::white.withAlpha(0.2f));

        setColour(juce::TextEditor::backgroundColourId, base.darker(0.5f));
        setColour(juce::TextEditor::outlineColourId, juce::Colours::white.withAlpha(0.3f));
        setColour(juce::TextEditor::textColourId, juce::Colours::white);

        setColour(juce::ListBox::backgroundColourId, panel.darker(0.08f));
        setColour(juce::ListBox::outlineColourId, juce::Colours::white.withAlpha(0.15f));

        setDefaultSansSerifTypefaceName("Segoe UI");
    }

    juce::Colour getBaseColour() const { return base; }
    juce::Colour getPanelColour() const { return panel; }
    juce::Colour getAccentColour() const { return accent; }
    juce::Colour getShadowColour() const { return shadowColour; }

    void drawTextEditorOutline(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override
    {
        juce::ignoreUnused(width, height);
        auto outlineColour = textEditor.findColour(juce::TextEditor::outlineColourId);
        g.setColour(outlineColour);
        g.drawRoundedRectangle(textEditor.getLocalBounds().toFloat(), 6.0f, 1.5f);
    }

    void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override
    {
        juce::ignoreUnused(width, height);
        auto bg = textEditor.findColour(juce::TextEditor::backgroundColourId);
        g.setColour(bg);
        g.fillRoundedRectangle(textEditor.getLocalBounds().toFloat(), 6.0f);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        juce::DropShadow shadow(shadowColour, 4, { 2, 2 });
        shadow.drawForRectangle(g, bounds.toNearestInt());

        auto baseColour = backgroundColour.interpolatedWith(juce::Colours::black, isButtonDown ? 0.25f : 0.0f);
        if (isMouseOverButton)
            baseColour = baseColour.brighter(0.05f);

        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, 6.0f);

        g.setColour(juce::Colours::white.withAlpha(0.12f));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool isMouseOverButton,
                        bool isButtonDown) override
    {
        juce::ignoreUnused(isMouseOverButton, isButtonDown);
        juce::Font font(juce::FontOptions(14.0f, juce::Font::bold));
        g.setFont(font);
        g.setColour(button.findColour(juce::TextButton::textColourOffId));
        g.drawFittedText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, 1);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        auto bounds = button.getLocalBounds().toFloat().reduced(4);
        g.setColour(button.getToggleState() ? accent : panel);
        g.fillRoundedRectangle(bounds, 6.0f);

        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

        juce::Font font(juce::FontOptions(12.0f, juce::Font::bold));
        g.setFont(font);
        g.setColour(juce::Colours::white);
        g.drawFittedText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, 1);
    }

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        g.setColour(findColour(juce::PopupMenu::backgroundColourId));
        g.fillRoundedRectangle(0.0f, 0.0f, (float) width, (float) height, 6.0f);
        g.setColour(juce::Colours::white.withAlpha(0.15f));
        g.drawRoundedRectangle(0.0f, 0.0f, (float) width, (float) height, 6.0f, 1.0f);
    }

private:
    const juce::Colour base;
    const juce::Colour panel;
    const juce::Colour accent;
    const juce::Colour shadowColour;
};

class OSC_ClientAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::TextEditor::Listener
{
public:
	// Add the listener to the class
	void textEditorFocusLost(juce::TextEditor& editor) override;

    OSC_ClientAudioProcessorEditor (OSC_ClientAudioProcessor&);
    ~OSC_ClientAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    OSC_ClientAudioProcessor& audioProcessor;

	// Create a TextEditor component
	juce::TextEditor textEditor;

    // Create a label
	juce::Label label;

	// Create labels and text boxes for the IP address and port
	juce::Label ipAddressLabel;
	juce::TextEditor ipAddressEditor;
	juce::Label portLabel;
	juce::TextEditor portEditor;

	// Create a button for IP and port reconnection
	juce::TextButton reconnectButton;

	// Create a button to get the latest tags
	juce::TextButton getTagsButton;

    // About button
    juce::TextButton aboutButton;

	GlobalLookAndFeel globalLookAndFeel;

    void showAboutDialog();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSC_ClientAudioProcessorEditor)
};
