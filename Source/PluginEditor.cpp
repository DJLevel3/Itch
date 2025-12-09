/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ItchAudioProcessorEditor::ItchAudioProcessorEditor(ItchAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(BOX_W * 4 + 10, BOX_H * 3 + 10);
    setResizable(false, false);
    setTitle("Itch");

    addAndMakeVisible(loadButton);
    loadButton.addListener(this);

    addAndMakeVisible(nextSampleButton);
    nextSampleButton.addListener(this);

    addAndMakeVisible(prevSampleButton);
    prevSampleButton.addListener(this);

    addAndMakeVisible(ejectSampleButton);
    ejectSampleButton.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(128, 40, 40));
    ejectSampleButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(128, 40, 40));

    addAndMakeVisible(sampleNameBox);
    sampleNameBox.setJustificationType(juce::Justification::centred);
    sampleNameBox.setEditable(false, false);
    sampleNameBox.setColour(juce::Label::outlineColourId, getLookAndFeel().findColour(juce::Label::textColourId));

    updateSample();

    audioProcessor.addChangeListener(this);
}

ItchAudioProcessorEditor::~ItchAudioProcessorEditor()
{
    audioProcessor.removeChangeListener(this);
}

void ItchAudioProcessorEditor::buttonClicked(juce::Button* button) {
    if (button == &loadButton) {
        sampleChooser.launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this](const juce::FileChooser& chooser)
            {
                juce::File sampleFile(chooser.getResult());
                int result = loadFile(sampleFile);
                if (result >= 0) audioProcessor.synth.chooseSample(result);
                updateSample();
            });
    }
    else if (button == &nextSampleButton) {
        audioProcessor.synth.chooseNextSample();
        updateSample();
    }
    else if (button == &prevSampleButton) {
        audioProcessor.synth.choosePrevSample();
        updateSample();
    }
    else if (button == &ejectSampleButton) {
        audioProcessor.synth.unloadSample(audioProcessor.synth.getCurrentSample());
        audioProcessor.synth.chooseNextSample();
        updateSample();
    }
}

int ItchAudioProcessorEditor::loadFile(juce::File file) {
    if (file.getFileExtension() == ".wav" || file.getFileExtension() == ".flac") {
        int s = audioProcessor.synth.loadSample(file, 55.0, audioProcessor.synth.getOpenSample(), true);
        if (s >= 0) {
            return audioProcessor.synth.chooseSample(s);
        }
    }
    return -1;
}

//==============================================================================
void ItchAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    auto textArea = getLocalBounds().reduced(5).removeFromTop(BOX_H);

    g.setFont(juce::FontOptions(20.0f));
    g.drawFittedText("Itch v" + juce::String(ProjectInfo::versionString), textArea.removeFromLeft(BOX_W).reduced(5), juce::Justification::centred, 2, 1.0f);
}

void ItchAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    auto areaA = bounds.removeFromTop(BOX_H);

    areaA.removeFromLeft(BOX_W); // Title text
    loadButton.setBounds(areaA.removeFromLeft(BOX_W * 2).reduced(5));
    areaA.removeFromTop(BOX_H / 3);
    areaA.removeFromRight(BOX_W / 2);

    areaA = bounds.removeFromBottom(BOX_H);
    areaA.removeFromRight(BOX_W);
    nextSampleButton.setBounds(areaA.removeFromRight(BOX_W).reduced(5));
    prevSampleButton.setBounds(areaA.removeFromRight(BOX_W).reduced(5));
    ejectSampleButton.setBounds(areaA.removeFromLeft(BOX_W).reduced(5));

    areaA = bounds.removeFromBottom(BOX_H);
    areaA.removeFromRight(BOX_W);
    areaA.removeFromLeft(BOX_W);
    sampleNameBox.setBounds(areaA.reduced(5));
}

void ItchAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &audioProcessor) {
        updateSample();
    }
}

void ItchAudioProcessorEditor::updateSample() {
    if (audioProcessor.synth.getCurrentSampleName() == "Not Loaded") {
        sampleNameBox.setColour(juce::Label::textColourId, juce::Colour::fromRGB(192, 40, 40));
    }
    else {
        sampleNameBox.setColour(juce::Label::textColourId, getLookAndFeel().findColour(juce::Label::textColourId));

    }
    sampleNameBox.setText("Slot " + juce::String(audioProcessor.synth.getCurrentSample()) + " - " + audioProcessor.synth.getCurrentSampleName(), juce::dontSendNotification);
}
