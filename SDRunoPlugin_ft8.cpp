
#include	<unoevent.h>
#include	<iunoplugincontroller.h>
#include	<vector>
#include	<iostream>
#include	<fstream>
#include	<sstream>
#include	<chrono>
#include	<Windows.h>
#include	<mutex>
#include	<nana/gui.hpp>
#include	"pack-handler.h"
#include	"SDRunoPlugin_ft8.h"
#include	"SDRunoPlugin_ft8Ui.h"

#include	"psk-writer.h"

#define  _USE_MATH_DEFINES
#include <math.h>

#define	LOG_BASE	240
#define	INPUT_RATE	192000
#define	WORKING_RATE	12000

	SDRunoPlugin_ft8::
	            SDRunoPlugin_ft8 (IUnoPluginController& controller) :
	                                   IUnoPlugin (controller),
	                                   m_form (*this, controller),
	                                   m_worker (nullptr),
	                                   ft8Buffer (32768),
	                                   passbandFilter (15,
	                                                   -3000,
	                                                   3000,
	                                                   INPUT_RATE),
	                                   theDecimator (INPUT_RATE / WORKING_RATE),
	                                   theProcessor (this, &m_form, 14) {
	this	-> toneLength	= WORKING_RATE / 6.25;
	for (int i = 0; i < nrBUFFERS; i ++)
	   theBuffer [i] = new float [2 * toneLength];
	window			= new float [toneLength];
	inputBuffer		= new std::complex<float> [toneLength];
	inBuffer		= new std::complex<float> [toneLength / FRAMES_PER_TONE];
	fftVector_in		= new kiss_fft_cpx [2 * toneLength];
	fftVector_out		= new kiss_fft_cpx [2 * toneLength];
	plan			= kiss_fft_alloc (2 * toneLength, false, 0, 0);

	for (int i = 0; i < toneLength; i ++)  {
	   window [i] =  (0.42 -
	                  0.5 * cos (2 * M_PI * (float)i / toneLength) +
	                  0.08 * cos (4 * M_PI * (float)i / toneLength));
	}

	memset (is_syncTable, 0, 79);
	for (int i = 0; i < 7; i ++) {
	   is_syncTable [i] = true;
	   is_syncTable [36 + i] = true;
	   is_syncTable [72 + i] = true;
	}
	fillIndex	= 0;
	readIndex	= 0;
	inPointer	= 0;
	lineCounter	= 0;

	m_controller	        = &controller;
	running. store (false);

//	m_controller    -> RegisterStreamProcessor (0, this);
	m_controller    -> RegisterAudioProcessor (0, this);
	m_controller    -> SetDemodulatorType (0,
	                         IUnoPluginController::DemodulatorIQOUT);

	int m1		= m_form. ft8_getIterations ();
	int m2 		= m_form. ft8_getSearchWidth ();
	if ((10 > m1) || (m1 > 40))
	   m1 = 20;
	if ((m2 < 2000) || (m2 > 5000))
	   m2 = 4000;
	maxIterations. store (m1);
	spectrumWidth. store (m2);
	theWriter	= nullptr;
	pskReady	= false;
	showMessage	= false;
	teller          = 0;
	m_form. hide_showMessageButton (false);
	m_worker        = new std::thread (&SDRunoPlugin_ft8::WorkerFunction,
	                                                               this);
}

	SDRunoPlugin_ft8::~SDRunoPlugin_ft8 () {	
	running. store (false);
	m_worker -> join ();
	if (theWriter != nullptr)
	   delete theWriter;
//	m_controller    -> UnregisterStreamProcessor (0, this);
	m_controller    -> UnregisterAudioProcessor (0, this);
	delete m_worker;
	m_worker = nullptr;
	delete []	window;
	delete []	inputBuffer;
	delete []	inBuffer;
	delete []	fftVector_in;
	delete []	fftVector_out;
}
//
//	actually not used:
void    SDRunoPlugin_ft8::StreamProcessorProcess (channel_t    channel,
	                                          Complex      *buffer,
	                                          int          length,
	                                          bool         &modified) {
	(void)channel; (void)buffer; (void)length;
	modified = false;
}

void    SDRunoPlugin_ft8::AudioProcessorProcess (channel_t channel,
	                                         float* buffer,
	                                         int length,
	                                         bool& modified) {
//	Handling IQ input, note that SDRuno interchanges I and Q elements
//	bit since we actually only look at amplitudes that is no big deal
	if (!modified) {
	   for (int i = 0; i < length; i++) {
	      std::complex<float> sample =
	                   std::complex<float>(buffer [2 * i +  1],
	                                       buffer [2 * i]);
	      sample = passbandFilter.Pass (sample);
	      if (theDecimator.Pass (sample, &sample))
	         ft8Buffer.putDataIntoBuffer (&sample, 1);
	   }
	}
	modified = true;
}

void	SDRunoPlugin_ft8::HandleEvent (const UnoEvent& ev) {
	switch (ev. GetType ()) {
	   case UnoEvent::FrequencyChanged:
	      break;

	   case UnoEvent::CenterFrequencyChanged:
	      break;

	   default:
	      m_form. HandleEvent (ev);
	      break;
	}
}

#define	BUFFER_SIZE	4096
void	SDRunoPlugin_ft8::WorkerFunction () {
std::complex<float> buffer [BUFFER_SIZE];
std::string version = "V2.3";

	running. store (true);
	m_form. show_version(version);
	while (running. load ()) {
	   while (running. load () &&
	           (ft8Buffer. GetRingBufferReadAvailable () < BUFFER_SIZE))
	      Sleep (1);
	   if (!running. load ())
	      break;

	   ft8Buffer. getDataFromBuffer (buffer, BUFFER_SIZE);
	   for (int i = 0; i < BUFFER_SIZE; i++) {
	      processSample (buffer [i]);
	   }  
	}
	
	Sleep (1000);
}

static inline
std::complex<float> cmul (std::complex<float> x, float y) {
	return std::complex<float>(real(x) * y, imag(x) * y);
}

static inline
float abs (kiss_fft_cpx x) {
	return x.r * x.r + x.i * x.i;
}

//
//	This is where it really starts, we process (decimated) sample
//	by sample
void	SDRunoPlugin_ft8::processSample (std::complex<float> z) {
	inBuffer [inPointer ++] = z;
	if (inPointer < toneLength / FRAMES_PER_TONE)
	   return;
	
	teller += inPointer;
	inPointer = 0;
//
//	If the PSKReporter is "on", we send messages (if any) each
//	60 second
	locker. lock ();
	if ((theWriter != nullptr) && (teller > WORKING_RATE * 60)) {
	   teller = 0;
	   theWriter -> sendMessages ();
	}
	locker. unlock ();

	int content = (FRAMES_PER_TONE - 1) * toneLength / FRAMES_PER_TONE;
	int newAmount = toneLength / FRAMES_PER_TONE;
//
//	shift the inputBuffer to left
	memmove (inputBuffer, &inputBuffer [newAmount],
	                   content * sizeof (std::complex<float>));
//
//	copy the data that is read, into the buffer
	memcpy (&inputBuffer [content], inBuffer, 
	                    newAmount * sizeof (std::complex<float>));
//
//	prepare for fft
	for (int i = 0; i < toneLength; i ++) {
	   fftVector_in [i]. r = real (inputBuffer [i]) * window [i];
	   fftVector_in [i]. i = imag (inputBuffer [i]) * window [i];
	   fftVector_in [toneLength + i]. r = 0;
	   fftVector_in [toneLength + i]. i = 0;
	}

	kiss_fft (plan, fftVector_in, fftVector_out);
//
//	compute avg strength
	float sum	= 0;
	for (int i = 0; i < 2 * toneLength; i ++)
	   sum += abs (fftVector_out [i]);
	sum /= 2 * toneLength;
//
//	copy the spectrum into the buffer, negative frequencies first
//	No idea how log10 reacts on values like "nan" or "inf"
	for (int i = 0; i < toneLength; i ++) {
	   float x = abs (fftVector_out [i]);
	   if ((x < 0) || (x > 10000000)) {
	      fprintf (stderr, "misery\n");
	      x = 0;
	   }
	   float x_p = 10 * log10 (0.0001 + x);

	   float y = abs (fftVector_out [toneLength + i]);
	   if ((y < 0) || (y > 10000000)) {
	      fprintf (stderr, "misery\n");
	      y = 0;
	   }
	   float x_n = 10 * log10 (0.0001 + y);

	   theBuffer [fillIndex][toneLength + i] = x_p + LOG_BASE;
	   theBuffer [fillIndex][i] = x_n + LOG_BASE;
	}
//
//	Since the "end" (begin) of the buffer is not used in the processing
//	we store the average signal value of this "tone" into 0
	theBuffer [fillIndex][0] = 10 * log10 (0.0001 + sum);

	fillIndex = (fillIndex + 1) % nrBUFFERS;
	lineCounter ++;
	if (lineCounter < FRAMES_PER_TONE * nrTONES) {
	   return;
	}
//
//	there is a constant relationship between the fill- and read-index
	readIndex	= fillIndex - FRAMES_PER_TONE * nrTONES;
	if (readIndex < 0)
	   readIndex += nrBUFFERS;
	processLine (readIndex);
}
//
//	we apply a Costas test on selected elements in the line
//	and if we are (more or less) convinced the data is recorded
//	in the cache
void	SDRunoPlugin_ft8::processLine (int lineno) {
std::vector<costasValue> cache;
int	Width	= spectrumWidth. load ();

	int	lowBin	= - Width / 2 / BINWIDTH + toneLength;
	int	highBin	=   Width / 2 / BINWIDTH + toneLength;
	float *xxx = (float *)_alloca (2 * toneLength * sizeof (float));
	for (int bin = lowBin; bin < highBin; bin ++) {
	   float tmp = testCostas (lineno, bin);
	   xxx [bin] = tmp;
	}

	peakFinder (xxx, lowBin, highBin, cache);
	
	if (cache. size () == 0)
	   return;

	float	max	= 0;
	int	maxIndex	= 0;
	for (int i = 0; i < cache. size (); i ++) {
	   if (cache. at (i). relative > max) {
	      max = cache. at (i). relative;
	      maxIndex = i;
	   }
	}

	costasValue it = cache. at (maxIndex);
	float message [174];
//	for (auto it: cache) {
	it. value = decodeTones (lineno, it. index, message);
	theProcessor . PassOn (lineCounter, it.value,
	                 (int)(it. index - toneLength) * BINWIDTH, message);
}

//      with an FFT of 3840 over een band of 12 K, the binwidth = 3.125,
//      i.e. one tone fits takes 2 bins
//      cache 0 should contain the start tone
static
int costasPattern [] = {3, 1, 4, 0, 6, 5, 2};
static
float	getScore	(float *p8, int bin, int tone) {
	int index = bin + 2 * costasPattern [tone];
	float res =  8 * (p8 [index] + p8 [index + 1]);
	for (int i = -3; i <= 3; i ++)
	   res -= p8 [index + 2 * i] + p8 [index + 2 * i + 1];
	return res;
}

float	SDRunoPlugin_ft8::testCostas (int row, int bin) {
float* theLine;
float	score = 0;

	for (int tone = 0; tone < 7; tone ++) {
	   theLine	= theBuffer [(row  +  FRAMES_PER_TONE * tone) % nrBUFFERS];
	   score += getScore (theLine, bin, tone);
	}

	for (int tone = 0; tone < 7; tone ++) {
	   theLine	= theBuffer [(row + FRAMES_PER_TONE * (36 +  tone)) % nrBUFFERS];
	   score += getScore (theLine, bin, tone);
	}

	for (int tone = 0; tone < 7; tone ++) {
	   theLine	= theBuffer [(row + FRAMES_PER_TONE * (72 + tone)) % nrBUFFERS];
	   score += getScore (theLine, bin, tone);
	}

	return score / 21;
}
//	
//
//      we compute the "strength" of the tone bins as relation beetween
//      the bin with the most energy compared to the average of the
//      bins for each tone, and take the average over the tones.
float	SDRunoPlugin_ft8::decodeTones (int row, int bin, float *out) {
int	filler	= 0;
float	strength	= 0;
float	refStrength	= theBuffer [row][0];

	for (int i = 0; i < nrTONES; i ++) {
	   if (is_syncTable [i])
	      continue;

	   int theRow	= (row + FRAMES_PER_TONE * i) % nrBUFFERS;
	   float f = decodeTone (theRow, bin, &out [filler]);
//         strength += f - 10 * log10 (8192);
	   strength += f - refStrength;
	   filler += 3;
	}
	normalize_message (out);
	return strength / (nrTONES - 21);
}

static inline
float max2 (float a, float b) {
	return (a >= b) ? a : b;
}

static inline
float max4 (float a, float b, float c, float d) {
	return max2 (max2 (a, b), max2 (c, d));
}

static
const uint8_t FT8_Gray_map [8] = {0, 1, 3, 2, 5, 6, 4, 7};
float	SDRunoPlugin_ft8::decodeTone (int row, int bin, float *out) {
float s1 [8];
float s2 [8];
float	strength	= 0;

	for (int i = 0; i < 8; i ++) {
	   float a1 =        theBuffer [row][bin + 2 * i];
	   float a2 =        theBuffer [row][bin + 2 * i + 1];
	   if ((a1 + a2) / 2 > strength)
	      strength = (a1 + a2) / 2;
	   s1 [i] =  (a1 + a2) / 2;
	}

	for (int i = 0; i < 8; i ++)
	   s2 [i] = s1 [FT8_Gray_map [i]];

	out [0] = max4 (s2 [4], s2 [5], s2 [6], s2 [7]) -
	                         max4 (s2 [0], s2 [1], s2 [2], s2 [3]);
	out [1] = max4 (s2 [2], s2 [3], s2 [6], s2 [7]) -
	                         max4 (s2 [0], s2 [1], s2 [4], s2 [5]);
	out [2] = max4 (s2 [1], s2 [3], s2 [5], s2 [7]) -
	                        max4 (s2 [0], s2 [2], s2 [4], s2 [6]);
	return strength - LOG_BASE;
}

void	SDRunoPlugin_ft8::normalize_message (float *logMessage) {
float sum = 0;
float sum2 = 0;
//	Compute the variance of the message

	for (int i = 0; i < 174; ++i) {
	   sum  += logMessage [i];
	   sum2 += logMessage [i] * logMessage [i];
	}

	float inv_n = 1.0f / 174;
	float variance = (sum2 - (sum * sum * inv_n)) * inv_n;

//	Normalize log174 distribution and scale it with
//	experimentally found coefficient
	float norm_factor = sqrtf (48.0f / variance);
//	float norm_factor = sqrtf(24.0f / variance);
	for (int i = 0; i < 174; ++i) {
	   logMessage [i] *= norm_factor;
	}
}

#define	KK	3

static inline
float	sum (float *V, int index) {
float result = 0;
	for (int i = 0; i < 2 * KK; i ++)
	   result += V [index - KK + i];
	return result / KK;
}

void	SDRunoPlugin_ft8::peakFinder (float *V, int begin, int end,
	                                    std::vector<costasValue> &cache) {
costasValue	E;
float *workVector = (float *) _alloca (2 * toneLength * sizeof (float));
float	avg = 0;

	for (int i = begin; i < end; i ++) {
	   workVector [i] = sum (V, i);
	   avg += workVector [i];
	}
	avg /= (end - begin);

	for (int index = begin + 5; index < end - 5; index ++) {
	   if (workVector [index] < 1.05 * avg)	// 
	      continue;

	   if (!(1.1 * workVector [index - 5] < workVector [index]) &&
	        (1.1 * workVector [index + 5] < workVector [index]))
	      continue;

	   float max = 0;
	   int maxIndex = 0;
	   for (int i = -KK; i < KK; i ++) {
	      if (workVector [index + i] > max) {
	         max = workVector [index + i];
	         maxIndex = index + i;
	      }
	   }

	   E. index = maxIndex;
	   E. value = theBuffer [readIndex][maxIndex];
	   E. relative = workVector [maxIndex] / workVector [maxIndex + 5];
	   cache. push_back (E);
	   index += 5;
	}
}

void	SDRunoPlugin_ft8::ft8_ldpcDepth   (int val) {
	theProcessor. set_maxIterations (val);
}

void	SDRunoPlugin_ft8::ft8_searchWidth (int val) {
	spectrumWidth. store (val);
}

void	SDRunoPlugin_ft8::set_ft8Dump	() {
	bool b = theProcessor. set_ft8Dump ();
	m_form. show_dumpMode (b);
}

int	SDRunoPlugin_ft8::getVFO () {
	return  m_controller -> GetVfoFrequency (0);
}

bool	SDRunoPlugin_ft8::pskReporterReady () {
	return pskReady;
}

void	SDRunoPlugin_ft8::handle_pskReporterButton() {
	nana::form fm;
	nana::inputbox::text callsign ("Callsign");
	nana::inputbox::text antenna ("antenna");
	nana::inputbox::text grid ("(maidenhead) grid");
	nana::inputbox inbox (fm, "please fill in");
	if (inbox. show (callsign, grid, antenna)) {
	   std::string n = callsign. value ();		//nana::string
	   std::string a = antenna. value ();		//nana::string
	   std::string g = grid. value ();		//nana::string
	   m_form. ft8_textBlock (n + "  " + g + " " + a);;
	   m_form. Store_callsign (n);
	   m_form. Store_grid     (g);
	   m_form. Store_antenna	(a);
	   return;
	}
}

void	SDRunoPlugin_ft8::handle_showMessageButton	() {
	showMessage = !showMessage;
}

void	SDRunoPlugin_ft8::handle_startstopButton () {
	locker.lock();
	if (theWriter != nullptr) {
	   delete theWriter;
	   pskReady = false;
	   theWriter = nullptr;
	   locker.unlock();
	   m_form.show_pskStatus (false);
	   m_form.hide_showMessageButton(false);
	   return;
	}

	try {
	   theWriter = new reporterWriter (&m_form);
	   pskReady = true;
	} catch (int e) {
	   pskReady = false;
	}
	locker. unlock ();
	m_form. show_pskStatus (pskReady);
	m_form. hide_showMessageButton (pskReady);
}

void	SDRunoPlugin_ft8::handle_freqSetter	(int freq) {
	m_controller	-> SetCenterFrequency (0, freq);
}

std::string SDRunoPlugin_ft8::handle_readPresets	()  {
std::string result;

	char* home = getenv ("HOMEPATH");
	nana::filebox fb (nullptr, true);
	fb.add_filter ("text files", "*.txt");
	fb. init_path (home);
	auto files = fb();
	if (!files. empty ()) {
	   std::string fileName = files. front (). string ();
	   std::ifstream newfile;
	   newfile. open (fileName. c_str ());
	   if (newfile. is_open ()) {
	      std::string s;
	      while (std::getline (newfile, s)) {
	         int freq = getFreq (s);
	         if (freq != -1)
	            result += std::to_string (freq) + ";";
	      }
	      newfile. close ();
	   }
	}
	return result;
}

void	SDRunoPlugin_ft8::handle_savePresets (const std::string &presetList) {
char* home = getenv ("HOMEPATH");

	nana::filebox fb (nullptr, false);
	fb.add_filter ("text files", "*.txt");
	fb. init_path (home);
	auto files = fb ();
	if (!files. empty ()) {
	   std::string fileName = files. front (). string ();
	   FILE *f = fopen (fileName. c_str (), "w");
	   if (f != nullptr) {
	      std::istringstream  pL (presetList);
	      std::string s;
	      while (getline (pL, s, ';'))
	         fprintf (f, "%s\n", s. c_str ());
	      fclose (f);
	   }
	}
}

int	SDRunoPlugin_ft8::getFreq	(const std::string  &freq) {
int res;
	if (sscanf (freq. c_str (), "%d", &res) == -1)
	   return -1;
	return res;
}

void	SDRunoPlugin_ft8::addMessage	(const std::string  &call,
	                                 const std::string  &grid,
	                                 int frequency,
	                                 int snr) {
	locker. lock ();
	if (theWriter != nullptr) 
	   theWriter ->  addMessage (call, grid, frequency, snr, showMessage);
	locker. unlock ();
}

void	SDRunoPlugin_ft8::show_pskStatus	(bool b) {
	m_form. show_pskStatus (b);
}

