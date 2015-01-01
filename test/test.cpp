#include <gtest/gtest.h>

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <assert.h>

// define some type to the closest one available on arduino
typedef uint8_t byte;
typedef std::string String;

#include "../ring_buffer.h"

TEST(StringBuffer, append) {
  StringBuffer<2> buff;
  ASSERT_EQ(buff.indexOf("1"), -1);
  ASSERT_TRUE(buff.append('1'));
  ASSERT_EQ(buff.indexOf("1"), 0);
}

TEST(StringBuffer, pop_first) {
  StringBuffer<2> buff;
  ASSERT_EQ(buff.length(), 0);
  ASSERT_TRUE(buff.append('1'));
  ASSERT_EQ(buff.length(), 1);
  ASSERT_TRUE(buff.append('2'));
  ASSERT_EQ(buff.length(), 2);
  ASSERT_FALSE(buff.append('3'));
  ASSERT_EQ(buff.pop_first(), '1');
  ASSERT_EQ(buff.length(), 1);
  ASSERT_EQ(buff.pop_first(), '2');
  ASSERT_EQ(buff.length(), 0);
}

TEST(StringBuffer, pop_last) {
  StringBuffer<3> buff;
  ASSERT_TRUE(buff.append("123"));
  ASSERT_EQ(buff.pop_last(), '3');
  ASSERT_EQ(buff.length(), 2);
  ASSERT_TRUE(buff.append('4'));
  ASSERT_EQ(buff[2], '4');
  ASSERT_EQ(strncmp((const char*)buff.buffer(), "124", buff.length()), 0);
}

TEST(StringBuffer, accessor) {
  StringBuffer<3> buff;
  ASSERT_TRUE(buff.append("123"));
  ASSERT_EQ(buff.length(), 3);
  ASSERT_EQ(buff[0], '1');
  ASSERT_EQ(buff[1], '2');
  ASSERT_EQ(buff[2], '3');
  ASSERT_EQ(buff.pop_first(), '1');
  ASSERT_TRUE(buff.append('4'));
  ASSERT_EQ(buff[2], '4');    
  ASSERT_EQ(buff.length(), 3);
  ASSERT_FALSE(buff.append('5'));
  ASSERT_EQ(buff.pop_first(), '2');
  ASSERT_EQ(buff.pop_first(), '3');
  ASSERT_EQ(buff.length(), 1);
}

TEST(StringBuffer, indexOf) {
  StringBuffer<10> buff;
  for (auto c: {'1', '2', '3', '4', 'O', 'K', 'A', 'Y', '5', '6'})
    ASSERT_TRUE(buff.append(c));
  ASSERT_EQ(buff.indexOf("1"), 0);
  ASSERT_EQ(buff.indexOf("2"), 1);
  ASSERT_EQ(buff.indexOf("12"), 0);
  ASSERT_EQ(buff.indexOf("23"), 1);
  ASSERT_EQ(buff.indexOf("34"), 2);
  ASSERT_EQ(buff.indexOf("OKAY"), 4);
  ASSERT_EQ(buff.pop_first(), '1');
  ASSERT_EQ(buff.pop_first(), '2');
  ASSERT_EQ(buff.indexOf("OKAY"), 2);
  {
    uint16_t pos = buff.indexOf("OKAY") + 4;
    while (pos--)
      buff.pop_first();
    ASSERT_EQ(buff.length(), 2);
  }
  ASSERT_EQ(buff.indexOf("56"), 0);
}

TEST(StringBuffer, indexOf_offset) {
  StringBuffer<10> buff;
  for (auto c: {'1', '2', '3', '1', '2', '3', '1', '2', '3', '1'})
    ASSERT_TRUE(buff.append(c));
  int pos = 0;
  uint16_t count = 0;
  while (true) {
    pos = buff.indexOf("1", pos);
    if (pos == -1)
      break;
    ASSERT_EQ(buff[pos], '1');
    ++pos;
    ++count;
  }
  ASSERT_EQ(count, 4);
}

TEST(StringBuffer, full_empty) {
  StringBuffer<1> buff;
  ASSERT_TRUE(buff.empty());
  buff.append('1');
  ASSERT_TRUE(buff.full());
  buff.clear();
  ASSERT_TRUE(buff.empty());
}

TEST(StringBuffer, append_string) {
  StringBuffer<3> buff;
  buff.append("123");
  ASSERT_TRUE(buff.full());
  ASSERT_EQ(strncmp((const char*)buff.buffer(), "123", buff.length()), 0);
}

TEST(StringBuffer, buffer) {
  StringBuffer<3> buff;
  buff.append("123");
  ASSERT_TRUE(buff.full());
  buff.pop_first();
  ASSERT_EQ(strncmp((const char*)buff.buffer(), "23", buff.length()), 0);
}

TEST(RingBuffer, memmove_with_int) {
  RingBuffer<3, int> buff;
  for (auto c: {1, 2, 3})
    ASSERT_TRUE(buff.append(c));
  buff.pop_first();
  ASSERT_TRUE(buff.append(4)); // should not be continous anymore in internal mem
  ASSERT_EQ(buff.length(), 3);
  ASSERT_EQ(buff[0], 2);
  ASSERT_EQ(buff[1], 3);
  ASSERT_EQ(buff[2], 4);
  const int *arr = buff.buffer(); // should trigger an internal memmove
  ASSERT_EQ(arr[0], 2);
  ASSERT_EQ(arr[1], 3);
  ASSERT_EQ(arr[2], 4);
}

TEST(RingBuffer, memmove_append) {
  RingBuffer<3, short> buff;
  for (auto c: {1, 2, 3})
    ASSERT_TRUE(buff.append(c)); // |1|2|3|
  buff.pop_first(); // | |2|3|
  buff.pop_first(); // | | |3|
  // should not be continous anymore in internal mem
  ASSERT_TRUE(buff.append(4)); // |4| |3|
  ASSERT_EQ(buff.length(), 2);
  // should trigger an internal memmove
  buff.buffer(); // | |3|4|
  // check that indexes are correct avec memmove
  ASSERT_TRUE(buff.append(5)); // |5|3|4|
  // should trigger an internal memmove
  const short *arr = buff.buffer(); // |3|4|5|
  ASSERT_EQ(arr[0], 3);
  ASSERT_EQ(arr[1], 4);
  ASSERT_EQ(arr[2], 5);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
