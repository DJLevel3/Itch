/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ItchAudioProcessor::ItchAudioProcessor()
{
    synth.chooseSample(0);
    addParameter(slotNum = new juce::AudioParameterInt("slotNum", "Slot #", 0, MAX_SAMPLES - 1, 0));
    addParameter(scratch = new juce::AudioParameterFloat("scratch", "Scratch", 0, 1, 0));
    addParameter(volume = new juce::AudioParameterFloat("volume", "Volume", 0, 1, 1));
    addParameter(slewRate = new juce::AudioParameterFloat("slewRate", "Slew Rate", 0, 1, 0.5));
    addParameter(slewCurve = new juce::AudioParameterFloat("slewCurve", "Slew Curve", 0, 1, 0));
    addParameter(start = new juce::AudioParameterFloat("start", "Scratch Start", 0, 1, 0));
    addParameter(end = new juce::AudioParameterFloat("end", "Scratch End", 0, 1, 1));

    slotNum->addListener(this);
    scratch->addListener(this);
    volume->addListener(this);
}

ItchAudioProcessor::~ItchAudioProcessor()
{
}


void ItchAudioProcessor::parameterValueChanged(int parameterIndex, float newValue) {
    if (parameterIndex == slotNum->getParameterIndex()) {
        if (*slotNum != lastSlotNum) {
            synth.chooseSample(*slotNum);
            lastSlotNum = *slotNum;
            sendChangeMessage();
        }
    }
    else if (parameterIndex == scratch->getParameterIndex()) {
        lastScratch = *scratch;
    }
    else if (parameterIndex == volume->getParameterIndex()) {
        lastVolume = *volume;
    }
    else {
        return;
    }
}

//==============================================================================
const juce::String ItchAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ItchAudioProcessor::acceptsMidi() const
{
    return true;
}

bool ItchAudioProcessor::producesMidi() const
{
    return false;
}

bool ItchAudioProcessor::isMidiEffect() const
{
    return false;
}

double ItchAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ItchAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int ItchAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ItchAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String ItchAudioProcessor::getProgramName(int index)
{
    return {};
}

void ItchAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void ItchAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.prepareToPlay(sampleRate);
}

void ItchAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void ItchAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    int totalNumInputChannels = getTotalNumInputChannels();
    int totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());


    std::vector<MidiOnOff> mid;
    MidiOnOff tempMid;
    juce::AudioPlayHead* transport = getPlayHead();
    auto transportState = transport->getPosition();

    int numSamples = buffer.getNumSamples();
    float targetScratch = lastLastScratch;
    float targetVolume = lastVolume;
    float delta = powf(abs(targetScratch - lastScratch) * (*slewCurve * (sqrtf(2.f) - 1) + 1.f), *slewCurve + 1.f) * juce::jmin(*slewRate * numSamples * 48 / getSampleRate(), 1.0);
    if (targetScratch < lastScratch) {
        targetScratch += delta;
    }
    else if (targetScratch > lastScratch) {
        targetScratch -= delta;
    }

    synth.processBlock(buffer, 0, buffer.getNumSamples(), lastLastScratch * (*end - *start) + *start , targetScratch * (*end - *start) + *start, lastLastVolume, targetVolume);
    lastLastScratch = targetScratch;
    lastLastVolume = targetVolume;

}

//==============================================================================
bool ItchAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ItchAudioProcessor::createEditor()
{
    return new ItchAudioProcessorEditor(*this);
}

//==============================================================================
void ItchAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    std::unique_ptr<juce::XmlElement> itchState(new juce::XmlElement("Itch"));
    itchState->setAttribute("slotNum", *slotNum);
    itchState->setAttribute("lastSlotNum", lastSlotNum);

    itchState->setAttribute("scratch", *scratch);
    itchState->setAttribute("lastScratch", lastScratch);
    itchState->setAttribute("lastLastScratch", lastLastScratch);

    itchState->setAttribute("volume", *volume);
    itchState->setAttribute("lastVolume", lastVolume);
    itchState->setAttribute("lastLastVolume", lastLastVolume);

    itchState->setAttribute("slewRate", *slewRate);

    itchState->setAttribute("start", *start);
    itchState->setAttribute("end", *end);

    synth.getXmlState(itchState.get());
    copyXmlToBinary(*itchState, destData);
}

void ItchAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> itchState(getXmlFromBinary(data, sizeInBytes));
    if (itchState.get() != nullptr) {
        if (itchState->hasTagName("Itch")) {
            *slotNum = itchState->getIntAttribute("slotNum", 0);
            lastSlotNum = itchState->getIntAttribute("lastSlotNum", 0);

            *scratch = itchState->getDoubleAttribute("scratch", 0);
            lastScratch = itchState->getDoubleAttribute("lastScratch", 0);
            lastLastScratch = itchState->getDoubleAttribute("lastLastScratch", 0);

            *volume = itchState->getDoubleAttribute("volume", 1);
            lastVolume = itchState->getDoubleAttribute("lastVolume", 1);
            lastLastVolume = itchState->getDoubleAttribute("lastLastVolume", 1);

            *slewRate = itchState->getDoubleAttribute("slewRate", 0.25);

            *start = itchState->getDoubleAttribute("start", 0);
            *end = itchState->getDoubleAttribute("end", 1);

            synth.loadXmlState(itchState->getChildByName("Synth"));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ItchAudioProcessor();
}