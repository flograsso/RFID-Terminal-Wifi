  /**
    RFID Terminal

    @author Federico Lo Grasso
    @version 1.0


	Info: RC522 reads Mifare cards, a 13.56 MHz contactless smart card standard.
	The standard cards that we will use are the MIFARE Classic® 13.56Mhz 1Kb. It has 1024 bytes of memory
	divided in 16 sectors and protected by 2 different security keys (A and B). Each sector has 4 blocks with 16Kb
	The card could have UID (Unique ID) or NUID (Non-Unique ID). It is a 4 bytes number and it saved on the Sector 0 Block 0
	in the card. We can modifiy it in the Mifare Classic Cards (in specials card we can do it)
*/


#include "globals.h"


#define DEBUG true // Debug Flag

bool isMifareCard();
void setup(void);
void loop(void);


void setup(void)
{

#if DEBUG == true
	Serial.begin(9600);
	Serial.println("Init...");
#endif

	SPI.begin();		// Init SPI bus
	mfrc522.PCD_Init(); // Init MFRC522 card

	uint16_t ID = tft.readID();
	if (ID == 0xD3D3) ID = 0x9481;                     
	tft.begin(ID);

	//Watchdog 8s
	wdt_enable(WDTO_8S); // enable watchdog timer with 8 second timeout (max setting)
						 // wdt will reset the arduino if there is an infinite loop or other hangup; this is a failsafe
} /*END SETUP*/

void loop(void)
{
	readCard();
	if (isMifareCard())
	{
		getCardID();
	}
	//RESETEO EL WATCHDOG EN CADA LOOP
	wdt_reset(); // reset the watchdog timer (once timer is set/reset, next reset pulse must be sent before timeout or arduino reset will occur)

} /*END LOOP*/

/** 
  *   @brief  Check if the card read is Mifare  
  *  
  *   @param  void
  *   @return (bool)
  */
bool isMifareCard()
{

	MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

	wdt_reset();
#if DEBUG == true
	Serial.print(F("PICC type: "));
	Serial.println(mfrc522.PICC_GetTypeName(piccType));
#endif

	// Check is the PICC of Classic MIFARE type
	if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K)
	{
#if DEBUG == true
		Serial.println("The Card isnt a Mifare Card");
#endif
		return false;
	}
	else
	{
		return true;
	}
}

/** 
  *   @brief  Get the Card ID
  *  
  *   @param  void
  *   @return (String) return the Card ID in 4 bytes of HEXA
  */
String getCardID()
{
	String aux;

	byte letter;
	for (byte i = 0; i < mfrc522.uid.size; i++)
	{
		aux.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
		aux.concat(String(mfrc522.uid.uidByte[i], HEX));
		wdt_reset();
	}

	aux.toUpperCase();
#if DEBUG == true
	Serial.print("Card ID:");
	Serial.println(aux.substring(1));
#endif
	wdt_reset();
	return aux.substring(1); //Remove a space in the beginning of the string
}

/** 
  *   @brief  Read a RFID Card  
  *  
  *   @param  void
  *   @return void
  */
void readCard()
{
	// Look for new cards
	while (!mfrc522.PICC_IsNewCardPresent())
	{
		wdt_reset();
	}
	// Select one of the cards
	mfrc522.PICC_ReadCardSerial();
	wdt_reset();

	/*
	while (!mfrc522.PICC_ReadCardSerial())
	{
		wdt_reset();
		
	}
	*/
}
