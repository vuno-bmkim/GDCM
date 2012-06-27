/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library

  Copyright (c) 2006-2011 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmByteValue.h"
#include "gdcmUIDGenerator.h"

#include <cstring> // memcpy

namespace gdcm
{

  void ByteValue::PrintASCII(std::ostream &os, VL maxlength ) const {
    VL length = std::min(maxlength, Length);
    // Special case for VR::UI, do not print the trailing \0
    if( length && length == Length )
      {
      if( Internal[length-1] == 0 )
        {
        length = length - 1;
        }
      }
    // I cannot check IsPrintable some file contains \2 or \0 in a VR::LO element
    // See: acr_image_with_non_printable_in_0051_1010.acr
    //assert( IsPrintable(length) );
    std::vector<char>::const_iterator it = Internal.begin();
    for(; it != Internal.begin()+length; ++it)
      {
      const char &c = *it;
      if ( !( isprint((unsigned char)c) || isspace((unsigned char)c) ) ) os << ".";
      else os << c;
      }
  }
  void ByteValue::PrintHex(std::ostream &os, VL maxlength ) const {
    VL length = std::min(maxlength, Length);
    // WARNING: Internal.end() != Internal.begin()+Length
    std::vector<char>::const_iterator it = Internal.begin();
    os << std::hex;
    for(; it != Internal.begin()+length; ++it)
      {
      //const char &c = *it;
      uint8_t v = *it;
      if( it != Internal.begin() ) os << "\\";
      os << std::setw( 2 ) << std::setfill( '0' ) << (uint16_t)v;
      //++it;
      //os << std::setw( 1 ) << std::setfill( '0' ) << (int)*it;
      }
    os << std::dec;
  }

  bool ByteValue::GetBuffer(char *buffer, unsigned long length) const {
    // SIEMENS_GBS_III-16-ACR_NEMA_1.acr has a weird pixel length
    // so we need an inequality
    if( length <= Internal.size() )
      {
      memcpy(buffer, &Internal[0], length);
      return true;
      }
    gdcmDebugMacro( "Could not handle length= " << length );
    return false;
    }
  
  void ByteValue::PrintASCII_XML(std::ostream &os) const
    {
      //VL length = std::min(maxlength, Length);
      // Special case for VR::UI, do not print the trailing \0
      
      /*if(Length && Internal[Length-1] == 0 )
        {
        Length = Length - 1;
        }
      */
      // Check for non printable characters
      
      int count = 1;
      
      os << "<Value number = \"" << count << "\" >" ;
      
      std::vector<char>::const_iterator it = Internal.begin();
      
      for(; it != (Internal.begin() + Length); ++it)
        {
        const char &c = *it;
        
        if ( c == '\\' )
        {
            count++;
            os << "</Value>\n";
            os << "<Value number = \"" << count << "\" >" ;
        }
        
        else if ( !( isprint((unsigned char)c) || isspace((unsigned char)c) ) ) 
            os << ".";
        
        else 
            os << c;
        
        }
      os << "</Value>";
    
    }  
    
    void ByteValue::PrintHex_XML(std::ostream &os ) const 
    {
      //VL length = std::min(maxlength, Length);
      // WARNING: Internal.end() != Internal.begin()+Length
     
      std::vector<char>::const_iterator it = Internal.begin();
      os << std::hex;
      for(; it != Internal.begin() + Length; ++it)
        {
        //const char &c = *it;
        uint8_t v = *it;
        if( it != Internal.begin() ) os << "\\";
        os << std::setw( 2 ) << std::setfill( '0' ) << (uint16_t)v;
        //++it;
        //os << std::setw( 1 ) << std::setfill( '0' ) << (int)*it;
        }
      os << std::dec;
      
     }  


}
