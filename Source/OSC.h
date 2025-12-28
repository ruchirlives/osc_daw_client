/*
  ==============================================================================

    OSC.h
    Created: 27 Oct 2024 9:39:00pm
    Author:  Desktop

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_osc/juce_osc.h>

class OSCMulticastReceiver : public juce::OSCReceiver
{
public:
    OSCMulticastReceiver(const juce::String& multicastIP, int port)
        : multicastAddress(multicastIP), multicastPort(port)
    {
        // Bind to the port and join multicast group
        if (multicastSocket.bindToPort(multicastPort))
        {
            if (multicastSocket.joinMulticast(multicastAddress))
            {
                DBG("Successfully joined multicast group on " + multicastIP + ":" + juce::String(port));
            }
            else
            {
                DBG("Failed to join multicast group.");
            }
        }
        else
        {
            DBG("Failed to bind to port " + juce::String(port));
        }
    }

    ~OSCMulticastReceiver()
    {
        multicastSocket.leaveMulticast(multicastAddress);
    }

    // Method to poll the multicast socket on demand
    void pollMulticastSocket()
    {
        char buffer[1024];
        int bytesRead = 0;
        int lastBytesRead = 0;

        // Keep reading until the buffer is empty, but only keep the last message
        do
        {
            lastBytesRead = bytesRead;
            bytesRead = multicastSocket.read(buffer, sizeof(buffer), false);
        } while (bytesRead > 0);

        // Process only the most recent message
        if (lastBytesRead > 0)
        {
            DBG("Received latest data from multicast group: " + juce::String(lastBytesRead) + " bytes.");

            juce::MemoryInputStream stream(buffer, static_cast<size_t>(lastBytesRead), false);
            parseOSCMessage(stream);
        }
    }

    void parseOSCMessage(juce::MemoryInputStream& stream)
    {
        // Clear previous parsed message
        parsedOSCMessage.clear();

        // Read the OSC address pattern (null-terminated string, aligned to 4 bytes)
        juce::String addressPattern;
        addressPattern = readAlignedString(stream);
        parsedOSCMessage.addressPattern = addressPattern;
        DBG("Parsed OSC Address: " + parsedOSCMessage.addressPattern);

        // Read the type tag string (should start with a comma)
        juce::String typeTagString = readAlignedString(stream);
        if (!typeTagString.startsWithChar(','))
        {
            DBG("Invalid type tag string");
            return;
        }

        // Iterate over each type tag and read the corresponding argument
        for (int i = 1; i < typeTagString.length(); ++i) // Start from 1 to skip the comma
        {
            const auto typeTag = typeTagString[i];
            switch (typeTag)
            {
            case 's':
            {
                auto str = readAlignedString(stream);
                parsedOSCMessage.arguments.add(str);
                DBG("OSC Argument (String): " + str);
                break;
            }
            case 'f':
            {
                float floatValue = stream.readFloatBigEndian();
                parsedOSCMessage.arguments.add(juce::String(floatValue));
                DBG("OSC Argument (Float): " + juce::String(floatValue));
                break;
            }
            case 'i':
            {
                int intValue = stream.readIntBigEndian();
                parsedOSCMessage.arguments.add(juce::String(intValue));
                DBG("OSC Argument (Int): " + juce::String(intValue));
                break;
            }
            default:
            {
                DBG("OSC Argument (Unknown Type): " + juce::String(typeTag));
                break;
            }
            }
        }
    }

    // Helper function to read an OSC string and align to 4-byte boundaries
    juce::String readAlignedString(juce::MemoryInputStream& stream)
    {
        juce::String result;

        // Read characters until null terminator is found
        char c;
        while ((c = stream.readByte()) != '\0')
        {
            result += c;
        }

        // Skip padding bytes to maintain 4-byte alignment
        int padding = (4 - (result.length() + 1) % 4) % 4;
        stream.skipNextBytes(padding);

        return result;
    }


    // Getter method for the parsed OSC message
    const juce::String& getParsedOSCAddress() const
    {
        return parsedOSCMessage.addressPattern;
    }

    juce::StringArray getLatestTags()
    {
		DBG("Getting latest tags...");
        pollMulticastSocket();

        juce::StringArray tags;
        for (const auto& arg : parsedOSCMessage.arguments)
        {
            tags.add(arg);
        }
        return tags;
    }

private:
    struct ParsedOSCMessage
    {
        juce::String addressPattern;
        juce::Array<juce::String> arguments;

        void clear()
        {
            addressPattern.clear();
            arguments.clear();
        }
    } parsedOSCMessage;

    juce::DatagramSocket multicastSocket;
    juce::String multicastAddress;
    int multicastPort;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCMulticastReceiver)
};
