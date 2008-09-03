/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2008 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmImageChangePlanarConfiguration.h"
#include "gdcmSequenceOfFragments.h"
#include "gdcmSequenceOfItems.h"
#include "gdcmFragment.h"

namespace gdcm
{

size_t ImageChangePlanarConfiguration::RGBPlanesToRGBPixel(char *out, const char *r, const char *g, const char *b, size_t s)
{
  char *pout = out;
  for(size_t i = 0; i < s; ++i )
    {
    *pout++ = *r++;
    *pout++ = *g++;
    *pout++ = *b++;
    }

  assert( (size_t)(pout - out) == 3 * s );
  return pout - out;
}

bool ImageChangePlanarConfiguration::Change()
{
  if( Input->GetPixelFormat().GetSamplesPerPixel() != 3 )
    {
    return false;
    }
  assert( Input->GetPhotometricInterpretation() == PhotometricInterpretation::YBR_FULL
    || Input->GetPhotometricInterpretation() == PhotometricInterpretation::RGB );
  Output = Input;
  if( Input->GetPlanarConfiguration() == PlanarConfiguration )
    {
    return true;
    }

  const Image &image = *Input;

  const unsigned int *dims = image.GetDimensions();
  unsigned long len = image.GetBufferLength();
  char *p = new char[len];
  image.GetBuffer( p );

  assert( len % 3 == 0 );
  size_t framesize = dims[0] * dims[1] * 3;
  assert( framesize * dims[2] == len );

  char *copy = new char[len];
  for(unsigned int z = 0; z < dims[2]; ++z)
    {
    const char *frame = p + z * framesize;
    size_t size = framesize / 3;
    const char *r = frame + 0;
    const char *g = frame + size;
    const char *b = frame + size + size;

    char *framecopy = copy + z * framesize;
    ImageChangePlanarConfiguration::RGBPlanesToRGBPixel(framecopy, r, g, b, size);
    }
  delete[] p;

  DataElement &de = Output->GetDataElement();
  de.SetByteValue( copy, len );
  delete[] copy;

  Output->SetPlanarConfiguration( PlanarConfiguration );

  return true;
}


} // end namespace gdcm

