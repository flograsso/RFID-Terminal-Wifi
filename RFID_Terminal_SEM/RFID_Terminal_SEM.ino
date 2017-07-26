/**
    RFID Terminal

    @author Federico Lo Grasso
    @version 1.0


	Info: RC522 reads Mifare cards, a 13.56 MHz contactless smart card standard.
	The standard cards that we will use are the MIFARE Classic® 13.56Mhz 1Kb. It has 1024 bytes of memory
	divided in 16 sectors and protected by 2 different security keys (A and B). Each sector has 4 blocks with 16Kb
	The card could have UID (Unique ID) or NUID (Non-Unique ID). It is a 4 bytes number and it saved on the Sector 0 Block 0
	in the card. We can modifiy it in the Mifare Classic Cards (in specials card we can do it)

	TFT: 
	Si uso la font default el x,y al escibir es la esq sup izq del txt pero si cambio la font
	el x,y pasado es el de la izq inferior. En la letra FreeMonoBold18pt7b arrancaria en x=0 e y=25
	La pantalla comienza en:
		Para Imagen y drawRect x=0 e y=0
	Configuracion para mensaje de 2 lineas:
		tft.drawRect(10, 45, 300, 150, BLACK); //(x,y,ancho,largo)
		tft.setCursor(20, 105);
		tft.println("MENSAJE"); HASTA 10 CARACTERES
		tft.setCursor(20, 165);
		tft.println("MENSAJE"); HASTA 10 CARACTERES
	
	ESP8266-12E
		AT Commands Before Flash: http://www.instructables.com/id/ESP-12F-ESP8266-Module-Connection-Test/
		Para usar el ESP de forma autónoma. Hay que flashearlo con el firmware del NodeMcu, una vez 
		flasheado. Se puede subir programas desde el sketch de arduino seleccionando como placa "ESP8266 Generic"
		Para flashearlo hay que descargar este soft:
			https://github.com/nodemcu/nodemcu-flasher
		Y bajar el nodemcu_integer_0.9.6-dev_20150704.bin de aca:
			https://github.com/nodemcu/nodemcu-firmware/releases
		Las conexiones para realizar el flash son: (utilizar un USB2UART)
			VCC - > 3.3V 
			GND - > GND 
			RX - > TX 
			TX - > RX
			CH_PD (EN)- > 3.3V 
			GPIO15 - > GND
			GPIO0 (FLASH MODE) -> BUTTON TO GND (Lo reinicio con el boton apretado para poder cargarle un programa)
			Conviene alimentarlo con una fuente externa, en ese caso conectar el GND del USB2UART al GND de la fuente.

		En el soft selecciono 115200 en baud rate y elijo el bin que baje.
		https://roboindia.com/tutorials/Flash-esp8266-with-nodemcu-firmware
		http://www.whatimade.today/loading-the-nodemcu-firmware-on-the-esp8266-windows-guide/

		Puedo programarlo desde el IDE de Arduino o bien desde un soft llamado ESplorer y LUA

		Luego para cargar los programas al ESP debo iniciar el modulo con el GPIO0 sin conectar, y Luego
		volver a iniciarlo con el GPIO0 en GND.

		
*/

#include "globals.h"

#define DEBUG true // Debug Flag

void showBMPFromName(int x, int y, String name, bool center = false);
void print2LinesMsg(String line1 = "", String line2 = "", int letterSize = 24);
void printTFT(screenState state, String line1 = "", String line2 = "", int letterSize = 24);
void initESP8266();
bool isMifareCard();
char *getCardID();
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

	pinMode(ESP8266_RESET_PIN, INPUT); //ESP8266 Reset Button
	Serial2.begin(9600);			   //ESP8266 uses Serial2

	initTFT();

	//Watchdog 8s
	wdt_enable(WDTO_8S); // enable watchdog timer with 8 second timeout (max setting)
						 // wdt will reset the arduino if there is an infinite loop or other hangup; this is a failsafe

	/*Boton de reset del ESP8266. Solo lo toma si se prende el modulo con el boton presionado*/
	if (digitalRead(ESP8266_RESET_PIN) == HIGH)
	{
#if DEBUG == true
		Serial.println(F("Reseteando..."));
#endif
		print2LinesMsg("RESETEANDO", "WIFI");
		wdt_reset();
		ESP8266Status = false;
		sendESP8266Command("RESET_WIFI", 10, "RESET_OK");
		wdt_reset();
		sendESP8266Command("RECONNECT_WIFI", 1, "WIFI_OK");
	}

	initESP8266();

} /*END SETUP*/

void loop(void)
{
#if DEBUG == true
	Serial.println("Starting loop...");
#endif

	printTFT(STANDBY);
	readCard();
	if (isMifareCard())
	{
		UID_Readed = (String)getCardID();
		/*Chequeo que si el UID fue el ultimo leido hallan pasado UID_EXPIRE_MS antes de la proxima lectura */
		if (UID_Readed.indexOf(Last_UID_Readed) != -1 && (millis() - cardStartTime < UID_EXPIRE_MS ))
		{
			printTFT(ERROR);
			printTFT(MENSAJE, "VUELVA A", "INTENTAR");
			delay(2000);
		}
		else
		{
			/*....... */
			cardStartTime = millis();
			printTFT(WAIT);
			printTFT(OK);
			printTFT(ERROR);
		}
		Last_UID_Readed = UID_Readed;
	}

	//RESETEO EL WATCHDOG EN CADA LOOP
	wdt_reset(); // reset the watchdog timer (once timer is set/reset, next reset pulse must be sent before timeout or arduino reset will occur)

} /*END LOOP*/

/** 
  *   @brief  Initialize TFT screen
  *  
  *   @return void
  */
void initTFT()
{
	ID = tft.readID();
	tft.begin(ID);
	wdt_reset();
	tft.setRotation(1);		 //Landscape
	tft.fillScreen(0xFFFFF); //Pongo todos los pixels a blanco
	wdt_reset();

#if DEBUG == true
	Serial.print(F("TFT ID = 0x"));
	Serial.println(ID, HEX);
#endif

	//Inicializo SD
	SD_Status = SD.begin(SD_CS);
	wdt_reset();
	if (!SD_Status)
	{
#if DEBUG == true
		Serial.print(F("cannot start SD"));
#endif
		//PONER UN BACKUP EN TXT SI NO INICIA SD
	}

	//Leo la carpeta
	root = SD.open(namebuf);
	pathlen = strlen(namebuf);
	nm = namebuf + pathlen;

	//Leo ancho y alto de la pantalla.
	wid = tft.width();
	ht = tft.height();
	wdt_reset();
}

/** 
  *   @brief  Initialize ESP8266 through serial Commands. Modify ESP8266Status variable
  *  
  *   @return void
  */
void initESP8266()
{
	printTFT(MENSAJE, "CONECTANDO", "WIFI");

	while (!sendESP8266Command("WIFI_STATUS", 130, "WIFI_OK"))
	{
		wdt_reset();
	}

	printTFT(MENSAJE, "CONECTANDO", "A INTERNET");
	wdt_reset();

	while (!sendESP8266Command("CHECK_CONNECTION", 10, "CONNECTION_OK"))
	{
		wdt_reset();
	}

	ESP8266Status = true;
	printTFT(MENSAJE, "CONEXION", "OK");
	delay(1500);
}

bool sendESP8266Command(String command, int timeout, String responseOK)
{
	Serial2.println(command);
	windowStartTime = millis();

	while (!Serial2.available() && ((millis() - windowStartTime) < timeout * 1000))
	{
		wdt_reset();
	};
	if (Serial2.available())
	{
		recibido = Serial2.readString();
#if DEBUG == true
		Serial.print("Recibido por Serial2=");
		Serial.println(recibido);
#endif
		if (recibido.indexOf(responseOK) != -1)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	else
	{
#if DEBUG == true
		Serial.println("Serial2 Timeout");
#endif
		return false;
	}
}

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
  *	  The Card ID are 4 bytes of Hexa (Example: 1A B0 51 CA) Max FF FF FF FF 
  *   This function convert it to Decimal (base 10) Max 255 255 255 255
  *
  *   @param  void
  *   @return (char []) return the Card ID in decimal without spaces. Max 255255255255
  *						if number have less than 12 characters, complete with 0s at left
  */
char *getCardID()
{
	String aux;
	char cadena[UID_LENGTH];
	byte letter;

	//Convert Hexa to Dec
	for (byte i = 0; i < mfrc522.uid.size; i++)
	{
		aux.concat(String(mfrc522.uid.uidByte[i], DEC));
		wdt_reset();
	}

	//Completo con ceros en el vector a izquierda. Es necesario para que el ESP logre entender el comando
	for (int i = 0; i < UID_LENGTH; i++)
	{
		wdt_reset();

		if (i < UID_LENGTH - aux.length())
			cadena[i] = '0';

		else
			cadena[i] = aux[i - (UID_LENGTH - aux.length())];
	}

#if DEBUG == true
	Serial.print("Card ID:");
	Serial.println((String)cadena);
#endif
	wdt_reset();
	return cadena;
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

/** 
  *   @brief  Show BMP
  *  
  *   @param  string bmp name
  *   @param  x x point where start to plot
  *   @param  y y point where start to plot
  *   @param center (boolean) if center o no. Default=false
  *   @return void
  */
void showBMPFromName(int x, int y, String name, bool center = false)
{
	root.rewindDirectory();
	File f = root.openNextFile();
	bool ok = false;
	uint8_t ret;
	uint32_t start;
	while (!ok && (f != NULL))
	{
		wdt_reset();
		f.getName(nm, 32 - pathlen);
		f.close();
		strlwr(nm);
		if (strstr(nm, ".bmp") != NULL && strstr(nm, (const char *)name.c_str()) != NULL)
		{
			ok = true;
#if DEBUG == true
			Serial.print("BMP Name: ");
			Serial.print(namebuf);
			Serial.println("Encontrado!");
#endif
			wdt_reset();
			ret = showBMP(namebuf, x, y, center);
			switch (ret)
			{
			case 0:
#if DEBUG == true
				Serial.println(F("OK"));
#endif
				break;
			case 1:
#if DEBUG == true
				Serial.println(F("bad position"));
#endif
				break;
			case 2:
#if DEBUG == true
				Serial.println(F("bad BMP ID"));
#endif
				break;
			case 3:
#if DEBUG == true
				Serial.println(F("wrong number of planes"));
#endif
				break;
			case 4:
#if DEBUG == true
				Serial.println(F("unsupported BMP format"));
#endif
				break;
			default:
#if DEBUG == true
				Serial.println(F("unknown"));
#endif
				break;
			}
		}
		f = root.openNextFile();
	}
}

#define BMPIMAGEOFFSET 54
#define PALETTEDEPTH 8
#define BUFFPIXEL 20

uint16_t read16(File &f)
{
	uint16_t result;		 // read little-endian
	result = f.read();		 // LSB
	result |= f.read() << 8; // MSB
	return result;
}

uint32_t read32(File &f)
{
	uint32_t result;
	result = f.read(); // LSB
	result |= f.read() << 8;
	result |= f.read() << 16;
	result |= f.read() << 24; // MSB
	return result;
}

/** 
  *   ***Not use this function in program. Call "showBMPFromName" who call this function***
  *   @brief  Show bmp on the display
  *  
  *   @param  *nm pointer to the bmp name
  *   @param  x x point where start to plot
  *   @param  y y point where start to plot
  *   @return uint8_t
  */
uint8_t showBMP(char *nm, int x, int y, bool center)
{
	File bmpFile;
	int bmpWidth, bmpHeight;		 // W+H in pixels
	uint8_t bmpDepth;				 // Bit depth (currently must be 24, 16, 8, 4, 1)
	uint32_t bmpImageoffset;		 // Start of image data in file
	uint32_t rowSize;				 // Not always = bmpWidth; may have padding
	uint8_t sdbuffer[3 * BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
	uint16_t lcdbuffer[(1 << PALETTEDEPTH) + BUFFPIXEL], *palette = NULL;
	uint8_t bitmask, bitshift;
	boolean flip = true; // BMP is stored bottom-to-top
	int w, h, row, col, lcdbufsiz = (1 << PALETTEDEPTH) + BUFFPIXEL, buffidx;
	uint32_t pos;		   // seek position
	boolean is565 = false; //

	uint16_t bmpID;
	uint16_t n; // blocks read
	uint8_t ret;
	wdt_reset();
	if ((x >= tft.width()) || (y >= tft.height()))
		return 1; // off screen

	bmpFile = SD.open(nm);			  // Parse BMP header
	bmpID = read16(bmpFile);		  // BMP signature
	(void)read32(bmpFile);			  // Read & ignore file size
	(void)read32(bmpFile);			  // Read & ignore creator bytes
	bmpImageoffset = read32(bmpFile); // Start of image data
	(void)read32(bmpFile);			  // Read & ignore DIB header size
	bmpWidth = read32(bmpFile);
	bmpHeight = read32(bmpFile);
	n = read16(bmpFile);		// # planes -- must be '1'
	bmpDepth = read16(bmpFile); // bits per pixel
	pos = read32(bmpFile);		// format
	wdt_reset();

	//Centro la imagen
	if (center == true)
	{
		x = 160 - bmpWidth / 2;
		y = 120 - bmpHeight / 2;
	}

	if (bmpID != 0x4D42)
		ret = 2; // bad ID
	else if (n != 1)
		ret = 3; // too many planes
	else if (pos != 0 && pos != 3)
		ret = 4; // format: 0 = uncompressed, 3 = 565
	else
	{
		bool first = true;
		is565 = (pos == 3); // ?already in 16-bit format
		// BMP rows are padded (if needed) to 4-byte boundary
		rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
		if (bmpHeight < 0)
		{ // If negative, image is in top-down order.
			bmpHeight = -bmpHeight;
			flip = false;
		}

		w = bmpWidth;
		h = bmpHeight;
		if ((x + w) >= tft.width()) // Crop area to be loaded
			w = tft.width() - x;
		if ((y + h) >= tft.height()) //
			h = tft.height() - y;

		if (bmpDepth <= PALETTEDEPTH)
		{								  // these modes have separate palette
			bmpFile.seek(BMPIMAGEOFFSET); //palette is always @ 54
			bitmask = 0xFF;
			if (bmpDepth < 8)
				bitmask >>= bmpDepth;
			bitshift = 8 - bmpDepth;
			n = 1 << bmpDepth;
			lcdbufsiz -= n;
			palette = lcdbuffer + lcdbufsiz;
			for (col = 0; col < n; col++)
			{
				pos = read32(bmpFile); //map palette to 5-6-5
				palette[col] = ((pos & 0x0000F8) >> 3) | ((pos & 0x00FC00) >> 5) | ((pos & 0xF80000) >> 8);
			}
		}
		wdt_reset();
		// Set TFT address window to clipped image bounds
		tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
		for (row = 0; row < h; row++)
		{ // For each scanline...
			// Seek to start of scan line.  It might seem labor-
			// intensive to be doing this on every line, but this
			// method covers a lot of gritty details like cropping
			// and scanline padding.  Also, the seek only takes
			// place if the file position actually needs to change
			// (avoids a lot of cluster math in SD library).
			uint8_t r, g, b, *sdptr;
			wdt_reset();
			int lcdidx, lcdleft;
			if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
				pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
			else // Bitmap is stored top-to-bottom
				pos = bmpImageoffset + row * rowSize;
			if (bmpFile.position() != pos)
			{ // Need seek?
				bmpFile.seek(pos);
				buffidx = sizeof(sdbuffer); // Force buffer reload
			}

			for (col = 0; col < w;)
			{
				wdt_reset();
				//pixels in row
				lcdleft = w - col;
				if (lcdleft > lcdbufsiz)
					lcdleft = lcdbufsiz;
				for (lcdidx = 0; lcdidx < lcdleft; lcdidx++)
				{ // buffer at a time
					uint16_t color;
					// Time to read more pixel data?
					if (buffidx >= sizeof(sdbuffer))
					{ // Indeed
						bmpFile.read(sdbuffer, sizeof(sdbuffer));
						buffidx = 0; // Set index to beginning
						r = 0;
					}
					switch (bmpDepth)
					{ // Convert pixel from BMP to TFT format
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
			}														// end cols
		}															// end rows
		tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1); //restore full screen
		ret = 0;
		wdt_reset(); // good render
	}
	bmpFile.close();
	return (ret);
}

/** 
  *   @brief  Plot on the tft display
  *  
  *   @param  screenState  (screen to show)
  *	  @param line 1
  *	  @param line 2
  *	  @param  letterSize 24 (max 10 char) or 18 (max 13 char) per line
  *   @return void
  */
void printTFT(screenState state, String line1 = "", String line2 = "", int letterSize = 24)
{
	switch (state)
	{
	//Esperando una tarjeta...
	case STANDBY:
		tft.fillScreen(0xFFFFF);			   //Limpio pantalla
		tft.drawRect(10, 10, 300, 170, BLACK); //(x,y,ancho,largo)
		tft.setFont(&FreeMonoBold24pt7b);
		tft.setTextSize(1);
		tft.setTextColor(BLACK);
		wdt_reset();
		tft.setCursor(47, 70);
		tft.println("APROXIME");
		tft.setCursor(20, 140);
		tft.println("SU TARJETA");
		showBMPFromName(180, 185, "logocespi");
		wdt_reset();
		break;

	//Procesando...
	case WAIT:
		tft.fillScreen(0xFFFFF); //Limpio pantalla
		if (SD_Status)			 //Si esta insertada la SD
		{
			tft.setFont(&FreeMonoBold18pt7b);
			tft.setTextSize(1);
			tft.setTextColor(BLACK);
			tft.drawRect(10, 10, 300, 220, BLACK); //(x,y,ancho,largo)
			wdt_reset();
			tft.setCursor(30, 70);
			tft.println("PROCESANDO...");
			showBMPFromName(120, 130, "wait");
			wdt_reset();
			delay(5000);
			wdt_reset();
		}
		else
		{
			print2LinesMsg("PROCESANDO");
		}
		break;
	case OK:
		tft.fillScreen(0xFFFFF); //Limpio pantalla
		if (SD_Status)			 //Si esta insertada la SD
		{
			showBMPFromName(0, 0, "logook", true);
			wdt_reset();
			delay(600);
			wdt_reset();
		}
		else
		{
			print2LinesMsg("OPERACION", "CORRECTA");
			delay(2000);
			wdt_reset();
		}
		break;

	case ERROR:
		tft.fillScreen(0xFFFFF); //Limpio pantalla
		if (SD_Status)			 //Si esta insertada la SD
		{
			showBMPFromName(0, 0, "logoerror", true);
			wdt_reset();
			delay(600);
			wdt_reset();
		}
		else
		{
			print2LinesMsg("ERROR");
			delay(2000);
			wdt_reset();
		}
		break;
	case MENSAJE:
		print2LinesMsg(line1, line2, letterSize);
		wdt_reset();
		break;
	}
}

/** 
  *   @brief  Plot 2 lines in a box.
  *  
  *   @param  line 1 
  *   @param  line 2 
  *   @param  letterSize 24 (max 10 char) or 18 (max 13 char) per line
  */
void print2LinesMsg(String line1 = "", String line2 = "", int letterSize = 24)
{
	tft.fillScreen(0xFFFFF); //Limpio pantalla
	tft.setTextColor(BLACK);

	if (letterSize == 24)
	{
		tft.setFont(&FreeMonoBold24pt7b);
		/*
		* X=0-10--> margen 10-->310 cuadrado 310-320-->margen
		* Y=0-45--> margen 45-->195 cuadrado 195-240-->margen
		*/
		tft.drawRect(10, 45, 300, 150, BLACK); //(x,y,ancho,largo)

		/*Tamaño de letra 24: Max 10 caracteres x linea. Corto el resto*/
		line1 = line1.substring(0, 10);
		line2 = line2.substring(0, 10);

		/* Cada letra en x me ocupa aprox 28px
		 * Calculo el x de la primera linea, si tengo 10 caracteres debe ser 20.
		 * Entonces proporcionalmente hago 20 + la cantidad de caracteres de menos que tengo
		 * y lo multiplico por 14 (la mitad de 28) asi dejo el mismo margen a izq y a der
		*/

		int x1 = 20 + (10 - line1.length()) * 14;
		int x2 = 20 + (10 - line2.length()) * 14;
		tft.setCursor(x1, 105);
		tft.println(line1);
		tft.setCursor(x2, 165);
		tft.println(line2);
	}
	else
	{
		if (letterSize == 18)
		{
			tft.setFont(&FreeMonoBold18pt7b);
			/*
			* X=0-10--> margen 10-->310 cuadrado 310-320-->margen
			* Y=0-45--> margen 45-->195 cuadrado 195-240-->margen
			*/
			tft.drawRect(10, 45, 300, 150, BLACK); //(x,y,ancho,largo)

			/*Tamaño de letra 18: Max 13 caracteres x linea. Corto el resto*/
			line1 = line1.substring(0, 13);
			line2 = line2.substring(0, 13);

			/* Cada letra en x me ocupa aprox 22px
			 * Calculo el x de la primera linea, si tengo 13 caracteres debe ser 20.
			 * Entonces proporcionalmente hago 20 + la cantidad de caracteres de menos que tengo
			 * y lo multiplico por 11 (la mitad de 22) asi dejo el mismo margen a izq y a der
			*/

			int x1 = 20 + (13 - line1.length()) * 11;
			int x2 = 20 + (13 - line2.length()) * 11;

			tft.setCursor(x1, 105);
			tft.println(line1);
			tft.setCursor(x2, 165);
			tft.println(line2);
		}
	}
}
