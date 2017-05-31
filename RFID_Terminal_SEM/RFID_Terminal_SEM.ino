/**
    Control de Fermentacion

    @author Federico Lo Grasso
    @version 1.2
*/


#include <avr/wdt.h> //Watchdog
#include "globals.h"


#define DEBUG true // Debug Flag



void setup(void)
{

#if DEBUG == true
	Serial.begin(9600);
#endif

	
	//Watchdog 8s
	wdt_enable(WDTO_8S); // enable watchdog timer with 8 second timeout (max setting)
						 // wdt will reset the arduino if there is an infinite loop or other hangup; this is a failsafe
}/*END SETUP*/

void loop(void)
{
	//RESETEO EL WATCHDOG EN CADA LOOP
	wdt_reset();  // reset the watchdog timer (once timer is set/reset, next reset pulse must be sent before timeout or arduino reset will occur)
	

} /*END LOOP*/

