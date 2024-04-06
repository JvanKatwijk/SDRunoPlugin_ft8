#
/*
 *    Copyright (C) 2022
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Computing
 *
 *    This file is part of the ft8 plugin
 *
 *    swradio is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    swradio is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with swradio; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef	__FT8_PROCESSOR__
#define	__FT8_PROCESSOR__

#include	<thread>
#include	"ft8-constants.h"
#include        <mutex>
#include        <atomic>
#include        <vector>
#include	<stdio.h>
#include	<mutex>
#include        "semaphore.h"
#include	"dl-cache.h"

class	SDRunoPlugin_ft8;
class	SDRunoPlugin_ft8Ui;
class	reporterWriter;

#define	nrBlocks	100

class	ft8_processor {
public:
		ft8_processor	(SDRunoPlugin_ft8 *,
	                         SDRunoPlugin_ft8Ui *,
	                         int);
		~ft8_processor	();

	void	PassOn			(int, float, int, float *);
	void	set_maxIterations	(int);
	bool	set_ft8Dump		();
	void	set_cqSelector		(bool b);
private:
	dlCache		theCache;
	void	run		();
	bool	check_crc_bits	(uint8_t *message, int nrBits);
	std::mutex	locker;
	SDRunoPlugin_ft8Ui *m_form;

	std::string	callsign;
	std::string	grid;

	std::atomic<int> blocks_in;
	struct {
	   int lineno;
	   int value;
	   int frequency;
	   float log174 [174];
	} theBuffer [nrBlocks];

	Semaphore	freeSlots;
	Semaphore	usedSlots;
	std::atomic<bool> running;
	std::atomic<bool> cqSelector;
	FILE		*filePointer;
	SDRunoPlugin_ft8	*theDecoder;
	std::atomic<int>	maxIterations;
	int		blockToRead;
	int		blockToWrite;
	std::thread	threadHandle;
	void		showLine (int, int, int, const std::string &);
};
#endif

