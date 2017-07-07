#ifndef GLOBALS_H
#define GLOBALS_H


#include <SPI.h>
#include "MFRC522.h"
#include <avr/wdt.h> //Watchdog
#include "Adafruit_GFX.h"// Hardware-specific library
#include <MCUFRIEND_kbv.h>
#include <SdFat.h>


/*RFID READER*/
/*
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             49         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            31        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/

#define RST_PIN             49           // Configurable, see typical pin layout above
#define SDA_PIN             31          //SPI SS CHIP SELECT = 31

MFRC522 mfrc522(SDA_PIN, RST_PIN);   // Create MFRC522 instance.

/*TFT DISPLAY*/
/*
 * -----------------------------------------------------------------------------------------
 *     TFT         Arduino Mega        
 *     Pin          Pin          
 * -----------------------------------------------------------------------------------------
 * LCD_RST          A4            
 * LCD_CS           A3            
 * LCD_RS(CD)       A2  
 * LCD_WR           A1 
 * LCD_RD           A0
 *
 * LCD_D2           D2
 * LCD_D3           D3 
 * LCD_D4           D4 
 * LCD_D5           D5 
 * LCD_D6           D6 
 * LCD_D7           D7
 * LCD_D0           D8 
 * LCD_D2           D9        
 *
 * SD_SS(SDA)       53
 * SD_MOSI(DI)      51
 * SD_MISO(DO)      50
 * SD_SCK           52

*/


#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#define SD_CS SS  //SPI SS CHIP SELECT = 53

MCUFRIEND_kbv tft;
SdFat SD;

File root;
char namebuf[32] = "/bitmaps/"; //Nombre de la carpeta donde estan las imgs
int pathlen;
char *nm;
uint16_t dx, rgb, n, wid, ht, msglin;
uint16_t ID;

#include "Fonts/FreeMonoBold24pt7b.h"
#include "Fonts/FreeMonoBold18pt7b.h"
#include "Fonts/FreeMonoBold9pt7b.h"

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


//Son las diferentes pantallas del TFT
enum screenState {
    STANDBY,
    WAIT,
    OK,
    ERROR,
    MENSAJE
};

#endif
