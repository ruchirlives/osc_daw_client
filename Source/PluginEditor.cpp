/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
OSC_ClientAudioProcessorEditor::OSC_ClientAudioProcessorEditor (OSC_ClientAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&globalLookAndFeel);
    setSize (480, 320);

	const juce::Font headingFont("Segoe UI", 16.0f, juce::Font::bold);
	const juce::Font labelFont("Segoe UI", 13.0f, juce::Font::bold);
	const juce::Font editorFont("Segoe UI", 14.0f, juce::Font::plain);

	addAndMakeVisible(label);
	label.setText("Tags", juce::dontSendNotification);
	label.setFont(headingFont);
	label.setColour(juce::Label::textColourId, juce::Colours::whitesmoke);

	addAndMakeVisible(textEditor);
	textEditor.setMultiLine(true);
	textEditor.setReturnKeyStartsNewLine(true);
	textEditor.setReadOnly(false);
	textEditor.setScrollbarsShown(true);
	textEditor.addListener(this);
	textEditor.setWantsKeyboardFocus(true);
	textEditor.setFont(editorFont);
	textEditor.setJustification(juce::Justification::topLeft);

	textEditor.setText(audioProcessor.getTags());

	addAndMakeVisible(ipAddressLabel);
	ipAddressLabel.setText("IP Address", juce::dontSendNotification);
	ipAddressLabel.setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
	ipAddressLabel.setFont(labelFont);

	addAndMakeVisible(ipAddressEditor);
	ipAddressEditor.setMultiLine(false);
	ipAddressEditor.setReturnKeyStartsNewLine(false);
	ipAddressEditor.setReadOnly(false);
	ipAddressEditor.setScrollbarsShown(false);
	ipAddressEditor.addListener(this);
	ipAddressEditor.setFont(editorFont);
	ipAddressEditor.setJustification(juce::Justification::centredLeft);
	ipAddressEditor.setText(audioProcessor.getIpAddress());

	addAndMakeVisible(portLabel);
	portLabel.setText("Port", juce::dontSendNotification);
	portLabel.setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
	portLabel.setFont(labelFont);

	addAndMakeVisible(portEditor);
	portEditor.setMultiLine(false);
	portEditor.setReturnKeyStartsNewLine(false);
	portEditor.setReadOnly(false);
	portEditor.setScrollbarsShown(false);
	portEditor.addListener(this);
	portEditor.setFont(editorFont);
	portEditor.setJustification(juce::Justification::centredLeft);
	portEditor.setText(juce::String(audioProcessor.getPort()));

	addAndMakeVisible(reconnectButton);
	reconnectButton.setButtonText("Reconnect");
	reconnectButton.setColour(juce::TextButton::buttonColourId, globalLookAndFeel.getPanelColour().brighter(0.1f));
	reconnectButton.onClick = [this]() { audioProcessor.reConnect(); };

	addAndMakeVisible(getTagsButton);
	getTagsButton.setButtonText("Get Tags");
	getTagsButton.setColour(juce::TextButton::buttonColourId, globalLookAndFeel.getAccentColour());
	getTagsButton.onClick = [this]() 
		{ 
			auto& tags = audioProcessor.receiver.getLatestTags();
			// convert StringArray to new line separated string list
			juce::String tagsAsString = tags.joinIntoString("\n");
			
			textEditor.setText(tagsAsString);
			audioProcessor.setTags(tagsAsString);
				
		};

}



void OSC_ClientAudioProcessorEditor::textEditorFocusLost(juce::TextEditor& textEditor)
{
	if (&textEditor == &ipAddressEditor)
	{
		// Get the text from the text editor
		auto text = ipAddressEditor.getText();
		// Send the text to the processor
		audioProcessor.setIpAddress(text);
	}
	else if (&textEditor == &portEditor)
	{
		// Get the text from the text editor
		auto text = portEditor.getText();
		// Send the text to the processor
		audioProcessor.setPort(text.getIntValue());
	}
	else if (&textEditor == &this->textEditor)
	{
		// Get the text from the text editor
		auto text = textEditor.getText();
		// Send the text to the processor
		audioProcessor.setTags(text);
	}
}

OSC_ClientAudioProcessorEditor::~OSC_ClientAudioProcessorEditor()
{
	setLookAndFeel(nullptr);
}

//==============================================================================
void OSC_ClientAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (globalLookAndFeel.getBaseColour());

    auto bounds = getLocalBounds().reduced(16).toFloat();
    juce::DropShadow shadow(globalLookAndFeel.getShadowColour(), 12, { 0, 4 });
    shadow.drawForRectangle(g, bounds.toNearestInt());

    juce::ColourGradient gradient(globalLookAndFeel.getPanelColour().brighter(0.12f),
                                  bounds.getX(), bounds.getY(),
                                  globalLookAndFeel.getPanelColour().darker(0.08f),
                                  bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(gradient);
    g.fillRoundedRectangle(bounds, 12.0f);

    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.drawRoundedRectangle(bounds, 12.0f, 1.5f);

}

void OSC_ClientAudioProcessorEditor::resized()
{
	auto bounds = getLocalBounds().reduced(24);

	auto buttonRow = bounds.removeFromBottom(40);
	bounds.removeFromBottom(16);

	auto connectionArea = bounds.removeFromBottom(74);
	bounds.removeFromBottom(12);

	auto headerArea = bounds.removeFromTop(30);
	label.setBounds(headerArea.removeFromLeft(180));

	bounds.removeFromTop(8);
	auto tagsArea = bounds;
	textEditor.setBounds(tagsArea);

	auto columnWidth = (connectionArea.getWidth() - 16) / 2;
	auto ipColumn = connectionArea.removeFromLeft(columnWidth);
	connectionArea.removeFromLeft(16);
	auto portColumn = connectionArea;

	auto labelHeight = 20;
	auto editorHeight = 32;

	ipAddressLabel.setBounds(ipColumn.removeFromTop(labelHeight));
	ipColumn.removeFromTop(4);
	ipAddressEditor.setBounds(ipColumn.removeFromTop(editorHeight));

	portLabel.setBounds(portColumn.removeFromTop(labelHeight));
	portColumn.removeFromTop(4);
	portEditor.setBounds(portColumn.removeFromTop(editorHeight));

	auto buttonWidth = 150;

	reconnectButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
	buttonRow.removeFromLeft(16);
	getTagsButton.setBounds(buttonRow.removeFromLeft(buttonWidth));

}
