
#include "keys.h"
#include "j1850.h"

uint8_t key_arrays[KEY_CODE_COUNT][KEY_CODE_LEN];
uint8_t key_arrays_eep[KEY_CODE_COUNT][KEY_CODE_LEN] EEMEM =
{
    {1, 53, 92, 48},            // key PHONE
    {1, 52, 92, 124},           // key console PHONE
    {1, 52, 99, 141},           // key BACK
    {1, 52, 59, 187},          // key MAP
    //{1, 53, 80, 172},       // key INFO
   //{1, 52, 60, 232},       // key INFO console
    {1, 53, 84, 216},       // key Vol+
    {1, 53, 85, 197},       // key Vol-
    {1, 53, 86, 226},       // key FF
    {1, 53, 87, 255},       // key REW
   // {1, 52, 33, 164},       // key 1
  //  {1, 52, 34, 131},       // key 2
  //  {1, 52, 35, 158},       // key 3
   // {1, 52, 36, 205},       // key 4
  //  {1, 52, 37, 92},        // key 5
  //  {1, 52, 38, 247},       // key 6
    {1, 52, 71, 26},        // joystick UP
    {1, 52, 67, 10},        // joystick DOWN
    {1, 52, 69, 68},        // joystick LEFT
    {1, 52, 65, 48},        // joystick RIGHT
   // {1, 52, 102, 228},      // joystick PRESS
   // {1, 51, 230, 0},        // key audio off
    {1, 52, 56, 156},       // key TA
    {1, 52, 55, 39},        // key PTY
    {2, 58, 1, 0, 0, 84},       // key Vol+ CONSOLE
    {3, 58, 255, 0, 0, 191},       // key Vol- CONSOLE
    //{},       // key reserve
};

// read key code from eeprom
void keys_eepr_rd(void)
{
    for(uint8_t i=0; i<KEY_CODE_COUNT; i++)
    {
        for(uint8_t j=0; j<KEY_CODE_LEN; j++)
        {
            key_arrays[i][j] = eeprom_read_byte(&key_arrays_eep[i][j]);
        }
    }
}

/********************************************************
* J1850Key decode
param1 - pointer to read buffer
return value - key kode:
    0-key not present
    1-phone
    2-console phone
    3-back
    4-MAP
*******************************************************/
uint8_t key_find(uint8_t data_read_index)
{
    byte i;
    byte k=0;
    byte done=1;

    do
    {
        for (i=0; i<KEY_CODE_LEN; i++)
        {
            if(key_arrays[k][i]!=DataReadBuffer[data_read_index][i+3])
            {
                k++;
                break;
            }
            else if(i==KEY_CODE_LEN-1) done=0;
        }
        if(k==KEY_CODE_COUNT) return 0;
    }
    while(done);

    return _BV(k);
}
