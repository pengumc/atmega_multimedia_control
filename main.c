//atmega multimedia controller
//Author: Michiel van der Coelen (michiel.van.der.coelen@gmail.com)
//Date: 2013-07-31

//MCU = atmega88
//F_CPU = 20000000 defined in makefile
//hfuse: DF 
//lfuse: E6
//efuse: 0

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h> 
#include "usbdrv.h"
#include "oddebug.h"
#include "requests.h"

//macros
#define SET(x,y) (x|=(1<<y))
#define CLR(x,y) (x&=(~(1<<y)))
#define CHK(x,y) (x&(1<<y)) 
#define TOG(x,y) (x^=(1<<y))
#define VOLUME_UP 0
#define VOLUME_DOWN 1
#define NEXT_TRACK 2
#define PREV_TRACK 3
#define PAUSE_TRACK 4
#define MAX_VOLUME 63
#define BUTTONPORT PINB
#define BUTTON_PREV PB1
#define BUTTON_PAUSE PB2
#define BUTTON_NEXT PB3


static uchar    reportBuffer[1];    /* buffer for HID reports */
static uchar    idleRate;           /* in 4 ms units */
static uchar    msgBuffer;

static uchar button_states = 0;
#define STATE_NEXT 0
#define STATE_PREV 2
#define STATE_PAUSE 4

uchar changed = 0;
uchar adc_result = 0; 
uchar volume = MAX_VOLUME;
uchar first_adc = 1;

const PROGMEM char usbHidReportDescriptor[31] = {   /* USB report descriptor */
    0x05, 0x0C, // Usage Page (Consumer Devices)
    0x09, 0x01, // Usage (Consumer Control)
    0xA1, 0x01, // Collection (Application)
        0x15, 0x00, //Logical Minimum (0)
        0x25, 0x01, //Logical Maximum (1)
        0x09, 0xE9, //Usage (Volume Up)
        0x09, 0xEA, //Usage (Volume Down)
        0x09, 0xb5, //Usage (scan Next Track)
        0x09, 0xb6, //Usage (scan Previous Track)
        0x09, 0xcd, //Usage (media play/pause) ?undocumented?
        0x75, 0x01, //Report Size (1)
        0x95, 0x05, //Report Count (5)
        0x81, 0x06, //Input (Data, Variable, rel)
        0x95, 0x03, //Report Count (3)
        0x81, 0x07, //Input (Constant)
    0xC0 // End Collection
};


 
 uchar	usbFunctionSetup(uchar data[8]){
    usbRequest_t    *rq = (void *)data;
    usbMsgPtr = reportBuffer;
    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one report type, so don't look at wValue */
            return sizeof(reportBuffer);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &idleRate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            idleRate = rq->wValue.bytes[1];
        }
    }else{ //some debug stuff that's left in.
        if(rq->bRequest == USBRQ_GET_LAST_ADC_RESULT){
            msgBuffer = adc_result;
            usbMsgPtr = &msgBuffer;
            return 1;
        }
    }
	return 0;
}


int main(){
    SET(DDRC, PC5);//led
    //connect usb
    wdt_enable(WDTO_1S);
    usbInit();
    usbDeviceDisconnect();
    register uint8_t i=0;
    while(--i){
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    sei();
    //setup ports, timers, and init variables.
    ADMUX = (1<<REFS0) | (1<<ADLAR); //ref = avcc, left adjust result
    ADCSRA = (1<<ADEN) | (1<<ADSC) | 7; //turn on adc, prescale 128 -> 6.4 us conversion t
    DIDR0 = (1<<ADC0D); //digital input disabled for adc0 
    TCCR0B = 5; //prescale TC0 1024, 20 MHz -> 50 ns * 1024 * 255 = 13.056 ms
    reportBuffer[0] = 0x00; //usb report buffer
    SET(PORTC, PC5); //status led on
    while(1){
        usbPoll(); //check for usb communications
        wdt_reset(); //reset watchdog timer
        if(CHK(TIFR0, TOV0)){ //check if 13 ms have passed already
            TIFR0 = 1<<TOV0;    //clear timer overflow flag
            TCNT0 = 0;          //reset timer
            //check button states
            //NEXT 
            if(CHK(BUTTONPORT, BUTTON_NEXT)){           //if it's down
                SET(button_states, STATE_NEXT);           //set current state to down
                if(CHK(button_states, (STATE_NEXT+1))){   //if previous state was down
                                                            //nothing changed, do nothing
                }else{                                    //else
                    SET(reportBuffer[0], NEXT_TRACK);       //send down state
                    SET(button_states, (STATE_NEXT+1));     //update previous state
                    changed = 1;                            //set flag to send report
                }
            }else{                                      //button is up
                CLR(button_states, STATE_NEXT);           //set current state to up
                if(CHK(button_states, (STATE_NEXT+1))){   //if previous was down
                    CLR(reportBuffer[0], NEXT_TRACK);       //send up state
                    CLR(button_states, (STATE_NEXT+1));     //update previous state
                    changed = 1;                            //set flag to send report
                }else{                                    //else button already was up
                                                            //nothing changed, do nothing
                }
            }
            // PREVIOUS
            if(CHK(BUTTONPORT, BUTTON_PREV)){         
                SET(button_states, STATE_PREV);       
                if(CHK(button_states, (STATE_PREV+1))){ 
                                                      
                }else{                                
                    SET(reportBuffer[0], PREV_TRACK);
                    SET(button_states, (STATE_PREV+1)); 
                    changed = 1;
                }
            }else{                                    
                CLR(button_states, STATE_PREV);       
                if(CHK(button_states, (STATE_PREV+1))){ 
                    CLR(reportBuffer[0], PREV_TRACK);
                    CLR(button_states, (STATE_PREV+1)); 
                    changed = 1;
                }else{
                                                      
                }
            }
            // PAUSE
            if(CHK(BUTTONPORT, BUTTON_PAUSE)){         
                SET(button_states, STATE_PAUSE);       
                if(CHK(button_states, (STATE_PAUSE+1))){ 
                                                      
                }else{                                
                    SET(reportBuffer[0], PAUSE_TRACK);
                    SET(button_states, (STATE_PAUSE+1)); 
                    changed = 1;
                }
            }else{                                    
                CLR(button_states, STATE_PAUSE);       
                if(CHK(button_states, (STATE_PAUSE+1))){ 
                    CLR(reportBuffer[0], PAUSE_TRACK);
                    CLR(button_states, (STATE_PAUSE+1)); 
                    changed = 1;
                }else{
                                                      
                }
            }
            //check adc
            if(!CHK(ADCSRA, ADSC)){ //if a conversion is done.
                adc_result = (ADCH>>2); //store the result (discard some accuray to reduce jitter)
                SET(ADCSRA, ADIF); //clear the conversion done flag
                SET(ADCSRA, ADSC); //start the next conversion
                if(first_adc){  //if this is the first adc we do since startup
                    volume = adc_result; //sync volume and adc_result
                    first_adc = 0; //next time won't be the first.
                }else{
                    if(changed == 0){ //if we're not already going to send other button presses
                        if(adc_result > volume){ //reversed because soldering.
                            SET(reportBuffer[0], VOLUME_DOWN);  //send volume down
                            if(volume < MAX_VOLUME) volume++;   //increase our local volume as well
                            changed = 1;                        //set flag to send report
                        }else if (adc_result < volume){       //volume going down
                            SET(reportBuffer[0], VOLUME_UP);    //send volume down
                            if(volume > 0) volume--;            //decrease our local volume as well
                            changed = 1;                        //set flag to send report
                        }
                    }
                }
            }
        }
        if(changed){ //something changed, send a report
            changed = 0;
            TIFR0 = 1<<TOV0;
            TCNT0 = 0;
            wdt_reset();
            usbSetInterrupt(reportBuffer, sizeof(reportBuffer)); //send report
            while(!usbInterruptIsReady()){} //wait till it's sent.
            //immediately sent a report that all buttons are up.
            reportBuffer[0] = 0x00; 
            usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
        }
    } //while
} //main