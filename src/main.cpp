#include <Arduino.h>
#define T4_V13

#if defined(T4_V13)
#include "T4_V13.h"
#else
#error "please select board version"
#endif


#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include <Ticker.h>
#include <Button2.h>
#include <SD.h>

TFT_eSPI tft = TFT_eSPI();
int tftRotation = 1;

// used for GifDraw logic
// https://github.com/bitbank2/AnimatedGIF/blob/master/examples/TFT_eSPI_memory/GIFDraw.ino
#define DISPLAY_WIDTH  tft.width()
#define DISPLAY_HEIGHT tft.height()
#define BUFFER_SIZE 320 // Optimum is >= GIF width or integral division of width
#ifdef USE_DMA
  uint16_t usTemp[2][BUFFER_SIZE]; // Global to support DMA use
#else
  uint16_t usTemp[1][BUFFER_SIZE];    // Global to support DMA use
#endif
bool dmaBuf = 0;

#include <AnimatedGIF.h>
AnimatedGIF gif;

//#include <beemovie_wide.h>
//int yOffset = 35; // custom offset for vertical positioning
#include <beemovie_full.h>
int yOffset = 0; // custom offset for vertical positioning
#define GIF_IMAGE beemovie

int font_id = 1;


SPIClass sdSPI(VSPI);
#define IP5306_ADDR         0X75
#define IP5306_REG_SYS_CTL0 0x00


uint8_t state = 0;
Button2 *pBtns = nullptr;
uint8_t g_btns[] =  BUTTONS_MAP;
char buff[512];
Ticker btnscanT;

void GIFDraw(GIFDRAW *pDraw)
{
  uint8_t *s;
  uint16_t *d, *usPalette;
  int x, y, iWidth, iCount;

  // Display bounds check and cropping
  iWidth = pDraw->iWidth;
  if (iWidth + pDraw->iX > DISPLAY_WIDTH)
    iWidth = DISPLAY_WIDTH - pDraw->iX;
  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y + yOffset; // current line
  if (y >= DISPLAY_HEIGHT || pDraw->iX >= DISPLAY_WIDTH || iWidth < 1)
    return;

  // Old image disposal
  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2) // restore to background color
  {
    for (x = 0; x < iWidth; x++)
    {
      if (s[x] == pDraw->ucTransparent)
        s[x] = pDraw->ucBackground;
    }
    pDraw->ucHasTransparency = 0;
  }

  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency) // if transparency used
  {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    pEnd = s + iWidth;
    x = 0;
    iCount = 0; // count non-transparent pixels
    while (x < iWidth)
    {
      c = ucTransparent - 1;
      d = &usTemp[0][0];
      while (c != ucTransparent && s < pEnd && iCount < BUFFER_SIZE )
      {
        c = *s++;
        if (c == ucTransparent) // done, stop
        {
          s--; // back up to treat it like transparent
        }
        else // opaque
        {
          *d++ = usPalette[c];
          iCount++;
        }
      } // while looking for opaque pixels
      if (iCount) // any opaque pixels?
      {
        // DMA would degrtade performance here due to short line segments
        tft.setAddrWindow(pDraw->iX + x, y, iCount, 1);
        tft.pushPixels(usTemp, iCount);
        x += iCount;
        iCount = 0;
      }
      // no, look for a run of transparent pixels
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd)
      {
        c = *s++;
        if (c == ucTransparent)
          x++;
        else
          s--;
      }
    }
  }
  else
  {
    s = pDraw->pPixels;

    // Unroll the first pass to boost DMA performance
    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    if (iWidth <= BUFFER_SIZE)
      for (iCount = 0; iCount < iWidth; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];
    else
      for (iCount = 0; iCount < BUFFER_SIZE; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];

#ifdef USE_DMA // 71.6 fps (ST7796 84.5 fps)
    tft.dmaWait();
    tft.setAddrWindow(pDraw->iX, y, iWidth, 1);
    tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
    dmaBuf = !dmaBuf;
#else // 57.0 fps
    tft.setAddrWindow(pDraw->iX, y, iWidth, 1);
    tft.pushPixels(&usTemp[0][0], iCount);
#endif

    iWidth -= iCount;
    // Loop if pixel buffer smaller than width
    while (iWidth > 0)
    {
      // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
      if (iWidth <= BUFFER_SIZE)
        for (iCount = 0; iCount < iWidth; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];
      else
        for (iCount = 0; iCount < BUFFER_SIZE; iCount++) usTemp[dmaBuf][iCount] = usPalette[*s++];

#ifdef USE_DMA
      tft.dmaWait();
      tft.pushPixelsDMA(&usTemp[dmaBuf][0], iCount);
      dmaBuf = !dmaBuf;
#else
      tft.pushPixels(&usTemp[0][0], iCount);
#endif
      iWidth -= iCount;
    }
  }
} /* GIFDraw() */

bool setPowerBoostKeepOn(int en)
{
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en)
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    else
        Wire.write(0x35); // 0x37 is default reg value
    return Wire.endTransmission() == 0;
}

void button_handle(uint8_t gpio)
{
    switch (gpio) {
#ifdef BUTTON_1
    case BUTTON_1: {
        state = 1;
    }
    break;
#endif

#ifdef BUTTON_2
    case BUTTON_2: {
        state = 2;
    }
    break;
#endif

#ifdef BUTTON_3
    case BUTTON_3: {
        state = 3;
    }
    break;
#endif

#ifdef BUTTON_4
    case BUTTON_4: {
        state = 4;
    }
    break;
#endif
    default:
        break;
    }
}

void button_callback(Button2 &b)
{
    for (int i = 0; i < sizeof(g_btns) / sizeof(g_btns[0]); ++i) {
        if (pBtns[i] == b) {
            Serial.printf("btn: %u press\n", pBtns[i].getAttachPin());
            button_handle(pBtns[i].getAttachPin());
        }
    }
}

void button_init()
{
    uint8_t args = sizeof(g_btns) / sizeof(g_btns[0]);
    pBtns = new Button2 [args];
    for (int i = 0; i < args; ++i) {
        pBtns[i] = Button2(g_btns[i]);
        pBtns[i].setPressedHandler(button_callback);
    }
#if defined(T4_V13)
#if defined(T4_V13)
    pBtns[0].setLongClickHandler([](Button2 & b) {
#endif

        int x = tft.width() / 2 ;
        int y = tft.height() / 2 - 30;
        int r = digitalRead(TFT_BL);
        tft.setTextSize(1);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString("Press again to wake up", x - 20, y + 30);

#ifndef ST7735_SLPIN
#define ST7735_SLPIN   0x10
#define ST7735_DISPOFF 0x28
#endif

        delay(3000);
        tft.writecommand(ST7735_SLPIN);
        tft.writecommand(ST7735_DISPOFF);
        digitalWrite(TFT_BL, !r);
        delay(1000);
        // esp_sleep_enable_ext0_wakeup((gpio_num_t )BUTTON_1, LOW);
        esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_1)), ESP_EXT1_WAKEUP_ALL_LOW);
        esp_deep_sleep_start();
    });
#endif
}

void button_loop() {
    for (int i = 0; i < sizeof(g_btns) / sizeof(g_btns[0]); ++i) {
        pBtns[i].loop();
    }
}

void spisd_test() {
    tft.fillScreen(TFT_BLACK);
    if (SD_CS >  0) {
        tft.setTextDatum(MC_DATUM);
        sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
        if (!SD.begin(SD_CS, sdSPI)) {
            tft.setTextFont(2);
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.drawString("SDCard MOUNT FAIL", tft.width() / 2, tft.height() / 2);
        } else {
            uint32_t cardSize = SD.cardSize() / (1024 * 1024);
            String str = "SDCard Size: " + String(cardSize) + "MB";
            tft.setTextFont(2);
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.drawString(str, tft.width() / 2, tft.height() / 2);
        }
        delay(2000);
    }
}

void playSound(void) {
    if (SPEAKER_OUT > 0) {
        if (SPEAKER_PWD > 0) {
            digitalWrite(SPEAKER_PWD, HIGH);
            delay(200);
        }
        ledcWriteTone(CHANNEL_0, 1000);
        delay(200);
        ledcWriteTone(CHANNEL_0, 0);
        if (SPEAKER_PWD > 0) {
            delay(200);
            digitalWrite(SPEAKER_PWD, LOW);
        }
    }
}

void listDir(fs::FS & fs, const char *dirname, uint8_t levels) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(0, 0);

    tft.println("Listing directory:" + String(dirname));

    File root = fs.open(dirname);
    if (!root) {
        tft.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        tft.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            tft.print("  DIR : ");
            tft.println(file.name());
            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            tft.print("  FILE: ");
            tft.print(file.name());
            tft.print("  SIZE: ");
            tft.println(file.size());
        }
        file = root.openNextFile();
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    //Pin out Dump
    Serial.printf("Current select %s version\n", BOARD_VRESION);
    Serial.printf("TFT_MISO:%d\n", TFT_MISO);
    Serial.printf("TFT_MOSI:%d\n", TFT_MOSI);
    Serial.printf("TFT_SCLK:%d\n", TFT_SCLK);
    Serial.printf("TFT_CS:%d\n", TFT_CS);
    Serial.printf("TFT_DC:%d\n", TFT_DC);
    Serial.printf("TFT_RST:%d\n", TFT_RST);
    Serial.printf("TFT_BL:%d\n", TFT_BL);
    Serial.printf("SD_MISO:%d\n", SD_MISO);
    Serial.printf("SD_MOSI:%d\n", SD_MOSI);
    Serial.printf("SD_SCLK:%d\n", SD_SCLK);
    Serial.printf("SD_CS:%d\n", SD_CS);
    Serial.printf("I2C_SDA:%d\n", I2C_SDA);
    Serial.printf("I2C_SCL:%d\n", I2C_SCL);
    Serial.printf("SPEAKER_PWD:%d\n", SPEAKER_PWD);
    Serial.printf("SPEAKER_OUT:%d\n", SPEAKER_OUT);
    Serial.printf("ADC_IN:%d\n", ADC_IN);
    Serial.printf("BUTTON_1:%d\n", BUTTON_1);
    Serial.printf("BUTTON_2:%d\n", BUTTON_2);
    Serial.printf("BUTTON_3:%d\n", BUTTON_3);
#ifdef BUTTON_4
    Serial.printf("BUTTON_4:%d\n", BUTTON_4);
#endif

// // !
// #define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
// #define TIME_TO_SLEEP  10        /* Time ESP32 will go to sleep (in seconds) */

//     Wire.begin(I2C_SDA, I2C_SCL);
//     setupMPU9250();
//     readMPU9250();
//     delay(100);
//     IMU.setSleepEnabled(true);
//     delay(2000);
//     esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * TIME_TO_SLEEP);
//     esp_deep_sleep_start();
// // //!

    tft.init();
    tft.setRotation(tftRotation);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(0, 0);

    if (TFT_BL > 0) {
        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, HIGH);
    }

    //spisd_test();
    button_init();
    tft.setTextFont(1);
    //tft.setTextSize(1);

    tft.setTextSize(1);
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(0, 0);
    tft.println("hey disciples, has anyone seen The Bee Movie?");
    tft.println("    - a gift for Junior");
    tft.println();
    tft.println("project info at https://github.com/galenriley/BeeMovieBeamer");
    tft.println();
    tft.println("based on an IG reel by @dungeons.and.dragon.memes that got stuck in my head");
    tft.println("with thanks to Don Sayers and Becca Bryan");
    tft.println();
    tft.println("[put battery info here]");

    if (I2C_SDA > 0) {
        Wire.begin(I2C_SDA, I2C_SCL);
    }
    btnscanT.attach_ms(30, button_loop);

    gif.begin(BIG_ENDIAN_PIXELS);
}


void loop() {
    switch (state) {
    case 1:
        state = 0;
        digitalWrite(TFT_BL, HIGH); // enable TFT backlight
        if (gif.open((uint8_t *)GIF_IMAGE, sizeof(GIF_IMAGE), GIFDraw))
        {
            Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
            tft.startWrite(); // The TFT chip select is locked low
            while (gif.playFrame(true, NULL))
            {
                yield();
            }
            gif.close();
            tft.endWrite(); // Release TFT chip select for other SPI devices
        }
        tft.fillScreen(TFT_BLACK);
        digitalWrite(TFT_BL, LOW); // disable TFT backlight
        break;
    case 2:
        state = 0;
        tft.fillScreen(TFT_BLACK);
        tft.setTextFont(font_id);
        tft.setTextSize(3);
        tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("font " + String(font_id), tft.width() / 2, tft.height() / 2);
        font_id++;
        break;
    case 3:
        state = 0;
        listDir(SD, "/", 2);
        break;
    case 4:
        state = 0;
        /*
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Undefined function", tft.width() / 2, tft.height() / 2);
        */
        
        digitalWrite(TFT_BL, HIGH); // enable TFT backlight
        if (1 == tftRotation)
            tftRotation = 3;
        else if (3 == tftRotation)
            tftRotation = 1;
        tft.setRotation(tftRotation);
        tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(3);
        tft.drawString("tftRotation " + String(tftRotation), tft.width() / 2, tft.height() / 2);
        delay(3000);
        tft.fillScreen(TFT_BLACK);
        digitalWrite(TFT_BL, LOW); // disable TFT backlight

        break;
    default:
        break;
    }
}