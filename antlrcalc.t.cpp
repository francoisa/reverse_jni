#include <memory>
#include <gtest/gtest.h>
#include "antlr_interface.h"

namespace {

using namespace std;

class ParseTest : public ::testing::Test {
 protected:
  ParseTest() : _antlr(new antlr) {
  }

  unique_ptr<antlr> _antlr;
};

TEST_F(ParseTest, Numbers) {
  string actualResult;
  string expectedResult = "1";
  _antlr->parse("1", actualResult);
  ASSERT_EQ(expectedResult, actualResult);
}

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
