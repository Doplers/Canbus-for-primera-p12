#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <Arduino.h>
//#include <SoftwareSerial.h>

#define ton_max_obj             5
#define timer_key_sndBack       2
#define timer_key_brd_filt      4


struct timer_on_delay
{
    bool Q;
    bool S;
    bool R;
    unsigned long pre_millis;
    unsigned int period;
};
extern timer_on_delay ton[ton_max_obj];
void ton_S(uint8_t num, unsigned int period_ms);
void ton_R(uint8_t num);
void ton_routine();

#endif // UTIL_H_INCLUDED
