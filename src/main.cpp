#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include <Ticker.h>
#include <Button2.h>
//#include <IP5306_I2C.h>
#include <AnimatedGIF.h>

//#include <beemovie_wide.h>
#include <beemovie_full.h>

#define T4_V13
#if defined(T4_V13)
#include "T4_V13.h"
#endif

#define ST7735_SLPIN 0x10
#define ST7735_SLPOUT 0x11
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON 0x29


TFT_eSPI tft = TFT_eSPI();
RTC_DATA_ATTR int tftRotation = 1;
//IP5306 ip5306 = IP5306(I2C_SDA,I2C_SCL);

AnimatedGIF gif;

//int yOffset = 35; // custom offset for vertical positioning
int yOffset = 0; // custom offset for vertical positioning
#define GIF_IMAGE beemovie

uint8_t state = 0;
RTC_DATA_ATTR bool hasShownDisplay = false;

Button2 *pBtns = nullptr;
uint8_t g_btns[] =  BUTTONS_MAP;
Ticker btnscanT;

// adapted from https://github.com/bitbank2/AnimatedGIF/blob/master/examples/TFT_eSPI_memory/GIFDraw.ino
// added yOffset for setting vertical position
void GIFDraw(GIFDRAW *pDraw)
{
    uint8_t *s;
    uint16_t *d, *usPalette;
    int x, y, iWidth, iCount;

    int16_t DISPLAY_WIDTH = tft.width();
    int16_t DISPLAY_HEIGHT = tft.height();

    int BUFFER_SIZE = 320; // Optimum is >= GIF width or integral division of width
    uint16_t usTemp[BUFFER_SIZE];    // Global to support DMA use

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
            d = &usTemp[0];
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
            for (iCount = 0; iCount < iWidth; iCount++) usTemp[iCount] = usPalette[*s++];
        else
            for (iCount = 0; iCount < BUFFER_SIZE; iCount++) usTemp[iCount] = usPalette[*s++];

        tft.setAddrWindow(pDraw->iX, y, iWidth, 1);
        tft.pushPixels(&usTemp[0], iCount);

        iWidth -= iCount;
        // Loop if pixel buffer smaller than width
        while (iWidth > 0)
        {
            // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
            if (iWidth <= BUFFER_SIZE)
                for (iCount = 0; iCount < iWidth; iCount++) usTemp[iCount] = usPalette[*s++];
            else
                for (iCount = 0; iCount < BUFFER_SIZE; iCount++) usTemp[iCount] = usPalette[*s++];

            tft.pushPixels(&usTemp[0], iCount);

            iWidth -= iCount;
        }
    }
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
/*
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

        delay(3000);
        tft.writecommand(ST7735_SLPIN);
        tft.writecommand(ST7735_DISPOFF);
        digitalWrite(TFT_BL, !r);
        delay(1000);
        // esp_sleep_enable_ext0_wakeup((gpio_num_t )BUTTON_1, LOW);
        esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_1)), ESP_EXT1_WAKEUP_ALL_LOW);
        esp_deep_sleep_start();
    });
    */
#endif
}

void button_loop() {
    for (int i = 0; i < sizeof(g_btns) / sizeof(g_btns[0]); ++i) {
        pBtns[i].loop();
    }
}


void setDisplayEnabled(bool enabled)
{
    if (enabled)
    {
        tft.fillScreen(TFT_BLACK);
        tft.writecommand(ST7735_DISPON);
        //tft.fillScreen(TFT_BLACK);
        digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
    }
    else
    {
        tft.writecommand(ST7735_DISPOFF);
        digitalWrite(TFT_BL, LOW);
    }
}


void displayStartupScreen()
{
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextFont(1);
    tft.setTextSize(2);
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.println("hey disciples, has anyone seen The Bee Movie?");
    tft.println();

    tft.setTextSize(1);
    tft.println("(a stupid gift for Junior)");
    
    tft.setTextDatum(ML_DATUM); // for some reason this isn't working
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.println();
    tft.println();
    tft.println();
    tft.println();
    tft.println("project info:");
    tft.println("https://github.com/galenriley/BeeMovieBeamer");
    tft.println("with help from Becca and Don");
    tft.println();
    tft.println("based on a tiktok post by @jacuto");
    
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.setTextDatum(BL_DATUM); // for some reason this isn't working
    tft.setTextSize(2);
    //tft.drawString("Battery: " + String(ip5306.check_battery_status()) + " " + String(ip5306.check_charging_status()), 0, tft.height());
    tft.drawString("[put battery info here]", 0, tft.height());

    hasShownDisplay = true;
}

void sleep()
{
    //tft.writecommand(ST7735_SLPIN);
    esp_deep_sleep_start();
}

void flipDisplay()
{
    if (1 == tftRotation)
        tftRotation = 3;
    else if (3 == tftRotation)
        tftRotation = 1;
    tft.setRotation(tftRotation);
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(3);
    tft.drawString("this side up", tft.width() / 2, tft.height() / 2);
}

void beeMovie()
{
    //setDisplayEnabled(true);
    if (gif.open((uint8_t *)GIF_IMAGE, sizeof(GIF_IMAGE), GIFDraw))
    {
        //Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
        tft.startWrite(); // The TFT chip select is locked low
        while (gif.playFrame(true, NULL))
        {
            yield();
        }
        gif.close();
        tft.endWrite(); // Release TFT chip select for other SPI devices
    }
    tft.fillScreen(TFT_BLACK);
    //setDisplayEnabled(false);
}

void print_wakeup_reason()
{
    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
        case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("Wakeup caused by external signal using RTC_IO"); break;
        case ESP_SLEEP_WAKEUP_EXT1:     Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
        case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("Wakeup caused by timer"); break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
        case ESP_SLEEP_WAKEUP_ULP:      Serial.println("Wakeup caused by ULP program"); break;
        default:                        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
    }
}

void setup() {
    Serial.begin(115200);
    // delay(1000); // why is this delay here in every example?
    print_wakeup_reason();

    esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_1)), ESP_EXT1_WAKEUP_ALL_LOW);

    tft.init();
    tft.setRotation(tftRotation);
    tft.fillScreen(TFT_BLACK);

    button_init();
    btnscanT.attach_ms(30, button_loop);

    gif.begin(BIG_ENDIAN_PIXELS);

    if (!hasShownDisplay)
    {
        displayStartupScreen();
    }
    else
    {
        beeMovie();   
        sleep();
    }
}


void loop() {
    switch (state) {
    case 1:
        state = 0;
        
        beeMovie();
        sleep();

        break;
    case 2:
        state = 0;
        /*
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Undefined function", tft.width() / 2, tft.height() / 2);
        */

        // infinite mode!
        while (true)
            beeMovie();

        break;
    case 3:
        state = 0;
        /*
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Undefined function", tft.width() / 2, tft.height() / 2);
        */
        break;
    case 4:
        state = 0;
        
        flipDisplay();       

        break;
    default:
        state = 0;
        break;
    }
}