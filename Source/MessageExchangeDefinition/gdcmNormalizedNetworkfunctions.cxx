/*=========================================================================
*
*  Copyright Insight Software Consortium
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0.txt
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*=========================================================================*/

#include "gdcmNormalizedNetworkFunctions.h"

#include <socket++/echo.h>

#include "gdcmReader.h"
#include "gdcmPrinter.h"
#include "gdcmAttribute.h"
#include "gdcmULConnectionManager.h"
#include "gdcmULConnection.h"
#include "gdcmDataSet.h"
#include "gdcmVersion.h"
#include "gdcmGlobal.h"
#include "gdcmSystem.h"
#include "gdcmUIDGenerator.h"
#include "gdcmWriter.h"
#include "gdcmSimpleSubjectWatcher.h"
#include "gdcmProgressEvent.h"
#include "gdcmQueryFactory.h"
#include "gdcmULWritingCallback.h"
#include "gdcmULBasicCallback.h"
#include "gdcmPresentationContextGenerator.h"

namespace gdcm
{
	BaseQuery* 
		NormalizedNetworkFunctions::ConstructQuery(	const std::string & sopInstanceUID, 
		const DataSet& queryds, ENQueryType queryType /*= eMMPS*/ )
	{
		BaseQuery* outQuery = NULL ;
		if( queryType == eMMPS )
			outQuery = QueryFactory::ProduceQuery( sopInstanceUID, queryType );

		if (!outQuery)
		{
			gdcmErrorMacro( "Specify the query" );
			return NULL;
		}

		outQuery->AddQueryDataSet(queryds);

		// setup the special character set
		std::vector<ECharSet> inCharSetType;
		inCharSetType.push_back( QueryFactory::GetCharacterFromCurrentLocale() );
		DataElement de = QueryFactory::ProduceCharacterSetDataElement(inCharSetType);
		std::string param ( de.GetByteValue()->GetPointer(),
			de.GetByteValue()->GetLength() );
		outQuery->SetSearchParameter(de.GetTag(), param );

		// Print info:
		if (Trace::GetDebugFlag())
		{
			outQuery->Print( Trace::GetStream() );
		}

		return outQuery;
	}

	bool 
		NormalizedNetworkFunctions::NEventReport( const char *remote, uint16_t portno,
		const BaseQuery* query, std::vector<DataSet> &retDataSets,
		const char *aetitle, const char *call )
	{
		return false ;
	}
	bool 
		NormalizedNetworkFunctions::NGet( const char *remote, uint16_t portno,
		const BaseQuery* query, std::vector<DataSet> &retDataSets,
		const char *aetitle, const char *call )
	{
		return false ;
	}
	bool 
		NormalizedNetworkFunctions::NSet( const char *remote, uint16_t portno,
		const BaseQuery* query, std::vector<DataSet> &retDataSets,
		const char *aetitle, const char *call )
	{
		if( !remote ) return false;
		if( !aetitle )
		{
			aetitle = "GDCMSCU";
		}
		if( !call )
		{
			call = "ANY-SCP";
		}

		// $ findscu -v  -d --aetitle ACME1 --call ACME_STORE  -P -k 0010,0010="X*"
		//   dhcp-67-183 5678  patqry.dcm
		// Add a query:

		// Generate the PresentationContext array from the query UID:
		PresentationContextGenerator generator;
		if( !generator.GenerateFromUID( query->GetAbstractSyntaxUID() ) )
		{
			gdcmErrorMacro( "Failed to generate pres context." );
			return false;
		}

		network::ULConnectionManager theManager;
		if (!theManager.EstablishConnection(aetitle, call, remote, 0, portno, 1000,
			generator.GetPresentationContexts()))
		{
			gdcmErrorMacro( "Failed to establish connection." );
			return false;
		}
		std::vector<DataSet> theDataSets;
		std::vector<DataSet> theResponses;
		//theDataSets = theManager.SendFind( query );
		network::ULBasicCallback theCallback;
		theManager.SendNSet(query, &theCallback);
		theDataSets = theCallback.GetDataSets();
		theResponses = theCallback.GetResponses();

		bool ret = false; // by default an error
		if( theResponses.empty() )
		{
			gdcmErrorMacro( "Failed to GetResponses." );
			return false;
		}
		assert( theResponses.size() >= 1 );
		// take the last one:
		const DataSet &ds = theResponses[ theResponses.size() - 1 ]; // FIXME
		assert ( ds.FindDataElement(Tag(0x0, 0x0900)) );
		Attribute<0x0,0x0900> at;
		at.SetFromDataSet( ds );

		//          Table CC.2.1-2
		//				STATUS VALUES
		const uint16_t theVal = at.GetValue();
		switch( theVal )
		{
		case 0x0: 
			gdcmDebugMacro( "The requested modification of the attribute values is performed." );
			// Append the new DataSet to the ret one:
			retDataSets.insert( retDataSets.end(), theDataSets.begin(), theDataSets.end() );
			ret = true;
			break;
		case 0x1: 
			{
				gdcmWarningMacro( "Requested optional Attributes are not supported." );
			}
		case 0xB305: 
			{
				gdcmWarningMacro( "The UPS is already in the requested state of COMPLETED" );
			}																													   
			break;			
		case 0xC310: 
			{
				gdcmErrorMacro( "Refused: The UPS is not in the �IN PROGRESS� state " );
			}
			break; 
		case 0xC301: 
			{
				gdcmErrorMacro( "Refused: The correct Transaction UID was not provided " );
			}
			break; 
		case 0xC300: 
			{
				gdcmErrorMacro( "Refused: The UPS may no longer be updated " );
					 }
					 break; 
		case 0xC307: 
			{
			gdcmErrorMacro( "Specified SOP Instance UID does not exist or is not a UPS Instance managed by this SCP " );
			}
					 break; 
		default:
			break ;
		}
		theManager.BreakConnection(-1);//wait for a while for the connection to break, ie, infinite

		return ret;
	}
	bool 
		NormalizedNetworkFunctions::NAction( const char *remote, uint16_t portno,
		const BaseQuery* query, std::vector<DataSet> &retDataSets,
		const char *aetitle, const char *call )
	{
		return false ;
	}
	bool 
		NormalizedNetworkFunctions::NCreate( const char *remote, uint16_t portno,
		const BaseQuery* query, std::vector<DataSet> &retDataSets,
		const char *aetitle, const char *call )
	{
		if( !remote ) return false;
		if( !aetitle )
		{
			aetitle = "GDCMSCU";
		}
		if( !call )
		{
			call = "ANY-SCP";
		}

		// $ findscu -v  -d --aetitle ACME1 --call ACME_STORE  -P -k 0010,0010="X*"
		//   dhcp-67-183 5678  patqry.dcm
		// Add a query:

		// Generate the PresentationContext array from the query UID:
		PresentationContextGenerator generator;
		if( !generator.GenerateFromUID( query->GetAbstractSyntaxUID() ) )
		{
			gdcmErrorMacro( "Failed to generate pres context." );
			return false;
		}

		network::ULConnectionManager theManager;
		if (!theManager.EstablishConnection(aetitle, call, remote, 0, portno, 1000,
			generator.GetPresentationContexts()))
		{
			gdcmErrorMacro( "Failed to establish connection." );
			return false;
		}
		std::vector<DataSet> theDataSets;
		std::vector<DataSet> theResponses;
		//theDataSets = theManager.SendFind( query );
		network::ULBasicCallback theCallback;
		theManager.SendNDelete(query, &theCallback);
		theDataSets = theCallback.GetDataSets();
		theResponses = theCallback.GetResponses();

		bool ret = false; // by default an error
		if( theResponses.empty() )
		{
			gdcmErrorMacro( "Failed to GetResponses." );
			return false;
		}
		assert( theResponses.size() >= 1 );
		// take the last one:
		const DataSet &ds = theResponses[ theResponses.size() - 1 ]; // FIXME
		assert ( ds.FindDataElement(Tag(0x0, 0x0900)) );
		Attribute<0x0,0x0900> at;
		at.SetFromDataSet( ds );

		//          Table CC.2.1-2
		//				STATUS VALUES
		const uint16_t theVal = at.GetValue();
		switch( theVal )
		{
		case 0x0: // The requested state change was performed
			gdcmDebugMacro( "The requested state change was performed." );
			// Append the new DataSet to the ret one:
			retDataSets.insert( retDataSets.end(), theDataSets.begin(), theDataSets.end() );
			ret = true;
			break;
		case 0xB304: // The UPS is already in the requested state of CANCELED
			{
				gdcmWarningMacro( "Offending Element: The UPS is already in the requested state of CANCELED" );
			}
		case 0xB306: // The UPS is already in the requested state of COMPLETED
			{
				gdcmWarningMacro( "The UPS is already in the requested state of COMPLETED" );
			}																													   
			break;			
		case 0xC300: 
			{
				gdcmErrorMacro( "Refused: The UPS may no longer be updated " );
			}
			break; 
		case 0xC301: 
			{
				gdcmErrorMacro( "Refused: The correct Transaction UID was not provided " );
			}
			break; 
		case 0xC302: 
			{
				gdcmErrorMacro( "Refused: The UPS is already IN PROGRESS " );
					 }
					 break; 
		case 0xC303: 
			{
			gdcmErrorMacro( "Refused: The UPS may only become SCHEDULED via NCREATE, not N-SET or N-ACTION " );
					 }
					 break; 
		case 0xC304: 
			{
			gdcmErrorMacro( "Refused: The UPS has not met final state requirements for the requested state change " );
					 }
					 break; 
		case 0xC307: 
			{
			gdcmErrorMacro( "Specified SOP Instance UID does not exist or is not a UPS Instance managed by this SCP " );
					 }
					 break; 
		case 0xC310: 
			{
			gdcmErrorMacro( "Refused: The UPS is not yet in the �IN PROGRESS� state " );
					 }
					 break;	

		default:
			break ;
		}
		theManager.BreakConnection(-1);//wait for a while for the connection to break, ie, infinite

		return ret;
	}

	bool 
		NormalizedNetworkFunctions::NDelete( const char *remote, uint16_t portno,
		const BaseQuery* query, std::vector<DataSet> &retDataSets,
		const char *aetitle, const char *call )
	{
		return false ;
	}
} // end namespace gdcm
