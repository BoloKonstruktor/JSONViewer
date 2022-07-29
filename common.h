#ifndef COMMON_H
#define COMMON_H
#include "EEPROM.h"
#ifdef ESP32
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>	
#endif

	template< typename Arr > void arrcpy( Arr& to, const Arr from, unsigned size ){
			for( unsigned i = 0; i < size; i++ ){
				to[i] = from[i];
			}     
	}

  extern String explode( String, uint8_t, const char* );
  extern String extractJSON( String );
  extern String* extractPath( const char*, unsigned& );
  extern bool is_digit( const char* );
  extern unsigned eep_read( unsigned, void*, uint16_t );
  extern unsigned eep_write( unsigned, void*, uint16_t );
  bool eep_empty( void*, uint16_t );
  template<typename D1, typename D2> void load_param( unsigned& addr, D1& data, D2 def, bool loaddef = false ){
    size_t size = sizeof( data );
    uint16_t _addr = eep_read( addr, &data, size );
  
      if( eep_empty( &data, size ) || loaddef ){
        memcpy( &data, &def, size );
        eep_write( addr, &data, size );
      }
  
    addr = _addr;
  } 
  template<typename D> void save_param( unsigned& addr, D data ){
    size_t size = sizeof( data );
    eep_write( addr, &data, size );
    addr = addr+size;
  }
#endif