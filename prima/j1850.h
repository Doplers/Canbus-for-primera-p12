
#include <Arduino.h>

#define my_DBG
//#define use_filtering

#define J1850_RCV_NUM_PACKET_MAX    6
#define J1850_RCV_LEN_PACKET_MAX    17
#define J1850_SND_BTN_PACKET_LEN    7
#define J1850_SND_ATTEMP_CNT_MAX    15

extern uint8_t WriterIndexPackets;
extern uint8_t ReaderIndexPackets;
extern uint8_t ByteCountBuffer[J1850_RCV_NUM_PACKET_MAX];
extern uint8_t DataReadBuffer[J1850_RCV_NUM_PACKET_MAX][J1850_RCV_LEN_PACKET_MAX];

extern int16_t j1850_buttons;

/** j1850 bus states */
#define bus_idle        1
#define bus_error       2
#define byte_reciving   3
#define byte_recived    4
#define packet_recived  5

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

/**Buttons Code*/
#define J1850_BTN_BACK              (99 | 52<<8)
#define J1850_BTN_PHONE_CONSOLE     (92 | 52<<8)
#define J1850_BTN_PHONE             (92 | 53<<8)
#define J1850_BTN_MAP               (59 | 52<<8)
//#define J1850_BTN_INFO              (80 | 53<<8)
//#define J1850_BTN_INFO_CONSOLE      (60 | 52<<8)
#define J1850_BTN_VOL_UP            (84 | 53<<8)
#define J1850_BTN_VOL_DOWN          (85 | 53<<8)
#define J1850_BTN_FF                (86 | 53<<8)
#define J1850_BTN_REW               (87 | 53<<8)
//#define J1850_BTN_K1                (33 | 52<<8)
//#define J1850_BTN_K2                (34 | 52<<8)
//#define J1850_BTN_K3                (35 | 52<<8)
//#define J1850_BTN_K4                (36 | 52<<8)
//#define J1850_BTN_K5                (37 | 52<<8)
//#define J1850_BTN_K6                (38 | 52<<8)
#define J1850_BTN_JOY_UP            (71 | 52<<8)
#define J1850_BTN_JOY_DOWN          (67 | 52<<8)
#define J1850_BTN_JOY_LEFT          (69 | 52<<8)
#define J1850_BTN_JOY_RIGHT         (65 | 52<<8)
//#define J1850_BTN_JOY_PRESS         (102| 52<<8)
#define J1850_BTN_TA                (56 | 52<<8)
#define J1850_BTN_PTY               (55 | 52<<8)
#define J1850_BTN_VOL_UP_CONSOLE    (1 | 58<<8)
#define J1850_BTN_VOL_DOWN_CONSOLE  (255 | 58<<8)

//#define J1850_BTN_
//#define J1850_BTN_
//#define J1850_BTN_

/**** Button Packet Structure example ********************************
        byte0   byte1   byte2   byte3   byte4   byte5   byte6
    {---100-----0-------40------1-------52------99------141-----}
        header  src     dest    attemp  data1   data2   crc
                addr    addr    cnt
*********************************************************************/

/** shared functions definition */
void j1850_init();
void j1850_routine();
uint8_t j1850_sendButtons(uint16_t cmd_code);
