#include <iostream>

#include <gtkmm/messagedialog.h>

#include "msoptionwindow.h"

#include "../strategy/actions/strategy.h"

#include "../strategy/imagesets/msimageset.h"

#include "../strategy/control/defaultstrategy.h"

#include "rfiguiwindow.h"

MSOptionWindow::MSOptionWindow(RFIGuiWindow &rfiGUiWindow, StrategyController &strategyController, const std::string &filename) :
	Gtk::Window(),
	_rfiGuiWindow(rfiGUiWindow),
	_strategyController(strategyController),
	_filename(filename),
	_openButton("_Open", true),
	_dataKindFrame("Columns to read"),
	_polarisationFrame("Polarisation to read"),
	_observedDataButton("Observed"), _correctedDataButton("Corrected"), _modelDataButton("Model"), _residualDataButton("Residual"),
	_otherColumnButton("Other:"),
	_allDipolePolarisationButton("Dipole (xx,xy,yx,yy separately)"),
	_autoDipolePolarisationButton("Dipole auto-correlations (xx and yy)"),
	_stokesIPolarisationButton("Stokes I"),
	_directReadButton("Direct IO"),
	_indirectReadButton("Indirect IO"),
	_memoryReadButton("Memory-mode IO"),
	_readUVWButton("Read UVW"),
	_loadOptimizedStrategy("Load optimized strategy")
{
	set_title("Options for opening a measurement set");

	initDataTypeButtons();
	initPolarisationButtons();

	_openButton.set_image_from_icon_name("document-open");
	_openButton.signal_clicked().connect(sigc::mem_fun(*this, &MSOptionWindow::onOpen));
	_bottomButtonBox.pack_start(_openButton);

	_rightVBox.pack_start(_directReadButton);
	_rightVBox.pack_start(_indirectReadButton);
	_rightVBox.pack_start(_memoryReadButton);
	Gtk::RadioButton::Group group;
	_directReadButton.set_group(group);
	_indirectReadButton.set_group(group);
	_memoryReadButton.set_group(group);
	_directReadButton.set_active(true);

	_rightVBox.pack_start(_readUVWButton);
	_readUVWButton.set_active(true);

	_rightVBox.pack_start(_loadOptimizedStrategy);
	_loadOptimizedStrategy.set_active(true);

	_rightVBox.pack_start(_bottomButtonBox);

	_topHBox.pack_start(_leftVBox);
	_topHBox.pack_start(_rightVBox);

	add(_topHBox);
	show_all();
}

MSOptionWindow::~MSOptionWindow()
{
}

void MSOptionWindow::initDataTypeButtons()
{
	Gtk::RadioButton::Group group = _observedDataButton.get_group();
	_correctedDataButton.set_group(group);
	_modelDataButton.set_group(group);
	_residualDataButton.set_group(group);
	_otherColumnButton.set_group(group);

	_dataKindBox.pack_start(_observedDataButton);
	_dataKindBox.pack_start(_correctedDataButton);
	_dataKindBox.pack_start(_modelDataButton);
	_dataKindBox.pack_start(_residualDataButton);
	
	_otherColumnBox.pack_start(_otherColumnButton);
	_otherColumnBox.pack_start(_otherColumnEntry);
	_dataKindBox.pack_start(_otherColumnBox);

	_dataKindFrame.add(_dataKindBox);

	_leftVBox.pack_start(_dataKindFrame);
}

void MSOptionWindow::initPolarisationButtons()
{
	Gtk::RadioButton::Group group = _allDipolePolarisationButton.get_group();
	_autoDipolePolarisationButton.set_group(group);
	_stokesIPolarisationButton.set_group(group);

	_polarisationBox.pack_start(_allDipolePolarisationButton);
	_polarisationBox.pack_start(_autoDipolePolarisationButton);
	_polarisationBox.pack_start(_stokesIPolarisationButton);

	_polarisationFrame.add(_polarisationBox);
	_leftVBox.pack_start(_polarisationFrame);
}

void MSOptionWindow::onOpen()
{
	std::cout << "Opening " << _filename << std::endl;
	try
	{
		BaselineIOMode ioMode = DirectReadMode;
		if(_indirectReadButton.get_active()) ioMode = IndirectReadMode;
		else if(_memoryReadButton.get_active()) ioMode = MemoryReadMode;
		bool readUVW = _readUVWButton.get_active();
		rfiStrategy::ImageSet *imageSet = rfiStrategy::ImageSet::Create(_filename, ioMode);
		if(dynamic_cast<rfiStrategy::MSImageSet*>(imageSet) != 0)
		{
			rfiStrategy::MSImageSet *msImageSet = static_cast<rfiStrategy::MSImageSet*>(imageSet);
			msImageSet->SetSubtractModel(false);
			if(_observedDataButton.get_active())
				msImageSet->SetDataColumnName("DATA");
			else if(_correctedDataButton.get_active())
				msImageSet->SetDataColumnName("CORRECTED_DATA");
			else if(_modelDataButton.get_active())
				msImageSet->SetDataColumnName("MODEL_DATA");
			else if(_residualDataButton.get_active())
			{
				msImageSet->SetDataColumnName("DATA");
				msImageSet->SetSubtractModel(true);
			}
			else if(_otherColumnButton.get_active())
				msImageSet->SetDataColumnName(_otherColumnEntry.get_text());
	
			if(_allDipolePolarisationButton.get_active())
				msImageSet->SetReadAllPolarisations();
			else if(_autoDipolePolarisationButton.get_active())
				msImageSet->SetReadDipoleAutoPolarisations();
			else
				msImageSet->SetReadStokesI();
	
			msImageSet->SetReadUVW(readUVW);
		}
		imageSet->Initialize();
		
		if(_loadOptimizedStrategy.get_active())
		{
			rfiStrategy::DefaultStrategy::TelescopeId telescopeId;
			unsigned flags;
			double frequency, timeResolution, frequencyResolution;
			rfiStrategy::DefaultStrategy::DetermineSettings(*imageSet, telescopeId, flags, frequency, timeResolution, frequencyResolution);
			_strategyController.Strategy().RemoveAll();
			rfiStrategy::DefaultStrategy::LoadStrategy(
				_strategyController.Strategy(),
				telescopeId,
				flags | rfiStrategy::DefaultStrategy::FLAG_GUI_FRIENDLY,
				frequency,
				timeResolution,
				frequencyResolution
			);
			_strategyController.NotifyChange();
		}
	
		_rfiGuiWindow.SetImageSet(imageSet);
	} catch(std::exception &e)
	{
		Gtk::MessageDialog dialog(*this, e.what(), false, Gtk::MESSAGE_ERROR);
		dialog.run();
	}
	hide();
}

