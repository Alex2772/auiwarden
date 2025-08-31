#include "MainWindow.h"
#include <gtest/gtest.h>

TEST(SumTest, Case1) {
  ASSERT_EQ(MainWindow::sum(2, 2), 4);
}
