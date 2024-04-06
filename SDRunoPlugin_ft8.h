#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <iunoplugincontroller.h>
#include <iunoplugin.h>
#include <iunostreamobserver.h>
#include <iunoaudioobserver.h>
#include <iunoaudioprocessor.h>
#include <iunostreamobserver.h>
#include <iunoannotator.h>

#include "SDRunoPlugin_ft8Ui.h"
//
//	ft8 specifics
#include	"ringbuffer.h"
#include	"lowpassfilter.h"
#include	"ft8-bandfilter.h"
#include	"decimator.h"
#include	<stdio.h>
#include        <atomic>
#include        ".\fft\kiss_fft.h"
#include        "ft8-constants.h"
#include	"ft8-processor.h"
#include        "ldpc.h"

class reporterWriter;

typedef struct {
	int 	index;
	float	value;
	float	relative;
} costasValue;

class SDRunoPlugin_ft8 : public IUnoPlugin,
	                          public IUnoStreamProcessor,
	                          public IUnoAudioProcessor {
public:
	
		SDRunoPlugin_ft8	(IUnoPluginController& controller);
	virtual ~SDRunoPlugin_ft8	();
	void    ft8_ldpcDepth   (int);
	void    ft8_searchWidth (int);
	void	set_ft8Dump	();
	void	handle_pskReporterButton	();
	void	handle_startstopButton		();
	void	handle_showMessageButton	();
	void	handle_freqSetter		(int);
	std::string	handle_readPresets	();
	void	handle_savePresets	(const std::string &);
	void    ft8_textBlock   (const std::string &);

	int	getVFO		();
	bool	pskReporterReady();
	void	show_pskStatus	(bool b);

	void	addMessage	(const std::string &,
	                         const std::string &, int, int);
	void	set_cqSelector		(bool);
virtual
	const char* GetPluginName() const override { return "SDRuno ft8-decoder Plugin"; }

	// IUnoPlugin
virtual
	void	HandleEvent (const UnoEvent& ev) override;
//
private:
	std::mutex	        m_lock;
	SDRunoPlugin_ft8Ui	m_form;
	std::mutex		locker;
	IUnoPluginController    *m_controller;
	void		        WorkerFunction		();
	std::thread*	        m_worker;
	RingBuffer<Complex>	ft8Buffer;
	bandpassFilter          passbandFilter;
	decimator		theDecimator;
//	ldpc			ldpcHandler;
	reporterWriter* theWriter;

	ft8_processor		theProcessor;
	std::atomic<bool> 	running;

	bool			showMessage;
	bool			pskReady;
	int32_t			inputRate;
	int32_t			outputRate;
	int32_t			samplesperSymbol;

	int			selectedFrequency;
	int			centerFrequency;
	void	                StreamProcessorProcess (channel_t    channel,
	                               Complex* buffer,
	                               int          length,
	                               bool& modified);
	void	                AudioProcessorProcess (channel_t channel,
	                               float* buffer,
		                       int length,
		                       bool& modified);
	int	                resample (std::complex<float> in,
	                                  std::complex<float>* out);
	void	                process (std::complex<float> z);
	void	                processSample (std::complex<float>);

	void			peakFinder (float *V, int begin, int end,
	                                    std::vector<costasValue> &cache);

	void			processLine	(int);
	float			testCostas	(int, int);
	float			decodeTones	(int row, int bin,
	                                                  float *log174);
	float			decodeTone	(int row, int bin,
	                                                  float *logl);
	void			normalize_message	(float *);
	int			addCache	(float,
	                                 int, std::vector<costasValue> &);

	kiss_fft_cfg	plan;
	kiss_fft_cpx	*fftVector_in;
	kiss_fft_cpx	*fftVector_out;
	float		*window;

	int		toneLength;
	int		messageLength;
	int		costasLength;
	float		*theBuffer [nrBUFFERS];
	std::complex<float> *inputBuffer;
	std::complex<float> *inBuffer;
	int		fillIndex;
	int		readIndex;
	int		inPointer;
	int		lineCounter;
	std::atomic<int>	maxIterations;
	std::atomic<int>	spectrumWidth;
	bool		is_syncTable [nrTONES];
	int		teller;
	int		getFreq		(const std::string &);
};
