#include "JSONViewer.h"
#include "common.h"

#define sizearr( x ) sizeof( x ) / sizeof( x[0] )


//WiFiMulti WiFiMulti;
StaticJsonDocument<512> doc;

JSONViewer* JSONViewer::int_inst = NULL;

//Metody prywatne
void JSONViewer::load( unsigned& addr, bool def ){
	this->eep_addr = addr;
	load_param( addr, this->cfghttp, this->defhttp, def );
	load_param( addr, this->cfgwifi, this->defwifi, def );
}

void JSONViewer::save( void ){
	unsigned addr = this->eep_addr;
	save_param( addr, this->cfghttp );
	save_param( addr, this->cfgwifi );
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
			this->atcmd.registerCallback( "AT+URL", url_service );

				if( !EEPROM.begin( 1000 ) ) {
					this->monitor->println("Failed to initialise EEPROM");
					this->monitor->println("Restarting...");
					delay(500);
					ESP.restart();
				}

			this->load( addr );
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

			this->monitor->print( F("Waiting for WiFi to connect ") );
			this->monitor->print( this->cfgwifi.ssid );
			this->monitor->print( F(" ...") );
			
				while( (WiFi.status() != WL_CONNECTED) ) {
					this->monitor->print(".");
					delay( 500 );
				}
		  
			this->monitor->println(" connected");
		}
}

void JSONViewer::register_callback( const char* path, void(*callback)( const char* ) ){
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
    auto& data_register = this->data_register[this->idx];
    strcpy( data_register.path, path );
	data_register.callback = callback;
    this->idx = 0;		
}

bool JSONViewer::reload( String& log ){	 
	HTTPClient https;	
	this->monitor->print( F("\n[HTTP] Opening URL: ") );
	this->monitor->print( this->cfghttp.url );
	this->monitor->println( F(" ...") );
            
		if( https.begin( this->cfghttp.url ) ) {
			int httpCode = https.GET();

				if( httpCode > 0 ){  
					this->monitor->printf( "[HTTP] Returned code: %d\n", httpCode );
                    
						if( httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY ){
							String json = extractJSON( https.getString() );	
							this->monitor->print( F("RAW JSON:") );
							this->monitor->println( json );
                  
							deserializeJson( doc, json );
							
											
								for( uint8_t i=0; i<this->size; i++ ){
									auto dr = this->data_register[i];
									unsigned s = 0;
									String* path = extractPath( dr.path, s );
									JsonObject obj = doc.as<JsonObject>();					
									
										for( uint8_t ii=0; ii<s-1; ii++ ){
											obj = obj[path[ii]];
										}
													
										if( dr.callback && s ){
											dr.callback( obj[path[s-1]] );
										}
										
								}
						}
				} else {
					this->monitor->printf( "[HTTP] GET... failed, error: %s\n", https.errorToString( httpCode ).c_str() );
					log = https.errorToString( httpCode ).c_str();
					https.end();
					return false;
				}
                              
			https.end();
		} else {
			log = "URL opening error";
			this->monitor->print( F("[HTTP] ") );
			this->monitor->println( log );
			return false;
		}

	return true;
}

void JSONViewer::reload( void ){
	String log = "";
	this->reload( log );
}

void JSONViewer::begin( Stream* monitor ){
	unsigned addr = 0;
	this->begin( monitor, addr );
}

void JSONViewer::loop( bool stop ){
	
		if( stop ) return;
		
	uint32_t curr = millis();
	static uint32_t update = 0;

		if( curr - update >= this->cfghttp.interval ){
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

int8_t JSONViewer::url_service( uint8_t inout, char * cmd, char * params ) {

		if( int_inst == NULL ) return -1;
		
		switch( inout ){
			case 0:{
				int_inst->monitor->print( F("Zapisany URL: ") );
				int_inst->monitor->println( int_inst->cfghttp.url );
			}break;
			case 1:{
				String url = params;
				url.trim();
				
				int http = url.indexOf("http://"), https = url.indexOf("https://");
				
					if( http == -1 && https == -1 ){
						int_inst->monitor->println( F("Nieprawidłowy URL") );
						return -1;
					}
					
					if( url.length() > 64 ){
						int_inst->monitor->println( F("URL może zawierać maksymalnie 64 znaki") );
						return -1;
					}
					
				strcpy( int_inst->cfghttp.url, url.c_str() );
				int_inst->save();
			}break;
		}
	
	return 0;
}

void JSONViewer::set_url( const char* url ){
	strcpy( this->cfghttp.url, url );
}
