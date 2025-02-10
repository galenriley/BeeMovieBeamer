#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <TFT_eSPI.h>
#include <T4_V13.h>
#include <SPI.h>
#include <Wire.h>
#include <Ticker.h>
#include <Button2.h>
#include <AnimatedGIF.h>

#define IP5306_ADDRESS 0x75

uint8_t state = 0;

TFT_eSPI tft = TFT_eSPI();
RTC_DATA_ATTR int tftRotation = 3;
// determines behavior when waking up from sleep mode
RTC_DATA_ATTR bool hasShownDisplay = false;

Button2 *pBtns = nullptr;
uint8_t g_btns[] =  BUTTONS_MAP;
Ticker btnscanT;

//#include <beemovie_wide.h>
//int yOffset = 35;
#include <beemovie_full.h>
int yOffset = 0;
#define GIF_IMAGE beemovie
AnimatedGIF gif;

String beamCountPath = "/beam_count.txt";
File beamCountFile;
bool beamCountResetPrompt = false;
// log of resets, because Becca insisted
// Note: no counting was done previous to 1/31 but the Bee Movie was certainly beamed a few dozen times during early development and testing and fuckin' around with it
// - 2025/01/31: 27
// - 2025/02/08: 61

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

// Button handling functions, from lilygo example
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
}

void button_loop() {
    for (int i = 0; i < sizeof(g_btns) / sizeof(g_btns[0]); ++i) {
        pBtns[i].loop();
    }
}

// "Lifetime Bees Movied" counter is stored in a local file
String getBeamCount()
{
    beamCountFile = LittleFS.open(beamCountPath, "r", false);
    if (beamCountFile.available())
    {
        String value = beamCountFile.readString();
        value.trim();
        beamCountFile.close();

        return value;
    }

    return "Error getting " + beamCountPath; 
}

void displayResetBeamCountConfirmation()
{
    beamCountResetPrompt = true;

    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.println("Do you want to reset the \"Lifetime Bees Movied\" counter to zero? Press button again to confirm.");
}

void resetBeamCount()
{
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.println("Resetting \"Lifetime Bees Movied\" counter to zero.");
    tft.println("(Previously " + getBeamCount() + ")");

    Serial.println("Resetting \"Lifetime Bees Movied\" counter to zero.");
    Serial.println("(Previously " + getBeamCount() + ")");
    beamCountFile = LittleFS.open(beamCountPath, "w", true);
    beamCountFile.println("0");
    beamCountFile.close();

    beamCountResetPrompt = false;
}

void incrementBeamCount()
{
    beamCountFile = LittleFS.open(beamCountPath, "r+", false);
    String value = beamCountFile.readString();
    String incremented = String(value.toInt() + 1);
    beamCountFile.seek(0, SeekSet);
    beamCountFile.println(incremented);
    beamCountFile.close();
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
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(3);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("^^^", tft.width() / 2, 0);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("this side up", tft.width() / 2, tft.height() / 2);
}

void beeMovie()
{
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

    incrementBeamCount();
}

bool getBatteryExists()
{
    Wire.begin(I2C_SDA,I2C_SCL);
    Wire.beginTransmission(IP5306_ADDRESS);
    uint8_t result = Wire.endTransmission();
    if (0 == result)
    {
        //Serial.println("Detected IP5306 battery");
        return true;
    }
    else
    {
        //Serial.println("Did not detect IP5306 battery?");
    }

    return false;
}

bool getIsChargerConnected()
{
    if (getBatteryExists())
    {
        Wire.begin(I2C_SDA,I2C_SCL);
        Wire.beginTransmission(IP5306_ADDRESS);
        Wire.write(0x70);
        if (Wire.endTransmission(false) == 0 && Wire.requestFrom(IP5306_ADDRESS, 1))
        {
            return (Wire.read() & (1 << 3)) ? true : false;
        }
    }
}

bool getIsChargeFull()
{
    if (getBatteryExists())
    {
        Wire.begin(I2C_SDA,I2C_SCL);
        Wire.beginTransmission(IP5306_ADDRESS);
        Wire.write(0x71);
        if (Wire.endTransmission(false) == 0 && Wire.requestFrom(IP5306_ADDRESS, 1))
        {
            return (Wire.read() & (1 << 3)) ? false : true;
        }
    }
}

uint8_t getBatteryLevel()
{
    if (getBatteryExists())
    {
        Wire.begin(I2C_SDA,I2C_SCL);
        Wire.beginTransmission(IP5306_ADDRESS);
        Wire.write(0x78);
        if (Wire.endTransmission(false) == 0 && Wire.requestFrom(IP5306_ADDRESS, 1))
        {
            switch (Wire.read() & 0xF0)
            {
                case 0xE0: return 25;
                case 0xC0: return 50;
                case 0x80: return 75;
                case 0x00: return 100;
                default: return 0;
            }
        }
    }
}

void displayStartupScreen()
{
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextFont(1);
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.println("Hey disciples, has anyone seen The Bee Movie?");
    tft.println();

    tft.setTextSize(1);
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.println("(a stupid gift for Junior)");
    
    tft.setTextDatum(ML_DATUM); // for some reason this isn't working
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.println();
    tft.println();
    tft.println();
    tft.println();
    tft.println("project info:");
    tft.println("https://github.com/galenriley/BeeMovieBeamer");
    tft.println("based on a tiktok post by @jacuto");
    tft.println("with help from Becca and Don");
    tft.println();
    tft.println();
    tft.println();
    tft.println("Lifetime Bees Movied: " + getBeamCount());
    
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.setTextDatum(BL_DATUM); // for some reason this isn't working
    tft.setTextSize(2);
    
    if (getBatteryExists())
    {
        String battery = "";
        battery += String(getBatteryLevel()) + "%";
        if (!getIsChargerConnected())
            battery += " (Not charging)";
        else
            battery += " (Charging)";
        tft.drawString(battery, 0, tft.height());
    }

    hasShownDisplay = true;
}

void setup() {
    Serial.begin(115200);
    // delay(1000); // why is this delay here in every example?

    esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_1)), ESP_EXT1_WAKEUP_ALL_LOW);

    if(getBatteryExists())
    {
        Serial.println("Charger connected?");
        Serial.println(getIsChargerConnected());

        Serial.println("Charge full?");
        Serial.println(getIsChargeFull());

        Serial.println("Battery level?");
        Serial.println(getBatteryLevel());
    }

    tft.init();
    tft.setRotation(tftRotation);
    tft.fillScreen(TFT_BLACK);

    button_init();
    btnscanT.attach_ms(30, button_loop);

    if (!LittleFS.begin(true))
    {
        Serial.println("LittleFS Mount Failed");
        return;
    }
    else
    {        
        // initialize new beam_count.txt
        if (!LittleFS.exists(beamCountPath))
        {
            Serial.println(beamCountPath + " does not exist, creating new counter for \"Lifetime Bees Movied\"");
            
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.setCursor(0,0);
            tft.setTextFont(1);
            tft.setTextSize(2);
            tft.println(beamCountPath + " does not exist, creating new counter for \"Lifetime Bees Movied\"");
            
            resetBeamCount();
            // hold on this screen for 5s before moving on
            delay(5000);
        }
    }

    gif.begin(BIG_ENDIAN_PIXELS);

    // setup() runs on boot or when waking from sleep, play gif immediately if waking
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
        beamCountResetPrompt = false;
        
        beeMovie();
        sleep();

        break;
    case 2:
        state = 0;
        beamCountResetPrompt = false;
        /*
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Undefined function", tft.width() / 2, tft.height() / 2);
        */
        flipDisplay();

        break;
    case 3:
        state = 0;
        beamCountResetPrompt = false;
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
    case 4:
        state = 0;

        if (!beamCountResetPrompt)
            displayResetBeamCountConfirmation();
        else
            resetBeamCount();

        break;
    default:
        state = 0;
        break;
    }
}