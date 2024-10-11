#ifndef _APP_TESTS_H
#define _APP_TESTS_H

#include "eeprom.h"
// #include "m24sr64.h"
#include "epd.h"
#include "st25.h"

void test_run_external_eeprom(external_eeprom_context_t* external_eeprom);
void test_run_st25(st25_context_t* st25);
void test_run_epd(epd_context_t* epd);

#endif // !_APP_TESTS_H
