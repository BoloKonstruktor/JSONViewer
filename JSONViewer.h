#ifndef JSONViewer_H
#define JSONViewer_H
#include "Arduino.h"
#include "at.hpp"


class JSONViewer {
	
	private:
		typedef struct {
			char path[64];
			void(*callback)( const char* );
		}TDATA_REGISTER;
		
		typedef struct {
			char ssid[33];
			char pass[33];  
		} TWIFI; 
  
		typedef struct {
			uint32_t interval;
			char url[64];
		} TRDS;
		
		uint8_t size = 0, idx = 0;
		int8_t AT_MODE_PIN = -1;
		unsigned eep_addr = 0;
		String devname = "", devver = "";
		TDATA_REGISTER* data_register = NULL;
		Stream* monitor = NULL;
		ATCMD atcmd;
		static JSONViewer* int_inst;
		
		TWIFI cfgwifi;
		const TWIFI defwifi = { 
			"                                ",
			"                                "
		};
		TRDS  cfgrds;
		const TRDS defrds = {
			5000, "https://rds.eurozet.pl/reader/var/antyradio.json"
		};

		
	protected:	
		void load( unsigned&, bool=false );
		void save( void );
		static int8_t wifi_service( uint8_t, char*, char* );
		static int8_t wifi_scan_service( uint8_t, char*, char* );
		static int8_t url_service( uint8_t, char*, char* );

	public:
		JSONViewer( int8_t AT_MODE_PIN=-1, String devname="", String devver="" );
		void begin( Stream* monitor, unsigned& addr );
		void begin( Stream* monitor );
		void register_callback( const char*, void(*callback)( const char* ) );
		void reload( void );
		void loop( bool=false );
		void set_url( const char* url );
};
#endif