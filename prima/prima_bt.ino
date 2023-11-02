#include <Arduino.h>
#include "j1850.h"
//#include "keys.h"
#include "util.h"

#define CPU_FREQ 16000000


#define kbrd_filtr_time     120

//#define use_back_snd


uint16_t key_brd_button;

void key_brd_fnc();

/****************** Initial Function******************/
void setup()
{
   DDRD = DDRD | 0x3C;
      DDRB = DDRB | 0xFF;
    // init
    j1850_init();

    delay(2000);


    // prepare Timer (8sec) Send Command BACK to Display
#ifdef use_back_snd
    ton_S(timer_key_sndBack,5000);
#endif // use_back_snd

    ton_S(timer_key_brd_filt,kbrd_filtr_time);

    key_brd_button=0;
}

/****************** Main Loop ******************/
void loop()
{
    // timers routine
    ton_routine();

    // j1850 routine
    j1850_routine();


    key_brd_fnc();

    // send BACK Button pressed after Start System
#ifdef use_back_snd
    if(ton[timer_key_sndBack].Q)
    {
        ton_R(timer_key_sndBack);
        j1850_sendButtons(J1850_BTN_BACK);
    }
#endif // use_back_snd


}

void key_brd_fnc()
{

    if(ton[timer_key_brd_filt].Q)
    {
        ton_R(timer_key_brd_filt);
        // cleare buttons code
        j1850_buttons=0;
    }
    if((j1850_buttons>0) && !ton[timer_key_brd_filt].S)
    {
        key_brd_button=j1850_buttons;
        ton_S(timer_key_brd_filt,kbrd_filtr_time);
    }



    if(key_brd_button>0)
    {

        /** first press button */
       if((key_brd_button == J1850_BTN_PHONE) || (key_brd_button == J1850_BTN_PHONE_CONSOLE)) // D2
          {
              PORTD |= 0x04; //установить высокий уровень
        delay(300);//пауза 2сек
        PORTD &= ~0x04;//установить низкий уровень
          }


       else if((key_brd_button == J1850_BTN_VOL_UP) || (key_brd_button == J1850_BTN_VOL_UP_CONSOLE)) //D3
        {
            PORTD |= 0x10; //установить высокий уровень
        delay(100);//пауза 2сек
        PORTD &= ~0x10;//установить низкий уровень
         }

        else if((key_brd_button == J1850_BTN_BACK)) //D4
         {
             PORTD |= 0x8; //установить высокий уровень
        delay(300);//пауза 2сек
        PORTD &= ~0x8;//установить низкий уровень
         }


       else  if((key_brd_button == J1850_BTN_VOL_DOWN) || (key_brd_button == J1850_BTN_VOL_DOWN_CONSOLE)) //D5
       {
           PORTD |= 0x20; //установить высокий уровень
       delay(100);//пауза 2сек
       PORTD &= ~0x20;//
       }

         else if((key_brd_button == J1850_BTN_MAP)) //D8
         {
             PORTB |= 0x1; //установить высокий уровень
        delay(300);//пауза 2сек
        PORTB &= ~0x1;//установить низкий уровень
         }


      else  if((key_brd_button == J1850_BTN_FF)|| (key_brd_button == J1850_BTN_JOY_RIGHT)) //D9
         {
             PORTB |= 0x2; //установить высокий уровень
        delay(300);//пауза 2сек
        PORTB &= ~0x2;//установить низкий уровень
         }

        else if((key_brd_button == J1850_BTN_REW) || (key_brd_button == J1850_BTN_JOY_LEFT)) //D10
          {
              PORTB |= 0x4; //установить высокий уровень
        delay(300);//пауза 2сек
        PORTB &= ~0x4;//установить низкий уровень
          }

       else if((key_brd_button == J1850_BTN_TA))  //D11
           {
               PORTB |= 0x10; //установить высокий уровень
        delay(300);//пауза 2сек
        PORTB &= ~0x10;//установить низкий уровень
           }

else if((key_brd_button == J1850_BTN_PTY))  //D12
       {
       PORTB |= 0x8; //установить высокий уровень
       delay(300);//пауза 2сек
       PORTB &= ~0x8;//установить низкий уровень
       }
       // cleare kbrd key
        key_brd_button=0;
    }
}
