#include "lab.h"
#include <gtest/gtest.h>

TEST(LeakTest, asan)
{
  char *version = getVersion();
  ASSERT_STREQ("1.0", version);
}

TEST(SegFaultTest, asan)
{
  segfault();
}


TEST(OutOfBoundsTest, asan)
{
  outOfBounds();
}
