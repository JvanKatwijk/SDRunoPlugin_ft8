#include <sstream>
#include <iostream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <unoevent.h>

#include "SDRunoPlugin_ft8.h"
#include "SDRunoPlugin_ft8Ui.h"
#include "SDRunoPlugin_ft8Form.h"

// Ui constructor - load the Ui control into a thread
	SDRunoPlugin_ft8Ui::
	         SDRunoPlugin_ft8Ui (SDRunoPlugin_ft8& parent,
	                               IUnoPluginController& controller) :
	                                            m_parent (parent),
	                                            m_form (nullptr),
	                                            m_controller(controller) {
	m_thread = std::thread (&SDRunoPlugin_ft8Ui::ShowUi, this);
}

//	Ui destructor (the nana::API::exit_all();) is required if using
//	Nana UI library
	SDRunoPlugin_ft8Ui::~SDRunoPlugin_ft8Ui () {	
	nana::API::exit_all();
	m_thread.join();	
}

// Show and execute the form
void	SDRunoPlugin_ft8Ui::ShowUi () {	
	m_lock. lock ();
	m_form = std::make_shared<SDRunoPlugin_ft8Form> (*this, m_controller);
	m_lock. unlock();
	m_form  -> Run ();
}

// Load X from the ini file (if exists)
int	SDRunoPlugin_ft8Ui::LoadX () {
	std::string tmp;
	m_controller. GetConfigurationKey("ft8.X", tmp);
	if (tmp.empty ()) {
	   return -1;
	}
	return stoi(tmp);
}

// Load Y from the ini file (if exists)
int	SDRunoPlugin_ft8Ui::LoadY () {
std::string tmp;
	m_controller.GetConfigurationKey ("ft8.Y", tmp);
	if (tmp.empty ()) {
	   return -1;
	}
	return stoi(tmp);
}

std::string SDRunoPlugin_ft8Ui::Load_callsign () {
std::string callsign;
	m_controller. GetConfigurationKey ("ft8.callsign", callsign);
	if (callsign. empty ())
	   return "";
	if (callsign. length () < 3)
	   return "";
	return callsign;
}

void	SDRunoPlugin_ft8Ui::Store_callsign (const std::string &s) {
	if (s. empty ())
	   return;

	m_controller. SetConfigurationKey ("ft8.callsign", s);
}

std::string SDRunoPlugin_ft8Ui::Load_antenna () {
std::string antenna;
	m_controller. GetConfigurationKey ("ft8.antenna", antenna);
	if (antenna. empty ())
	   return "";
	if (antenna. length () < 3)
	   return "";
	return antenna;
}

void	SDRunoPlugin_ft8Ui::Store_antenna (const std::string &s) {
	if (s. empty ())
	   return;

	m_controller. SetConfigurationKey ("ft8.antenna", s);
}

std::string SDRunoPlugin_ft8Ui::Load_grid () {
std::string grid;
	m_controller. GetConfigurationKey ("ft8.grid", grid);
	if (grid. empty ())
	   return "";
	return grid;
}

void	SDRunoPlugin_ft8Ui::Store_grid (const std::string &s) {
	if (s. empty ())
	   return;

	m_controller. SetConfigurationKey ("ft8.grid", s);
}

std::string	SDRunoPlugin_ft8Ui::load_presets	() {
std::string presetList;
	m_controller. GetConfigurationKey ("ft8.presets", presetList);
	if (presetList. empty ())
	   return "";
	return presetList;
}


void	SDRunoPlugin_ft8Ui::add_presetFrequency (int freq) {
std::string presetList;
	m_controller. GetConfigurationKey ("ft8.presets", presetList);
	presetList += std::to_string (freq);
	m_controller. SetConfigurationKey ("ft8.presets", presetList);
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> add_presetFrequency (std::to_string (freq));
}

void	SDRunoPlugin_ft8Ui::clear_presets	() {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr) {
	   m_form -> clear_presets ();
	   m_controller. SetConfigurationKey ("ft8.presets", "");
	}
}

std::vector<int32_t>	SDRunoPlugin_ft8Ui::handle_readPresets	() {
std::string newList;
std::vector<int32_t> presetList;

	newList = m_parent. handle_readPresets ();
	if (newList != "") {
	   std::istringstream f (newList);
	   std::string s;
	   while (getline(f, s, ';'))
		   presetList.push_back(atoi(s.c_str()));
	   m_controller. SetConfigurationKey ("ft8.presets", newList);
	}
	return presetList;
}

int     SDRunoPlugin_ft8Ui::get_startFreq       () {
std::string startFreq_string;
        m_controller. GetConfigurationKey ("ft8.startFreq", startFreq_string);
        if (startFreq_string == "")
           return -1;
        return atoi (startFreq_string. c_str ());
}

void    SDRunoPlugin_ft8Ui::save_startFreq      (int startFreq) {
        if (startFreq < 0)
           return; 
        m_controller. SetConfigurationKey ("ft8.startFreq",
                                                std::to_string (startFreq));
}


void	SDRunoPlugin_ft8Ui::handle_savePresets	() {
std::string presetList;
	m_controller. GetConfigurationKey ("ft8.presets", presetList);
	if (presetList. empty ())
	   return;
	m_parent. handle_savePresets (presetList);
}

void	SDRunoPlugin_ft8Ui::handle_presetButton () {
std::string presetList;
int freq = m_parent. getVFO ();
	freq /= 1000;
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr) 
	   if (m_form -> add_presetFrequency (std::to_string (freq))) {
	      m_controller. GetConfigurationKey ("ft8.presets", presetList);
	      presetList += std::to_string (freq) + ";";
	      m_controller. SetConfigurationKey ("ft8.presets", presetList);
	   }
}

void	SDRunoPlugin_ft8Ui::handle_presetDelete	() {
	std::string presets;
std::vector<std::string> presetList;
	m_controller. GetConfigurationKey ("ft8.presets", presets);
	if (presets. empty ())
	   return;
	std::istringstream f (presets);
        std::string s;
        while (getline (f, s, ';'))
           presetList. push_back (s);
	presets	= "";
	for (int i = 0; i < presetList. size () - 1; i ++)
	   presets += presetList. at (i) + ";";
	m_controller. SetConfigurationKey ("ft8.presets", presets);
}

// Handle events from SDRuno
// TODO: code what to do when receiving relevant events
void	SDRunoPlugin_ft8Ui::HandleEvent (const UnoEvent& ev) {
	switch (ev.GetType()) {
	   case UnoEvent::StreamingStarted:
	      break;

	   case UnoEvent::StreamingStopped:
	      break;

	   case UnoEvent::SavingWorkspace:
	      break;

	   case UnoEvent::ClosingDown:
	      FormClosed ();
	      break;

	   default:
	      break;
	}
}

// Required to make sure the plugin is correctly unloaded when closed
void	SDRunoPlugin_ft8Ui::FormClosed () {
	m_controller. RequestUnload (&m_parent);
}

int	SDRunoPlugin_ft8Ui::loadSearchWidth	() {
std::string tmp = "";

	m_controller. GetConfigurationKey ("ft8.SearchWidth", tmp);
	if (tmp. empty ())
	   return -1;
	else
	   return stoi (tmp);
}

int	SDRunoPlugin_ft8Ui::loadIterations	() {
std::string tmp = "";
	m_controller. GetConfigurationKey ("ft8.Iterations", tmp);
	if (tmp. empty ())
	   return -1;
	else
	   return stoi (tmp);
}

void	SDRunoPlugin_ft8Ui::ft8_ldpcDepth	(int val) {
	m_parent. ft8_ldpcDepth (val);
	m_controller. SetConfigurationKey ("ft8.Iterations", std::to_string (val));
}

void	SDRunoPlugin_ft8Ui::ft8_searchWidth	(int val) {
	m_parent. ft8_searchWidth (val);
	m_controller. SetConfigurationKey ("ft8.SearchWidth", std::to_string (val));
}

int	SDRunoPlugin_ft8Ui::ft8_getSearchWidth	() {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   return m_form -> ft8_getSearchWidth ();
	else
	   return 0;
}

int	SDRunoPlugin_ft8Ui::ft8_getIterations	() {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   return m_form -> ft8_getIterations ();
	else
	   return 0;
}
	
void	SDRunoPlugin_ft8Ui::set_ft8Dump		() {
	m_parent. set_ft8Dump ();
}

void	SDRunoPlugin_ft8Ui::handle_pskReporterButton	() {
	m_parent. handle_pskReporterButton ();
}

void	SDRunoPlugin_ft8Ui::handle_startstopButton	() {
	m_parent. handle_startstopButton ();
}

void	SDRunoPlugin_ft8Ui::handle_showMessageButton	() {
	m_parent. handle_showMessageButton ();
}

void	SDRunoPlugin_ft8Ui::handle_freqSetter		(int f) {
	m_parent. handle_freqSetter (f);
}


void	SDRunoPlugin_ft8Ui::hide_showMessageButton	(bool b) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> hide_showMessageButton (b);
}

void	SDRunoPlugin_ft8Ui::show_dumpMode	(bool b) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> show_dumpMode (b);
}

void	SDRunoPlugin_ft8Ui::show_pskStatus	(bool b) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> show_pskStatus (b);
}

void	SDRunoPlugin_ft8Ui::show_version	(const std::string &s) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> show_version (s);
}

void	SDRunoPlugin_ft8Ui::ft8_textBlock	(const std::string s) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> ft8_textBlock (s);
}

void	SDRunoPlugin_ft8Ui::hide_pskButtons	() {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> hide_pskButtons ();
}

void	SDRunoPlugin_ft8Ui::set_cqSelector	(bool b) {
	m_parent. set_cqSelector (b);
}

void    SDRunoPlugin_ft8Ui::display_startFreq   (int freq) {
        std::lock_guard<std::mutex> l (m_lock); 
        if (m_form != nullptr)
           m_form -> display_startFreq (freq);
}       

void    SDRunoPlugin_ft8Ui::save_pskStatus      (bool status) {
        m_controller. SetConfigurationKey ("ft8.pskStatus",
                                                std::to_string (status));
}

bool    SDRunoPlugin_ft8Ui::get_pskStatus() {
        std::string status;
        m_controller.GetConfigurationKey("ft8.pskStatus", status);
        if (status == "")
                return false;
        return  (bool)(stoi(status));
}

