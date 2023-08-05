#include "unity.h"
#include "disassemble.h"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void test_show_jmp(void) {
  uint8_t data[] = {
    0x4C, 0xF5, 0xC5,
  };

  auto instr = DecodedInstr(data, 3);

  char buf[MAX_INSTR_DISPLAY_SIZE+1];

  instr.format(buf);

  TEST_ASSERT_EQUAL_STRING("jmp $C5F5", buf);
}

void test_function_should_doAlsoDoBlah(void) {
  // more test stuff
}

int runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_show_jmp);
  RUN_TEST(test_function_should_doAlsoDoBlah);
  return UNITY_END();
}

// WARNING!!! PLEASE REMOVE UNNECESSARY MAIN IMPLEMENTATIONS //

/**
  * For native dev-platform or for some embedded frameworks
  */
int main(void) {
  return runUnityTests();
}