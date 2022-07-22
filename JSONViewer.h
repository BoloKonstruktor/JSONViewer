#ifndef JSONViewer_H
#define JSONViewer_H
#include "Arduino.h"
#include <ArduinoJson.h>
#include "at.hpp"


class JSONViewer {
	
	private:
		typedef struct {
			char path[64];
			void(*str_callback)( const char* );
			void(*json_callback)( JsonObject );
		}TDATA_REGISTER;
		
		typedef struct {
			char ssid[33];
			char pass[33];  
		} TWIFI; 
  
		typedef struct {
			uint32_t interval;
			char url[64];
		} THTTP;
		
		uint8_t size = 0, idx = 0;
		int8_t AT_MODE_PIN = -1;
		unsigned eep_addr = 0;
		String devname = "", devver = "";
		TDATA_REGISTER* data_register = NULL;
		Stream* monitor = NULL;
		static JSONViewer* int_inst;
		
		TWIFI cfgwifi;
		const TWIFI defwifi = { 
			"                                ",
			"                                "
		};
		THTTP  cfghttp;
		const THTTP defhttp = {
			5000, "https://weblukasz.pl/burze.dzis.net/json.php"
		};

		
	protected:	
		void load( unsigned&, bool=false );
		void save( void );
		void enlarge_array( void );
		static int8_t wifi_service( uint8_t, char*, char* );
		static int8_t wifi_scan_service( uint8_t, char*, char* );
		static int8_t url_service( uint8_t, char*, char* );

	public:
		ATCMD atcmd;
		JSONViewer( int8_t AT_MODE_PIN=-1, String devname="", String devver="" );
		void begin( Stream* monitor, unsigned& addr );
		void begin( Stream* monitor );
		void register_callback( const char*, void(*callback)( const char* ) );
		void register_callback( const char*, void(*callback)( JsonObject ) );
		bool reload( String& );
		void reload( void );
		void loop( bool=false );
		void set_url( const char* url );
};
#endif
