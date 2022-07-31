#include <JSONViewer.h>
#include <ShowText.h>
#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>


#define DEVNAME       "WeatherReports"
#define DEVVERSION    "1.1"


namespace LCD {
  const uint8_t ADDR = 0x27;
  const uint8_t COLS = 20;
  const uint8_t ROWS = 2; 
};

namespace MSG {
const uint8_t Rep = 3;
int8_t New = 0, Count = -1;
};


LiquidCrystal_PCF8574 lcd( LCD::ADDR );

JSONViewer weather_report( 2, DEVNAME, DEVVERSION );
ShowText show_text1;
ShowText show_text2;

void set_data1( JsonObject json );
void set_data2( JsonObject json );
void show1( const char* );
void show2( const char* );
void reload( void );
String lead_chars( const char* str, const char* mask = " " );



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
      lcd.print( F("Init...") );
    } else {
      Serial.println("[ERROR] LCD not found.");
    }
    
  weather_report.begin( &Serial, eep_addr );
  weather_report.register_callback( "https://zdalne.tk/json.php", "/data", set_data1 );
  weather_report.register_callback( "https://weblukasz.pl/burze.dzis.net/json.php", "/data", set_data2 );

  lcd.clear();
  lcd.setBacklight( 255 );
  
  show_text1.begin( LCD::COLS );
  show_text1.registerShowCallback( show1 );
  
  show_text2.begin( LCD::COLS );
  show_text2.registerShowCallback( show2 );
  show_text2.registerEndScrollCallback( reload );
}

void loop( void ){
  show_text1.show();
  show_text2.show();
  weather_report.loop( show_text2.isScrolling() );
}

void set_data1( JsonObject json ){
  String str = "";
  uint8_t size = json.size(), ii = 0;

    for( uint8_t i=0; i<size; i++ ){
        
        if( !json.containsKey( "s"+String(i) ) ) continue;
      
      auto s = json["s"+String(i)];

        if( ii ) str += "  ";

        

        if( s.containsKey( "value" ) ){
          int value = s["value"];
            if( s.containsKey( "type" ) ){
              String type = s["type"];

                if( type == "temp" ){
                  str += lead_chars( String( value ).c_str(), "   " );
                } else if( type == "humi" ){
                  str += lead_chars( String( value ).c_str(), "  " );
                } else if( type == "press" ){
                  str += lead_chars( String( value ).c_str(), "    " );  
                }
            }
        }

        if( s.containsKey( "unit" ) ){
          String unit = s["unit"];
          unit.replace( "°", "'" );
          str += unit;
        }

      ii++;
    }

  show_text1.set( str.c_str() );
}

void set_data2( JsonObject json ){
  String str = "";
  uint8_t size = json.size(), ii = 0;

    for( uint8_t i=0; i<size; i++ ){
        
        if( !json.containsKey( "p"+String(i) ) ) continue;
      
      auto p = json["p"+String(i)];

        if( !p.containsKey( "type" ) ) continue;
        
      String type = p["type"];

        if( type == "none" ) continue;

        if( !p.containsKey( "content" ) ) continue;
        
      auto content = p["content"];

        if( content.containsKey( "text" ) ){
          auto text = content["text"];

            if( text.containsKey( "message" ) ){
              String message = text["message"];

                if( message != "none" ){
                  message = type+": "+message;
                } else {
                  message = type;
                }

                if( text.containsKey( "expire" ) ){
                  String expire = text["expire"];

                    if( expire != "none" ){
                      message += " <> "+expire;
                    }
                }

                if( ii ) str += "   ***   ";

              message.replace( "&deg;", " st " );
              str += message;
              ii++;
            }
        }
    }
     
  show_text2.set( str.c_str() );
  show_text2.setSpacesOnStart( true );
  show_text2.setSpacesOnEnd( true );
  show_text2.setScrollSpeed( 250 );
}

void reload( void ){
  weather_report.reload();
}

String lead_chars( const char* str, const char* mask ){
  uint8_t slen = strlen( str ), mlen = strlen( mask );
  String lead = "";

    for( uint8_t i=0; i<mlen-slen; i++ ) lead += mask[i];

  return lead+String( str );
}

void show1( const char* str ){
  lcd.setCursor( show_text1.getX(), 0 );  
  lcd.print( str );  
}

void show2( const char* str ){
  lcd.setCursor( show_text2.getX(), 1 );
  lcd.print( str );  
}
