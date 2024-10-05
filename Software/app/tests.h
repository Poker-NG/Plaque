#ifndef _APP_TESTS_H
#define _APP_TESTS_H

#include "eeprom.h"
// #include "m24sr64.h"
#include "st25.h"

void test_run_external_eeprom(external_eeprom_context_t* external_eeprom);
void test_run_st25(st25_context_t* st25);

#endif // !_APP_TESTS_H
