#ifndef AT_HPP
#define AT_HPP
#include "Arduino.h"


class ATCMD {
	
		typedef struct {
			char cmd[33];
			int8_t (*callback)(uint8_t, char*, char*);
		} TAT;
		
		uint8_t size = 0, idx = 0;
		String devname = "";
		TAT* atlist = NULL;
		Stream* serial = NULL;
		static ATCMD* int_inst;
		
		template< typename Arr > void arrcpy( Arr& to, const Arr from, unsigned size ){
			for( unsigned i = 0; i < size; i++ ){
				to[i] = from[i];
			}     
		}
		
		static int8_t at_service( uint8_t, char*, char* );
		static int8_t ati_service( uint8_t, char*, char* );

	public:
		void begin( Stream* s, const char* devname="" ){
			int_inst = this;
			serial = s;
			this->devname = devname;
			this->registerCallback( "AT", at_service );
			this->registerCallback( "ATI", ati_service );
		}
		
		void registerCallback( const char* cmd, int8_t (*callback)( uint8_t inout, char * cmd, char * params ) ){

			this->size++;
			
				if( this->size == 1 ){
					this->atlist = new TAT[this->size];
				} else {
					TAT* buff = new TAT[this->size];
					arrcpy( buff, this->atlist, this->size-1 );
					delete[] this->atlist;
					this->atlist = new TAT[this->size];
					arrcpy( this->atlist, buff, this->size-1 );  
					delete[] buff;
				}

			this->idx = this->size-1;
			auto& atlist = this->atlist[this->idx];
			strcpy( atlist.cmd, cmd );
			atlist.callback = callback;
			this->idx = 0;
		}
	  
		void loop( void ){
			
				if( !this->size ) return;
				
			uint8_t buflen = 0, atlen = 0, opt = 0, i = 0;
			int8_t (*_at_srv)(uint8_t, char*, char*);
			char * params;
		
				if( serial->available() ){
					serial->setTimeout(5);
					String str = serial->readString();
					char* buf = (char*)str.c_str();
		  

						if( strncasecmp("AT", buf, 2) != 0 ) {
							return;
						}
			
						if( strpbrk( buf, "\r\n" ) ){
							buf = strtok_r( buf, "\r\n", &params );
						}
		  
						if( strpbrk(buf, "=?") ){
								
								if( strpbrk(buf, "?") ){
									opt = 0; 
									buf = strtok_r(buf, "?", &params);      
								} else {
									opt = 1;
									buf = strtok_r(buf, "=", &params);  
								}
								
							i = 0;
				
								while( params[i] == ' ' ) i++;
							
							params += i;
							i = strlen(params)-1;
				
								while(params[i] == ' ' && i > 0) i--;
			  
							params[i+1] = 0;
								
								if(opt == 1 && strcmp("", params) == 0) return;
						
						} else opt = 2;
	  
					buflen = strlen( buf );
	  
						for( i = 0; i < this->size; i++ ) {
							auto at = this->atlist[i];
							atlen = strlen( at.cmd );
							char* var = strchr( at.cmd, '#' );

								if( var != NULL ){
									uint8_t h = var-at.cmd;
									buflen = h;
									atlen -= 1;
								}
					
								if( buflen && buflen==atlen && strncasecmp( buf, at.cmd, buflen ) == 0 ) {
										
										if( at.callback ) {
												
												if( at.callback( opt, buf+2, params ) < 0 ){
													serial->println( F("FAIL") );
													return;  
												} else {
													serial->println( F("OK") );
													return;
												}
										}
								
									break;
								}
						}
	  
						if( i == this->size ) serial->println( F("UNKNOWN CMD") );

				}
		}
};
#endif
