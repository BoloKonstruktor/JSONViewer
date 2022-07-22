#ifndef TextLine_H
#define TextLine_H
#include "Arduino.h"


enum{ LEFT, CENTER, RIGHT };
#define MAX_STRINGS     10

  class ShowTextLine {
    uint32_t update = 0, update2 = 0, update3 = 0;
    uint16_t pos = 0, len = 0, max = 0, temp = 0;
    uint8_t al = 0, dl = 0, sc = 0, it = 0;
    bool once = false, blk = false, end_scroll = false;
    String str = "", _str = "", __str = "";
    String strings[MAX_STRINGS];
    String seps[MAX_STRINGS];

    void(*end_callback)( void ) = NULL;
  
    public:
      bool Preambule = false, ScrollDelayOnce = false;
      uint8_t ScrollDelay = 3, BgPosition = 0, Row = 0;
      ShowTextLine( void ){
      this->str = "";
      this->max = 0;
      this->len = 0;
      this->al = 0;
      }

      void scroll_end_callback( void(*callback)( void ) ){
        this->end_callback = callback;
      }

      void clear( String s = "***" ){
        this->set( "", this->len, this->al );
        this->_str = s;
      }

      void remove( void ){
          
          for( uint8_t i = 0; i < MAX_STRINGS; i++ ){
            this->strings[i] = "";
            this->seps[i] = "";
          }
        
        this->it = 0;
      }

      void add( const char* label, String text, String separator = "" ){
        text.trim();
          
          if( text.length() >= 3 && this->it < MAX_STRINGS ) {
            String _label = String(label);
            _label.trim();
              if( _label != "" ) text = _label+" "+text;
            this->strings[this->it] = text;
            this->seps[this->it] = separator;
            this->it++;
          }
      }

      void setMultiStr( uint16_t max_length = 0, uint8_t align = CENTER ){
        String text = "";
  
          for( uint8_t i = 0; i < MAX_STRINGS; i++ ){
            auto s = strings[i];
              if( s != "" ) {
                text += s;

                  if( i < (this->it-1) ){
                    text += this->seps[i];
                  }
              }
          }

          if( text != this->_str ){
            this->_str = text;
              if( max_length == 0 ) max_length = text.length();
            this->set( text, max_length, align );
          }
      }

      void set( String text, uint16_t max_length, uint8_t align = CENTER ){
          
          if( this->__str != text ){
            this->__str = text;
            this->temp = temp;
            this->update3 = this->temp ? millis() : 0;
            this->str = text;
            this->max = max_length-this->BgPosition;
            this->len = text.length();
            this->al = align;
            this->pos = 0;
            this->dl = 0;
            this->once = false;
          }      
      }

      void set( const char* text, uint16_t max_length, uint8_t align = CENTER ){
        String str = text;
        this->set( str, max_length, align );      
      }
  
      String get( String spchar = " ", bool getstring = false ){
        this->end_scroll = false;
        String space = "";
        String s = "";
        this->str.replace( "ą", "a" );
        this->str.replace( "ć", "c" );
        this->str.replace( "ę", "e" );
        this->str.replace( "ł", "l" );
        this->str.replace( "ń", "n" );
        this->str.replace( "ó", "o" );
        this->str.replace( "ś", "s" );
        this->str.replace( "ż", "z" );
        this->str.replace( "ź", "z" );
        this->str.replace( "Ą", "A" );
        this->str.replace( "Ć", "C" );
        this->str.replace( "Ę", "E" );
        this->str.replace( "Ł", "L" );
        this->str.replace( "Ń", "N" );
        this->str.replace( "Ó", "O" );
        this->str.replace( "Ś", "S" );
        this->str.replace( "Ż", "Z" );
        this->str.replace( "Ź", "Z" );

          if( getstring ) return this->str;
          
          if( this->len <= this->max ){
            uint8_t d = this->al == CENTER ? 2 : 1;
            uint8_t ns = (this->max-this->len)/d;
            
              for( uint16_t i = 0; i < ns; i++ )
                space += spchar;

              switch( this->al ){
                case LEFT:{
                  s = this->str; 
                }break;
                case CENTER:{
                  s = space;
                  s += this->str;
                  s += space;
                }break;
                case RIGHT:{
                  s = space;
                  s += this->str; 
                }break;
              }
                for( uint8_t i = 0; i < this->max-s.length(); i++ )
                  s += spchar;    
            return s;      
          } else {
              for( uint16_t i = 0; i < this->max; i++ )
                space += spchar;
  
            s = this->Preambule ? space : "";
            s += this->str;
            s += space;
            uint8_t len = this->Preambule ? this->len+this->max : this->len-this->max;

              if( this->end_callback && this->Preambule ){
                this->end_scroll = true;
                
                  if( this->pos == len )this->end_callback();  
              }

              if( this->pos < len ) {              
                  if( !this->Preambule ){
                      if( this->dl <= this->ScrollDelay ) this->dl++;
                      else this->pos++;
                  } else this->pos++;
              } else {
                  if( !this->ScrollDelayOnce ) {
                      if( this->dl ) this->dl--; 
                      else this->pos = 0;
                  } else this->pos = 0;
              }
             
            return s.substring( this->pos, this->pos+this->max );
          } 
      }

      String get( bool& scroll ){
        String str = this->get();
        scroll = this->end_scroll;
        return str;
      }

      String getMultiStr( uint8_t from=0, int8_t to=MAX_STRINGS ){
        String text = "";
        uint8_t i = from;

          for( ; i < to; i++ ){
            auto s = this->strings[i];
              if( s != "" ) {
                text += s;
                  if( i < (this->it-1) ){
                    text += this->seps[i];
                  }  
              }
          }

        String sep = this->seps[i-1];
                
          if( text.substring( text.length()-sep.length() ) == sep ){
            text = text.substring( 0, text.length()-sep.length() );         
          }

        return text;
      }

      String blinker( String str ){
        String space = "";
        uint32_t curr = millis();
          for( uint8_t i = 0; i < this->max; i++ ) space += " ";

          if( curr-this->update2 >= 250 ){
              this->update2 = curr;
              this->blk ^= 1;
          }

          if( this->blk ) return space;
          else return str;
      }

      bool ticker( const uint32_t period, bool once=true ){
        if( this->len <= this->max && once ){
            if( !this->once ){
              this->once = true;
              return true; 
            }        
        } else {
          uint32_t curr = millis();
  
            if( curr-this->update >= period ){
              this->update = curr;
              return true;
            }
        }
        
        return false;
      }
  };
#endif
