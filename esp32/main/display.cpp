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

#define USE_I2C_DISPLAY
//#define USE_SPI_DISPLAY

#if defined USE_I2C_DISPLAY
static const int displayAddress = 0x3C;
static const int displayWidth = 128;
static const int displayHeight = 64;
static const int resetPin = -1;
struct SSD1306_Device display;
#elif defined USE_SPI_DISPLAY
static const int displayChipSelect = 15;
static const int displayWidth = 128;
static const int displayHeight = 64;
static const int resetPin = 5;
struct SSD1306_Device display;
#else
#error Select display interface
#endif

bool default_bus_init( void ) {
    #if defined USE_I2C_DISPLAY
    SSD1306_I2CMasterInitDefault( );
    SSD1306_I2CMasterAttachDisplayDefault( &display, displayWidth, displayHeight, displayAddress, resetPin );
    #endif

    #if defined USE_SPI_DISPLAY
    SSD1306_SPIMasterInitDefault( );
    SSD1306_SPIMasterAttachDisplayDefault( &display, displayWidth, displayHeight, displayChipSelect, resetPin );
    #endif

    return true;
}

void display_clear(const struct SSD1306_FontDef* Font ) {
    SSD1306_Clear( &display, SSD_COLOR_BLACK );
    SSD1306_SetFont( &display, Font );
}

void display_set_font(const struct SSD1306_FontDef* Font ) {
    SSD1306_SetFont( &display, Font );
}

void display_print( const char* HelloText ) {
    SSD1306_FontDrawAnchoredString( &display, TextAnchor_Center, HelloText, SSD_COLOR_WHITE );
    SSD1306_Update( &display );
}



void display_init() {
    printf( "Ready...\n" );

    if ( default_bus_init( ) == true ) {
        printf( "BUS Init lookin good...\n" );
        printf( "Drawing.\n" );

        display_set_font( &Font_droid_sans_fallback_24x28 );
        display_print( "Hello i2c!" );

        printf( "Done!\n" );
    }
}
