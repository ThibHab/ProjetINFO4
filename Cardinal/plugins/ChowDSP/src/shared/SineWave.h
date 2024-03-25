#pragma once

#include <cmath>

namespace chowdsp
{
/** Sinusoidal oscillator using the "magic circle" algorithm.
 * See: https://ccrma.stanford.edu/~jos/pasp/Digital_Sinusoid_Generators.html, eq. 2
 */
template <typename T>
class SineWave
{
public:
    SineWave() = default;

    /** Sets the frequency of the oscillator. */
    void setFrequency (T newFrequency) noexcept;

    /** Returns the current frequency of the oscillator. */
    T getFrequency() const noexcept { return freq; }

    /** Prepares the oscillator to process at a given sample rate */
    void prepare (double sampleRate) noexcept;

    /** Resets the internal state of the oscillator */
    void reset() noexcept;

    /** Resets the internal state of the oscillator with an initial phase */
    void reset (T phase) noexcept;

    /** Returns the result of processing a single sample. */
    inline T processSample() noexcept
    {
        auto y = x2;
        x1 += eps * x2;
        x2 -= eps * x1;
        return y;
    }

private:
    T x1 = static_cast<T> (0.0);
    T x2 = static_cast<T> (0.0);
    T eps = static_cast<T> (0.0);

    T freq = static_cast<T> (0.0);
    T fs = static_cast<T> (44100.0);
};

} // namespace chowdsp