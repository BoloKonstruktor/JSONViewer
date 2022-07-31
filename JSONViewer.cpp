#include "JSONViewer.h"
#include "common.h"

#define sizearr( x ) sizeof( x ) / sizeof( x[0] )


//WiFiMulti WiFiMulti;
//StaticJsonDocument<2048> doc;

JSONViewer* JSONViewer::int_inst = NULL;

//Metody prywatne
void JSONViewer::load( unsigned& addr, bool def ){
	this->eep_addr = addr;
	load_param( addr, this->cfgwifi, this->defwifi, def );
}

void JSONViewer::save( void ){
	unsigned addr = this->eep_addr;
	save_param( addr, this->cfgwifi );
}

void JSONViewer::enlarge_array( void ){
	this->size++;
	
		if( this->size == 1 ){
			this->data_register = new TDATA_REGISTER[this->size];
		} else {
			TDATA_REGISTER* buff = new TDATA_REGISTER[this->size];
			arrcpy( buff, this->data_register, this->size-1 );
			delete[] this->data_register;
			this->data_register = new TDATA_REGISTER[this->size];
			arrcpy( this->data_register, buff, this->size-1 );  
			delete[] buff;
		}

    this->idx = this->size-1;	
}

//Metody publiczne
JSONViewer::JSONViewer( int8_t AT_MODE_PIN, String devname, String devver ){
	this->AT_MODE_PIN = AT_MODE_PIN;
	pinMode( this->AT_MODE_PIN, INPUT_PULLUP );
	this->devname = devname;
	this->devver = devver;
}

void JSONViewer::begin( Stream* monitor, unsigned& addr ){
	this->int_inst = this;
	this->monitor = monitor;
	this->monitor->println();
	this->monitor->println();
	
	String devname = ( this->devname != "" ) ? ( this->devname+" "+this->devver ) : "";
	
		if( this->AT_MODE_PIN > -1 ){
			this->atcmd.begin( this->monitor, devname.c_str() );
			this->atcmd.registerCallback( "AT+WIFI", wifi_service );
			this->atcmd.registerCallback( "AT+WIFI-SCAN", wifi_scan_service );
#ifdef ESP32
				if( !EEPROM.begin( 1000 ) ) {
					this->monitor->println("Failed to initialise EEPROM");
					this->monitor->println("Restarting...");
					delay(500);
					ESP.restart();
				}
#else
			EEPROM.begin( 512 );
#endif

			this->load( addr );
			delay( 1000 );
			bool cfg = digitalRead( this->AT_MODE_PIN );
			String ssid = this->cfgwifi.ssid;
			ssid.trim();
			uint8_t i = 0;

				while( !cfg || ssid == "" ){
					
						if( !i ){
						  this->monitor->println( F("AT CMD Mode") );
						  i++;
						}
					
					this->atcmd.loop();
					cfg = digitalRead( this->AT_MODE_PIN );
					ssid = this->cfgwifi.ssid;
					ssid.trim();       
				}

			WiFi.mode( WIFI_STA );
			WiFi.begin( this->cfgwifi.ssid, this->cfgwifi.pass );
			
				if( this->devname != "" ) WiFi.setHostname( this->devname.c_str() );

			this->monitor->println();
			this->monitor->println();
			this->monitor->print( F("Waiting for WiFi to connect ") );
			this->monitor->print( this->cfgwifi.ssid );
			this->monitor->print( F(" ") );
			
				while( (WiFi.status() != WL_CONNECTED) ) {
					this->monitor->print(".");
					delay( 500 );
				}
		  
			this->monitor->println(" connected");
			this->monitor->print( F("IP address: ") );
			this->monitor->println( WiFi.localIP() );
		}
}

void JSONViewer::register_callback( const char* url, const char* path, void(*callback)( const char* ) ){
	this->enlarge_array();
    auto& data_register = this->data_register[this->idx];
		if( data_register.url == NULL ) data_register.url = new char[strlen( url )+1];
    strcpy( data_register.url, url );
		if( data_register.path == NULL ) data_register.path = new char[strlen( path )+1];
    strcpy( data_register.path, path );
	data_register.str_callback = callback;
    this->idx = 0;		
}

void JSONViewer::register_callback( const char* url, const char* path, void(*callback)( JsonObject ) ){
	this->enlarge_array();
    auto& data_register = this->data_register[this->idx];
		if( data_register.url == NULL ) data_register.url = new char[strlen( url )+1];
    strcpy( data_register.url, url );
		if( data_register.path == NULL ) data_register.path = new char[strlen( path )+1];
    strcpy( data_register.path, path );
	data_register.json_callback = callback;
    this->idx = 0;		
}

void JSONViewer::reload( void ){
		for( uint8_t i=0; i<this->size; i++ ){
			auto dr = this->data_register[i];
			HTTPClient https;	
			this->monitor->print( F("\n[HTTP] Opening URL: ") );
			this->monitor->print( dr.url );
			this->monitor->println( F(" ...") );
			
#ifdef ESP8266
			WiFiClientSecure client;
			client.setInsecure(); //the magic line, use with caution
			client.connect( dr.url, 443 );
				
				if( https.begin( client, dr.url ) ) {
#else      
				if( https.begin( dr.url ) ) {
#endif
					int httpCode = https.GET();

						if( httpCode > 0 ){  
							this->monitor->printf( "[HTTP] Returned code: %d\n", httpCode );
							
								if( httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY ){
									String json = extractJSON( https.getString() );
									DynamicJsonDocument doc( 2048 );
									this->monitor->print( F("[JSON] ") );
									this->monitor->println( json );
									deserializeJson( doc, json );	
									unsigned s = 0;
									String* path = extractPath( dr.path, s );
									JsonObject obj = doc.as<JsonObject>();					
											
										for( uint8_t ii=0; ii<s-1; ii++ ){
													
											if( obj.containsKey( path[ii] ) ) obj = obj[path[ii]];
											else break;
										}
												 
										if( !obj.containsKey( path[s-1] ) ) {
											return;
										}
												
										if( obj[path[s-1]].is<JsonObject>() ){
														
												if( dr.json_callback && s ){
													dr.json_callback( obj[path[s-1]] );
												}			
												
										} else if( dr.str_callback && s ){
											dr.str_callback( obj[path[s-1]] );
										}
												
								}
						} else {
							this->monitor->printf( "[HTTP] GET... failed, error: %s\n", https.errorToString( httpCode ).c_str() );
							https.end();
						}
									  
					https.end();
				}
		}
}

void JSONViewer::begin( Stream* monitor ){
	unsigned addr = 0;
	this->begin( monitor, addr );
}

void JSONViewer::loop( bool stop ){
	
		if( stop ) return;
		
	uint32_t curr = millis();
	static uint32_t update = 0;

		if( curr - update >= 5000 ){
			update = curr;
			this->reload();
		}
}

//Metody statyczne
int8_t JSONViewer::wifi_service( uint8_t inout, char * cmd, char * params ) {

		if( int_inst == NULL ) return -1;
	
		switch( inout ){
			case 0:{
				int_inst->monitor->println( F("Ustawienia sieci WiFi") );
			}break;
			case 1:{
					
					if( strstr( params, "::" ) == NULL ){
						int_inst->monitor->println("Nieprawidłowa wartość.");
						int_inst->monitor->println("Poprawna składnia: at+wifi=ssid::hasło");
						return -1;
					}
				
				String ssid = explode( params, 0, "::" ), pass = explode( params, 1, "::" );
				ssid.trim();
				pass.trim();

					if( ssid == "" ){
						int_inst->monitor->println("Podaj SSID sieci WiFi");
						return -1;          
					}

					if( ssid.length() > 32 ){
						int_inst->monitor->println("SSID może zawierać maksymalnie 32 znaki");
						return -1;          
					}

					if( pass.length() > 32 ){
						int_inst->monitor->println("Hasło może zawierać maksymalnie 32 znaki");
						return -1;          
					}

				strcpy( int_inst->cfgwifi.ssid, ssid.c_str() );
				strcpy( int_inst->cfgwifi.pass, pass.c_str() );
				int_inst->save();
			}break;
		}

return 0;
}

int8_t JSONViewer::wifi_scan_service( uint8_t inout, char * cmd, char * params ) {

		if( int_inst == NULL ) return -1;
		
		switch( inout ){
			case 0:{
				int_inst->monitor->println("Skanowanie sieci ...");
				uint8_t cnt = WiFi.scanNetworks();
				int_inst->monitor->println("Lista dostępnych sieci WiFi:");

					if( cnt ){
							for( unsigned i = 0; i < cnt; i++ ){
								int_inst->monitor->print("\tSSID:");
								int_inst->monitor->print(WiFi.SSID(i));
								int_inst->monitor->print("\tSygnał:");
								int_inst->monitor->print(WiFi.RSSI(i));
								int_inst->monitor->print("dBm");
								//int_inst->monitor->print("\tSzyfrowanie:");
								//printEncryptionType( int_inst->monitor, WiFi.encryptionType(i) );
								int_inst->monitor->println();
							}
					} else {
						int_inst->monitor->println("Brak dostępnych sieci!"); 
					}
			}break;
			case 1:{
      
			}break;
		}
  
return 0;
}
