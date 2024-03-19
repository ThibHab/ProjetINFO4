#include "TestUtils.h"

TEST_CASE("Effects Test")
{
    SECTION("SINE")
    {
        TestUtils::runTest({sst::waveshapers::WaveshaperType::wst_sine,
                            {0.0f, 0.309016f, 1.0f, 0.0122715f, 0.707107f, -0.156434f, -0.987684f,
                             -1.22465e-16f}});
    }

    SECTION("DIGITAL")
    {
        TestUtils::runTest(
            {sst::waveshapers::WaveshaperType::wst_digital,
             {-0.0625f, 0.0625f, 0.4375f, 0.9375f, 0.1875f, -0.0625f, -0.5625f, -0.9375f}});
    }
}
