#include "common.h"

  String explode( String str, uint8_t pos, const char* sep ) {
    str = sep+str;
    size_t found = str.indexOf( sep );
    uint8_t seplen = strlen( sep );
    unsigned i = 0;

    while( found != -1 ){
      
      if( found != -1 ){
        size_t ed = str.substring( found+seplen ).indexOf( sep );
        
          if( i == pos && ed != -1 ) {
            String buff = str.substring( found+seplen, found+ed+seplen );
            buff.trim();
            return buff;
          }
        
        str = str.substring( found+seplen );
        found = ed;
        i++;
      }
    }
    
    return str;
  }
  
  String extractJSON( String str ){
	str.trim();
	unsigned len = str.length();
	unsigned i = 0;
	
		while( i < len ){
			
				if( str[i] == '{' ){
					str = str.substring( i );
					break;
				}
				
			i++;
		}
		
	i = len;
		
		while( i ){
			
				if( str[i] == '}' ){
					str = str.substring( 0, i );
					break;
				}
				
			i--;
		}
		
	return str;
  }
  
String* extractPath( const char* path, unsigned& size ){
	char* slash = strchr( (char*)path, '/' );
    char buff[64];
    int bg = 0, ed = 0, s = 1, i = 0;
    String* o = nullptr;

        if( path[0] != '/' ) s = 0;

        while( slash != NULL ){

            slash = strchr( (char*)path+ed+s, '/' );

                if( slash != NULL || strlen(path+bg+s) ){
                    ed = slash-path;
                    strcpy( buff, path+bg+s );

                        if( slash != NULL ) buff[ed-bg-1] = 0;

                    bg = ed;
                    size++;

                        if( size == 1 ){
                            o = new String[size];
                            o[size-1] = buff;
                        } else {
                            String* b = new String[size];
                            arrcpy( b, o, size-1 );
                            delete[] o;
                            o = NULL;
                            b[size-1] = buff;
                            o = new String[size];
                            arrcpy( o, b, size );
                            delete[] b;
                            b = NULL;
                        }
                }
        }

    return o;
}

bool is_digit( const char* str ){
	uint8_t len = strlen( str );
	uint8_t i = 0;

		while( i < len ){
				if( !isdigit( str[i] ) ) return false;
		}
		
	return true;
}

  unsigned eep_read( unsigned offset, void* data, uint16_t size ){
  uint8_t* buff = (uint8_t*)data;
  
      for ( uint16_t i = 0; i < size; i++ ){
        buff[i] = EEPROM.readUChar( offset );
        offset++;
  
          if ( offset == EEPROM.length() ) break;
      }
   
  return offset;
  }
  
  unsigned eep_write( unsigned offset, void* data, uint16_t size ){
    const uint8_t* buff = (uint8_t*)data;
      
      for ( uint16_t i = 0; i < size; i++ ){
        EEPROM.writeUChar( offset, buff[i] );
        offset++;
  
          if ( offset == EEPROM.length() ) break;
      }
  
    EEPROM.commit();
    return offset+size;
  }
  
  bool eep_empty( void* data, uint16_t size ){
    const uint8_t* buff = (uint8_t*)data;
  
      for( uint16_t i = 0; i < size; i++ ){
            
        if( buff[i] != 0xFF ) return false;
      }
  
    return true;
  }