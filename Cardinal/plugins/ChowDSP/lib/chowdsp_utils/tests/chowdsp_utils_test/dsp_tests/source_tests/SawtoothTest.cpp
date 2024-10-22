#include <TimedUnitTest.h>

namespace
{
constexpr auto _sampleRate = 1000.0;
constexpr auto _blockSize = 512;
constexpr auto testFreq = 100.0f;
} // namespace

class SawtoothTest : public TimedUnitTest
{
public:
    SawtoothTest() : TimedUnitTest ("Sawtooth Test") {}

    void referenceTest()
    {
        // our osc has 1/2 sample delay so, run the reference osc at 2x sample rate, and check every other.
        dsp::Oscillator<float> refOsc { [] (auto x) { return x / MathConstants<float>::pi; } };
        refOsc.prepare ({ 2.0 * _sampleRate, (uint32) _blockSize, 1 });
        refOsc.setFrequency (testFreq, true);

        chowdsp::SawtoothWave<float> testOsc;
        testOsc.prepare ({ _sampleRate, (uint32) _blockSize, 1 });
        testOsc.setFrequency (testFreq);
        expectEquals (testOsc.getFrequency(), testFreq, "Set frequency is incorrect!");

        testOsc.processSample(); // for half-sample delay
        for (int i = 0; i < 20; ++i)
        {
            refOsc.processSample (0.0f);
            expectWithinAbsoluteError (testOsc.processSample(), refOsc.processSample (0.0f), 0.01f, "Generated sample is incorrect!");
        }
    }

    void simdReferenceTest()
    {
        // our osc has 1/2 sample delay so, run the reference osc at 2x sample rate, and check every other.
        dsp::Oscillator<float> refOsc { [] (auto x) { return x / MathConstants<float>::pi; } };
        refOsc.prepare ({ 2.0 * _sampleRate, (uint32) _blockSize, 1 });
        refOsc.setFrequency (testFreq, true);

        chowdsp::SawtoothWave<dsp::SIMDRegister<float>> testOsc;
        testOsc.prepare ({ _sampleRate, (uint32) _blockSize, 1 });
        testOsc.setFrequency (testFreq);
        expectEquals (testOsc.getFrequency().get (0), testFreq, "Set frequency is incorrect!");

        testOsc.processSample(); // for half-sample delay
        for (int i = 0; i < 20; ++i)
        {
            refOsc.processSample (0.0f);

            auto expOut = refOsc.processSample (0.0f);
            auto testOut = testOsc.processSample();
            expectWithinAbsoluteError (testOut.get (0), expOut, 0.01f, "Generated sample is incorrect!");
            expectWithinAbsoluteError (testOut.get (1), expOut, 0.01f, "Generated sample is incorrect!");
        }
    }

    void processReplacingTest()
    {
        dsp::Oscillator<float> refOsc { [] (auto x) { return x / MathConstants<float>::pi; } };
        refOsc.prepare ({ 2.0 * _sampleRate, (uint32) _blockSize, 1 });
        refOsc.setFrequency (testFreq, true);

        AudioBuffer<float> refBuffer (1, 40);
        FloatVectorOperations::fill (refBuffer.getWritePointer (0), 1.0f, 40);
        auto refBlock = dsp::AudioBlock<float> { refBuffer };
        refOsc.process (dsp::ProcessContextReplacing<float> { refBlock });

        chowdsp::SawtoothWave<float> testOsc;
        testOsc.prepare ({ _sampleRate, (uint32) _blockSize, 1 });
        testOsc.setFrequency (testFreq);

        AudioBuffer<float> testBuffer (1, 21);
        FloatVectorOperations::fill (testBuffer.getWritePointer (0), 1.0f, 21);
        auto testBlock = dsp::AudioBlock<float> { testBuffer };
        testOsc.process (dsp::ProcessContextReplacing<float> { testBlock });

        for (int i = 0; i < 19; ++i)
        {
            auto expOut = refBuffer.getSample (0, 2 * i + 1);
            auto actualOut = testBuffer.getSample (0, i + 1);
            expectWithinAbsoluteError (actualOut, expOut, 0.01f, "Generated sample is incorrect!");
        }
    }

    void processNonReplacingTest()
    {
        dsp::Oscillator<float> refOsc { [] (auto x) { return x / MathConstants<float>::pi; } };
        refOsc.prepare ({ 2.0 * _sampleRate, (uint32) _blockSize, 2 });
        refOsc.setFrequency (testFreq, true);

        AudioBuffer<float> inputBuffer1 (1, 40);
        FloatVectorOperations::fill (inputBuffer1.getWritePointer (0), 1.0f, 40);
        AudioBuffer<float> refBuffer (2, 40);
        auto refBlock = dsp::AudioBlock<float> { refBuffer };
        refOsc.process (dsp::ProcessContextNonReplacing<float> { inputBuffer1, refBlock });

        chowdsp::SawtoothWave<float> testOsc;
        testOsc.prepare ({ _sampleRate, (uint32) _blockSize, 2 });
        testOsc.setFrequency (testFreq);

        AudioBuffer<float> inputBuffer2 (1, 21);
        FloatVectorOperations::fill (inputBuffer2.getWritePointer (0), 1.0f, 21);
        AudioBuffer<float> testBuffer (2, 21);
        auto testBlock = dsp::AudioBlock<float> { testBuffer };
        testOsc.process (dsp::ProcessContextNonReplacing<float> { inputBuffer2, testBlock });

        for (int i = 0; i < 19; ++i)
        {
            auto expOutL = refBuffer.getSample (0, 2 * i + 1);
            auto actualOutL = testBuffer.getSample (0, i + 1);
            expectWithinAbsoluteError (actualOutL, expOutL, 0.01f, "Generated sample is incorrect!");

            auto expOutR = refBuffer.getSample (1, 2 * i + 1);
            auto actualOutR = testBuffer.getSample (1, i + 1);
            expectWithinAbsoluteError (actualOutR, expOutR, 0.01f, "Generated sample is incorrect!");
        }
    }

    void zeroHzTest()
    {
        chowdsp::SawtoothWave<float> testOsc;
        testOsc.prepare ({ _sampleRate, (uint32) _blockSize, 1 });
        testOsc.setFrequency (0.0f);

        for (int i = 0; i < 10; ++i)
            expectEquals (testOsc.processSample(), 0.0f, "Zero Hz output is non-zero!");
    }

    void bypassTest()
    {
        dsp::Oscillator<float> refOsc { [] (auto x) { return x / MathConstants<float>::pi; } };
        refOsc.prepare ({ 2.0 * _sampleRate, (uint32) _blockSize, 1 });
        refOsc.setFrequency (testFreq, true);

        AudioBuffer<float> refBuffer (1, 40);
        FloatVectorOperations::fill (refBuffer.getWritePointer (0), 1.0f, 40);
        auto refBlock = dsp::AudioBlock<float> { refBuffer };
        refOsc.process (dsp::ProcessContextReplacing<float> { refBlock });

        chowdsp::SawtoothWave<float> testOsc;
        testOsc.prepare ({ _sampleRate, (uint32) _blockSize, 1 });
        testOsc.setFrequency (testFreq);

        AudioBuffer<float> testBuffer (1, 21);
        FloatVectorOperations::fill (testBuffer.getWritePointer (0), 1.0f, 21);

        auto testBlock = dsp::AudioBlock<float> { testBuffer };
        auto&& block1 = testBlock.getSubBlock (0, 11);
        auto&& ctx1 = dsp::ProcessContextReplacing<float> (block1);
        ctx1.isBypassed = true;
        testOsc.process (ctx1);

        auto&& block2 = testBlock.getSubBlock (11, 10);
        auto&& ctx2 = dsp::ProcessContextReplacing<float> (block2);
        ctx2.isBypassed = false;
        testOsc.process (ctx2);

        int i = 0;
        for (; i < 10; ++i)
        {
            auto actualOut = testBuffer.getSample (0, i + 1);
            expectWithinAbsoluteError (actualOut, 1.0f, 0.01f, "Bypassed sample is incorrect!");
        }

        for (; i < 19; ++i)
        {
            auto expOut = refBuffer.getSample (0, 2 * i + 1);
            auto actualOut = testBuffer.getSample (0, i + 1);
            expectWithinAbsoluteError (actualOut, expOut, 0.01f, "Generated sample is incorrect!");
        }
    }

    void runTestTimed() override
    {
        beginTest ("Reference Test");
        referenceTest();

        beginTest ("SIMD Reference Test");
        simdReferenceTest();

        beginTest ("Process Replacing Test");
        processReplacingTest();

        beginTest ("Process Non-Replacing Test");
        processNonReplacingTest();

        beginTest ("Zero Hz Test");
        zeroHzTest();

        beginTest ("Bypass Test");
        bypassTest();
    }
};

static SawtoothTest sawtoothTest;
