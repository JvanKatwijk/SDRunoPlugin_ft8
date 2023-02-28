#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/widgets/spinbox.hpp>

#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/gui/dragger.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <list>
#include	<mutex>
#include	<vector>

#include <iunoplugincontroller.h>

// Shouldn't need to change these
#define topBarHeight (27)
#define bottomBarHeight (8)
#define sideBorderWidth (8)

// TODO: Change these numbers to the height and width of your form
//#define formWidth (297)
#define formWidth (550)
#define formHeight (600)

class SDRunoPlugin_ft8Ui;

class SDRunoPlugin_ft8Form : public nana::form {
public:

		SDRunoPlugin_ft8Form (SDRunoPlugin_ft8Ui& parent,
	                                 IUnoPluginController& controller);		
		~SDRunoPlugin_ft8Form ();

	void	ft8_ldpcDepth		(int);
	void	ft8_searchWidth		(int);
	int	ft8_getIterations	();
	int	ft8_getSearchWidth	();
	void	set_ft8Dump		();
	void	show_pskStatus		(bool);
	void	show_version	(const std::string &);
	void	handle_showMessageButton	();
	void	hide_showMessageButton	(bool);
	void	handle_pskReporterButton	();
	void	handle_startstopButton	();
	void	handle_freqSetter	(const std::string &);
	void	handle_presetButton	();	
	void	handle_presetDelete	();
	void	handle_readPresets	(const std::string &);
	void	clear_presets		();
	void	show_dumpMode		(bool);
	void	ft8_textBlock		(const std::string);
	void	hide_pskButtons		();
	bool	add_presetFrequency	(const std::string);
	void	Run			();

private:

	void Setup();

	// The following is to set up the panel graphic to look like a standard SDRuno panel
	nana::picture bg_border{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::picture bg_inner{ bg_border, nana::rectangle(sideBorderWidth, topBarHeight, formWidth - (2 * sideBorderWidth), formHeight - topBarHeight - bottomBarHeight) };
	nana::picture header_bar{ *this, true };
	nana::label title_bar_label{ *this, true };
	nana::dragger form_dragger;
	nana::label form_drag_label{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::paint::image img_min_normal;
	nana::paint::image img_min_down;
	nana::paint::image img_close_normal;
	nana::paint::image img_close_down;
	nana::paint::image img_header;
	nana::picture close_button {*this, nana::rectangle(0, 0, 20, 15) };
	nana::picture min_button {*this, nana::rectangle(0, 0, 20, 15) };

	// Uncomment the following 4 lines if you want a SETT button and panel
	nana::paint::image img_sett_normal;
	nana::paint::image img_sett_down;
	nana::picture sett_button{ *this, nana::rectangle(0, 0, 40, 15) };
	void SettingsButton_Click();

	SDRunoPlugin_ft8Ui& m_parent;
	IUnoPluginController	& m_controller;

	// TODO: Now add your UI controls here
	std::mutex locker;
	nana::spinbox	ldpcDepth
	                  {*this, nana::rectangle (30, 30,  80, 20) };
	nana::spinbox	searchWidth
	                  {*this, nana::rectangle (120, 30, 80, 20)};
	nana::button	ft8Dump
	                  {*this, nana::rectangle (210, 30, 50, 20)};
	nana::label	dumpMode
	                  {*this, nana::rectangle (270, 30, 50, 20)};
	nana::label	version
	                  {*this, nana::rectangle (330, 30, 50, 20)};
	nana::button	presetButton
	                  {*this, nana::rectangle (390, 30, 50, 20)};
	nana::label	copyRightLabel
	                  {*this, nana::rectangle (450, 30, 50, 30)};
//	line 2
	nana::button	pskstartstopButton
	                  {*this, nana::rectangle (30, 70, 50, 20)};
	nana::label	pskStatus
	                  {*this, nana::rectangle (90, 70, 50, 20)};
	nana::button	pskReporterButton
	                  {*this, nana::rectangle (150, 70, 50, 20)};
	nana::button	messageButton
	                  {*this, nana::rectangle (210, 70, 50, 20)};
	nana::combox	freqSetter
	                  {*this, nana::rectangle (270, 70, 70, 20)};
	nana::button	presetDelete
	                  {*this, nana::rectangle (350, 70, 50, 20)};
	nana::combox	readPresets
	                  {*this, nana::rectangle (410, 70, 80, 20)};
	std::list<std::string> displayList;
	nana::textbox	textBlock {*this, nana::rectangle (30, 100, 480, 480) };	 
};
