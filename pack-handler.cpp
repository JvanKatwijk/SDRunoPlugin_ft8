#
/*
 *    Copyright (C) 2022
 *    J van Katwijk
 *    Lazy Chair Computing
 *
 *    Part of the algorithms are copied or derived from
 *    Karlis Goba
 *    YL3JG
 *
 *    This file is part of the ft8 plugin
 *
 *    ft8 plugin is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    ft8 plugin is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with ft8 plugin; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include	"pack-handler.h"
#include	<stdio.h>
#include	<sstream>
#include	<format>
//	Table 7 from "The FT4 and FT8 Communication Protocols"
//	Authors Sreve Franke, Bill Somerville and Joe Taylor"
//	reads
//	DE			0
//	QRZ			1
//	CQ			2
//	CQ 000 - CQ 999		3 .. 1002
//	CQ A - CQ Z		1004 .. 1029
//	CQ AA - CQ ZZ		1031 .. 1731
//	CQ AAA - CQ ZZZ		1760 .. 20685
//	CQ AAAA - CQ ZZZZ	21443 .. 532443
//	22-bit hash codes	2063592 + (0 .. 41943030
//	Standard call signs	6257896 + (0 .. 2684354550

#define	CQ_3DIGITS	3
#define	CQ_3DIGITS_E	1002
#define	CQ_1LETTER	1004
#define	CQ_1LETTER_E	1029
#define	CQ_2LETTER	1031
#define	CQ_2LETTER_E	1731
#define	CQ_3LETTER	1760
#define	CQ_3LETTER_E	20685
#define	CQ_4LETTER	21443
#define	CQ_4LETTER_E	532443
#define HASH_START	((uint32_t)2063592L)
#define HASH_END	(HASH_START + 4194304L)

static inline
uint16_t	getBits (const uint8_t *d,
	                 int32_t offset, int16_t amount) {
int16_t	res	= 0;

	for (int i = 0; i < amount; i ++) {
	   res <<= 1;
	   res |= (d [offset + i] & 01);
	}
	return res;
}

static inline
uint32_t	getLBits (const uint8_t *d,
	                 int32_t offset, int16_t amount) {
uint32_t	res	= 0;

	for (int i = 0; i < amount; i ++) {
	   res <<= 1;
	   res |= (d [offset + i] & 01);
	}
	return res;
}

static inline
uint64_t	getLLBits (const uint8_t *d,
	                   int32_t offset, int16_t amount) {
uint64_t	res	= 0;

	for (int i = 0; i < amount; i ++) {
	   res <<= 1;
	   res |= (d [offset + i] & 01);
	}
	return res;
}

static inline 
std::string trim (std::string str) {
const char* typeOfWhitespaces = " \t";
	str. erase (str. find_last_not_of (typeOfWhitespaces) + 1);
	str. erase (0, str. find_first_not_of (typeOfWhitespaces));
	return str;
}
// convert integer index to ASCII character according to one of 6 tables:
const char *table_0 = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ+-./?";
const char *table_1 = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *table_2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *table_3 = "0123456789";
const char *table_4 = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char *table_5 = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ/";
static
char	charn (int c, int table_idx) {
const char *theTable	= nullptr;
	switch (table_idx) {
	   case 0:
	      theTable = table_0;
	      break;
	   case 1:
	      theTable = table_1;
	      break;
	   case 2:
	      theTable = table_2;
	      break;
	   case 3:
	      theTable = table_3;
	      break;
	   case 4:
	      theTable = table_4;
	      break;
	   case 5:
	      theTable = table_5;
	      break;
	   default:
	      return '?';
	}
	if (theTable == nullptr)
	   return '_';		// cannot happen
	else
	if ((0 <= c) && (c <= strlen (theTable)))
	   return theTable [c];
	else
	   return '?';		// should not happen
}

	packHandler::packHandler 	():
	                                 the_hashHandler ("/tmp/xxx") {
}
	packHandler::~packHandler	() {}

void	packHandler::pack_bits (const uint8_t *input,
	                        int nrBits, uint8_t *out){
static
uint8_t bits [] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
int index	= 0;

	out [0] = 0;
	for (int i = 0; i < nrBits; i ++) {
	   if ((i > 0) && (i % 8 == 0)) {
	      index += 1;
	      out [index] = 0;
	   }
	   if (input [i] == 1)
	      out [index] |=  bits [i % 8];
	}
}

std::string	packHandler::unpackMessage (const uint8_t* m_in, bool &is_CQ) {
std::string	result;

	uint8_t i3 = getBits (m_in, 74, 3);
	switch (i3) {
	   case 0:	// dispatch further
	      return handle_type0 (m_in, getBits (m_in, 71, 3));

	   case 1:	// c28 r1 c28 r1 R1 g15
	      result = handle_type1 (m_in, i3, is_CQ);
              return result;

	   case 2:	// c28 p1 c28 p1 R1 g15
	      result = handle_type1 (m_in, i3, is_CQ);
              return result;

	   case 3:	// t1 c28 c28 R1 r3 s13
	      return handle_type3 (m_in);

	   case 4:	// h12 c58 h1 r2 c1
	      return handle_type4 (m_in);
	
	   case 5:	// h12 h22 R1 r3 s11 g25
 	      return handle_type5 (m_in);

	   default:
	      return "";	// should not happen
	}

	return result;	// cannot happen
}

std::string	packHandler::handle_type0 (const uint8_t *m_in, int n3) {
	switch (n3) {
	   case 0: 	//  t71
	      return handle_type00 (m_in);

	   case 1:	// c28 c28 h10 r5 
	      return handle_type01 (m_in);

	   case 2:	//  nothing
	      return "";

	   case 3:	// c28 c28 R1 n4 k3 S7
 	      return handle_type03 (m_in);

	   case 4:	// c28 c28 R1 n4 k3 S7
	      return handle_type04 (m_in);

	   case 5:	// t71
	      return handle_type05 (m_in);

	   default:	// should not happen
	      return "";
	}
	return "";	// cannot happen
}

//	handle type 1, i.e. "standard" messages
//	handles both type1 and type2 messages
//	c28 r1 c28 r1 R1 g15:	K1ABC/R PA0JAN/R EN35
std::string	packHandler::handle_type1 (const uint8_t *m_in,
	                                    uint8_t i3, bool &is_CQ) {
uint32_t c28a, c28b;
uint16_t g15;
uint8_t R1;
uint8_t r1, r2;
std::string	result = "type 1/2: ";

	c28a	= getLBits (m_in,  0, 28);	// callsign
	r1	= getBits  (m_in, 28,  1);	// /R or /P
	c28b	= getLBits (m_in, 29, 28);	// callsign
	r2	= getBits  (m_in, 57,  1);	// /R or  /P
	R1	= getBits  (m_in, 58,  1);	// R1
	g15	= getBits  (m_in, 59, 15); 	// g15

//	Unpack both callsigns
	std::string c1 = getCallsign (c28a);
	if (c1 == "")
	   return "";
	std::string ss = c1. substr (0, 2);
	if (ss == "CQ")
	   is_CQ = true;
//	Check if we should append /R or /P suffix
	if (r1) {
	   if (i3 == 1) {
	      c1 += "/R";
	   }
	   else
	   if (i3 == 2) {
	      c1 += "/P";
	   }
	}

	std::string c2 = getCallsign (c28b);
	if (c2 == "")
	   return "";
//	Check if we should append /R or /P suffix
	if (r2) {
	   if (i3 == 1) {
	      c2 += "/R";
	   }
	   else
	   if (i3 == 2) {
	      c2 += "/P";
	   }
	}

	result += c1 + " \t" + c2;

	if (g15 > 0) {
	   uint16_t n = g15;
	   char data [5];
	   data [4] = '\0';
	   data [3] = '0' + (n % 10);
	   n /= 10;
	   data [2] = '0' + (n % 10);
	   n /= 10;
	   data [1] = 'A' + (n % 18);
	   n /= 18;
	   data [0] = 'A' + (n % 18);
	   if (R1 > 0)
	      result += "\t R " + std::string (data);
	   else
	      result += "\t" + std::string (data);
	}
	else {
//	Extract report
	   int irpt = g15;
	   std::string res2;
           switch (irpt) {
	      case 1:
	         res2 = "";
	         break;
	      case 2:
	         res2 = "RRR";
	         break;
	      case 3:
	         res2 = "RR73";
	         break;
	      case 4:
	         res2 = "73";
	         break;
	      default:
	         if (R1 > 0) {
		    res2  = "R"; // Add "R" before report
	         }
	         res2 += number_2 (irpt - 35);
	      }
	      result += "\t" + res2;
	}
	return result;
}

//	d28 is a 28-bit integer containing the
//	call sign bits from a packed message.
std::string	packHandler::getCallsign (uint32_t d28) {
std::string res;
static
const char * t1 [] = {"DE", "QRZ", "CQ"};

//	Check for special tokens DE, QRZ, CQ, CQ_nnn, CQ_xxxx
	if (d28 < HASH_START) {
	   if (d28 < 3) 
	      return t1 [d28];

	   if (d28 <= CQ_3DIGITS_E) {
//	CQ_ddd with 3 digits
	      return  "CQ " + number_3 (d28 - 3);
	   }
//	CQ_A
	   if (CQ_1LETTER <= d28 && d28 <= CQ_1LETTER_E)
	      return std::string ("CQ") +
	              std::string (1, (charn (d28 - CQ_1LETTER, 4)));
//	CQ_AA
	   if (CQ_2LETTER <= d28 && d28 <= CQ_2LETTER_E)
	      return get_CQcode (d28 - CQ_2LETTER, 2);
//	CQ_AAA
	   if (CQ_3LETTER <= d28 && d28 <= CQ_3LETTER_E)
	      return get_CQcode (d28 - CQ_3LETTER, 3);
//	CQ_AAAA
	   if (CQ_4LETTER <= d28 && d28 <= CQ_4LETTER_E) {
	      return get_CQcode (d28 - CQ_4LETTER, 4);
	   }
	   return "";
	}

	if (d28 < HASH_END) {
           res = the_hashHandler. lookup ((d28 - HASH_START) & 0xFFF);
	   return std::string (res);
	}

//	Standard callsign
	uint32_t n = d28 - HASH_END;

	static int divTable [] = {27, 27, 27, 10, 36, 37};
	static int tabIndex [] = { 4,  4,  4,  3,  2,  1};

	char callsign [7];
	callsign [6] = 0;
	for (int i = 0; i < 6; i ++) {
	   callsign [5 - i] = charn (n % divTable [i], tabIndex [i]);
	   n /= divTable [i];
	}
	
	res = trim (std::string (callsign));
	if (res. size () == 0)
	   return "";

	return res;
}
//	Type 3 messages,RTTY RU t1 c28 c28 R1 r3 s12
//	K1ABC W9XYZ 579 EI
std::string	packHandler::handle_type3 (const uint8_t *m_in) {
uint8_t	 t1	= getBits (m_in,  0,  1);	// TU
uint32_t c28a	= getBits (m_in,  1, 28);	// standard callsign
uint32_t c28b	= getBits (m_in, 29, 28);	// standard callsign
uint8_t  R1	= getBits (m_in, 58,  1);	// R
uint8_t  r3	= getBits (m_in, 59,  3);	// report 2 ..  9, 53 .. 59
uint16_t s13	= getBits (m_in, 62, 13);	// serial number
std::string result = "type 3: ";

//	Unpack both callsigns
	std::string c1 = getCallsign (c28a);
	if (c1 == "")
	   return "";

	std::string c2 = getCallsign (c28b);
	if (c2 == "")
	   return "";
	result += c1 + " \t" + c2;

	std::string S_extra = "R" + std::to_string  (509 + 10 * r3);
	result += "\t" + std::to_string (s13);
	return result;
}

//	Type 4: Nonstandard calls, h12 c58 h1 r2 c1
//	e.g. <WA9XYZ> PJ4/KA1ABC RR73
std::string	packHandler::handle_type4 (const uint8_t* m_in) {
uint32_t h12;
uint8_t r2, c1;
uint8_t	h1;
uint64_t c58;
std::string result = "type 4: ";

	h12	= getBits	(m_in, 0,  12);	// hashed callsign, 12 b
	c58	= getLLBits	(m_in, 12, 58);	// non standard callsign
	h1	= getBits	(m_in, 70,  1); // hashed callsign is second
	r2	= getBits	(m_in, 71,  2);	// RRR, RR73, 73 or blank
	c1	= getBits	(m_in, 73,  1);	// first callsign is cq 

	char c11 [12];
	c11 [11] = '\0';
	for (int i = 0; i < 11; i++) {
	   c11 [10 - i] = charn (c58 % 38, 5);
	   c58 /= 38;
	}

	the_hashHandler. add_hash (h12, c11);
	char call_3 [15];
	std::stringstream sstream;
	sstream << std::hex << h12;
	std::string ss = "<" + sstream. str () + ">";
        strcpy (call_3, ss. c_str ());
	char* call_1 = (h1) ? c11 : call_3;
	char* call_2 = (h1) ? call_3 : c11;

	std::string s2 = std::string (call_2);
	s2 = trim (s2);
	if (c1 == 0) {
           result += std::string (call_1);
	   result = trim (result);
	   result  += " " + s2;
	   if (r2 == 1)
	      result += " " + std::string ("RRR");
	   else
	   if (r2 == 2)
	      result += " " + std::string ("RR73");
	   else
	   if (r2 == 3)
	      result += " " + std::string ("73");
	}
	else {
	   result += std::string ("CQ_");
	   result += " " +  s2;
	}
	return result;
}

//	type 5 EU VHF <G4ABC><PA9XYZ>R 570007 JO22DB
//	h12 h22 R1 r3 s11 g25
std::string	packHandler::handle_type5    (const uint8_t* m_in) {
uint16_t h12	= getBits (m_in, 0, 12);
uint32_t h22	= getBits (m_in, 12, 22);
char buffer1 [256];
char buffer2 [256];
	sprintf (buffer1, "%X", h12);
	sprintf (buffer2, "%X", h22);
	return std::string ("type 5: ") + std::string (buffer1) + " " +
	                              std::string (buffer2);
}

//
//	type 0.0 is a free text, packed as 71 bits
std::string	packHandler::handle_type00 (const uint8_t *m_in) {
uint8_t b9 [9];
uint8_t b72 [72];

//	Shift 71 bits right by 1 bit, so that
//	it's right-aligned in the byte array
	b72 [0] = 0;
	for (int i = 0; i < 71; i ++)
	   b72 [1 + i] = m_in [i];
	pack_bits (b72, 72, b9);

	char c14 [14];
	c14 [13] = 0;
	for (int idx = 12; idx >= 0; --idx) {
//	Divide the long integer in b71 by 42
	   uint16_t rem = 0;
	   for (int i = 0; i < 9; ++i) {
	      rem = (rem << 8) | b9 [i];
	      b9 [i] = rem / 42;
	      rem = rem % 42;
	   }
	   c14 [idx] = charn (rem, 0);
	}
	std::string result = "type 0.0: " + trim (std::string (c14));
	return result;
}

//	Type 0.1 DXpedition c28 c28 h10 r5
//	K1ABC RR73; W9CYZ <KH1/KH7Z> -08
std::string	packHandler::handle_type01 (const uint8_t *m_in) {
uint32_t c28a	= getBits (m_in, 0,  28);
uint32_t c28b	= getBits (m_in, 28, 28);
uint16_t h10	= getBits (m_in, 58, 10);
int16_t r5	= getBits (m_in, 68,  5);

	std::string res1	= getCallsign (c28a);
	std::string res2	= getCallsign (c28b);
	std::string s_h10	= "<....>";
	r5			= (r5 - 8) * 2;
	std::string s_r5	= std::to_string (r5);
	return "type -.1: " + res1 + " \t" + res2 + "\t" + s_h10 + "\t" + s_r5;
}
//	Type 0.3 Field day c28 c28 R1 n4 k3 S7
//	KA1ABC  W9XYZ 6A EI   
std::string	packHandler::handle_type03 (const uint8_t *m_in) {
uint32_t	c28a = getBits (m_in, 0, 28);	// callsign
uint32_t	c28b = getBits (m_in, 28, 28);	// callsign
uint8_t		R1   = getBits (m_in, 56, 1);	// R
uint8_t		n4   = getBits (m_in, 57, 4);	// Number of transmitter
uint8_t		k3   = getBits (m_in, 61, 3);	// Field day class A .. F
uint8_t		S7   = getBits (m_in, 64, 7);	// ARRL/RAC section

	std::string res1	= getCallsign (c28a);	
	std::string res2 	= getCallsign (c28b);
	std::string R		= R1 ? "R" : "";
	std::string s_n4	= std::to_string (n4);
	std::string s_k3	= std::string (1,  'A' + k3);
	std::string s_S7	= std::to_string (S7);
	return  "type 0.3: " + res1 + " \t" + res2 + " \t" + R + s_n4 + "\t" + s_k3 + " " + s_S7;
}
//	Type 0.4 Field day c28 c28 R1 n4 k3 S7
//	W9XYZ KqABC R1 18B EMA
std::string	packHandler::handle_type04 (const uint8_t *m_in) {
	return handle_type3 (m_in);
}
//	Type 0.5	t71
//	0123456789ABCDEF012
std::string	packHandler::handle_type05 (const uint8_t *m_in) {
uint8_t sym	= getBits (m_in, 0, 3);
int	index;
int	teller	= 0;
std::string result;

	result. push_back (table_2 [sym]);
	index = 3;
	while (index < 72) {
	   uint8_t sym = getBits (m_in, index, 4);
	   index += 4;
	   result. push_back (sym > 0 ? table_2 [sym] : 0);
	}
	return "type 0.5: " + result;
}

std::string	packHandler::number_3 (int number) {
std::string res;
int	divider	= 100;

	while (divider > 0) {
	   if (number / divider >= 10)
	      fprintf (stderr, "problem\n");
	   res. push_back (char (number / divider + '0'));
	   number %= divider;
	   divider /= 10;
	}
	return res;
}

std::string	packHandler::number_2 (int number) {
std::string res;

	if (number >= 0)
	   res = "+";
	else {
	   res = "-";
	   number = -number;
	}
	if (number > 100)
	   return "-00";
	res. push_back (char ('0' + number / 10));
	res. push_back (char ('0' + number % 10));
	return res;
}

std::string	packHandler::get_CQcode (uint32_t data, int size) {
char *cq = (char *)_alloca (size + 1);

	cq [size] = 0;
	for (int i = 0; i < size; i ++) {
	   cq [size - 1 - i] = charn (data % 27, 4);
	   data /= 27;
	}

	char *r = cq;
	for (int i = 0; i < size; i ++)
	   if (cq [i] != ' ')
	      break;
	   else
	      r = &cq [i];
	
	return  "CQ_" + std::string (r);
}

std::vector<std::string> packHandler::extractCall (const uint8_t * m_in) {
std::vector<std::string> result;

	uint8_t i3 = getBits (m_in, 74, 3);
	switch (i3) {
	   case 0:	// dispatch further
	      return extract_call_type_0 (m_in);
//
//	apparently: callee first, then caller
	   case 1:	// c28 r1 c28 r1 R1 g15
	   case 2:	// c28 p1 c28 p1 R1 g15
	      return extract_call_type_1 (m_in, i3);

	   case 3:	// t1 c28 c28 R1 r3 s13
	      return extract_call_type_3 (m_in);

	   case 4:	// h12 c58 h1 r2 c1
	      return result;
	
	   case 5:	// h12 h22 R1 r3 s11 g25
 	      return result;

	   default:
	      return result;	// should not happen
	}

	return result;	// cannot happen
}

bool	test_cq (uint32_t k) {
	if (k == 2)
	   return true;
	if ((k >= CQ_3DIGITS) && (k <= CQ_3DIGITS_E))
	   return true;
	if ((k >= CQ_1LETTER) && (k <= CQ_1LETTER_E))
	   return true;
	if ((k >= CQ_2LETTER) && (k <= CQ_2LETTER_E))
	   return true;
	if ((k >= CQ_3LETTER) && (k <= CQ_3LETTER_E))
	   return true;
	if ((k >= CQ_4LETTER) && (k <= CQ_4LETTER_E))
	   return true;
	return false;
}

std::vector<std::string>  packHandler::extract_call_type_0 (const uint8_t *m_in) {
uint32_t c28a;
uint32_t c28b;
std::vector<std::string> result;
std::string call;

	uint8_t i3 = getBits (m_in, 74, 3);
	if ((i3 == 0) ||(i3 == 5))
	   return result;
	c28a    = getLBits (m_in,  0, 28);      // callsign
        c28b    = getLBits (m_in, 29, 28);      // callsign
	if (c28b > HASH_END) { 	// normal call
	   for (int i = 0; i < gehad. size (); i ++)
	      if (gehad. at (i) == c28b)
	         return result;
	   call = getCallsign (c28b);
	   gehad. push_back (c28b);
	   result . push_back (call);
	   return result;
	}
	if ((c28a < CQ_4LETTER_E) && (test_cq (c28a))) {
	   for (int i = 0; i < gehad. size (); i ++)
	      if (gehad. at (i) == c28b)
	         return result;
	   call = getCallsign (c28b);
	   gehad. push_back (c28b);
	   result . push_back (call);
	   return result;
	}
	return result;
}
	
//
//	for now, we assume that interest is in the caller
std::vector<std::string>  packHandler::extract_call_type_1
	                               (const uint8_t *m_in, int type) {
uint32_t c28a;
uint32_t c28b;
uint8_t	R1;
uint16_t g15	= 0;
std::string call;
std::string locator;
std::vector<std::string> result;

	c28a	= getLBits (m_in,  0, 28);	// callsign callee
	c28b	= getLBits (m_in, 29, 28);	// callsign caller
	R1	= getBits  (m_in, 58,  1);	// R1
	g15	= getBits  (m_in, 59, 15); 	// g15, potential grid

//	Check for special tokens DE, QRZ, CQ, CQ_nnn, CQ_xxxx
//	if (test_cq (c28a)) {		// It is a cq
//	    call = getCallsign (c28b);
//	   for (int i = 0; i < gehad. size (); i ++)
//	      if (gehad. at (i) == c28b)
//	         return result;
//	   result. push_back (call);
//	   gehad. push_back (c28b);
//	   if (R1 > 0)
//		   return result;
//	   if (g15 > 0) {
//	      uint16_t n = g15;
//	      char data [5];
//	      data [4] = '\0';
//	      data [3] = '0' + (n % 10);
//	      n /= 10;
//	      data [2] = '0' + (n % 10);
//	      n /= 10;
//	      data [1] = 'A' + (n % 18);
//	      n /= 18;
//	      data [0] = 'A' + (n % 18);
//	      locator	= std::string (data);
//	      if ((data [0] == 'A') || (data [0] == 'R'))
//	         return result;
//	      result. push_back (locator);
//	      return result;
//	   }
//	}
//	else
	if (c28b > HASH_END) { 	// normal call
	   for (int i = 0; i < gehad. size (); i ++)
	      if (gehad. at (i) == c28b)
	         return result;
	   gehad.push_back (c28b);
	   call = getCallsign (c28b);
	   result .push_back (call);
	   if (g15 > 0) {
	      uint16_t n = g15;
	      char data [5];
	      data [4] = '\0';
	      data [3] = '0' + (n % 10);
	      n /= 10;
	      data [2] = '0' + (n % 10);
	      n /= 10;
	      data [1] = 'A' + (n % 18);
	      n /= 18;
	      data [0] = 'A' + (n % 18);
	      locator	= std::string (data);   
	      if ((data [0] == 'A') || (data [0] == 'R'))
	         return result;
	      result. push_back (locator);
	      return result;
	   }
	}
	return result;
}

//	type 3 format:  t1 c28 c28 R1 r3 s13
std::vector<std::string> packHandler::
	                      extract_call_type_3 (const uint8_t *m_in) {
uint8_t t1	= getBits  (m_in, 0, 1);
uint32_t c28a	= getLBits (m_in, 1,  28);      // callsign
uint32_t c28b	= getLBits (m_in, 29, 28);      // callsign
std::vector<std::string>	result;
std::string	call;
//	Check for special tokens DE, QRZ, CQ, CQ_nnn, CQ_xxxx
//	if ((c28a < CQ_4LETTER_E) && test_cq (c28a)) {		// It is a cq
//	   if (c28b <= HASH_END) 
//	      return result;
//	   for (int i = 0; i < gehad. size (); i ++)
//	      if (gehad. at (i) == c28b)
//	         return result;
//	   call = getCallsign (c28b);
//	   gehad. push_back (c28b);
//	   result.push_back (call);
//	   return result;
//	}
	if (c28b > HASH_END) {		// seems a regular call
	   for (int i = 0; i < gehad. size (); i ++)
	      if (gehad. at (i) == c28b)
	         return result;
	   gehad. push_back (c28b);
	   call = getCallsign (c28b);
	   result. push_back (call);
	   return result;
	}
	return result;
}
