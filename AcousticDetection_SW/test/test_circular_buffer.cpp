#include <unity.h>
#include "../include/circular_buffer.hpp"



void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_circular_buffer() {
    CircularBuffer<int, 4> buff;


	buff.push(1);
	buff.push(2);
	buff.push(3);
	buff.push(4);

    TEST_ASSERT_EQUAL_INT(buff[0], 1);
	TEST_ASSERT_EQUAL_INT(buff[1], 2);
	TEST_ASSERT_EQUAL_INT(buff[2], 3);
	TEST_ASSERT_EQUAL_INT(buff[3], 4);

	buff.push(5);
	TEST_ASSERT_EQUAL_INT(buff[0], 2);
	TEST_ASSERT_EQUAL_INT(buff[1], 3);
	TEST_ASSERT_EQUAL_INT(buff[2], 4);
	TEST_ASSERT_EQUAL_INT(buff[3], 5);


	buff.push(6);
	TEST_ASSERT_EQUAL_INT(buff[0], 3);
	TEST_ASSERT_EQUAL_INT(buff[1], 4);
	TEST_ASSERT_EQUAL_INT(buff[2], 5);
	TEST_ASSERT_EQUAL_INT(buff[3], 6);
	
	buff.push(7);
	TEST_ASSERT_EQUAL_INT(buff[0], 4);
	TEST_ASSERT_EQUAL_INT(buff[1], 5);
	TEST_ASSERT_EQUAL_INT(buff[2], 6);
	TEST_ASSERT_EQUAL_INT(buff[3], 7);

	buff.push(8);
	TEST_ASSERT_EQUAL_INT(buff[0], 5);
	TEST_ASSERT_EQUAL_INT(buff[1], 6);
	TEST_ASSERT_EQUAL_INT(buff[2], 7);
	TEST_ASSERT_EQUAL_INT(buff[3], 8);

	buff.push(9);
	TEST_ASSERT_EQUAL_INT(buff[0], 6);
	TEST_ASSERT_EQUAL_INT(buff[1], 7);
	TEST_ASSERT_EQUAL_INT(buff[2], 8);
	TEST_ASSERT_EQUAL_INT(buff[3], 9);
}



int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_circular_buffer);

    UNITY_END();
}
