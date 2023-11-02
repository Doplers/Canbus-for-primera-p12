
#include "j1850.h"
//#include "keys.h"
#include "util.h"

#define my_DBG_j 1

uint8_t SOF_OK;
uint8_t WriterIndexPackets;
uint8_t ReaderIndexPackets;
uint8_t dataBitCnt;
uint8_t dataByteCnt;
uint8_t dataBiteReciver;
uint8_t ByteCountBuffer[J1850_RCV_NUM_PACKET_MAX];
uint8_t DataReadBuffer[J1850_RCV_NUM_PACKET_MAX][J1850_RCV_LEN_PACKET_MAX];

uint8_t reciver_status;
uint8_t last_reciver_status;

// accept filter
uint8_t reciver_filter[3] = {100, 0, 40};

int16_t j1850_buttons;
uint8_t crc(uint8_t *data, uint8_t len);

/************* J1850PWM Control IO ****************/
#define J1850PWM_IN             ACSR & _BV(ACO)
#define J1850PWM_OUT_1          {sbi(PORTD,PIN6); cbi(PORTD,PIN7);}
#define J1850PWM_OUT_0          {cbi(PORTD,PIN6); sbi(PORTD,PIN7);}
#define J1850PWM_DDR_toOUT      {DDRD |= (_BV(PIN6) | _BV(PIN7)); DIDR1 = 0;}
#define J1850PWM_DDR_toIN       {DDRD &= ~(_BV(PIN6) | _BV(PIN7)); PORTD &= ~(_BV(PIN6) | _BV(PIN7));DIDR1 = 3;}

/****************** Timer Control******************/
#define J1850_TC_TCCR       TCCR2B      //TCCR0B      //TCCR1B
#define J1850_TC_TIMSK      TIMSK2      //TIMSK0      //TIMSK1
#define J1850_TC_TCNT       TCNT2       //TCNT0       //TCNT1L
#define J1850_TC_OCR        OCR2A       //OCR0A       //OCR1B
#define J1850_TC_OCVector   TIMER2_COMPA_vect   //TIMER0_COMPA_vect//TIMER1_COMPB_vect

void J1850_timer_start()
{
    J1850_TC_TCCR = 0x02;
}
void J1850_timer_stop()
{
    J1850_TC_TCCR = 0x00;
    J1850_TC_TCNT=  0x00;
}
void J1850_timer_init()
{
    J1850_TC_TIMSK = _BV(OCIE2A) ;
    J1850_TC_OCR = (51+24)*2;                  // (51+24)*2 - EOD+bitLen;  (72+24)*2 - EOF+bitLen
}

/************ transfer recived data to Serial port********************/
void dataShow(uint8_t pointToRCV)
{
    Serial.print("Rcv Packet: ");
    for(uint8_t i=0; i<ByteCountBuffer[pointToRCV]; i++)
    {
        Serial.print(DataReadBuffer[pointToRCV][i]);
        Serial.print(" ");
    }
    Serial.println("");
    Serial.println(ByteCountBuffer[pointToRCV]);
}

/************ initialisation j1850 pwm protocol *********************/
void j1850_init()
{
#ifdef my_DBG_j
    Serial.begin(115200);
    Serial.println("Initialisation j1850 proto...");
#endif
    // set port to input
    J1850PWM_DDR_toIN;
    //setup comparator interrupt
    ACSR =  _BV(ACIE);             // interrupt enable

    //setup timer 1
    J1850_timer_init();

    // reset indexes
    WriterIndexPackets=0;
    ReaderIndexPackets=0;
    j1850_buttons=0;
    // init status
    reciver_status = bus_idle;
    // start timer
    J1850_timer_start();
    // global enable interrupt
    sei();
}

/********* packets filtration *****/
void j1850_packet_filtr()
{
    if(reciver_status==byte_recived)
    {
        if(dataByteCnt<=3)
        {
            byte i = dataByteCnt-1;
            if(reciver_filter[i]!=DataReadBuffer[WriterIndexPackets][i])
            {
                SOF_OK=0;
            }
        }
        reciver_status = byte_reciving;
    }
}

/********* find button code in recived packet *************/
int16_t j1850_get_Button_code()
{
    if(crc(DataReadBuffer[ReaderIndexPackets],6)==DataReadBuffer[ReaderIndexPackets][6]) // if crc OK

    {
        return ((DataReadBuffer[ReaderIndexPackets][4]<<8) | DataReadBuffer[ReaderIndexPackets][5]);
    }
     if(crc(DataReadBuffer[ReaderIndexPackets],8)==DataReadBuffer[ReaderIndexPackets][8]) // if crc OK

    {
        return ((DataReadBuffer[ReaderIndexPackets][4]<<8) | DataReadBuffer[ReaderIndexPackets][5]);
    }
    else return -1;

}

/******************************************************************
********** General Routine Function of PWM protocol ***************
*******************************************************************/
void j1850_routine()
{
    if(reciver_status!=last_reciver_status)
    {
#ifdef my_DBG_j
        if(reciver_status&bus_error)
        {
            Serial.println(" --Bus ERROR!");
        }
#endif // my_DBG_j
        // packet filtration
        j1850_packet_filtr();
        last_reciver_status=reciver_status;
    }
    // check overflow buffer
    if(dataByteCnt>=J1850_RCV_LEN_PACKET_MAX) dataByteCnt=0;
    // check new packet recived
    if(WriterIndexPackets!=ReaderIndexPackets)
    {
        j1850_buttons = j1850_get_Button_code();

#ifdef my_DBG_j
        dataShow(ReaderIndexPackets);
        if(j1850_buttons>0)
        {
            Serial.print("Found Key: ");
            Serial.println(j1850_buttons);
            dataShow(ReaderIndexPackets);
        }
#endif
        ReaderIndexPackets++;           // increment Reader index
        if(ReaderIndexPackets >= J1850_RCV_NUM_PACKET_MAX) ReaderIndexPackets=0;
    }
}

/****************** Timmer Compare Interrupt ******************
*********************** BUS Status Control ********************/
ISR(J1850_TC_OCVector)
{
    J1850_timer_stop();
    if(J1850PWM_IN)     // bus in hi level very long time
        reciver_status = bus_error;
    else
    {
        reciver_status = bus_idle;

        if(SOF_OK)      // recive complete
        {
            reciver_status = packet_recived;
            SOF_OK=0;
            ByteCountBuffer[WriterIndexPackets] = dataByteCnt;
            WriterIndexPackets++;
            if(WriterIndexPackets==J1850_RCV_NUM_PACKET_MAX)
                WriterIndexPackets=0;

            J1850_TC_TCNT=0;
            J1850_timer_start();
        }
    }
}

/****************** Analog Comparator Interrupt ******************
 ********************** Recive Data ******************************/
ISR(ANALOG_COMP_vect)
{
    uint8_t t;
    t=J1850_TC_TCNT;

    if(J1850PWM_IN)
    {
        // rising edge
        // clear timer
        J1850_TC_TCNT=0x00;
        J1850_timer_start();
    }
    else
    {
        // falling edge
        if(t>60 & t<72)             //SOF
        {
            SOF_OK=1;
            dataBitCnt=0;
            dataByteCnt=0;
            dataBiteReciver=0;
        }
//        else if(t>74 & t<88)        //BREACK
//        {
//            SOF_OK=0;
//        }
        else if(t>=10 & t<=40)        //DATA BIT
        {
            if(SOF_OK)
            {
                dataBiteReciver = dataBiteReciver<<1;
                if(t<=24)
                {
                    dataBiteReciver |= 1;
                }

                dataBitCnt++;
                if(dataBitCnt>=8)   // DATA BYTE
                {
                    DataReadBuffer[WriterIndexPackets][dataByteCnt] = dataBiteReciver;
                    dataByteCnt++;
                    dataBitCnt=0;
                    reciver_status=byte_recived;
                }
            }
        }
    }
}

/******************************************************************
 ** pwm_put *** SEND RAW Data
 *******************************************************************/
uint8_t pwm_put(uint8_t *data, uint8_t count)
{
    uint8_t i=0;
    int8_t bit;
    uint8_t time;

    //first check that the PWM bus is idle
    // wait IDLE
    while(reciver_status != bus_idle);

    //disable Analog comparator ISR
    ACSR=0x00;

    //disable Timer ISR
    J1850_timer_stop();

    //Transmit SOF
    J1850_TC_TCNT=0x00;
    // set driver hi level
    J1850PWM_OUT_1;
    //set port direction to OUT
    J1850PWM_DDR_toOUT;

    J1850_timer_start();
    while (J1850_TC_TCNT<64);   // TP4

    // check bus ready
    J1850PWM_DDR_toIN;
    while (J1850_TC_TCNT<66);   // wait setup level
    if (J1850PWM_IN)            // if BUS busy
    {
        //enable reciever
        ACSR=_BV(ACIE);
        return 0;
    }
    while (J1850_TC_TCNT<96);    // TP5

    // transfer bytes and bits
    while (i!=count)
    {
        uint8_t t;

        //t=*data++;
        t=data[i++];
        for (bit=7; bit>=0; bit--)
        {
            uint8_t b;

            b=(t>>bit) & 0x01;
            time=14;                //TP2
            if (b==0)
            {
                time=30;            //TP3
            }

            J1850PWM_OUT_1;
            J1850PWM_DDR_toOUT;
            J1850_TC_TCNT=0;
            while(J1850_TC_TCNT<= time);

            //check colision
            J1850PWM_DDR_toIN;
            while(J1850_TC_TCNT<= time+2);   // wait setup level
            if (J1850PWM_IN)                // if collision
            {
                //enable reciever
                ACSR=_BV(ACIE);
                return i;
            }
            while(J1850_TC_TCNT< 48) ;   //TP1
        }
    }

    while(J1850_TC_TCNT < 144);    //TP8 - End of Frame

    //enable reciever
    ACSR=_BV(ACIE);
    J1850_TC_TCNT=0;
    J1850_timer_start();

    return i;
}

/******************************************************************
 ** Send Buttons code Function **
 *******************************************************************/
uint8_t j1850_sendButtons(uint16_t cmd_code)
{
    uint8_t transfered_bytes;
    uint8_t transfer_packet_cnt=0;
    uint8_t i;
    uint8_t array_key_code[J1850_SND_BTN_PACKET_LEN+1] = {100,0,40,1,52,99,141,0}; // key BACK for example

    for(i=1; i<4; i++)
    {
        array_key_code[3]=i;
        array_key_code[4]=(uint8_t)((cmd_code & 0xFF00)>>8);
        array_key_code[5]=(uint8_t)cmd_code & 0x00FF;
        array_key_code[6]=crc(array_key_code,6);
        transfered_bytes = pwm_put(array_key_code,J1850_SND_BTN_PACKET_LEN);

        // if transfer was fault we retransmitt last packet
        if(transfered_bytes!=J1850_SND_BTN_PACKET_LEN) i--;
        // increment transfer counter and check max attemp
        if(transfer_packet_cnt++ >= J1850_SND_ATTEMP_CNT_MAX) return 0;

        // small delay befor next transfer
        delay(5);
    }

#ifdef my_DBG_j
    Serial.print("j1850 snd cnt: ");
    Serial.println(transfer_packet_cnt);
#endif // my_DBG_j

    return 1;

}

/******************************************************************
 **  CRC calculations ** it from https://code.google.com/p/avrobdii/source/browse/#svn%2Ftrunk%2Fcode
 *******************************************************************/
uint8_t crc(uint8_t *data, uint8_t len)
{
    uint8_t result;
    uint8_t i;
    uint8_t mask;
    uint8_t j;
    uint8_t temp;
    uint8_t poly;

    result=0xFF;
    for(i=0; i<len; i++)
    {
        mask=0x80;
        temp=data[i];
        for(j=0; j<8; j++)
        {
            if(temp & mask)	  //bit is 1
            {
                poly=0x1c;
                if(result & 0x80)
                {
                    poly=1;
                }
                result= ((result<<1) | 0x01) ^ poly;
            }
            else
            {
                poly=0;
                if(result & 0x80)
                {
                    poly=0x1D;
                }
                result= ((result<<1)) ^ poly;
            }
            mask=mask>>1;
        }
    }
    return ~result;
}

