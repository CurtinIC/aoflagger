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
#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <gtkmm/drawingarea.h>

#include "plotable.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class PlotWidget : public Gtk::DrawingArea {
	public:
		PlotWidget() : _plot(0)
		{
			signal_draw().connect(sigc::mem_fun(*this, &PlotWidget::onDraw) );
			set_size_request(400, 300);
		}
		
		~PlotWidget()
		{
		}

		Plotable &Plot() const
		{
			return *_plot; 
		}
		void SetPlot(Plotable &plot)
		{
			_plot = &plot;
			redraw();
		}
		void Clear()
		{
			_plot = 0;
			redraw();
		}
		void Update()
		{
			redraw();
		}
	private:
		Plotable *_plot;

		bool onDraw(const Cairo::RefPtr<Cairo::Context >& cr)
		{
			redraw();
			return true;
		}

		void redraw()
		{
			if(_plot != 0)
				_plot->Render(*this);
			else {
				Glib::RefPtr<Gdk::Window> window = get_window();
				Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
				cr->set_source_rgba(1, 1, 1, 1);
				cr->paint();
				cr->fill();
			}
		}
};

#endif
