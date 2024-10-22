#pragma once

namespace chowdsp
{
/**
 * Base class for synth plugin processors.
 * 
 * Derived classes must override `prepareToPlay` and `releaseResources`
 * (from `juce::AudioProcessor`), as well as `processSynth`, and
 * `addParameters`.
*/
template <class Processor>
class SynthBase : public PluginBase<Processor>
{
public:
    explicit SynthBase (const juce::AudioProcessor::BusesProperties& layout = juce::AudioProcessor::BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)) : PluginBase<Processor> (layout)
    {
    }
    ~SynthBase() override = default;

    bool acceptsMidi() const override { return true; }

    virtual void processSynth (juce::AudioBuffer<float>&, juce::MidiBuffer&) = 0;

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override
    {
        juce::ScopedNoDenormals noDenormals;

        buffer.clear();

#if CHOWDSP_USE_FOLEYS_CLASSES
        this->magicState.processMidiBuffer (midi, buffer.getNumSamples(), true);
#endif

        processSynth (buffer, midi);
    }

private:
    void processAudioBlock (juce::AudioBuffer<float>&) override {}
};

} // namespace chowdsp
