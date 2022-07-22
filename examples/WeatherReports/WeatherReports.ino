#include <JSONViewer.h>
#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>
#include "TextLine.h"


#define DEVNAME       "weather_reportViewer"
#define DEVVERSION    "1.1"

namespace LCD {
  const uint8_t ADDR = 0x27;
  const uint8_t COLS = 16;
  const uint8_t ROWS = 2; 
};

LiquidCrystal_PCF8574 lcd( LCD::ADDR );

JSONViewer weather_report( 12, DEVNAME, DEVVERSION );
ShowTextLine show_data;

void reload( void );
void set_data( JsonObject json );


void setup( void ){
  Serial.begin( 115200 );
  unsigned eep_addr = 0;
  Wire.begin();
  Wire.beginTransmission( LCD::ADDR );
  int error = Wire.endTransmission();
  
    if( error == 0 ) {
      lcd.begin( LCD::COLS, LCD::ROWS );
      lcd.setBacklight( 255 );
      lcd.home();
      lcd.clear();
      lcd.print( F("Loading...") );
    } else {
      Serial.println("[ERROR] LCD not found.");
    }
    
  weather_report.begin( &Serial, eep_addr );
  weather_report.register_callback( "/data", set_data );
  show_data.scroll_end_callback( reload );
  lcd.clear();
}

void loop( void ){
  static bool stop = false;

    if( show_data.ticker( 350 ) ){
      lcd.setCursor( 0, 0 );
      lcd.print( show_data.get( stop ) );
    }

  weather_report.loop( stop );
}

void reload( void ){
  weather_report.reload();
}

void set_data( JsonObject json ){
  String str = "";
  uint8_t ii = 0;

    for( uint8_t i=0; i<(uint8_t)json.size(); i++ ){
      auto p = json["p"+String(i)];
      String type = p["type"];

        if( type == "none" ) continue;
        
      auto content = p["content"];
      auto text = content["text"];
      String message = text["message"];
      message = type+": "+message;

        if( ii ) str += "  ***  ";

      message.replace( "&deg;", " st " );
      str += message;
      ii++;
    }
 
  Serial.println( str );
  show_data.set( str, LCD::COLS );
  show_data.Preambule = true;
}
