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

#define SD_CS SS

#include <SPI.h>          // f.k. for Arduino-1.5.2
#include "Adafruit_GFX.h"// Hardware-specific library
#include <MCUFRIEND_kbv.h>
#include <SdFat.h>


#include "Fonts/FreeMonoBold24pt7b.h"
#include "Fonts/FreeMonoBold18pt7b.h"

MCUFRIEND_kbv tft;
SdFat SD;
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




void setup(void);
void loop(void);
unsigned long testFillScreen();
unsigned long testText();
uint16_t read16(File& f);
uint32_t read32(File& f);
uint8_t showBMP(char *nm, int x, int y);
File root;
char namebuf[32] = "/bitmaps/"; //nombre carpeta
int pathlen;

void progmemPrint(const char *str);
void progmemPrintln(const char *str);
void showhanzi(unsigned int x, unsigned int y, unsigned char index);
void windowScroll(int16_t x, int16_t y, int16_t wid, int16_t ht, int16_t dx, int16_t dy, uint16_t *buf);
void runtests(void);
void showBMPFromName( int x, int y, String name);
void printmsg(int row, const char *msg);

uint16_t g_identifier;

char *nm;

uint8_t aspect;
uint16_t pixel;
const char *aspectname[] = {
  "PORTRAIT", "LANDSCAPE", "PORTRAIT_REV", "LANDSCAPE_REV"
};
const char *colorname[] = { "BLUE", "GREEN", "RED", "GRAY" };
uint16_t colormask[] = { 0x001F, 0x07E0, 0xF800, 0xFFFF };
uint16_t dx, rgb, n, wid, ht, msglin;

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
  tft.setRotation(1);    //LANDSCAPE
  bool good = SD.begin(SD_CS);
  if (!good) {
    Serial.print(F("cannot start SD"));
    //PONER UN BACKUP EN TXT SI NO INICIA SD
  }
  //lee la carpeta
  root = SD.open(namebuf);
  pathlen = strlen(namebuf);
  nm = namebuf + pathlen;
  tft.fillScreen(0xFFFFF); //Limpio pantalla

  wid = tft.width();
  ht = tft.height();
  msglin = (ht > 160) ? 200 : 112;


}




void showBMPFromName( int x, int y, String name)
{
  root.rewindDirectory();
  File f = root.openNextFile();
  bool ok = false;
  uint8_t ret;
  uint32_t start;
  Serial.println("IN");
  while (!ok && (f != NULL))
  {
    Serial.println("IN");

    f.getName(nm, 32 - pathlen);
    f.close();
    strlwr(nm);
    if (strstr(nm, ".bmp") != NULL && strstr(nm, (const char *)name.c_str()) != NULL) {
      ok = true;
      Serial.println("IN");
      Serial.print(namebuf);
      Serial.print(F(" - "));

     
      ret = showBMP(namebuf, x, y);
      switch (ret) {
        case 0:
          Serial.println(F("OK"));
 
          break;
        case 1:
          Serial.println(F("bad position"));
          break;
        case 2:
          Serial.println(F("bad BMP ID"));
          break;
        case 3:
          Serial.println(F("wrong number of planes"));
          break;
        case 4:
          Serial.println(F("unsupported BMP format"));
          break;
        default:
          Serial.println(F("unknown"));
          break;
      }
    }
    f = root.openNextFile();
  }
}


void loop(void) {


tft.drawRect(10, 10, 300, 170, BLACK); //(x,y,ancho,largo)
tft.setFont(&FreeMonoBold24pt7b);
  showBMPFromName(180, 185, "logocespi");
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
   tft.setCursor(47, 70);
  tft.println("APROXIME");
  
  tft.setCursor(20, 140);
  tft.println("SU TARJETA");
  
  delay (2000);

  

  tft.fillScreen(0xFFFFF); //Limpio pantalla
  tft.setFont(&FreeMonoBold18pt7b);
  tft.drawRect(10, 10, 300, 220, BLACK); //(x,y,ancho,largo)
 
    tft.setCursor(30, 70);
  tft.println("PROCESANDO...");
   showBMPFromName(120, 130, "wait");
  delay (10000);
  tft.fillScreen(0xFFFFF); //Limpio pantalla



   

#if 0


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
    showBMP(namebuf, 150, 0);
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

#endif
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



#define BMPIMAGEOFFSET 54

#define PALETTEDEPTH   8
#define BUFFPIXEL 20

uint16_t read16(File& f) {
  uint16_t result;         // read little-endian
  result = f.read();       // LSB
  result |= f.read() << 8; // MSB
  return result;
}

uint32_t read32(File& f) {
  uint32_t result;
  result = f.read(); // LSB
  result |= f.read() << 8;
  result |= f.read() << 16;
  result |= f.read() << 24; // MSB
  return result;
}

uint8_t showBMP(char *nm, int x, int y)
{
  File bmpFile;
  int bmpWidth, bmpHeight;    // W+H in pixels
  uint8_t bmpDepth;           // Bit depth (currently must be 24, 16, 8, 4, 1)
  uint32_t bmpImageoffset;    // Start of image data in file
  uint32_t rowSize;           // Not always = bmpWidth; may have padding
  uint8_t sdbuffer[3 * BUFFPIXEL];    // pixel in buffer (R+G+B per pixel)
  uint16_t lcdbuffer[(1 << PALETTEDEPTH) + BUFFPIXEL], *palette = NULL;
  uint8_t bitmask, bitshift;
  boolean flip = true;        // BMP is stored bottom-to-top
  int w, h, row, col, lcdbufsiz = (1 << PALETTEDEPTH) + BUFFPIXEL, buffidx;
  uint32_t pos;               // seek position
  boolean is565 = false;      //

  uint16_t bmpID;
  uint16_t n;                 // blocks read
  uint8_t ret;

  if ((x >= tft.width()) || (y >= tft.height()))
    return 1;               // off screen

  bmpFile = SD.open(nm);      // Parse BMP header
  bmpID = read16(bmpFile);    // BMP signature
  (void) read32(bmpFile);     // Read & ignore file size
  (void) read32(bmpFile);     // Read & ignore creator bytes
  bmpImageoffset = read32(bmpFile);       // Start of image data
  (void) read32(bmpFile);     // Read & ignore DIB header size
  bmpWidth = read32(bmpFile);
  bmpHeight = read32(bmpFile);
  n = read16(bmpFile);        // # planes -- must be '1'
  bmpDepth = read16(bmpFile); // bits per pixel
  pos = read32(bmpFile);      // format
  if (bmpID != 0x4D42) ret = 2; // bad ID
  else if (n != 1) ret = 3;   // too many planes
  else if (pos != 0 && pos != 3) ret = 4; // format: 0 = uncompressed, 3 = 565
  else {
    bool first = true;
    is565 = (pos == 3);               // ?already in 16-bit format
    // BMP rows are padded (if needed) to 4-byte boundary
    rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
    if (bmpHeight < 0) {              // If negative, image is in top-down order.
      bmpHeight = -bmpHeight;
      flip = false;
    }

    w = bmpWidth;
    h = bmpHeight;
    if ((x + w) >= tft.width())       // Crop area to be loaded
      w = tft.width() - x;
    if ((y + h) >= tft.height())      //
      h = tft.height() - y;

    if (bmpDepth <= PALETTEDEPTH) {   // these modes have separate palette
      bmpFile.seek(BMPIMAGEOFFSET); //palette is always @ 54
      bitmask = 0xFF;
      if (bmpDepth < 8)
        bitmask >>= bmpDepth;
      bitshift = 8 - bmpDepth;
      n = 1 << bmpDepth;
      lcdbufsiz -= n;
      palette = lcdbuffer + lcdbufsiz;
      for (col = 0; col < n; col++) {
        pos = read32(bmpFile);    //map palette to 5-6-5
        palette[col] = ((pos & 0x0000F8) >> 3) | ((pos & 0x00FC00) >> 5) | ((pos & 0xF80000) >> 8);
      }
    }

    // Set TFT address window to clipped image bounds
    tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
    for (row = 0; row < h; row++) { // For each scanline...
      // Seek to start of scan line.  It might seem labor-
      // intensive to be doing this on every line, but this
      // method covers a lot of gritty details like cropping
      // and scanline padding.  Also, the seek only takes
      // place if the file position actually needs to change
      // (avoids a lot of cluster math in SD library).
      uint8_t r, g, b, *sdptr;
      int lcdidx, lcdleft;
      if (flip)   // Bitmap is stored bottom-to-top order (normal BMP)
        pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
      else        // Bitmap is stored top-to-bottom
        pos = bmpImageoffset + row * rowSize;
      if (bmpFile.position() != pos) { // Need seek?
        bmpFile.seek(pos);
        buffidx = sizeof(sdbuffer); // Force buffer reload
      }

      for (col = 0; col < w; ) {  //pixels in row
        lcdleft = w - col;
        if (lcdleft > lcdbufsiz) lcdleft = lcdbufsiz;
        for (lcdidx = 0; lcdidx < lcdleft; lcdidx++) { // buffer at a time
          uint16_t color;
          // Time to read more pixel data?
          if (buffidx >= sizeof(sdbuffer)) { // Indeed
            bmpFile.read(sdbuffer, sizeof(sdbuffer));
            buffidx = 0; // Set index to beginning
            r = 0;
          }
          switch (bmpDepth) {          // Convert pixel from BMP to TFT format
            case 24:
              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];
              color = tft.color565(r, g, b);
              break;
            case 16:
              b = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];
              if (is565)
                color = (r << 8) | (b);
              else
                color = (r << 9) | ((b & 0xE0) << 1) | (b & 0x1F);
              break;
            case 1:
            case 4:
            case 8:
              if (r == 0)
                b = sdbuffer[buffidx++], r = 8;
              color = palette[(b >> bitshift) & bitmask];
              r -= bmpDepth;
              b <<= bmpDepth;
              break;
          }
          lcdbuffer[lcdidx] = color;

        }
        tft.pushColors(lcdbuffer, lcdidx, first);
        first = false;
        col += lcdidx;
      }           // end cols
    }               // end rows
    tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1); //restore full screen
    ret = 0;        // good render
  }
  bmpFile.close();
  return (ret);
}

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




void windowScroll(int16_t x, int16_t y, int16_t wid, int16_t ht, int16_t dx, int16_t dy, uint16_t *buf)
{
  if (dx) for (int16_t row = 0; row < ht; row++) {
    
      tft.setAddrWindow(x, y + row, x + wid - 1, y + row);
      tft.pushColors(buf + dx, wid - dx, 1);
      tft.pushColors(buf + 0, dx, 0);
    }
  if (dy) for (int16_t col = 0; col < wid; col++) {
     
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
