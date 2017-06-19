// All the mcufriend.com UNO shields have the same pinout.
// i.e. control pins A0-A4.  Data D2-D9.  microSD D10-D13.
// Touchscreens are normally A1, A2, D7, D6 but the order varies
//
// This demo should work with most Adafruit TFT libraries
// If you are not using a shield,  use a full Adafruit constructor()
// e.g. Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

#include <SPI.h>          // f.k. for Arduino-1.5.2
#include "Adafruit_GFX.h"// Hardware-specific library
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
//#include <Adafruit_TFTLCD.h>
//Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

void setup(void);
void loop(void);
unsigned long testFillScreen();
unsigned long testText();


void progmemPrint(const char *str);
void progmemPrintln(const char *str);

void runtests(void);

uint16_t g_identifier;

extern const uint8_t hanzi[];
void showhanzi(unsigned int x, unsigned int y, unsigned char index)
{
    uint8_t i, j, c, first = 1;
    uint8_t *temp = (uint8_t*)hanzi;
    uint16_t color;
    tft.setAddrWindow(x, y, x + 31, y + 31); //设置区域
    temp += index * 128;
    for (j = 0; j < 128; j++)
    {
        c = pgm_read_byte(temp);
        for (i = 0; i < 8; i++)
        {
            if ((c & (1 << i)) != 0)
            {
                color = RED;
            }
            else
            {
                color = BLACK;
            }
            tft.pushColors(&color, 1, first);
            first = 0;
        }
        temp++;
    }
}

void setup(void) {
    Serial.begin(9600);
    uint32_t when = millis();
    //    while (!Serial) ;   //hangs a Leonardo until you connect a Serial
    if (!Serial) delay(5000);           //allow some time for Leonardo
    Serial.println("Serial took " + String((millis() - when)) + "ms to start");
    //    tft.reset();                 //hardware reset
    uint16_t ID = tft.readID(); //
    Serial.print("ID = 0x");
    Serial.println(ID, HEX);
    if (ID == 0xD3D3) ID = 0x9481; // write-only shield
//    ID = 0x9329;                             // force ID
    tft.begin(ID);
}

#if defined(MCUFRIEND_KBV_H_)
uint16_t scrollbuf[320];    // my biggest screen is 320x480
#define READGRAM(x, y, buf, w, h)  tft.readGRAM(x, y, buf, w, h)
#else
uint16_t scrollbuf[320];    // Adafruit only does 240x320
// Adafruit can read a block by one pixel at a time
int16_t  READGRAM(int16_t x, int16_t y, uint16_t *block, int16_t w, int16_t h)
{
    uint16_t *p;
    for (int row = 0; row < h; row++) {
        p = block + row * w;
        for (int col = 0; col < w; col++) {
            *p++ = tft.readPixel(x + col, y + row);
        }
    }
}
#endif

void windowScroll(int16_t x, int16_t y, int16_t wid, int16_t ht, int16_t dx, int16_t dy, uint16_t *buf)
{
    if (dx) for (int16_t row = 0; row < ht; row++) {
            READGRAM(x, y + row, buf, wid, 1);
            tft.setAddrWindow(x, y + row, x + wid - 1, y + row);
            tft.pushColors(buf + dx, wid - dx, 1);
            tft.pushColors(buf + 0, dx, 0);
        }
    if (dy) for (int16_t col = 0; col < wid; col++) {
            READGRAM(x + col, y, buf, 1, ht);
            tft.setAddrWindow(x + col, y, x + col, y + ht - 1);
            tft.pushColors(buf + dy, ht - dy, 1);
            tft.pushColors(buf + 0, dy, 0);
        }
}

void printmsg(int row, const char *msg)
{
    tft.setTextColor(YELLOW, BLACK);
    tft.setCursor(0, row);
    tft.println(msg);
}

void loop(void) {
    uint8_t aspect;
    uint16_t pixel;
    const char *aspectname[] = {
        "PORTRAIT", "LANDSCAPE", "PORTRAIT_REV", "LANDSCAPE_REV"
    };
    const char *colorname[] = { "BLUE", "GREEN", "RED", "GRAY" };
    uint16_t colormask[] = { 0x001F, 0x07E0, 0xF800, 0xFFFF };
    uint16_t dx, rgb, n, wid, ht, msglin;

    if (tft.height() > 64) {
            aspect = 1;
            tft.setRotation(aspect);
            wid = tft.width();
            ht = tft.height();
            msglin = (ht > 160) ? 200 : 112;
            testText();
            dx = wid / 32;
            for (n = 0; n < 32; n++) {
                rgb = n * 8;
                rgb = tft.color565(rgb, rgb, rgb);
                tft.fillRect(n * dx, 48, dx, 63, rgb & colormask[aspect]);
            }
            tft.drawRect(0, 48 + 63, wid, 1, WHITE);
            tft.setTextSize(2);
            tft.setTextColor(colormask[aspect], BLACK);
            tft.setCursor(0, 72);
            tft.print(colorname[aspect]);
            tft.setTextColor(WHITE);
            tft.println(" COLOR GRADES");
            tft.setTextColor(WHITE, BLACK);
            printmsg(184, aspectname[aspect]);
            delay(1000);
            tft.drawPixel(0, 0, YELLOW);
            pixel = tft.readPixel(0, 0);
            tft.setTextSize((ht > 160) ? 2 : 1); //for messages
#if defined(MCUFRIEND_KBV_H_)
#if 1
            extern const uint8_t wifi_full[];
            tft.setAddrWindow(wid - 40 - 40, 20 + 0, wid - 40 - 40 + 31, 20 + 31);
            tft.pushColors(wifi_full, 1024, 1, true);
#elif 1
            extern const uint8_t wifi_full[];
            tft.setAddrWindow(wid - 40 - 40, 20 + 0, wid - 40 - 40 + 31, 20 + 31);
            tft.pushColors(wifi_full, 1024, 1);
#elif 1
            extern const uint8_t icon_40x40[];
            tft.setAddrWindow(wid - 40 - 40, 20 + 0, wid - 1 - 40, 20 + 39);
            tft.pushColors(icon_40x40, 1600, 1);
#endif
            tft.setAddrWindow(0, 0, wid - 1, ht - 1);
            if (aspect & 1) tft.drawRect(wid - 1, 0, 1, ht, WHITE);
            else tft.drawRect(0, ht - 1, wid, 1, WHITE);
            printmsg(msglin, "VERTICAL SCROLL UP");
            uint16_t maxscroll;
            if (tft.getRotation() & 1) maxscroll = wid;
            else maxscroll = ht;
            for (uint16_t i = 1; i <= maxscroll; i++) {
                tft.vertScroll(0, maxscroll, i);
                delay(10);
            }
            delay(1000);
			printmsg(msglin, "VERTICAL SCROLL DN");
            for (uint16_t i = 1; i <= maxscroll; i++) {
                tft.vertScroll(0, maxscroll, 0 - (int16_t)i);
                delay(10);
            }
			tft.vertScroll(0, maxscroll, 0);
            printmsg(msglin, "SCROLL DISABLED   ");

            delay(1000);
            if ((aspect & 1) == 0) { //Portrait
                tft.setTextColor(BLUE, BLACK);
                printmsg(msglin, "ONLY THE COLOR BAND");
                for (uint16_t i = 1; i <= 64; i++) {
                    tft.vertScroll(48, 64, i);
                    delay(20);
                }
                delay(1000);
            }
#endif
            tft.setTextColor(YELLOW, BLACK);
            if (pixel == YELLOW) {
                printmsg(msglin, "SOFTWARE SCROLL    ");
#if 0
                // diagonal scroll of block
                for (int16_t i = 45, dx = 2, dy = 1; i > 0; i -= dx) {
                    windowScroll(24, 8, 90, 40, dx, dy, scrollbuf);
                }
#else
                // plain horizontal scroll of block
                n = (wid > 320) ? 320 : wid;
                for (int16_t i = n, dx = 4, dy = 0; i > 0; i -= dx) {
                    windowScroll(0, 200, n, 16, dx, dy, scrollbuf);
                }
#endif
            }
            else if (pixel == CYAN)
                tft.println("readPixel() reads as BGR");
            else if ((pixel & 0xF8F8) == 0xF8F8)
                tft.println("readPixel() should be 24-bit");
            else {
                tft.print("readPixel() reads 0x");
                tft.println(pixel, HEX);
            }
            delay(5000);
        
    }
    printmsg(msglin, "INVERT DISPLAY ");
    tft.invertDisplay(true);
    delay(2000);
    tft.invertDisplay(false);
}

typedef struct {
    PGM_P msg;
    uint32_t ms;
} TEST;
TEST result[12];

#define RUNTEST(n, str, test) { result[n].msg = PSTR(str); result[n].ms = test; delay(500); }

void runtests(void)
{
    uint8_t i, len = 24, cnt;
    uint32_t total;
    RUNTEST(1, "Text                     ", testText());
 
}



unsigned long testText() {
    unsigned long start;
    tft.fillScreen(BLACK);
    start = micros();
    tft.setCursor(0, 0);
    tft.setTextColor(WHITE);  tft.setTextSize(1);
    tft.println("Hello World!");
    tft.setTextColor(YELLOW); tft.setTextSize(2);
    tft.println(123.45);
    tft.setTextColor(RED);    tft.setTextSize(3);
    tft.println(0xDEADBEEF, HEX);
    tft.println();
    tft.setTextColor(GREEN);
    tft.setTextSize(5);
    tft.println("Groop");
    tft.setTextSize(2);
    tft.println("I implore thee,");
    tft.setTextSize(1);
    tft.println("my foonting turlingdromes.");
    tft.println("And hooptiously drangle me");
    tft.println("with crinkly bindlewurdles,");
    tft.println("Or I will rend thee");
    tft.println("in the gobberwarts");
    tft.println("with my blurglecruncheon,");
    tft.println("see if I don't!");
    return micros() - start;
}





