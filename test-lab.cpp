#include "lab.h"
#include <gtest/gtest.h>


TEST(SegFaultTest, asan)
{
  segfault();
}


TEST(OutOfBoundsTest, fail)
{
  outOfBounds();
}

TEST(LeakTest, fail)
{
  int *actual = leakyFunction(2);
  ASSERT_EQ(2, *actual);
  free(actual);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
