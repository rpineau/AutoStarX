// BodyData.cpp: implementation of the CBodyData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BodyData.h"
#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// constructor for sfieldDesc
sfieldDesc::sfieldDesc()
{
	fieldSize = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CBodyData::CBodyData()
{
}

CBodyData::CBodyData(CBodyData &cpy)
{
	m_key = cpy.m_key;
	m_BodyType = cpy.m_BodyType;
}

CBodyData& CBodyData::operator=(CBodyData &Rdata)
{
	if (&Rdata != this)
	{
		m_key = Rdata.m_key;
		m_BodyType = Rdata.m_BodyType;
	}
	return *this;
}

CBodyData::~CBodyData()
{

}


void CBodyData::SetBodyType(BodyType type)
{
	m_BodyType = type;
}

BodyType CBodyData::GetBodyType()
{
	return m_BodyType;
}

/////////////////////////////////////////////
//
//	Name		:GetFieldLabel
//
//	Description :Returns the label for the field number entered.
//
//  Input		:Field number
//
//	Output		:Label
//
////////////////////////////////////////////
CString CBodyData::GetFieldLabel(int i)
{
sfieldDesc	*fDesc;	
	
	// get the field descriptor
	if ((fDesc = GetFieldDesc(i)) != NULL)
	{
		return fDesc->Label;
	}

		return "Not Valid";
}


/////////////////////////////////////////////
//
//	Name		:GetFieldData
//
//	Description :Returns the data referenced in the input as a CString
//				 suitable for framing.
//
//  Input		:field index
//
//	Output		:CString of the data
//
////////////////////////////////////////////
CString CBodyData::GetFieldData(int i)
{
sfieldDesc	*fDesc;	
CString		temp, form;
int			Hr,month, half;
double		dHRem,day;
	
	// get the field descriptor
	if ((fDesc = GetFieldDesc(i)) != NULL)
	{
		form = fDesc->Format;
		switch (fDesc->Type)
		{
		case KEY_TYPE :
			return GetKey();

		case STRING_TYPE :
			return *fDesc->StringPtr;

		case DATE_TYPE :	// extract the extra nibble from the month
							// and add it to the day decimal place
			month = fDesc->DatePtr->Epoch_mm & 15;
			day = (double)((0xF0 & fDesc->DatePtr->Epoch_mm) >> 4) / 10.0;
			day += (int)fDesc->DatePtr->Epoch_dd;
			temp.Format(form.GetBuffer(5), month, day, (int)fDesc->DatePtr->Epoch_yy);
//			temp.Format(form.GetBuffer(5), (int)fDesc->DatePtr->Epoch_mm, (int)fDesc->DatePtr->Epoch_dd, (int)fDesc->DatePtr->Epoch_yy);
			return temp;

		case JDATE_TYPE :
			temp.Format(form.GetBuffer(5), (int)fDesc->JDatePtr->EpochYear, fDesc->JDatePtr->EpochDay);
			return temp;

		case FLOAT_TYPE :
			temp.Format(form.GetBuffer(5), *fDesc->FloatPtr);
			return temp;

		case PREC_DEC_TYPE :
		case PREC_RA_TYPE :

			double dHr, dMn, dSc, dRm;

			// take the integer of (total seconds / 3600) to get the hours
			modf(*fDesc->DoublePtr / 3600, &dHr);

			// take the integer of (total seconds - hours in seconds / 60) to get the Minutes
			modf((*fDesc->DoublePtr - (dRm = (dHr * 3600))) / 60, &dMn);

			// take the Total seconds - hours in seconds - minutes in seconds to get the remaining seconds
			dSc = *fDesc->DoublePtr - dRm - (dMn * 60);

			// remove the sign from the Minutes and Seconds
			if (dMn < 0)
				dMn = -dMn;
			if (dSc < 0)
				dSc = -dSc;

			temp.Format(form.GetBuffer(5), dHr, dMn, dSc);

			return temp;

		case RA_TYPE :
			Hr = *fDesc->ShortPtr/900;
			dHRem = (double)(*fDesc->ShortPtr % 900);
			temp.Format(form.GetBuffer(5), Hr, dHRem/15);
			return temp;

		case DEC_TYPE :
		case AZ_TYPE :
			temp.Format(form.GetBuffer(5), (int)(*fDesc->ShortPtr/60), abs(*fDesc->ShortPtr%60));
			return temp;

		case LONG_RA_TYPE :
			int min,sec,rem;
			Hr = *fDesc->LongPtr/3600/15;
			rem = *fDesc->LongPtr % (3600 * 15);
			min = (int)(static_cast<double>(rem) / (60 * 15));
			sec = (int)((rem % (60 * 15)) / 15);
			temp.Format(form.GetBuffer(5), Hr, min, sec);
			return temp;

		case LONG_DEC_TYPE :
		case LONG_LONG_TYPE:
			Hr = *fDesc->LongPtr/3600;
			rem = abs(*fDesc->LongPtr % (3600));
			min = (int)(static_cast<double>(rem) / 60);
			sec = (int)(rem % 60);
			temp.Format(form.GetBuffer(5), Hr, min, sec);
//			temp.Format(form.GetBuffer(5), (int)(*fDesc->LongPtr/60), abs(*fDesc->LongPtr%60));
			return temp;


		case SHORT_TYPE :
			temp.Format(form.GetBuffer(5), (int)(*fDesc->ShortPtr));
			return temp;

		case DOUBLE_TYPE :
			temp.Format(form.GetBuffer(5), *fDesc->DoublePtr);
			return temp;

		case TIMEZONE_TYPE :
			Hr = (int) (*fDesc->CharPtr);
			half = Hr % 2;
			if (half > 0)
				half = 30;	// if there is a remainder, time zone is half-hour
			if (Hr < 24)
				Hr = 0 - Hr;
			else
				Hr = 48 - Hr;
			Hr /= 2;
			temp.Format(form.GetBuffer(5), Hr, half);
			return temp;

		case BOOL_TYPE :
			if (*fDesc->BoolPtr)
				return "Yes";
			else
				return "No";

		case LONG_TYPE :
			temp.Format(form.GetBuffer(5), *fDesc->LongPtr);
			return temp;
		}
	}
	return "Error";	
}


/////////////////////////////////////////////
//
//	Name		:SetFieldData
//
//	Description :This will take the data entered and convert it 
//				 for the field entered as i. It will then range check
//				 it and enter it if its within range or report back
//				 a range error.
//
//  Input		:field index, data
//
//	Output		:range status
//
////////////////////////////////////////////
RANGESTAT CBodyData::SetFieldData(int i, CString data, bool import)
{
sfieldDesc	*fDesc;	
CString		temp, form;
float		fTemp, day;
double		duTemp;
date		dTemp;
jdate		jTemp;
short		sTemp;
int			sRes;
int			mon, yr;
char		c1, c2;
int			Hr;
float		Mn;
double		dHr, dMn, dSc;
int			Deg, Min, Sec;
long int	lTemp;
int			iTemp;	

	
	// trim the data
	data.TrimLeft();
	data.TrimRight();

	// get the field descriptor
	if ((fDesc = GetFieldDesc(i)) != NULL)
	{
	// if we're not doing an import then check the modifiable flag
		if (!import && !fDesc->Modifiable)
			return NOT_MODIFIABLE;

	// get a copy of the format string
		form = fDesc->Format;

	// do it based on type
		switch (fDesc->Type)
		{

	// KEY TYPE
		case KEY_TYPE :
			if (data.GetLength() != 0)
			{
			// space fill to 16
				while(data.GetLength() < 16)
					data += " ";
			// set the data
				SetKey(data);
				return OK;
			}
			return RANGEERROR;

	// STRING TYPE
		case STRING_TYPE :
			if (this->IsCustom())
				CPersist::RemoveBadChars(data);
			*fDesc->StringPtr = data;
			return OK;

	// DATE TYPE
		case DATE_TYPE :
		// read the date string
			if (import && data.Find(" ",0) < 0) // if import and no spaces then split Year Month Day from continious string
			{
				sRes = sscanf(data.GetBuffer(5), "%4d%2d%f", &yr, &mon, &day);
				sRes += 2;		// make it 5 to be compatable with the delimited version
			}
			else if (import)	// else its delimited
			{
				sRes = sscanf(data.GetBuffer(5), "%d%c%d%c%f", &yr, &c1, &mon, &c2, &day);
			}
			else		// its user input
			{
				sRes = sscanf(data.GetBuffer(5), "%d%c%f%c%d", &mon, &c1, &day, &c2, &yr);
			}


			// transfer to the temp date for range check (ignore the extra nibble)
			day += 0.05F;
			dTemp.Epoch_dd = (char)(int) (day);
			dTemp.Epoch_mm =  mon;
			dTemp.Epoch_yy = yr;

			// check for day overrun from rounding
			switch(dTemp.Epoch_mm)
			{
			case 9:	// 30 days hath September
			case 4:	// April
			case 6:	// June
			case 11:// and November
				if (dTemp.Epoch_dd >= 31)
				{
					dTemp.Epoch_dd = 1;
					dTemp.Epoch_mm++;
					day = 0;
				}
				break;

			case 1:	// All the rest have 31
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
				if (dTemp.Epoch_dd >= 32)
				{
					dTemp.Epoch_dd = 1;
					dTemp.Epoch_mm++;
					day = 0;
				}
				break;

			case 2:	// Except February which has 28 Except in a leap year which has 29
					// which is every 4 years except every 10 years except every 400 years
				if (dTemp.Epoch_yy != 2000 && (dTemp.Epoch_yy % 4 !=0 || dTemp.Epoch_yy % 10 == 0))
				{
					if (dTemp.Epoch_dd >= 29)
					{
						dTemp.Epoch_dd = 1;
						dTemp.Epoch_mm++;
						day = 0;
					}
				}
				else if (dTemp.Epoch_dd >= 30)
				{
					dTemp.Epoch_dd = 1;
					dTemp.Epoch_mm++;
					day = 0;
				}
				break;

			case 12: // December increments the year
				if (dTemp.Epoch_dd >= 32)
				{
					dTemp.Epoch_dd = 1;
					dTemp.Epoch_mm = 1;
					dTemp.Epoch_yy++;
					day = 0;
				}
				break;
			}
			
			if(sRes == 5 && !FieldOutofRange(fDesc, &dTemp)) // now include the extra nibble
			{
				fDesc->DatePtr->Epoch_dd = dTemp.Epoch_dd;
				fDesc->DatePtr->Epoch_mm = ((int)((day - floor(day)) * 10) << 4) | dTemp.Epoch_mm;
				fDesc->DatePtr->Epoch_yy = dTemp.Epoch_yy;
				return OK;
			}

			return RANGEERROR;

	// JULIAN DATA TYPE
		case JDATE_TYPE :
		// read the date string
			if (import && data.Find(" ",0) < 0) // if import and no spaces then split Year Month Day from continious string
			{
				sRes =0;
				temp = data.Left(2);
				if (sscanf(temp.GetBuffer(5), "%d", &yr) == 1)
					sRes++;

				// fix the year
				if (yr > 58)
					yr += 1900;
				else 
					yr += 2000;

				temp = data.Right(data.GetLength() - 2);
				if (sscanf(temp.GetBuffer(5), "%f", &fTemp) == 1)
					sRes++;

				sRes += 1;		// make it 3 to be compatable with the delimited version
			}
			else
			{
				sRes = sscanf(data.GetBuffer(5), "%d%c%f", &yr, &c1, &fTemp);
			}

			jTemp.EpochYear = yr;
			jTemp.EpochDay	= fTemp;

			if(sRes == 3 && !FieldOutofRange(fDesc, &jTemp))
			{
				fDesc->JDatePtr->EpochDay = jTemp.EpochDay;
				fDesc->JDatePtr->EpochYear = jTemp.EpochYear;
				return OK;
			}
			return RANGEERROR;

			
	// FLOAT TYPE
		case FLOAT_TYPE :
		// read the float from the string
			if (sscanf(data.GetBuffer(5), "%f", &fTemp) == 1)
			{

			// check the range
				if (!FieldOutofRange(fDesc, &fTemp))
				{
			// range is ok so assign it to the member variable
					*fDesc->FloatPtr = fTemp;
					return OK;
				}
			}
			
			if (import)
				return OK;
			else
				return RANGEERROR;

	// DOUBLE TYPE
		case DOUBLE_TYPE :
		// read the float from the string
			if (sscanf(data.GetBuffer(5), "%lf", &duTemp) == 1)
			{

			// check the range
				if (!FieldOutofRange(fDesc, &duTemp))
				{
			// range is ok so assign it to the member variable
					*fDesc->DoublePtr = duTemp;
					return OK;
				}
			}
			
			if (import)
				return OK;
			else
				return RANGEERROR;
			

	// Precision DEC
		case PREC_DEC_TYPE :
			dSc = 0;
			if (sscanf(data.GetBuffer(5), "%lf%c%lf%c%lf", &dHr, &c1, &dMn, &c2, &dSc) == 5 ||
				sscanf(data.GetBuffer(5), "%3lf%4lf", &dHr, &dMn) == 2 )
				goto PREC_CONT;
			else
				return RANGEERROR;
			

	// Precision RA Type
		case PREC_RA_TYPE :
			// read the data
			if (sscanf(data.GetBuffer(5), "%lf%c%lf%c%lf", &dHr, &c1, &dMn, &c2, &dSc) == 5 ||
				sscanf(data.GetBuffer(5), "%2lf%2lf%2lf", &dHr, &dMn, &dSc) == 3 )
			{
PREC_CONT:
				// Check the 
				if (dHr < 0 || (dHr == 0 && data.Left(1) == "-"))
					duTemp = (dHr * 3600.0) - (dMn * 60.0) - dSc;
				else
					duTemp = (dHr * 3600.0) + (dMn * 60.0) + dSc;


				// check the whole range
				if (!FieldOutofRange(fDesc, &duTemp))
				{
					// convert it
					*fDesc->DoublePtr = duTemp;
					return OK;
				}
			}

			return RANGEERROR;
		
			
		case RA_TYPE :

			// read the data
			if (sscanf(data.GetBuffer(5), "%d%c%f", &Hr, &c1, &Mn) == 3)
			{
			
				// check minutes
				if (Mn < 0 || Mn > 59.99)
					return RANGEERROR;

				// convert
				sTemp = (short)((Hr * 900) + (Mn * 15.0));

				// check the whole range
				if (!FieldOutofRange(fDesc, &sTemp))
				{
					// convert it
					*fDesc->ShortPtr = sTemp;
					return OK;
				}
			}

			return RANGEERROR;
		
	// Declination Type
		case DEC_TYPE :
		case AZ_TYPE :

			// read the data
			if (sscanf(data.GetBuffer(5), "%d%c%d", &Deg, &c1, &Min) == 3)
			{

				// check the minutes
				if (Min < 0 || Min > 59)
					return RANGEERROR;

				// assign the value
				if (Deg >= 0)
					sTemp = (Deg * 60) + Min;
				else
					sTemp = (Deg * 60) - Min;

				// check the whole range
				if (!FieldOutofRange(fDesc, &sTemp))
				{
					// convert it
					*fDesc->ShortPtr = sTemp;
					return OK;
				}
			}

			return RANGEERROR;

	// Long RA Type (e.g., Autostar II)
		case LONG_RA_TYPE :

			// read the data
			if (sscanf(data.GetBuffer(5), "%d%c%d%c%d", &Deg, &c1, &Min, &c2, &Sec) == 5)
			{
			
				// check minutes & seconds
				if (Min < 0 || Min > 59 || Sec < 0 || Sec > 59)
					return RANGEERROR;

				// convert
				lTemp = (long)((Deg * 3600 * 15) + (Min * 60.0 *15) + (Sec * 15));

				// check the whole range
				if (!FieldOutofRange(fDesc, &lTemp))
				{
					// convert it
					*fDesc->LongPtr = lTemp;
					return OK;
				}
			}

			return RANGEERROR;
		
	// Long Declination Type
		case LONG_DEC_TYPE :
		case LONG_LONG_TYPE:

			// read the data
			if (sscanf(data.GetBuffer(5), "%d%c%d%c%d", &Deg, &c1, &Min, &c2, &Sec) == 5)
			{

				// check minutes & seconds
				if (Min < 0 || Min > 59 || Sec < 0 || Sec > 59)
					return RANGEERROR;

				// assign the value
				if (Deg >= 0)
					lTemp = (Deg * 3600) + (Min * 60) + Sec;
				else
					lTemp = (Deg * 3600) - (Min * 60) - Sec;

				// check the whole range
				if (!FieldOutofRange(fDesc, &lTemp))
				{
					// convert it
					*fDesc->LongPtr = lTemp;
					return OK;
				}
			}

			return RANGEERROR;

	// Short Type
		case SHORT_TYPE :
			// read the data
			if (sscanf(data.GetBuffer(5), "%d", &iTemp) == 1)
			{
				sTemp = iTemp;
				// check the range
				if (!FieldOutofRange(fDesc, &sTemp))
				{
			// range OK so assign it to the member variable
					*fDesc->ShortPtr = sTemp;
					return OK;
				}
			}
			
			return RANGEERROR;

	// Time Zone Type
		case TIMEZONE_TYPE :
			// read the data
			if (sscanf(data.GetBuffer(5), "%d%c%f", &Hr, &c1, &Mn) == 3)
			{
			
				// check minutes
				if (Mn < 0 || Mn > 59.99)
					return RANGEERROR;

				// convert
				dHr = (double)(Hr * 60);
				if (Hr < 0)
					dHr -= (double) Mn;
				else
					dHr += (double) Mn;
				dHr /= 60;	// a signed decimal hour value

				if (dHr < 0)
					dHr = 0 - dHr;
				else
					if (dHr <= 24)
						dHr = 24 - dHr;
				dHr *= 2;	
				
				sTemp = (short) (dHr + 0.5);		// an integer from 0 - 48 representing half-hour time zones


				// check the whole range
				if (!FieldOutofRange(fDesc, &sTemp))
				{
					// convert it
					*fDesc->CharPtr = (char) sTemp;
					return OK;
				}
			}

			return RANGEERROR;

	// Bool Type
		case BOOL_TYPE :
			if (data.Find('1') >= 0 ||
				data.Find('Y') >= 0 ||
				data.Find('y') >= 0 ||
				data.Find('T') >= 0 ||
				data.Find('t') >= 0)
				*fDesc->BoolPtr = true;
			else
				*fDesc->BoolPtr = false;
			return OK;

	// Long Type
		case LONG_TYPE :
			if (sscanf(data.GetBuffer(5), "%ld", &lTemp) == 1)
			{
				// check range
				if (!FieldOutofRange(fDesc, &lTemp))
				{
					// assign it and buy it
					*fDesc->LongPtr = lTemp;
					return OK;
				}
			}
			return RANGEERROR;
		}
	}
	return INVALIDTYPE;
}

/////////////////////////////////////////////
//
//	Name		:GetFieldRangeHigh
//
//	Description :Returns the High limit for the passed field 
//
//  Input		:Field Index
//
//	Output		:High limit as CString
//
////////////////////////////////////////////
CString CBodyData::GetFieldRangeHigh(int i)
{
sfieldDesc	*fDesc;	
CString		temp, form;
	
	// get the field descriptor
	if ((fDesc = GetFieldDesc(i)) != NULL)
	{
		form = fDesc->Format;

	// do it based on type
		switch (fDesc->Type)
		{
		case STRING_TYPE :
		case KEY_TYPE :
			if (fDesc->fieldSize)	// if the field size is specified
			{
				CString output = "";
				output += CPersist::ToString((int)fDesc->fieldSize);
				output += " Characters";
				return output;
			}
			else
				return "16 Characters";

		case DATE_TYPE :
			return "12/31.9/3000";

		case JDATE_TYPE :
			return "3000/365.25";

		case DOUBLE_TYPE :
		case FLOAT_TYPE :
			temp.Format("%1.4g",fDesc->HiLimit);
			return temp;

		case PREC_RA_TYPE :
			return "23:59:59.99";

		case RA_TYPE :
		case LONG_RA_TYPE:
			return "23:59:59";

		case PREC_DEC_TYPE :
			return "90°00'00.00\"";

		case DEC_TYPE :
			return "90°00'";

		case AZ_TYPE :
			return "360°00'";

		case LONG_DEC_TYPE:
			return "90°00'00\"";

		case LONG_LONG_TYPE:
			return "180°00'00\"";

		case SHORT_TYPE :
		case LONG_TYPE :
		case TIMEZONE_TYPE :
			temp.Format("%02.0f:00", fDesc->HiLimit);
			return temp;

		case BOOL_TYPE :
			return "1";
		}
	}
	return "Hi Limit";
}

/////////////////////////////////////////////
//
//	Name		:GetFieldRangeLow
//
//	Description :Returns the Low limit for the passed field 
//
//  Input		:Field Index
//
//	Output		:Low limit as CString
//
////////////////////////////////////////////
CString CBodyData::GetFieldRangeLow(int i)
{
sfieldDesc	*fDesc;	
CString		temp, form;
	
	// get the field descriptor
	if ((fDesc = GetFieldDesc(i)) != NULL)
	{
		form = fDesc->Format;

	// do it based on type
		switch (fDesc->Type)
		{
		case STRING_TYPE :
		case KEY_TYPE :
			return "1";

		case DATE_TYPE :
			return "1/1.0/1900";

		case JDATE_TYPE :
			return "1900/0.0";

		case DOUBLE_TYPE :
		case FLOAT_TYPE :
			temp.Format("%1.4g",fDesc->LoLimit);
			return temp;

		case PREC_RA_TYPE :
			return "00:00:00.00";

		case RA_TYPE :
		case LONG_RA_TYPE:
			return "00:00:00";

		case PREC_DEC_TYPE :
			return "-90°00'00.00\"";

		case DEC_TYPE :
			return "-90°00'";

		case AZ_TYPE :
			return "-360°00'";

		case LONG_DEC_TYPE:
			return "-90°00'00\"";

		case LONG_LONG_TYPE:
			return "-180°00'00\"";

		case SHORT_TYPE :
		case LONG_TYPE :
		case TIMEZONE_TYPE :
			temp.Format(CString("%02.0f:00"), fDesc->LoLimit);
			return temp;

		case BOOL_TYPE :
			return "0";
		}
	}
	return "Lo Limit";
}

/////////////////////////////////////////////
//
//	Name		:GetKey
//
//	Description :Returns the key which is usually the same as the name
//
//  Input		:None
//
//	Output		:Key
//
////////////////////////////////////////////
CString CBodyData::GetKey(bool replaceCommas)
{
	if (replaceCommas)
	{
		CString temp = m_key;
		temp.Replace(',',';');
		return temp;
	}
	else
		return m_key;
}

/////////////////////////////////////////////
//
//	Name		:SetKey
//
//	Description :Sets the Key value
//
//  Input		:key
//
//	Output		:None
//
////////////////////////////////////////////
void CBodyData::SetKey(CString key)
{
	// count the number of characters
	if (key.GetLength() < 16)
	{
		// if less than 16, space fill to the right
		CString spaces16 = "                ";
		key += spaces16.Left(16 - key.GetLength());
	}
	m_key = key;
}


/////////////////////////////////////////////
//
//	Name		:Compare
//
//	Description :Compares the data passed to it in the fields passed to it.
//
//  Input		:data - comparison data
//				 Asc  - Ascending flag True = Ascending
//				 Field1 - Key 
//				 Field2, 3 not implmented yet
//
//	Output		:True if Asc is true and this is less than data
//
////////////////////////////////////////////
bool CBodyData::Compare(CBodyData *data, bool Asc, int field1, int field2, int field3)
{
sfieldDesc	*fDesc, *dDesc;	
CString		S1, S2, S1r, S2r;
int			N1, N2;
	
	// check body type and get the field descriptor
	if (m_BodyType == data->GetBodyType() && (fDesc = GetFieldDesc(field1)) != NULL)
	{
	// get the compare data descriptor
		dDesc = data->GetFieldDesc(field1); // it must be ok if the above worked

	// do it based on type
		switch (fDesc->Type)
		{
		case KEY_TYPE :
		// get the strings
			S1r = S1 = GetFieldData(field1);
			S2r = S2 = data->GetFieldData(field1);

			bool chk1, chk2;

			// split the keys into numbers and strings
			chk1 = NumStr(S2, N2);
			chk2 = NumStr(S1, N1);

		// if the beginning of both the strings are numbers
		// then compare them by the numbers then the strings
			if (chk1 && chk2)
				return Asc ? N1 < N2 || (N1 == N2 && S1 < S2) || (N1 == N2 && S1 == S2 && S1r < S2r) :
						     N1 > N2 || (N1 == N2 && S1 > S2) || (N1 == N2 && S1 == S2 && S1r > S2r) ;

		// else if either one starts with numbers then compare the full strings
			else if (chk1 || chk2)
				return Asc ? S1r < S2r : S1r > S2r;


		// else compare the strings then the numbers
			else return Asc ? S1 < S2 || (S1 == S2 && N1 < N2) || (S1 == S2 && N1 == N2 && S1r < S2r): 
							 S1 > S2 || (S1 == S2 && N1 > N2) || (S1 == S2 && N1 == N2 && S1r > S2r) ;

		case DATE_TYPE :
			if (Asc)
				return	fDesc->DatePtr->Epoch_yy < dDesc->DatePtr->Epoch_yy ||

						(fDesc->DatePtr->Epoch_yy == dDesc->DatePtr->Epoch_yy &&
						fDesc->DatePtr->Epoch_mm < dDesc->DatePtr->Epoch_mm) ||

						(fDesc->DatePtr->Epoch_yy == dDesc->DatePtr->Epoch_yy &&
						fDesc->DatePtr->Epoch_mm == dDesc->DatePtr->Epoch_mm &&
						fDesc->DatePtr->Epoch_dd < dDesc->DatePtr->Epoch_dd) ||

						(fDesc->DatePtr->Epoch_yy == dDesc->DatePtr->Epoch_yy &&
						fDesc->DatePtr->Epoch_mm == dDesc->DatePtr->Epoch_mm &&
						fDesc->DatePtr->Epoch_dd == dDesc->DatePtr->Epoch_dd &&
						GetKey() < data->GetKey());

			else
				return	fDesc->DatePtr->Epoch_yy > dDesc->DatePtr->Epoch_yy ||

						(fDesc->DatePtr->Epoch_yy == dDesc->DatePtr->Epoch_yy &&
						fDesc->DatePtr->Epoch_mm > dDesc->DatePtr->Epoch_mm) ||

						(fDesc->DatePtr->Epoch_yy == dDesc->DatePtr->Epoch_yy &&
						fDesc->DatePtr->Epoch_mm == dDesc->DatePtr->Epoch_mm &&
						fDesc->DatePtr->Epoch_dd > dDesc->DatePtr->Epoch_dd) ||

						(fDesc->DatePtr->Epoch_yy == dDesc->DatePtr->Epoch_yy &&
						fDesc->DatePtr->Epoch_mm == dDesc->DatePtr->Epoch_mm &&
						fDesc->DatePtr->Epoch_dd == dDesc->DatePtr->Epoch_dd &&
						GetKey() > data->GetKey());
						

		case JDATE_TYPE :
			if (Asc)
				return	fDesc->JDatePtr->EpochYear < dDesc->JDatePtr->EpochYear ||

						(fDesc->JDatePtr->EpochYear == dDesc->JDatePtr->EpochYear &&
						fDesc->JDatePtr->EpochDay  < dDesc->JDatePtr->EpochDay) ||

						(fDesc->JDatePtr->EpochYear == dDesc->JDatePtr->EpochYear &&
						fDesc->JDatePtr->EpochDay  == dDesc->JDatePtr->EpochDay &&
						GetKey() < data->GetKey());

			else
				return	fDesc->JDatePtr->EpochYear > dDesc->JDatePtr->EpochYear ||

						(fDesc->JDatePtr->EpochYear == dDesc->JDatePtr->EpochYear &&
						fDesc->JDatePtr->EpochDay  > dDesc->JDatePtr->EpochDay) ||

						(fDesc->JDatePtr->EpochYear == dDesc->JDatePtr->EpochYear &&
						fDesc->JDatePtr->EpochDay  == dDesc->JDatePtr->EpochDay &&
						GetKey() > data->GetKey());

		case FLOAT_TYPE :
			if (Asc)
				return *fDesc->FloatPtr < *dDesc->FloatPtr ||
						(*fDesc->FloatPtr == *dDesc->FloatPtr &&
						GetKey() < data->GetKey());
			else
				return *fDesc->FloatPtr > *dDesc->FloatPtr ||
						(*fDesc->FloatPtr == *dDesc->FloatPtr &&
						GetKey() > data->GetKey());

		case RA_TYPE :
		case DEC_TYPE :
		case AZ_TYPE :
		case SHORT_TYPE :
			if (Asc)
				return *fDesc->ShortPtr < *dDesc->ShortPtr ||
						(*fDesc->ShortPtr == *dDesc->ShortPtr &&
						GetKey() < data->GetKey());
			else
				return *fDesc->ShortPtr > *dDesc->ShortPtr ||
						(*fDesc->ShortPtr == *dDesc->ShortPtr &&
						GetKey() > data->GetKey());

		case DOUBLE_TYPE :
		case PREC_RA_TYPE :
		case PREC_DEC_TYPE :
			if (Asc)
				return *fDesc->DoublePtr < *dDesc->DoublePtr ||
						(*fDesc->DoublePtr == *dDesc->DoublePtr &&
						GetKey() < data->GetKey());
			else
				return *fDesc->DoublePtr > *dDesc->DoublePtr ||
						(*fDesc->DoublePtr == *dDesc->DoublePtr &&
						GetKey() > data->GetKey());

		case LONG_TYPE :
		case LONG_RA_TYPE:
		case LONG_DEC_TYPE:
		case LONG_LONG_TYPE:
			if (Asc)
				return *fDesc->LongPtr < *dDesc->LongPtr ||
						(*fDesc->LongPtr == *dDesc->LongPtr &&
						GetKey() < data->GetKey());
			else
				return *fDesc->LongPtr > *dDesc->LongPtr ||
						(*fDesc->LongPtr == *dDesc->LongPtr &&
						GetKey() > data->GetKey());

		case STRING_TYPE :
			if (Asc)
				return *fDesc->StringPtr < *dDesc->StringPtr ||
						(*fDesc->StringPtr == *dDesc->StringPtr &&
						GetKey() < data->GetKey());
			else
				return *fDesc->StringPtr > *dDesc->StringPtr ||
						(*fDesc->StringPtr == *dDesc->StringPtr &&
						GetKey() > data->GetKey());

		case TIMEZONE_TYPE :
			if (Asc)
				return *fDesc->CharPtr < *dDesc->CharPtr ||
						(*fDesc->CharPtr == *dDesc->CharPtr &&
						GetKey() < data->GetKey());
			else
				return *fDesc->CharPtr > *dDesc->CharPtr ||
						(*fDesc->CharPtr == *dDesc->CharPtr &&
						GetKey() > data->GetKey());

		case BOOL_TYPE :
			if (Asc)
				return *fDesc->BoolPtr < *dDesc->BoolPtr ||
						(*fDesc->BoolPtr == *dDesc->BoolPtr &&
						GetKey() < data->GetKey());
			else
				return *fDesc->BoolPtr > *dDesc->BoolPtr ||
						(*fDesc->BoolPtr == *dDesc->BoolPtr &&
						GetKey() > data->GetKey());
		}
	}
	return false;

}

/////////////////////////////////////////////
//
//	Name		:NumStr
//
//	Description :This function will check the beginning of the string
//				 for digits. If found it will convert all the digits found at
//				 the beginning of the string to an int and assign it to
//				 Num and return a true. If the first non blank character is
//				 not a digit it will search for a digit and convert the first
//				 group of digits to Num it will also assign the first group
//               of non digits to Str and return false.
//
//  Input		:Str, Num reference
//
//	Output		:true - if number found at start of string
//				 false - if number or no number at end of string
//				 Num = any number found either in front or back
//				 Str = Left most non digit part of string
//
////////////////////////////////////////////
bool CBodyData::NumStr(CString &Str, int &Num)
{
	LPTSTR tst = Str.GetBuffer(16);
	unsigned int i=0;

	// find the first non blank character
	while(i<strlen(tst) && tst[i] == ' ')
		i++;

	// return false if we went beyond the end or the first non blank character is not a digit
	if (i >= strlen(tst) || !isdigit(tst[i]))
	{
		// search for any digits
		while(i<strlen(tst) && !isdigit(tst[i]))
			i++;

		// scan the digits and put them in Num
		sscanf(&tst[i], "%d", &Num);

		// return back just the string portion
		Str = Str.Left(i-1);
		return false;
	}

	// scan the digits and put them in Num
	sscanf(&tst[i], "%d", &Num);

	// tell them what we did
	return true;

}

/////////////////////////////////////////////
//
//	Name		:ConvertPoolImage
//
//	Description :This will convert a pool position in memory image
//				 to a poolposition struct.
//
//  Input		:image pointer
//
//	Output		:poolposition
//
////////////////////////////////////////////
poolposition CBodyData::ConvertPoolImage(unsigned char *image)
{
poolposition pool;

	pool.page = *image;		// this is the easy part
	pool.offset = ConvertWordImage(&image[1]);	// from motorola to intel

	return pool;
}


/////////////////////////////////////////////
//
//	Name		:ConvertFloatImage
//
//	Description :This will convert a float in memory image
//				 to an intel float.
//
//  Input		:char *ptr
//
//	Output		:float
//
////////////////////////////////////////////
float CBodyData::ConvertFloatImage(unsigned char *ptr)
{
float conv = 0.0;
unsigned char *floatPtr = (unsigned char *)&conv;

// reverse the order of the bytes
	for (int i = 3; i>=0; i--)	
		*floatPtr++ = ptr[i];
	
	return conv;
}

/////////////////////////////////////////////
//
//	Name		:ConvertWordImage
//
//	Description :This will convert a word in memory image
//				 to an intel word.
//
//  Input		:char *image
//
//	Output		:word
//
////////////////////////////////////////////
word CBodyData::ConvertWordImage(unsigned char *image)
{
	return ((unsigned char)image[0] << 8) + (unsigned char)image[1];	// from motorola to intel

}

/////////////////////////////////////////////
//
//	Name		:ConvertLongImage
//
//	Description :This will convert a long in memory image
//				 to an intel long.
//
//  Input		:char *image
//
//	Output		:long
//
////////////////////////////////////////////
long CBodyData::ConvertLongImage(unsigned char *image)
{
	return ((unsigned char)image[0] << 24) + ((unsigned char)image[1] << 16)
		   + ((unsigned char)image[2] << 8) + ((unsigned char)image[3]);
}

/////////////////////////////////////////////
//
//	Name		:ConvertMediumImage
//
//	Description :This will convert a medium in memory image
//				 to an intel long.
//
//  Input		:char *image
//
//	Output		:long
//
////////////////////////////////////////////
long CBodyData::ConvertMediumImage(unsigned char *image)
{
	long temp;

	int hb;
	if (((unsigned char)image[0] >> 7) == 1)
		hb = 0xFF;
	else
		hb = 0;
	temp = (unsigned char) image[2] + ((unsigned char)image[1] << 8) + ((unsigned char)image[0] << 16) + (hb << 24);

	return temp;
}

/////////////////////////////////////////////
//
//	Name		:PutPoolImage
//
//	Description :puts the passed pool in to the Autostar memory image at
//				 the pointer.
//
//  Input		:pool, image pointer
//
//	Output		:None
//
////////////////////////////////////////////
void CBodyData::PutPoolImage(poolposition pool, unsigned char *ptr)
{
	*ptr = pool.page;		// this is the easy part
	PutWordImage(pool.offset, &ptr[1]);	// from intel to motorola

}

/////////////////////////////////////////////
//
//	Name		:PutWordImage
//
//	Description :puts the passed word in to the Autostar memory image at
//				 the pointer.
//
//  Input		:word, image pointer
//
//	Output		:None
//
////////////////////////////////////////////
void CBodyData::PutWordImage(word data, unsigned char *ptr)
{
	ptr[0] = (data & 0xFF00) >> 8;
	ptr[1] = data & 0xFF;
}

/////////////////////////////////////////////
//
//	Name		:PutFloatImage
//
//	Description :puts the passed float in to the Autostar memory image at
//				 the pointer.
//
//  Input		:float, image pointer
//
//	Output		:None
//
////////////////////////////////////////////
void CBodyData::PutFloatImage(float data, unsigned char *ptr)
{
float conv = 0.0;
unsigned char *floatPtr = (unsigned char *)&data;

// reverse the order of the bytes
	for (int i = 3; i>=0; i--)	
		ptr[i] = *floatPtr++;
	
}


/////////////////////////////////////////////
//
//	Name		:PutDoubleImage
//
//	Description :puts the passed Double in to the Autostar memory image at
//				 the pointer.
//
//  Input		:double, image pointer
//
//	Output		:None
//
////////////////////////////////////////////
void CBodyData::PutDoubleImage(double data, unsigned char *ptr)
{
double conv = 0.0;
unsigned char *doublePtr = (unsigned char *)&data;

// reverse the order of the bytes
	for (int i = sizeof(double) - 1; i>=0; i--)	
		ptr[i] = *doublePtr++;
	
}

/////////////////////////////////////////////
//
//	Name		:PutLongImage
//
//	Description :puts the passed Long in to the Autostar memory image at
//				 the pointer.
//
//  Input		:long, image pointer
//
//	Output		:None
//
////////////////////////////////////////////
void CBodyData::PutLongImage(unsigned long data, unsigned char *ptr)
{
long conv = 0;
unsigned char *longPtr = (unsigned char *)&data;

// reverse the order of the bytes
	for (int i = sizeof(long) - 1; i>=0; i--)	
		ptr[i] = *longPtr++;
	

}

/////////////////////////////////////////////
//
//	Name		:PutMediumImage
//
//	Description :puts the passed long in to the Autostar memory image at
//				 the pointer.
//
//  Input		:long, image pointer
//
//	Output		:None
//
////////////////////////////////////////////
void CBodyData::PutMediumImage(long data, unsigned char *ptr)
{
	unsigned char *mediumPtr = (unsigned char *)&data;

	ptr[0] = mediumPtr[2];
	ptr[1] = mediumPtr[1];
	ptr[2] = mediumPtr[0];

// reverse the order of the bytes
//	for (int i = sizeof(long) - 1; i>=0; i--)	
//		ptr[i] = *mediumPtr++;
	

}


/////////////////////////////////////////////
//
//	Name		:CheckFieldRanges
//
//	Description :Check all the fields for range errors
//
//  Input		:None
//
//	Output		:0 - no range error
//				 n - First field with range error
//
////////////////////////////////////////////
int CBodyData::CheckFieldRanges()
{
	sfieldDesc		*desc;
	for (int i = 0; i < GetNumFields(); i++)
	{
		desc = GetFieldDesc(i);
		if (FieldOutofRange(desc))
			return i;
	}
	return 0;
}


/////////////////////////////////////////////
//
//	Name		:FieldOutofRange
//
//	Description :Checks the passed Field descriptor and ptr
//				 for range error. If ptr is NULL then the 
//				 member data is checked
//
//  Input		:Descriptor pointer, data pointer
//
//	Output		:1 - out of range
//
////////////////////////////////////////////
bool CBodyData::FieldOutofRange(sfieldDesc *desc, void *ptr)
{
	date*	dPtr;
	jdate*	jPtr;
	float*	fPtr;
	short*	sPtr;
	double*	duPtr;
	long int *lPtr;
	char*	cPtr;

		switch (desc->Type)
		{
		case STRING_TYPE :
		case  KEY_TYPE :
		case BOOL_TYPE :
			return false;
			
		case DATE_TYPE :

			if (ptr == NULL)
				dPtr = desc->DatePtr;
			else
				dPtr = (date *)ptr;

			if (dPtr->Epoch_dd > 31 || dPtr->Epoch_dd < 1 ||
				(dPtr->Epoch_mm & 0x0F) > 12 || (dPtr->Epoch_mm & 0x0F) < 1 ||
				dPtr->Epoch_yy > 3000 || dPtr->Epoch_yy < 1900)
				return true;
			else
				return false;

		case JDATE_TYPE:
			if (ptr == NULL)
				jPtr = desc->JDatePtr;
			else
				jPtr = (jdate *)ptr;

			if (jPtr->EpochDay < 0.0 || jPtr->EpochDay > 365.3 ||
				jPtr->EpochYear < 1900 || jPtr->EpochYear > 3000)
				return true;
			else
				return false;

		case FLOAT_TYPE :
			if (ptr == NULL)
				fPtr = desc->FloatPtr;
			else
				fPtr = (float *)ptr;

			if (*fPtr < desc->LoLimit || *fPtr > desc->HiLimit)
				return true;
			else
				return false;

		case RA_TYPE :
			if (ptr == NULL)
				sPtr = desc->ShortPtr;
			else
				sPtr = (short *)ptr;

			if (*sPtr < desc->LoLimit || *sPtr > desc->HiLimit)
				return true;
			else
				return false;
			
		case DEC_TYPE :
		case AZ_TYPE :
			if (ptr == NULL)
				sPtr = desc->ShortPtr;
			else
				sPtr = (short *)ptr;

			if (*sPtr < desc->LoLimit || *sPtr > desc->HiLimit)
				return true;
			else
				return false;

		case SHORT_TYPE :
			if (ptr == NULL)
				sPtr = desc->ShortPtr;
			else
				sPtr = (short *)ptr;

			if (*sPtr > desc->HiLimit || *sPtr < desc->LoLimit)
				return true;
			else
				return false;

		case TIMEZONE_TYPE :
			if (ptr == NULL)
				cPtr = desc->CharPtr;
			else
				cPtr = (char *)ptr;

			if (*cPtr > 48 || *cPtr < 0)
				return true;
			else
				return false;

		case DOUBLE_TYPE :
		case PREC_RA_TYPE :
		case PREC_DEC_TYPE :
			if (ptr == NULL)
				duPtr = desc->DoublePtr;
			else
				duPtr = (double *)ptr;

			if (*duPtr > desc->HiLimit || *duPtr < desc->LoLimit)
				return true;
			else
				return false;

		case LONG_TYPE :
		case LONG_RA_TYPE:
		case LONG_DEC_TYPE:
		case LONG_LONG_TYPE:
			if (ptr == NULL)
				lPtr = desc->LongPtr;
			else
				lPtr = (long int *)ptr;

			if (*lPtr > desc->HiLimit || *lPtr < desc->LoLimit)
				return true;
			else
				return false;


		}

		return true;
}

/////////////////////////////////////////////
//
//	Name		:ReadRomData
//
//	Description :Converts char ptr to Body data and converts the name
//				 to the key. This is used to import old Lib files.
//
//  Input		:pointer to memory image
//
//	Output		:ok
//
////////////////////////////////////////////
bool CBodyData::ReadRomData(CPersist &per)
{
	

// get the structure from the pointer
	ReadData((unsigned char *)per.m_indexPtr);

// increment to the next body
	per.IncrementIndex(GetSizeOf());

	return true;
}


int * CBodyData::GetImportTable()
{
	return NULL;
}

/////////////////////////////////////////////
//
//	Name		:ReadTxtData
//
//	Description :This will read the txt data of a body type that supports
//				 the import table.
//
//  Input		:Persistance object
//
//	Output		:true - good
//				 false - bad
//
////////////////////////////////////////////
bool CBodyData::ReadTxtData(CPersist& per)
{
int			*iTbl;
int			len;
CString		sTmp;
RANGESTAT	tStat, rStat = OK;


	if ((iTbl = GetImportTable()) != NULL)
	{
		int i = 0;
		while ( iTbl[i] != -1 )
		{
		// parse this field
			len = ParseField(per, i, iTbl);

			if (len > 0)
			{
			// separate this piece	
				sTmp = CString((char *)per.m_indexPtr, len);

			// if this field is used then run it through
				if (iTbl[i] < GetNumFields() && (tStat = SetFieldData(iTbl[i], sTmp, true)) != OK)
					rStat = tStat;

			}
			// fail if we ran to the end of the file
			else if(per.m_dataIndex == per.m_dataReadCnt)
				return false;

		// increment to the next field
			per.IncrementIndex(len);
			i++;
		}
	}

	if (rStat != OK)
		return false;
	else
		return true;

}

/////////////////////////////////////////////
//
//	Name		:ParseField
//
//	Description :This will move the persistance index past any delimiters
//				 and then find the length of the next field and return it.
//
//  Input		:Persistance, Input index, Input table pointer
//
//	Output		:Length of parsed field
//
////////////////////////////////////////////
int CBodyData::ParseField(CPersist &per, int indx, int *iTbl)
{
int		len = 0;

	if (GetBodyType() == Hip || GetBodyType() == HipXref)
	{
		// search past all delimiters except space and set this as the start
		while ((*((char *)per.m_indexPtr) == '|' || 
			*((char *)per.m_indexPtr) == '\n' ||
			*((char *)per.m_indexPtr) == '\r') 
			&& per.m_dataIndex < per.m_dataReadCnt)
			per.IncrementIndex(1);
	}
	else
	{
		// search past all delimiters and set this as the start
		while ((*((char *)per.m_indexPtr) == '|' || 
			*((char *)per.m_indexPtr) == ' ' ||
			*((char *)per.m_indexPtr) == ',' ||
			*((char *)per.m_indexPtr) == '\n' ||
			*((char *)per.m_indexPtr) == '\r' || 
			*((char *)per.m_indexPtr) == '\t')
			&& per.m_dataIndex < per.m_dataReadCnt)
			per.IncrementIndex(1);
	}
	
	// if this is not a skipped field then check if we are looking for a string or a date
	if (iTbl[indx] < GetNumFields() && GetFieldDesc(iTbl[indx])->Type == KEY_TYPE)
	{
		if (GetBodyType() == Satellite)
		{
			// this is a satellite so search for the end of the line ONLY
			while (*(((char *)per.m_indexPtr) + len) != '\r' &&
				*(((char *)per.m_indexPtr) + len) != '\n' &&
				per.m_dataIndex + len < per.m_dataReadCnt)
				len++;
		}
		else
		{
			// search for any delimiter except spaces 
			while (*(((char *)per.m_indexPtr) + len) != ',' && 
				*(((char *)per.m_indexPtr) + len) != '.' &&
				*(((char *)per.m_indexPtr) + len) != '|' &&
				*(((char *)per.m_indexPtr) + len) != '\r' &&
				*(((char *)per.m_indexPtr) + len) != '\n' &&
				*(((char *)per.m_indexPtr) + len) != '\t' &&
				per.m_dataIndex + len < per.m_dataReadCnt)
				len++;
		}

	} // if this is a Comet or Asteroid import then we know we have pipes
	else if (GetBodyType() == Comet || GetBodyType() == Asteroid || GetBodyType() == Hip || GetBodyType() == HipXref) 
	{
		// search for any delimiter except spaces and period
		while (*(((char *)per.m_indexPtr) + len) != '|' &&
			*(((char *)per.m_indexPtr) + len) != '\r' &&
			*(((char *)per.m_indexPtr) + len) != '\n' &&
			per.m_dataIndex + len < per.m_dataReadCnt)
			len++;
	
	}
	else
	{
		// search for any delimiter except period
		while (*(((char *)per.m_indexPtr) + len) != ',' && 
			*(((char *)per.m_indexPtr) + len) != ' ' &&
			*(((char *)per.m_indexPtr) + len) != '|' &&
			*(((char *)per.m_indexPtr) + len) != '\r' &&
			*(((char *)per.m_indexPtr) + len) != '\n' &&
			*(((char *)per.m_indexPtr) + len) != '\t' &&
			per.m_dataIndex + len < per.m_dataReadCnt)
			len++;
	}

	return len;

}

/////////////////////////////////////////////
//
//	Name		: ParseToEOL
//
//	Description : Will skip to the next CR or LF character found.
//
//  Input		: CPersist
//
//	Output		: None
//
////////////////////////////////////////////
void CBodyData::ParseToEOL(CPersist &per)
{
	// search for CR or LF
	while ((*((char *)per.m_indexPtr) != '\n' &&
		*((char *)per.m_indexPtr) != '\r') 
		&& per.m_dataIndex < per.m_dataReadCnt)
		per.IncrementIndex(1);

	// find the first character after CR/LF
	while ((*((char *)per.m_indexPtr) == '\n' ||
		*((char *)per.m_indexPtr) == '\r') 
		&& per.m_dataIndex < per.m_dataReadCnt)
		per.IncrementIndex(1);

}

/////////////////////////////////////////////
//
//	Name		: ParseBackEOL
//
//	Description : Will skip back to the previous CR or LF character found.
//
//  Input		: CPersist
//
//	Output		: None
//
////////////////////////////////////////////
void CBodyData::ParseBackEOL(CPersist &per)
{
	// go back one first
	per.IncrementIndex(-1);

	// search backwards for CR or LF
	while ((*((char *)per.m_indexPtr) != '\n' &&
		*((char *)per.m_indexPtr) != '\r') 
		&& per.m_dataIndex > 0)
		per.IncrementIndex(-1);


}

/////////////////////////////////////////////
//
//	Name		:GetActiveFlag
//
//	Description :Returns the state of the active flag;
//
//  Input		:None
//
//	Output		:active flag
//
////////////////////////////////////////////
bool CBodyData::GetActiveFlag()
{
	return true;
}

/////////////////////////////////////////////
//
//	Name		:SetActiveFlag
//
//	Description :Sets the state of the active flag
//
//  Input		:flag
//
//	Output		:None
//
////////////////////////////////////////////
void CBodyData::SetActiveFlag(bool flag)
{
}

/////////////////////////////////////////////
//
//	Name		:GetPosition
//
//	Description :returns the pool position for this HipXref
//
//  Input		:None
//
//	Output		:poolposition of this Body
//
////////////////////////////////////////////
const poolposition & CBodyData::GetPosition()
{
	static poolposition	temp;
	return temp;
}

/////////////////////////////////////////////
//
//	Name		:SetPosition
//
//	Description :Sets the poolposition for this Body
//
//  Input		:poolposition
//
//	Output		:None
//
////////////////////////////////////////////
void CBodyData::SetPosition(const poolposition &position)
{
}

/////////////////////////////////////////////
//
//	Name		:ReadData
//
//	Description :Converts char ptr to Hip data and converts the name
//				 to the key. This is used by import of the old Lib files.
//
//  Input		:pointer to memory image
//
//	Output		:None
//
////////////////////////////////////////////
void CBodyData::ReadData(unsigned char *ptr)
{
}


/////////////////////////////////////////////
//
//	Name		:GetFieldDataDirect
//
//	Description :returns a pointer directly to the data
//
//  Input		:field
//
//	Output		:void pointer
//
////////////////////////////////////////////
void * CBodyData::GetFieldDataDirect(int field)
{
	sfieldDesc *fdt = GetFieldDesc(field);
	
	switch(fdt->Type)
	{
		case STRING_TYPE :
			return (void *)fdt->StringPtr;

		case KEY_TYPE :
			return (void *)&m_key;

		case DATE_TYPE :
			return (void *)fdt->DatePtr;

		case JDATE_TYPE :
			return (void *)fdt->JDatePtr;

		case PREC_DEC_TYPE :
		case PREC_RA_TYPE :
		case DOUBLE_TYPE :
			return (void *)fdt->DoublePtr;

		case FLOAT_TYPE :
			return (void *)fdt->FloatPtr;

		case SHORT_TYPE :
		case DEC_TYPE :
		case AZ_TYPE :
		case RA_TYPE :
			return (void *)fdt->ShortPtr;

		case LONG_TYPE :
		case LONG_RA_TYPE:
		case LONG_DEC_TYPE:
		case LONG_LONG_TYPE:
			return (void *)fdt->LongPtr;

		case BOOL_TYPE :
			return (void *)fdt->BoolPtr;

		case TIMEZONE_TYPE :
			return (void *)fdt->CharPtr;

		}
	return NULL;

}



/////////////////////////////////////////////
//
//	Name		:IsDynamic
//
//	Description :returns true if data length is dynamic
//
//  Input		:none
//
//	Output		:true - dynamic
//				 false - fixed length
//
////////////////////////////////////////////
bool CBodyData::IsDynamic()
{
	return false;	// fixed by default
}


/////////////////////////////////////////////
//
//	Name		:IsExtended
//
//	Description :returns true if features are only supported on ASII
//
//  Input		:none
//
//	Output		:true - Extended features
//				 false - standard features
//
////////////////////////////////////////////
bool CBodyData::IsExtended()
{
	return false; // assume object uses only standard features as default
}

/////////////////////////////////////////////
//
//	Name		:IsCustom
//
//	Description :returns true if features are only supported on ASII
//
//  Input		:none
//
//	Output		:true - custom user objects
//				 false - standard objects
//
////////////////////////////////////////////
bool CBodyData::IsCustom()
{
	return false; // assume object uses only standard features as default
}



