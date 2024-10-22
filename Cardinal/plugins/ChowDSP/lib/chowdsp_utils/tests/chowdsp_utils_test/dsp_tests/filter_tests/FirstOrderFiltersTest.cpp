#include <test_utils.h>
#include <TimedUnitTest.h>

namespace Constants
{
constexpr float fs = 48000.0f;
constexpr float fc = 500.0f;
} // namespace Constants

class FirstOrderFiltersTest : public TimedUnitTest
{
public:
    FirstOrderFiltersTest() : TimedUnitTest ("First Order Filters Test", "Filters")
    {
    }

    template <typename T, typename FilterType, typename NumericType = typename chowdsp::SampleTypeHelpers::ElementType<T>::Type>
    void testFrequency (FilterType& filter, NumericType freq, NumericType expGainDB, NumericType maxError, const String& message)
    {
        auto buffer = test_utils::makeSineWave<NumericType> (freq, Constants::fs, (NumericType) 1);

        HeapBlock<char> dataBlock;
        auto block = test_utils::bufferToBlock<T> (dataBlock, buffer);

        filter.reset();
        filter.processBlock (block.getChannelPointer (0), buffer.getNumSamples());

        test_utils::blockToBuffer (buffer, block);
        const auto halfSamples = buffer.getNumSamples() / 2;
        auto magDB = Decibels::gainToDecibels (buffer.getMagnitude (halfSamples, halfSamples));
        expectWithinAbsoluteError (magDB, expGainDB, maxError, message);
    }

    template <typename T, typename NumericType = typename chowdsp::SampleTypeHelpers::ElementType<T>::Type>
    void firstOrderLPFTest (NumericType maxError)
    {
        chowdsp::FirstOrderLPF<T> lpFilter;
        lpFilter.calcCoefs ((T) Constants::fc, (NumericType) Constants::fs);

        testFrequency<T> (lpFilter, (NumericType) 1, (NumericType) 0, maxError, "Incorrect gain at low frequencies.");
        testFrequency<T> (lpFilter, (NumericType) Constants::fc, (NumericType) -3.01, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (lpFilter, Constants::fc * (NumericType) 4, (NumericType) -12.3, (NumericType) 0.1, "Incorrect gain at high frequencies.");
    }

    template <typename T, typename NumericType = typename chowdsp::SampleTypeHelpers::ElementType<T>::Type>
    void firstOrderHPFTest (NumericType maxError)
    {
        chowdsp::FirstOrderHPF<T> hpFilter;
        hpFilter.calcCoefs ((T) Constants::fc, (NumericType) Constants::fs);

        testFrequency<T> (hpFilter, (NumericType) Constants::fc / 4, (NumericType) -12.3, (NumericType) 0.1, "Incorrect gain at low frequencies.");
        testFrequency<T> (hpFilter, (NumericType) Constants::fc, (NumericType) -3, maxError, "Incorrect gain at cutoff frequency.");
        testFrequency<T> (hpFilter, (NumericType) Constants::fs * (NumericType) 0.498, (NumericType) 0, maxError, "Incorrect gain at high frequencies.");
    }

    void runTestTimed() override
    {
        beginTest ("First Order LPF Test");
        firstOrderLPFTest<float> (1.0e-2f);
        firstOrderLPFTest<double> (1.0e-2);

        beginTest ("First Order LPF SIMD Test");
        firstOrderLPFTest<dsp::SIMDRegister<float>> (1.0e-2f);
        firstOrderLPFTest<dsp::SIMDRegister<double>> (1.0e-2);

        beginTest ("First Order HPF Test");
        firstOrderHPFTest<float> (1.0e-2f);
        firstOrderHPFTest<double> (1.0e-2);

        beginTest ("First Order HPF SIMD Test");
        firstOrderHPFTest<dsp::SIMDRegister<float>> (1.0e-2f);
        firstOrderHPFTest<dsp::SIMDRegister<double>> (1.0e-2);
    }
};

static FirstOrderFiltersTest firstOrderFiltersTest;
