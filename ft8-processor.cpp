#
/*
 *    Copyright (C) 2022
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the SDRunoPlugin_ft8
 *
 *    SDRUnoPlugin_ft8 is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDRunoPlugin_ft8 is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDRunoPlugin_ft8; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include	<Windows.h>
#include	"ft8-constants.h"
#include	"SDRunoPlugin_ft8.h"
#include	"ldpc.h"
#include	"pack-handler.h"
#include	"ft8-processor.h"
#include	"psk-writer.h"
#include	"SDRunoPlugin_ft8Ui.h"
#include	<time.h>
#include	<mutex>

	ft8_processor::ft8_processor	(SDRunoPlugin_ft8 *theDecoder,
	                                 SDRunoPlugin_ft8Ui *m_form,
	                                 int maxIterations):
	                                        theCache (30),
	                                        freeSlots (nrBlocks) {
	this	-> theDecoder	= theDecoder;
	this	-> m_form	= m_form;
	this	-> maxIterations. store (maxIterations);
	this	-> blockToRead	= 0;
	this	-> blockToWrite	= 0;
	this	-> filePointer	= nullptr;
	blocks_in. store (0);
	running. store (false);
	threadHandle    = std::thread (&ft8_processor::run, this);
}

	ft8_processor::~ft8_processor	() {
	if (running. load ()) {
	   running. store (false);
	   Sleep (1);
	   threadHandle. join ();
	}
}

//
//	entry for processing
//	The preprocessing was done, so the ldpc check will be done here
//	but firstL buffering size the ldpc is run in another thread
void	ft8_processor::PassOn (int lineno,
	               float refVal, int frequency, float *log174) {
	if (blocks_in. load () > 5)
	   return;
	while (!freeSlots. tryAcquire (200))
	   if (!running. load ())
	      return;
	blocks_in.store(blocks_in.load() + 1);
	theBuffer [blockToWrite]. lineno	= lineno;
	theBuffer [blockToWrite]. value 	= refVal;
	theBuffer [blockToWrite]. frequency	= frequency;
	for (int i = 0; i < 174; i ++)
	   theBuffer [blockToWrite]. log174 [i] = log174 [i];
	blockToWrite = (blockToWrite + 1 ) % nrBlocks;;
	usedSlots. Release ();
}


void	ft8_processor::set_maxIterations	(int n) {
	maxIterations. store (n);
}

void	ft8_processor::run () {
//uint8_t plain174 [FTX_LDPC_N];
uint8_t plain174 [1500];
ldpc ldpcHandler;
int	errors;
packHandler unPacker;
	running. store (true);

	while (running. load()) {
	   while (!usedSlots. tryAcquire (200)) {
	      if (!running. load ())
	         return;
	   }

//	   ldpcHandler. bp_decode (theBuffer [blockToRead]. log174,
//	                           maxIterations. load (),
//	                           plain174, &errors);
	   ldpcHandler. ldpc_decode (theBuffer [blockToRead]. log174,
	                             maxIterations. load (),
	   	                     plain174, &errors);
//         Release the buffer now
	   freeSlots. Release ();
	   blockToRead = (blockToRead + 1) % (nrBlocks);
	   blocks_in. store (blocks_in. load () - 1);
//
	   if (errors != 0) 
	      continue;

//	the check bits are to be moved from 77 up to 82 up
//	and 77 .. 82 should be 0, the crc is from 0 .. 96
	   for (int i = 96; i > 82; i --)
	      plain174 [i] = plain174 [i - 14];

	   for (int i = 77; i < 82; i ++)
	      plain174 [i] = 0;

	   if (check_crc_bits (plain174, 96)) {
//	crc is correct, unpack  the message
	      std::string res = unPacker. unpackMessage  (plain174);
	      if (res != "") {
	         showLine (theBuffer [blockToRead]. lineno,
	                   theBuffer [blockToRead]. value,
	                   theBuffer [blockToRead]. frequency,
	                   res);
	         if (theDecoder -> pskReporterReady ()) {
	            std::vector<std::string> call =
	                               unPacker. extractCall (plain174);
	            int xxx = 0;
//
//	if the return > 0 then we create an adif string
	            if (call.size () > 0) {
	               std::string callIdent = call. at (0);
	               std::string locator;
	               if (call. size () == 2)
	                  locator = call. at (1);
	               int snr  = theBuffer [blockToRead]. value;
	               int freq = theBuffer [blockToRead]. frequency +
	                                            theDecoder -> getVFO ();
	               theDecoder -> addMessage (callIdent, locator,
	                                                   freq, snr);
	            }
	         }
	      }
	   }
//	prepare for the next round
	}
}

static
uint8_t crcPolynome [] = 
	{1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1};	// MSB .. LSB
bool	ft8_processor::check_crc_bits (uint8_t *in, int nrBits) {
uint8_t	b [14];
int16_t	Sum	= 0;

	memset (b, 0, 14);

	for (int i = nrBits - 14; i < nrBits; i ++)
	   in [i] = ~in [i];

	for (int i = 0; i < nrBits; i++) {
	   if ((b [0] ^ in [i]) == 1) {
	      for (int f = 0; f < 13; f++) 
	         b [f] = b [f + 1]; 
	      b [13] = 0;
	      for (int f = 0; f < 14; f ++)
	         b [f] ^= crcPolynome [f];
	   }
	   else {
	      memmove (&b [0], &b[1], sizeof (uint8_t ) * 13); // Shift
	      b [13] = 0;
	   }
	}
	for (int i = 0; i < 14; i ++)
	   Sum += b [i];
	return Sum == 0;
}

void	insertString (char *target, int pos, const std::string &s) {
	for (int i = 0; s. c_str () [i] != 0; i ++)
	   target [pos + i] = s. c_str () [i];
}

void	insertNumber (char *target, int pos, int number) {
std::string s = std::to_string (number);
	insertString (target, pos, s);
}

void	insert_2_Number (char *target, int pos, int number) {
std::string s;
	if ((number >= 100) || (number < 0)) {
	   insertString (target, pos, std::to_string (number));
	   return;
	}
	if (number >= 10) {
	   s. push_back ('0' + number / 10);
	   s. push_back ('0' + number % 10);
	   insertString (target, pos, s);
	   return;
	}
	s. push_back (' ');
	s. push_back ('0' + number);
	insertString (target, pos, s);
}

std::string makeLine (std::string time,
	              int value, int freq,
	              std::string message)  {
char res [256];

static
int posTable[] = { 0, 20, 30, 45, 85 };

	for (int i = 0; i < 256; i ++)
	   res [i] = ' ';

	insertString (res, posTable [0], time);
	insert_2_Number (res, posTable [1], value > 100 ? 101 : value);
	insertNumber (res, posTable [2], freq);
	insertString (res, posTable [3], message);
	res[posTable [4]] = 0;
	return std::string (res);
}

void	ft8_processor::showLine (int line, int val,
	                                 int freq, const std::string &s) {

	if (theCache. update (val, freq, s)) 
	   return;

	int currentFreq = theDecoder -> getVFO ();
	time_t rawTime;
	struct tm *timeinfo;
	char buffer [100];
	time (&rawTime);
	timeinfo = localtime (&rawTime);
	strftime (buffer, 80, "%I:%M:%S%p", timeinfo);

	std::string temp = makeLine (buffer, val, currentFreq + freq, s);
	m_form -> ft8_textBlock (temp);
	locker. lock();
	if (filePointer != nullptr)
	   fprintf (filePointer, "%s\n", temp. c_str ());
	locker. unlock ();
}

bool	ft8_processor::set_ft8Dump	() {
	if (filePointer == nullptr) {
	   nana::filebox fb (0, false);
	   fb.add_filter ("Text File", "*.text");
	   fb.add_filter("All Files", "*.*");
	   auto files = fb();
	   if (!files. empty ()) {
	      locker. lock ();
	      filePointer =
	             fopen (files. front (). string (). c_str (), "w");
	      locker. unlock ();
	   }
	}
	else {
	   locker. lock ();
	   fclose (filePointer);
	   filePointer = nullptr;
	   locker. unlock ();
	}
	return filePointer != nullptr;
}

