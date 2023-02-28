#include <sstream>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "SDRunoPlugin_ft8Form.h"
#include "SDRunoPlugin_ft8Ui.h"
#include "resource.h"
#include <io.h>
#include <shlobj.h>


//	Form constructor with handles to parent and uno controller
//	- launches form Setup
	SDRunoPlugin_ft8Form::
	           SDRunoPlugin_ft8Form (SDRunoPlugin_ft8Ui& parent,
	                                    IUnoPluginController& controller) :
	nana::form (nana::API::make_center(formWidth, formHeight),
		nana::appearance(false, true, false, false, true, false, false)),
	m_parent(parent),
	m_controller(controller) {
	Setup();
}

// Form deconstructor
	SDRunoPlugin_ft8Form::~SDRunoPlugin_ft8Form () {
}
//
//	Going up
void	SDRunoPlugin_ft8Form::ft8_ldpcDepth	(int val) {
	m_parent. ft8_ldpcDepth (val);
}

void	SDRunoPlugin_ft8Form::ft8_searchWidth	(int val) {
	m_parent. ft8_searchWidth (val);
}

void	SDRunoPlugin_ft8Form::set_ft8Dump	() {
	m_parent.  set_ft8Dump ();
}

void	SDRunoPlugin_ft8Form::show_dumpMode	(bool b) {
	if (b)
	   dumpMode. bgcolor (nana::colors::red);
	else
	   dumpMode. bgcolor (nana::colors::green);
}

void	SDRunoPlugin_ft8Form::show_version (const std::string &v) {
	version. caption (v);
}
//
//	Coming down
void	SDRunoPlugin_ft8Form::ft8_textBlock	(const std::string s) {
std::string fin;
	if (displayList. size () > 30)
	   displayList. pop_back ();
	displayList. push_front (s);
	for (std::string e: displayList)
	   fin += e + "\n";

	textBlock.caption (fin);
}

// Start Form and start Nana UI processing
void	SDRunoPlugin_ft8Form::Run () {
	show();
	nana::exec();
}

// Create the initial plugin form
void	SDRunoPlugin_ft8Form::Setup() {
	// This first section is all related to the background and border
	// it shouldn't need to be changed
	nana::paint::image img_border;
	nana::paint::image img_inner;
	HMODULE hModule = NULL;
	HRSRC rc_border = NULL;
	HRSRC rc_inner = NULL;
	HRSRC rc_close = NULL;
	HRSRC rc_close_over = NULL;
	HRSRC rc_min = NULL;
	HRSRC rc_min_over = NULL;
	HRSRC rc_bar = NULL;
	HRSRC rc_sett = NULL;
	HRSRC rc_sett_over = NULL;
	HBITMAP bm_border = NULL;
	HBITMAP bm_inner = NULL;
	HBITMAP bm_close = NULL;
	HBITMAP bm_close_over = NULL;
	HBITMAP bm_min = NULL;
	HBITMAP bm_min_over = NULL;
	HBITMAP bm_bar = NULL;
	HBITMAP bm_sett = NULL;
	HBITMAP bm_sett_over = NULL;
	BITMAPINFO bmInfo_border = { 0 };
	BITMAPINFO bmInfo_inner = { 0 };
	BITMAPINFO bmInfo_close = { 0 };
	BITMAPINFO bmInfo_close_over = { 0 };
	BITMAPINFO bmInfo_min = { 0 };
	BITMAPINFO bmInfo_min_over = { 0 };
	BITMAPINFO bmInfo_bar = { 0 };
	BITMAPINFO bmInfo_sett = { 0 };
	BITMAPINFO bmInfo_sett_over = { 0 };
	BITMAPFILEHEADER borderHeader = { 0 };
	BITMAPFILEHEADER innerHeader = { 0 };
	BITMAPFILEHEADER closeHeader = { 0 };
	BITMAPFILEHEADER closeoverHeader = { 0 };
	BITMAPFILEHEADER minHeader = { 0 };
	BITMAPFILEHEADER minoverHeader = { 0 };
	BITMAPFILEHEADER barHeader = { 0 };
	BITMAPFILEHEADER settHeader = { 0 };
	BITMAPFILEHEADER settoverHeader = { 0 };
	HDC hdc = NULL;
	BYTE* borderPixels = NULL;
	BYTE* innerPixels = NULL;
	BYTE* closePixels = NULL;
	BYTE* closeoverPixels = NULL;
	BYTE* minPixels = NULL;
	BYTE* minoverPixels = NULL;
	BYTE* barPixels = NULL;
	BYTE* barfocusedPixels = NULL;
	BYTE* settPixels = NULL;
	BYTE* settoverPixels = NULL;
	const unsigned int rawDataOffset = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
	hModule = GetModuleHandle(L"SDRunoPlugin_ft8");
	hdc = GetDC(NULL);
	rc_border = FindResource(hModule, MAKEINTRESOURCE(IDB_BG_BORDER), RT_BITMAP);
	rc_inner = FindResource(hModule, MAKEINTRESOURCE(IDB_BACKGROUND), RT_BITMAP);
	rc_close = FindResource(hModule, MAKEINTRESOURCE(IDB_CLOSE), RT_BITMAP);
	rc_close_over = FindResource(hModule, MAKEINTRESOURCE(IDB_CLOSE_DOWN), RT_BITMAP);
	rc_min = FindResource(hModule, MAKEINTRESOURCE(IDB_MIN), RT_BITMAP);
	rc_min_over = FindResource(hModule, MAKEINTRESOURCE(IDB_MIN_DOWN), RT_BITMAP);
	rc_bar = FindResource(hModule, MAKEINTRESOURCE(IDB_HEADER), RT_BITMAP);
	rc_sett = FindResource(hModule, MAKEINTRESOURCE(IDB_SETT), RT_BITMAP);
	rc_sett_over = FindResource(hModule, MAKEINTRESOURCE(IDB_SETT_DOWN), RT_BITMAP);
	bm_border = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_BG_BORDER), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_inner = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_BACKGROUND), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_close = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_CLOSE), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_close_over = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_CLOSE_DOWN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_min = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_MIN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_min_over = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_MIN_DOWN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_bar = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_HEADER), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_sett = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_SETT), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bm_sett_over = (HBITMAP)LoadImage(hModule, MAKEINTRESOURCE(IDB_SETT_DOWN), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
	bmInfo_border.bmiHeader.biSize = sizeof(bmInfo_border.bmiHeader);
	bmInfo_inner.bmiHeader.biSize = sizeof(bmInfo_inner.bmiHeader);
	bmInfo_close.bmiHeader.biSize = sizeof(bmInfo_close.bmiHeader);
	bmInfo_close_over.bmiHeader.biSize = sizeof(bmInfo_close_over.bmiHeader);
	bmInfo_min.bmiHeader.biSize = sizeof(bmInfo_min.bmiHeader);
	bmInfo_min_over.bmiHeader.biSize = sizeof(bmInfo_min_over.bmiHeader);
	bmInfo_bar.bmiHeader.biSize = sizeof(bmInfo_bar.bmiHeader);
	bmInfo_sett.bmiHeader.biSize = sizeof(bmInfo_sett.bmiHeader);
	bmInfo_sett_over.bmiHeader.biSize = sizeof(bmInfo_sett_over.bmiHeader);
	GetDIBits(hdc, bm_border, 0, 0, NULL, &bmInfo_border, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_inner, 0, 0, NULL, &bmInfo_inner, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close, 0, 0, NULL, &bmInfo_close, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close_over, 0, 0, NULL, &bmInfo_close_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min, 0, 0, NULL, &bmInfo_min, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min_over, 0, 0, NULL, &bmInfo_min_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_bar, 0, 0, NULL, &bmInfo_bar, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett, 0, 0, NULL, &bmInfo_sett, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett_over, 0, 0, NULL, &bmInfo_sett_over, DIB_RGB_COLORS);
	bmInfo_border.bmiHeader.biCompression = BI_RGB;
	bmInfo_inner.bmiHeader.biCompression = BI_RGB;
	bmInfo_close.bmiHeader.biCompression = BI_RGB;
	bmInfo_close_over.bmiHeader.biCompression = BI_RGB;
	bmInfo_min.bmiHeader.biCompression = BI_RGB;
	bmInfo_min_over.bmiHeader.biCompression = BI_RGB;
	bmInfo_bar.bmiHeader.biCompression = BI_RGB;
	bmInfo_sett.bmiHeader.biCompression = BI_RGB;
	bmInfo_sett_over.bmiHeader.biCompression = BI_RGB;
	borderHeader.bfOffBits = rawDataOffset;
	borderHeader.bfSize = bmInfo_border.bmiHeader.biSizeImage;
	borderHeader.bfType = 0x4D42;
	innerHeader.bfOffBits = rawDataOffset;
	innerHeader.bfSize = bmInfo_inner.bmiHeader.biSizeImage;
	innerHeader.bfType = 0x4D42;
	closeHeader.bfOffBits = rawDataOffset;
	closeHeader.bfSize = bmInfo_close.bmiHeader.biSizeImage;
	closeHeader.bfType = 0x4D42;
	closeoverHeader.bfOffBits = rawDataOffset;
	closeoverHeader.bfSize = bmInfo_close_over.bmiHeader.biSizeImage;
	closeoverHeader.bfType = 0x4D42;
	minHeader.bfOffBits = rawDataOffset;
	minHeader.bfSize = bmInfo_min.bmiHeader.biSizeImage;
	minHeader.bfType = 0x4D42;
	minoverHeader.bfOffBits = rawDataOffset;
	minoverHeader.bfSize = bmInfo_min_over.bmiHeader.biSizeImage;
	minoverHeader.bfType = 0x4D42;
	barHeader.bfOffBits = rawDataOffset;
	barHeader.bfSize = bmInfo_bar.bmiHeader.biSizeImage;
	barHeader.bfType = 0x4D42;
	settHeader.bfOffBits = rawDataOffset;
	settHeader.bfSize = bmInfo_sett.bmiHeader.biSizeImage;
	settHeader.bfType = 0x4D42;
	settoverHeader.bfOffBits = rawDataOffset;
	settoverHeader.bfSize = bmInfo_sett_over.bmiHeader.biSizeImage;
	settoverHeader.bfType = 0x4D42;
	borderPixels = new BYTE[bmInfo_border.bmiHeader.biSizeImage + rawDataOffset];
	innerPixels = new BYTE[bmInfo_inner.bmiHeader.biSizeImage + rawDataOffset];
	closePixels = new BYTE[bmInfo_close.bmiHeader.biSizeImage + rawDataOffset];
	closeoverPixels = new BYTE[bmInfo_close_over.bmiHeader.biSizeImage + rawDataOffset];
	minPixels = new BYTE[bmInfo_min.bmiHeader.biSizeImage + rawDataOffset];
	minoverPixels = new BYTE[bmInfo_min_over.bmiHeader.biSizeImage + rawDataOffset];
	barPixels = new BYTE[bmInfo_bar.bmiHeader.biSizeImage + rawDataOffset];
	settPixels = new BYTE[bmInfo_sett.bmiHeader.biSizeImage + rawDataOffset];
	settoverPixels = new BYTE[bmInfo_sett_over.bmiHeader.biSizeImage + rawDataOffset];
	*(BITMAPFILEHEADER*)borderPixels = borderHeader;
	*(BITMAPINFO*)(borderPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_border;
	*(BITMAPFILEHEADER*)innerPixels = innerHeader;
	*(BITMAPINFO*)(innerPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_inner;
	*(BITMAPFILEHEADER*)closePixels = closeHeader;
	*(BITMAPINFO*)(closePixels + sizeof(BITMAPFILEHEADER)) = bmInfo_close;
	*(BITMAPFILEHEADER*)closeoverPixels = closeoverHeader;
	*(BITMAPINFO*)(closeoverPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_close_over;
	*(BITMAPFILEHEADER*)minPixels = minHeader;
	*(BITMAPINFO*)(minPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_min;
	*(BITMAPFILEHEADER*)minoverPixels = minoverHeader;
	*(BITMAPINFO*)(minoverPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_min_over;
	*(BITMAPFILEHEADER*)barPixels = barHeader;
	*(BITMAPINFO*)(barPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_bar;
	*(BITMAPFILEHEADER*)settPixels = settHeader;
	*(BITMAPINFO*)(settPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_sett;
	*(BITMAPFILEHEADER*)settoverPixels = settoverHeader;
	*(BITMAPINFO*)(settoverPixels + sizeof(BITMAPFILEHEADER)) = bmInfo_sett_over;
	GetDIBits(hdc, bm_border, 0, bmInfo_border.bmiHeader.biHeight, (LPVOID)(borderPixels + rawDataOffset), &bmInfo_border, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_inner, 0, bmInfo_inner.bmiHeader.biHeight, (LPVOID)(innerPixels + rawDataOffset), &bmInfo_inner, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close, 0, bmInfo_close.bmiHeader.biHeight, (LPVOID)(closePixels + rawDataOffset), &bmInfo_close, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_close_over, 0, bmInfo_close_over.bmiHeader.biHeight, (LPVOID)(closeoverPixels + rawDataOffset), &bmInfo_close_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min, 0, bmInfo_min.bmiHeader.biHeight, (LPVOID)(minPixels + rawDataOffset), &bmInfo_min, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_min_over, 0, bmInfo_min_over.bmiHeader.biHeight, (LPVOID)(minoverPixels + rawDataOffset), &bmInfo_min_over, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_bar, 0, bmInfo_bar.bmiHeader.biHeight, (LPVOID)(barPixels + rawDataOffset), &bmInfo_bar, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett, 0, bmInfo_sett.bmiHeader.biHeight, (LPVOID)(settPixels + rawDataOffset), &bmInfo_sett, DIB_RGB_COLORS);
	GetDIBits(hdc, bm_sett_over, 0, bmInfo_sett_over.bmiHeader.biHeight, (LPVOID)(settoverPixels + rawDataOffset), &bmInfo_sett_over, DIB_RGB_COLORS);
	img_border.open(borderPixels, bmInfo_border.bmiHeader.biSizeImage);
	img_inner.open(innerPixels, bmInfo_inner.bmiHeader.biSizeImage);
	img_close_normal.open(closePixels, bmInfo_close.bmiHeader.biSizeImage);
	img_close_down.open(closeoverPixels, bmInfo_close_over.bmiHeader.biSizeImage);
	img_min_normal.open(minPixels, bmInfo_min.bmiHeader.biSizeImage);
	img_min_down.open(minoverPixels, bmInfo_min_over.bmiHeader.biSizeImage);
	img_header.open(barPixels, bmInfo_bar.bmiHeader.biSizeImage);
	img_sett_normal.open(settPixels, bmInfo_sett.bmiHeader.biSizeImage);
	img_sett_down.open(settoverPixels, bmInfo_sett_over.bmiHeader.biSizeImage);
	ReleaseDC(NULL, hdc);
	bg_border.load(img_border, nana::rectangle(0, 0, 590, 340));
	bg_border.stretchable(0, 0, 0, 0);
	bg_border.transparent(true);
	bg_inner.load(img_inner, nana::rectangle(0, 0, 582, 299));
	bg_inner.stretchable(sideBorderWidth, 0, sideBorderWidth, bottomBarHeight);
	bg_inner.transparent(false);

	// TODO: Form code starts here

	// Load X and Y location for the form from the ini file (if exists)
	int posX = m_parent.LoadX();
	int posY = m_parent.LoadY();
	move	(posX, posY);

	// This code sets the plugin size, title and what to do when the X is pressed
	size(nana::size(formWidth, formHeight));
	caption ("SDRuno Plugin ft8");
	events().destroy([&] { m_parent.FormClosed(); });

	//Initialize header bar
	header_bar.size(nana::size(122, 20));
	header_bar.load(img_header, nana::rectangle(0, 0, 122, 20));
	header_bar.stretchable(0, 0, 0, 0);
	header_bar.move(nana::point((formWidth / 2) - 61, 5));
	header_bar.transparent(true);

	//Initial header text 
	title_bar_label.size(nana::size(65, 12));
	title_bar_label.move(nana::point((formWidth / 2) - 5, 9));
	title_bar_label.format(true);
	title_bar_label.caption("< bold size = 6 color = 0x000000 font = \"Verdana\">ft8 decoder</>");
	title_bar_label.text_align(nana::align::center, nana::align_v::center);
	title_bar_label.fgcolor(nana::color_rgb(0x000000));
	title_bar_label.transparent(true);

	//Iniitialize drag_label
	form_drag_label.move(nana::point(0, 0));
	form_drag_label.transparent(true);

	//Initialize dragger and set target to form, and trigger to drag_label 
	form_dragger.target(*this);
	form_dragger.trigger(form_drag_label);

	//Initialise the "Minimize button"
	min_button.load(img_min_normal, nana::rectangle(0, 0, 20, 15));
	min_button.bgcolor(nana::color_rgb(0x000000));
	min_button.move(nana::point(formWidth - 51, 9));
	min_button.transparent(true);
	min_button.events().mouse_down([&] { min_button.load(img_min_down, nana::rectangle(0, 0, 20, 15)); });
	min_button.events().mouse_up([&] { min_button.load(img_min_normal, nana::rectangle(0, 0, 20, 15)); nana::API::zoom_window(this->handle(), false); });
	min_button.events().mouse_leave([&] { min_button.load(img_min_normal, nana::rectangle(0, 0, 20, 15)); });

	//Initialise the "Close button"
	close_button.load(img_close_normal, nana::rectangle(0, 0, 20, 15));
	close_button.bgcolor(nana::color_rgb(0x000000));
	close_button.move(nana::point(formWidth - 26, 9));
	close_button.transparent(true);
	close_button.events().mouse_down([&] { close_button.load(img_close_down, nana::rectangle(0, 0, 20, 15)); });
	close_button.events().mouse_up([&] { close_button.load(img_close_normal, nana::rectangle(0, 0, 20, 15)); close(); });
	close_button.events().mouse_leave([&] { close_button.load(img_close_normal, nana::rectangle(0, 0, 20, 15)); });

	//Uncomment the following block of code to Initialise the "Setting button"
	sett_button.load(img_sett_normal, nana::rectangle(0, 0, 40, 15));
	sett_button.bgcolor(nana::color_rgb(0x000000));
	sett_button.move(nana::point(10, 9));
	sett_button.events().mouse_down([&] { sett_button.load(img_sett_down, nana::rectangle(0, 0, 40, 15)); });
	sett_button.events().mouse_up([&] { sett_button.load(img_sett_normal, nana::rectangle(0, 0, 40, 15)); SettingsButton_Click(); });
	sett_button.events().mouse_leave([&] { sett_button.load(img_sett_normal, nana::rectangle(0, 0, 40, 15)); });
	sett_button.tooltip ("Show settings window");
	sett_button.transparent(true);

// TODO: Extra Form code goes here

	ldpcDepth.  range (5, 50, 1);
	int	v1	= m_parent. loadIterations ();
	if ((v1 < 5) || (v1 > 50))
	   v1 = 10;
	ldpcDepth.  value (std::to_string (v1));
	ldpcDepth.  events (). text_changed ([&](const nana::arg_spinbox &s) {
	                              ft8_ldpcDepth (ldpcDepth. to_int ());});
	ldpcDepth. tooltip ("set the repair/recovery depth for the ldpc");
	ldpcDepth. bgcolor(nana::colors::black);
	ldpcDepth. fgcolor(nana::colors::white);

	searchWidth.  range (2000, 50000, 500);
	v1	= m_parent. loadSearchWidth ();
	if ((v1 < 2000) || (v1 > 5000) || ((v1 % 500) != 0))
	   v1 = 2000;
	searchWidth.  value (std::to_string (v1));
	searchWidth.  events (). text_changed ([&](const nana::arg_spinbox &s) {
	                              ft8_searchWidth (searchWidth. to_int ());});
	searchWidth. tooltip ("set the width (in Hz) for ft8 detection");
	searchWidth. bgcolor(nana::colors::black);
	searchWidth. fgcolor(nana::colors::white);

	ft8Dump. caption ("file");
	ft8Dump. events().click([&]() {set_ft8Dump ();});
	dumpMode. bgcolor (nana::colors::green);
	ft8Dump. tooltip ("The window will show the last 30 messages recognized, touch this button to save all output in a (text) file");

	copyRightLabel. transparent (true);
	copyRightLabel. fgcolor (nana::colors::white);
	copyRightLabel. caption ("\xa9 2022");
	copyRightLabel. tooltip ("This plugin is created by Jan van Katwijk\n\
Lazy Chair Computing\n\
all rights reserved\n\
the ldpc decoder is taken from work of Garlis Goba, his righte are acknowledged\n\
FFT's are computed using the kiss library, all rights acknowledged");

	pskStatus. caption ("psk reporter");	
	pskStatus. bgcolor (nana::colors::red);
	pskReporterButton. caption ("callsign");
	pskReporterButton. tooltip ("Touching the button will show a small widget\nwhere you can give your callsign and grid locator\n\
the values are maintained between invocations of the plugin");
	pskReporterButton.events().click([&]() {handle_pskReporterButton(); });

	pskstartstopButton.caption ("reporter");
	pskstartstopButton.tooltip ("Touching the button will start or stop the interface to the pskreporter");
	pskstartstopButton.events().click([&]() {handle_startstopButton (); });

	messageButton.caption ("show");
	messageButton.tooltip ("Having touched the button, all messages passed on to the PSKReporter will be made visible");
	messageButton.events().click([&]() {handle_showMessageButton (); });

	freqSetter. push_back ("presets");
        freqSetter. push_back ("3575");
        freqSetter. push_back ("7075");
        freqSetter. push_back ("10136");
        freqSetter. push_back ("14075");
	std::string presetList = m_parent. load_presets ();
	std::istringstream f (presetList);
	
	std::string s;
	while (getline (f, s, ';'))
	   freqSetter. push_back (s);

        freqSetter. option (0);
        freqSetter. events(). selected ([&](const nana::arg_combox &ar_cbx){
                        handle_freqSetter (ar_cbx. widget. caption ());});

	presetButton. caption ("preset");
	presetButton. tooltip ("Touching this button will add the currently selected frequency to the presetlist");
	presetButton. events().click([&]() {handle_presetButton (); });

	presetDelete. caption ("delete");
	presetDelete. tooltip ("Touching this button will delete the last user-added preset in the list");
	presetDelete. events().click([&]() {handle_presetDelete (); });

	readPresets. push_back ("presetfile");
	readPresets. push_back ("load presets");
	readPresets. push_back ("save presets");
	readPresets. option (0);
	readPresets. tooltip (" Loading presets from a file or saving presets \
into a text file can be selected.");
        readPresets. events(). selected ([&](const nana::arg_combox &ar_cbx){
                        handle_readPresets (ar_cbx. widget. caption ());});

	delete[] borderPixels;
	delete[] innerPixels;
	delete[] closePixels;
	delete[] closeoverPixels;
	delete[] minPixels;
	delete[] minoverPixels;
	delete[] barPixels;
	delete[] barfocusedPixels;
	delete[] settPixels;
	delete[] settoverPixels;
}

void	SDRunoPlugin_ft8Form::SettingsButton_Click () {
	// TODO: Insert code here to show settings panel
}

int	SDRunoPlugin_ft8Form::ft8_getIterations () {
	return ldpcDepth. to_int ();
}

int	SDRunoPlugin_ft8Form::ft8_getSearchWidth () {
	return searchWidth. to_int ();
}

void	SDRunoPlugin_ft8Form::handle_pskReporterButton	() {
	m_parent.  handle_pskReporterButton ();
}

void	SDRunoPlugin_ft8Form::handle_startstopButton	() {
	m_parent.  handle_startstopButton ();
}

void	SDRunoPlugin_ft8Form::handle_freqSetter		(const std::string &s) {
int32_t	freq;
	try {
	   freq = std::stoi (s);
	   m_parent. handle_freqSetter (freq * 1000);
	} catch (int e) {
	   return;
	}
}

void	SDRunoPlugin_ft8Form::handle_showMessageButton	() {
	m_parent. handle_showMessageButton ();
}

void	SDRunoPlugin_ft8Form::hide_showMessageButton	(bool b) {
	if (b)
	   messageButton. show ();
	else
	   messageButton. hide ();
}
	
void	SDRunoPlugin_ft8Form::show_pskStatus	(bool b) {
	if (b)
	   pskStatus. bgcolor (nana::colors::green);
	else
	   pskStatus. bgcolor (nana::colors::red);
}

void	SDRunoPlugin_ft8Form::hide_pskButtons	() {
	pskReporterButton. hide ();
}

void	SDRunoPlugin_ft8Form::handle_presetButton () {
	m_parent. handle_presetButton ();
}

void	SDRunoPlugin_ft8Form::handle_presetDelete	() {
	m_parent. handle_presetDelete ();
	if (freqSetter.the_number_of_options  () > 5) 
	   freqSetter. erase (freqSetter. the_number_of_options () - 1);
}

void	SDRunoPlugin_ft8Form::handle_readPresets	(const std::string &s) {
	if (s == "load presets") {
	   std::vector<int32_t> result = m_parent. handle_readPresets ();
	   if (result. size () != 0)
	      clear_presets ();
	   for (int i = 0; i < result. size (); i ++)
	      freqSetter. push_back (std::to_string (result. at (i)));
	}
	else
	if (s == "save presets")
	   m_parent. handle_savePresets ();
	readPresets. option (0);
}

void	SDRunoPlugin_ft8Form::clear_presets	() {
	while (freqSetter. the_number_of_options () > 5)
	   freqSetter. erase (freqSetter. the_number_of_options () - 1);
}

bool	SDRunoPlugin_ft8Form::add_presetFrequency (const std::string freq) {
	for (int i = 0; i < freqSetter. the_number_of_options (); i ++)
	   if (freqSetter. text (i) == freq)
	      return false;
	freqSetter. push_back (freq);
	return true;
}

