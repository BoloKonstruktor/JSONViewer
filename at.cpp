#include "at.hpp"

ATCMD* ATCMD::int_inst = NULL;


//Metody statyczne
int8_t ATCMD::at_service( uint8_t inout, char * cmd, char * params ) {

		if( !inout && int_inst ) {
			int_inst->serial->println( F("Lista komend AT:") );
				
				for (; inout < int_inst->size; inout++) {
					int_inst->serial->print( F("\t") );
					auto at = int_inst->atlist[inout];
					int_inst->serial->println( at.cmd );
				}
		}
  
return 0;
}

int8_t ATCMD::ati_service( uint8_t inout, char * cmd, char * params ) {

		if( int_inst ) {
				
				if( int_inst->devname != "" ){
					int_inst->serial->print( F("Devname:\t") );
					int_inst->serial->println( int_inst->devname );					
				}
			
			int_inst->serial->print( F("Build:\t\t") );
			int_inst->serial->print( __DATE__ );
			int_inst->serial->print( F(" ") );
			int_inst->serial->println( __TIME__ );
			int_inst->serial->printf( "Chip model:\t%s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision() );
		}
		
  return 0;
}