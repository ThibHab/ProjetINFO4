#include <TimedUnitTest.h>

class LNFAllocatorTest : public TimedUnitTest
{
public:
    LNFAllocatorTest() : TimedUnitTest ("LNF Allocator Test")
    {
    }

    void getLookAndFeelTest()
    {
        chowdsp::LNFAllocator allocator;
        auto* lnf = allocator.getLookAndFeel<chowdsp::ChowLNF>();
        auto* lnf2 = allocator.getLookAndFeel<chowdsp::ChowLNF>();

        expect (lnf == lnf2, "Look and feel pointers should be the same!");
    }

    void addLookAndFeelTest()
    {
        chowdsp::LNFAllocator allocator;
        auto* lnf = allocator.addLookAndFeel<chowdsp::ChowLNF>();
        auto* lnf2 = allocator.getLookAndFeel<chowdsp::ChowLNF>();

        expect (lnf == lnf2, "Look and feel pointers should be the same!");
    }

    void containsLookAndFeelTest()
    {
        chowdsp::LNFAllocator allocator;
        allocator.getLookAndFeel<chowdsp::ChowLNF>();
        expect (allocator.containsLookAndFeelType<chowdsp::ChowLNF>(), "Allocator should contain this look and feel!");
        expect (! allocator.containsLookAndFeelType<LookAndFeel_V4>(), "Allocator should not contain this look and feel!");
    }

    void runTestTimed() override
    {
        beginTest ("Get LookAndFeel Test");
        getLookAndFeelTest();

        beginTest ("Add LookAndFeel Test");
        addLookAndFeelTest();

        beginTest ("Contains LookAndFeel Test");
        containsLookAndFeelTest();
    }
};

static LNFAllocatorTest lnfAllocatorTest;
