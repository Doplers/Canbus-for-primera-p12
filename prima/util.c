
#include "util.h"


/**-------------------my TIMERS-------------------------------------*/
unsigned long del_millis_cur;

timer_on_delay ton[ton_max_obj];

// timers routine
void ton_routine()
{
    for(uint8_t i=0; i!=ton_max_obj; i++)
    {
        if(ton[i].S)
        {
            del_millis_cur = millis();
            if((del_millis_cur-ton[i].pre_millis)>ton[i].period)
            {
                ton[i].S=false;
                ton[i].Q=true;
            }
        }
    }
}

void ton_S(uint8_t num, unsigned int period_ms)
{
    ton[num].S=true;
    ton[num].Q=false;
    ton[num].period = period_ms;

    ton[num].pre_millis = millis();
}

void ton_R(uint8_t num)
{
    ton[num].S=false;
    ton[num].Q=false;
}

ISR(TIMER1_COMPA_vect)
{
    TCNT1=0;
}

