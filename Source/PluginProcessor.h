/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "OSC.h"

//==============================================================================
/**
*/
class OSC_ClientAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    OSC_ClientAudioProcessor();
    ~OSC_ClientAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

	// ==============================================================================
    juce::String getTags();
    void setTags(const juce::String& tagsString);

    void sendOscMessage(const juce::MidiMessage& message);
    juce::OSCMessage createOscMessage(const juce::MidiMessage& message);

	// Get and set ip address and port
	juce::String getIpAddress();
	void setIpAddress(const juce::String& newIpAddress);
	int getPort();
	void setPort(int newPort);



    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	//==============================================================================
	void reConnect();

    // Create receiver
    OSCMulticastReceiver receiver;

private:
    juce::OSCSender oscSender;
    juce::StringArray tags = { juce::String("piano") }; // Explicit juce::String for clarity
    juce::String lastDebugMessage;

	// IP address and port
    juce::String ipAddress = "127.0.0.1";
	int port = 8000;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSC_ClientAudioProcessor)
};
