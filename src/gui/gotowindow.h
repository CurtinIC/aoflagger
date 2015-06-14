/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef GOTOWINDOW_H
#define GOTOWINDOW_H

#include <string>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include "../strategy/control/types.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class GoToWindow : public Gtk::Window {
	public:
		GoToWindow(class RFIGuiWindow &rfiGuiWindow);
		~GoToWindow();
	private:
		void onLoadClicked();

		Gtk::ButtonBox _buttonBox;
		Gtk::VBox _vBox;
		Gtk::HBox _hBox;
		Gtk::VBox _bandFrameBox;
		Gtk::Frame _antenna1Frame, _antenna2Frame, _bandFrame, _sequenceFrame;
		Gtk::TreeView _antenna1View, _antenna2View, _bandView, _sequenceView;
		Gtk::ScrolledWindow _antenna1Scroll, _antenna2Scroll, _bandScroll, _sequenceScroll;
		Gtk::Button _loadButton;

		class AntennaModelColumns : public Gtk::TreeModelColumnRecord
		{
		public:
			AntennaModelColumns()
				{ add(antennaIndex); add(antennaName); }
		
			Gtk::TreeModelColumn<size_t> antennaIndex;
			Gtk::TreeModelColumn<Glib::ustring> antennaName;
		};

		class BandModelColumns : public Gtk::TreeModelColumnRecord
		{
		public:
			BandModelColumns()
				{ add(bandIndex); add(bandDescription); }
		
			Gtk::TreeModelColumn<size_t> bandIndex;
			Gtk::TreeModelColumn<Glib::ustring> bandDescription;
		};

		class SequenceModelColumns : public Gtk::TreeModelColumnRecord
		{
		public:
			SequenceModelColumns()
				{ add(sequenceIndex); add(sequenceDescription); }
		
			Gtk::TreeModelColumn<size_t> sequenceIndex;
			Gtk::TreeModelColumn<Glib::ustring> sequenceDescription;
		};

		AntennaModelColumns _antennaModelColumns;
		BandModelColumns _bandModelColumns;
		SequenceModelColumns _sequenceModelColumns;

		Glib::RefPtr<Gtk::ListStore> _antennaeStore, _bandStore, _sequenceStore;

		RFIGuiWindow &_rfiGuiWindow;
		rfiStrategy::MSImageSet *_imageSet;
};

#endif
