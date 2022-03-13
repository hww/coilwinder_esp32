/**
 * Copyright (c) 2017-2018 Tara Keeling
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ssd1306.h"
#include "ssd1306_draw.h"
#include "ssd1306_font.h"
#include "ssd1306_default_if.h"
#include "config.h"
/**
 * Select one of available interefaces by uncomining one
 * one of the next lines
 **/
#define USE_I2C_DISPLAY
//#define USE_SPI_DISPLAY

#if defined USE_I2C_DISPLAY
static const int displayAddress = 0x3C;
static const int displayWidth = 128;
static const int displayHeight = 64;
static const int resetPin = -1;
#elif defined USE_SPI_DISPLAY
static const int displayChipSelect = 15;
static const int displayWidth = 128;
static const int displayHeight = 64;
static const int resetPin = 5;
#else
#error Select display interface
#endif

static const char TAG[] = "display";

struct SSD1306_Device display;


/** Initialize the buffer */
bool default_bus_init( void ) {
    #if defined USE_I2C_DISPLAY
        return SSD1306_I2CMasterInitDefault( ) && SSD1306_I2CMasterAttachDisplayDefault( &display, displayWidth, displayHeight, displayAddress, resetPin );
    #endif

    #if defined USE_SPI_DISPLAY
        assert( SSD1306_SPIMasterInitDefault( ) == true );
        assert( SSD1306_SPIMasterAttachDisplayDefault( &display, displayWidth, displayHeight, displayChipSelect, resetPin ) == true );
    #endif

    return true;
}

/** Clear the display */
void display_clear(const struct SSD1306_FontDef* font ) {
    SSD1306_Clear( &display, SSD_COLOR_BLACK );
    SSD1306_SetFont( &display, font );
}

/** Clear display */
void display_clear()
{
	display_clear( &AR_DEFAULT_FONT );
}

/** Set the display font */
void display_set_font(const struct SSD1306_FontDef* Font ) {
    SSD1306_SetFont( &display, Font );
}

/** Print the message */
void display_print(const char* text ) {
    SSD1306_FontDrawAnchoredString( &display, TextAnchor_Center, text, SSD_COLOR_WHITE );
}

void display_print(int x, int y, const char* text ) {
    x *= SSD1306_FontGetCharWidth( &display, ' ' );
    y *= SSD1306_FontGetCharHeight( &display );
    SSD1306_FontDrawString( &display, x, y, text, SSD_COLOR_WHITE );
}

void display_update() { SSD1306_Update(&display); }

/** Initialize the display engine */
bool display_init() {

    if ( default_bus_init( ) ) {
        ESP_LOGI(TAG,  "BUS Init lookin good...\n" );
        SSD1306_Rotate180(&display);
        display_set_font( &AR_DEFAULT_FONT );
        display_print( "OK" );
        display_update();
        return true;
    } else {
        return false;
    }
}
