/*=========================================================================

  Program: GDCM (Grass Root DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006 Mathieu Malaterre
  Copyright (c) 1993-2005 CREATIS
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmItem.h"
#include "gdcmStringStream.h"
#include "gdcmVR.h"
#include "gdcmExplicitDataElement.h"

void CreateDataElement(gdcm::ExplicitDataElement &de, int offset)
{
  gdcm::StringStream ss;

  gdcm::Tag tag(0x1234, 0x5678+offset);
  gdcm::VR vr = gdcm::VR::UN;
  const char str[] = "GDCM";
  uint32_t len = strlen(str);
  assert( sizeof(uint32_t) == 4 );
  gdcm::ByteValue val(str, len);
  tag.Write(ss);
  vr.Write(ss);
  const char *slen = reinterpret_cast<char*>(&len);
  ss.Write( slen, 4);
  val.Write(ss);
  de.Read( ss );

  std::cout << de << std::endl;
}

int TestItem(int , char *[])
{
  gdcm::Item item(gdcm::TS::Explicit);
  std::cout << item << std::endl;

  gdcm::Item it1(gdcm::TS::Explicit);
  gdcm::Item it2(gdcm::TS::Explicit);

  gdcm::DataSet ds;
  gdcm::ExplicitDataElement xde;
  CreateDataElement(xde,0);
  ds.InsertDataElement( xde );
  CreateDataElement(xde,1);
  ds.InsertDataElement( xde );
  CreateDataElement(xde,2);
  ds.InsertDataElement( xde );
  CreateDataElement(xde,3);
  ds.InsertDataElement( xde );

  std::cout << ds << std::endl;

  return 0;
}
