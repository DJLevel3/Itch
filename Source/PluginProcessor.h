/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SamplerSynthesizer.h"

struct MidiOnOff {
    int time = 0;
    int note = 0;
    bool on = false;
    bool transport = false;
    static bool sortTime(const MidiOnOff& a, const MidiOnOff& b)
    {
        return a.time < b.time;
    }
};

//==============================================================================
/**
*/
class ItchAudioProcessor  : public juce::AudioProcessor, public juce::AudioProcessorParameter::Listener, public juce::ChangeBroadcaster
{
public:
    //==============================================================================
    ItchAudioProcessor();
    ~ItchAudioProcessor() override;

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {};

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

    SamplerSynthesizer synth;
private:
    int lastSlotNum = 0;
    float lastScratch = 0;
    float lastLastScratch = 0;
    float lastVolume = 1;
    float lastLastVolume = 1;
    juce::AudioParameterInt* slotNum;
    juce::AudioParameterFloat* scratch;
    juce::AudioParameterFloat* volume;
    juce::AudioParameterFloat* slewRate;
    juce::AudioParameterFloat* slewCurve;
    juce::AudioParameterFloat* start;
    juce::AudioParameterFloat* end;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItchAudioProcessor)
};
