#include "lab.h"
#include <gtest/gtest.h>

TEST(SegFaultTest, asan)
{
    // Fix this test so that it does not segfault :)
    segfault();
}

// This tests are disabled because they will fail they are used to make sure
// that the sanitizer is working correctly. You can run them with the
// --gtest_also_run_disabled_tests flag
// ./out/build/x64-ASan/test-lab --gtest_filter=DISABLED_OutOfBoundsTest.fail --gtest_also_run_disabled_tests
// ./out/build/x64-ASan/test-lab --gtest_filter=DISABLED_LeakTest.fail --gtest_also_run_disabled_tests

//You will need to fix the errors in these tests and then delete the DISABLED_ part
//so they will run!

TEST(DISABLED_OutOfBoundsTest, fail)
{
    outOfBounds();
}

TEST(DISABLED_LeakTest, fail)
{
    int *actual = leakyFunction(2);
    ASSERT_EQ(2, *actual);
    free(actual);
}
