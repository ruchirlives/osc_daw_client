/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OSC_ClientAudioProcessor::OSC_ClientAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    :  AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
        
    ), receiver("239.255.0.1", 9000)
#endif
{
    DBG("OSC Client Plugin Constructor");

    bool success = oscSender.connect("127.0.0.1", 8000);
    if (success)
    {
        DBG("Connected to OSC server");
    }
    else
    {
        DBG("Failed to connect to OSC server");
    }

}


OSC_ClientAudioProcessor::~OSC_ClientAudioProcessor()
{
}

//==============================================================================
const juce::String OSC_ClientAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OSC_ClientAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool OSC_ClientAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool OSC_ClientAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double OSC_ClientAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OSC_ClientAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int OSC_ClientAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OSC_ClientAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String OSC_ClientAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void OSC_ClientAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void OSC_ClientAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    DBG("prepareToPlay called with sampleRate: " << sampleRate << ", samplesPerBlock: " << samplesPerBlock);

}

void OSC_ClientAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OSC_ClientAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

void OSC_ClientAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(buffer);
    juce::MidiMessage message;

    for (const auto meta : midiMessages)
    {
        message = meta.getMessage();

        DBG("Received MIDI message: " << message.getDescription()); // Add this line


        if (message.isNoteOnOrOff() || message.isController())
        {
            sendOscMessage(message);
        }
    }
}

void OSC_ClientAudioProcessor::setTags(const juce::String& tagsString)
{
    // split by newline
	tags = juce::StringArray::fromLines(tagsString);
	DBG("Tags set to: " << tagsString);

}

juce::String OSC_ClientAudioProcessor::getTags()
{
    juce::String tagsText = tags.joinIntoString("\n");

	DBG("Tags set to: " << tagsText);
	return tagsText;
}

void OSC_ClientAudioProcessor::sendOscMessage(const juce::MidiMessage& message)
{
    juce::OSCMessage oscMessage = createOscMessage(message);

    if (tags.isEmpty())
    {
        DBG("Skipping OSC send: no tags configured to target an instrument.");
        return;
    }

    for (const auto& tag : tags)
    {
        oscMessage.addString(tag);
    }

    // Attempt to send the message
    if (oscSender.send(oscMessage))
    {
        DBG("OSC message sent successfully: " << oscMessage.getAddressPattern().toString());
    }
    else
    {
        DBG("Failed to send OSC message");
        jassertfalse; // Assertion failure on failed send
    }
}


juce::OSCMessage OSC_ClientAudioProcessor::createOscMessage(const juce::MidiMessage& message)
{
	// Get current time in milliseconds
	const double now = juce::Time::getMillisecondCounterHiRes();
    const float timestamp = static_cast<float>(now);

    if (message.isNoteOn())
    {
        return juce::OSCMessage("/midi/message", juce::String("note_on"), message.getNoteNumber(), message.getVelocity(), timestamp);
    }
    else if (message.isNoteOff())
    {
        return juce::OSCMessage("/midi/message", juce::String("note_off"), message.getNoteNumber(), timestamp);
    }
    else if (message.isController())
    {
        return juce::OSCMessage("/midi/message", juce::String("controller"), message.getControllerNumber(), message.getControllerValue(), timestamp);
    }

    return juce::OSCMessage("/null");
}

juce::String OSC_ClientAudioProcessor::getIpAddress()
{
    return juce::String(ipAddress);
}

void OSC_ClientAudioProcessor::setIpAddress(const juce::String& newIpAddress)
{
	this->ipAddress = newIpAddress;
}

int OSC_ClientAudioProcessor::getPort()
{
	return port;
}

void OSC_ClientAudioProcessor::setPort(int newPort)
{
	this->port = newPort;
}



//==============================================================================
bool OSC_ClientAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* OSC_ClientAudioProcessor::createEditor()
{
    return new OSC_ClientAudioProcessorEditor(*this);
}

//==============================================================================
void OSC_ClientAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ValueTree state("OSCClientState");
    state.setProperty("IPAddress", ipAddress, nullptr);
    state.setProperty("Port", port, nullptr);
    state.setProperty("Tags", tags.joinIntoString("\n"), nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OSC_ClientAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr)
    {
        juce::ValueTree state = juce::ValueTree::fromXml(*xmlState);
        if (state.isValid())
        {
            ipAddress = state.getProperty("IPAddress").toString();
            port = state.getProperty("Port");
            setTags(state.getProperty("Tags").toString());
        }
    }
}

void OSC_ClientAudioProcessor::reConnect()
{
	// first disconnect
	oscSender.disconnect();

	bool success = oscSender.connect(ipAddress, port);
	if (success)
	{
		DBG("Connected to OSC server " << juce::String(ipAddress) << ":" << juce::String(port));
	}
	else
	{
		DBG("Failed to connect to OSC server");
	}
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new OSC_ClientAudioProcessor(); }
