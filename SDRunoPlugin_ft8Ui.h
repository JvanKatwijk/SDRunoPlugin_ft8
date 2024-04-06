#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <iunoplugin.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>
#include "SDRunoPlugin_ft8Form.h"

// Forward reference
class SDRunoPlugin_ft8;

class SDRunoPlugin_ft8Ui {
public:

		SDRunoPlugin_ft8Ui (SDRunoPlugin_ft8& parent,
	                               IUnoPluginController& controller);
		~SDRunoPlugin_ft8Ui ();

	void	HandleEvent	(const UnoEvent& evt);
	void	FormClosed	();
	void	ShowUi		();
	int	LoadX		();
	int	LoadY		();
	int	loadSearchWidth	();
	int	loadIterations	();
	std::string	Load_callsign	();
	std::string	Load_grid	();
	std::string	Load_antenna	();
	void	Store_callsign	(const std::string &);
	void	Store_grid	(const std::string &);
	void	Store_antenna	(const std::string &);

	void	set_cqSelector		(bool);
	void	ft8_ldpcDepth		(int);
	void	ft8_searchWidth		(int);

	int	ft8_getIterations	();
	int	ft8_getSearchWidth	();	
	void	set_ft8Dump	();
	void	handle_pskReporterButton	();
	void	handle_startstopButton	();
	void	handle_showMessageButton	();
	void	hide_showMessageButton		(bool);
	void	handle_freqSetter	(int);
	void	handle_presetButton	();
	void	clear_presets		();
	void	handle_presetDelete	();
	std::vector<int32_t>	handle_readPresets	();
	void	handle_savePresets	();

	std::string	load_presets		();
	void	add_presetFrequency	(int);
	void	show_dumpMode		(bool);
	void	show_pskStatus		(bool);
	void	ft8_textBlock		(const std::string);
	void	show_version		(const std::string &);
	void	hide_pskButtons		();
//
private:
	
	SDRunoPlugin_ft8 & m_parent;
	std::thread m_thread;
	std::shared_ptr<SDRunoPlugin_ft8Form> m_form;
	bool m_started;
	std::mutex m_lock;
	IUnoPluginController & m_controller;
};
