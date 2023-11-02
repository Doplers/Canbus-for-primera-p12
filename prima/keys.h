
#include <Arduino.h>
#include <avr/eeprom.h>


#define KEY_CODE_COUNT  17
#define KEY_CODE_LEN    6
extern uint8_t key_arrays[KEY_CODE_COUNT][KEY_CODE_LEN];


void keys_eepr_rd(void);
uint8_t key_find(uint8_t data_read_index);
