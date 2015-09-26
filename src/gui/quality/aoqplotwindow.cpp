#include "aoqplotwindow.h"

#include <limits>

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>

#include "../../structures/measurementset.h"

#include "../../quality/histogramtablesformatter.h"
#include "../../quality/histogramcollection.h"
#include "../../quality/statisticscollection.h"

#include "../../remote/clusteredobservation.h"
#include "../../remote/processcommander.h"

AOQPlotWindow::AOQPlotWindow() :
	_isOpen(false)
{
	set_default_icon_name("aoqplot");
	
	_notebook.append_page(_baselinePlotPage, "Baselines");
	_baselinePlotPage.SignalStatusChange().connect(sigc::mem_fun(*this, &AOQPlotWindow::onStatusChange));
	
	_notebook.append_page(_antennaePlotPage, "Antennae");
	_notebook.append_page(_bLengthPlotPage, "Baselines length");
	_notebook.append_page(_timePlotPage, "Time");
	_notebook.append_page(_frequencyPlotPage, "Frequency");
	_notebook.append_page(_timeFrequencyPlotPage, "Time-frequency");
	_timeFrequencyPlotPage.SignalStatusChange().connect(sigc::mem_fun(*this, &AOQPlotWindow::onStatusChange));
	//_timeFrequencyPlotPage.set_sensitive(false);
	
	_notebook.append_page(_summaryPage, "Summary");
	
	_notebook.append_page(_histogramPage, "Histograms");
	
	_vBox.pack_start(_notebook);
	_notebook.signal_switch_page().connect(sigc::mem_fun(*this, &AOQPlotWindow::onSwitchPage));
	
	_vBox.pack_end(_statusBar, Gtk::PACK_SHRINK);
	_statusBar.push("Quality plot util is ready. Author: André Offringa (offringa@gmail.com)");
	
	add(_vBox);
	
	_openOptionsWindow.SignalOpen().connect(sigc::mem_fun(*this, &AOQPlotWindow::onOpenOptionsSelected));
	signal_hide().connect(sigc::mem_fun(*this, &AOQPlotWindow::onHide));
}

void AOQPlotWindow::Open(const std::vector<std::string> &files)
{
	show_all();
	_openOptionsWindow.ShowForFile(files);
}

void AOQPlotWindow::onOpenOptionsSelected(const std::vector<std::string>& files, bool downsampleTime, bool downsampleFreq, size_t timeCount, size_t freqCount, bool correctHistograms)
{
	readStatistics(files, downsampleTime, downsampleFreq, timeCount, freqCount, correctHistograms);
	_baselinePlotPage.SetStatistics(_statCollection, _antennas);
	_antennaePlotPage.SetStatistics(_statCollection, _antennas);
	_bLengthPlotPage.SetStatistics(_statCollection, _antennas);
	_timePlotPage.SetStatistics(_statCollection, _antennas);
	_frequencyPlotPage.SetStatistics(_statCollection, _antennas);
	_timeFrequencyPlotPage.SetStatistics(_fullStats);
	_summaryPage.SetStatistics(_statCollection);
	if(_histogramPage.get_visible())
		_histogramPage.SetStatistics(*_histCollection);
	show();
}

void AOQPlotWindow::close()
{
	if(_isOpen)
	{
		_baselinePlotPage.CloseStatistics();
		_antennaePlotPage.CloseStatistics();
		_bLengthPlotPage.CloseStatistics();
		_timePlotPage.CloseStatistics();
		_frequencyPlotPage.CloseStatistics();
		_timeFrequencyPlotPage.CloseStatistics();
		_summaryPage.CloseStatistics();
		_histogramPage.CloseStatistics();
		delete _statCollection;
		delete _histCollection;
		delete _fullStats;
		_isOpen = false;
	}
}

void AOQPlotWindow::readDistributedObservation(const std::string& filename, bool correctHistograms)
{
	aoRemote::ClusteredObservation *observation = aoRemote::ClusteredObservation::Load(filename);
	_statCollection = new StatisticsCollection();
	_histCollection = new HistogramCollection();
	aoRemote::ProcessCommander commander(*observation);
	commander.PushReadAntennaTablesTask();
	commander.PushReadQualityTablesTask(_statCollection, _histCollection, correctHistograms);
	commander.Run();
	if(!commander.Errors().empty())
	{
		std::stringstream s;
		s << commander.Errors().size() << " error(s) occured while querying the nodes or measurement sets in the given observation. This might be caused by a failing node, an unreadable measurement set, or maybe the quality tables are not available. The errors reported are:\n\n";
		size_t count = 0;
		for(std::vector<std::string>::const_iterator i=commander.Errors().begin();i!=commander.Errors().end() && count < 30;++i)
		{
			s << "- " << *i << '\n';
			++count;
		}
		if(commander.Errors().size() > 30)
		{
			s << "... and " << (commander.Errors().size()-30) << " more.\n";
		}
		s << "\nThe program will continue, but this might mean that the statistics are incomplete. If this is the case, fix the issues and reopen the observation.";
		std::cerr << s.str() << std::endl;
		Gtk::MessageDialog dialog(*this, s.str(), false, Gtk::MESSAGE_ERROR);
		dialog.run();
	}
	
	_antennas = commander.Antennas();
	
	delete observation;
}

void AOQPlotWindow::readMetaInfoFromMS(const string& filename)
{
	MeasurementSet *ms = new MeasurementSet(filename);
	_polarizationCount = ms->PolarizationCount();
	unsigned antennaCount = ms->AntennaCount();
	_antennas.clear();
	for(unsigned a=0;a<antennaCount;++a)
		_antennas.push_back(ms->GetAntennaInfo(a));
	delete ms;
}

void AOQPlotWindow::readAndCombine(const string& filename)
{
	std::cout << "Adding " << filename << " to statistics...\n";
	QualityTablesFormatter qualityTables(filename);
	StatisticsCollection statCollection(_polarizationCount);
	statCollection.Load(qualityTables);
	_statCollection->Add(statCollection);
	
	HistogramTablesFormatter histogramTables(filename);
	if(histogramTables.HistogramsExist())
	{
		HistogramCollection histCollection(_polarizationCount);
		histCollection.Load(histogramTables);
		_histCollection->Add(histCollection);
	}
}

void AOQPlotWindow::readStatistics(const std::vector<std::string>& files, bool downsampleTime, bool downsampleFreq, size_t timeSize, size_t freqSize, bool correctHistograms)
{
	close();
	
	if(!files.empty())
	{
		const std::string& firstFile = *files.begin();
		if(aoRemote::ClusteredObservation::IsClusteredFilename(firstFile))
		{
			if(files.size() != 1)
				throw std::runtime_error("You are trying to open multiple distributed or clustered sets. Can only open multiple files if they are not distributed.");
			readDistributedObservation(firstFile, correctHistograms);
		}
		else {
			readMetaInfoFromMS(firstFile);
			
			_statCollection = new StatisticsCollection(_polarizationCount);
			_histCollection = new HistogramCollection(_polarizationCount);
			
			for(size_t i=0; i!=files.size(); ++i)
			{
				std::cout << " (" << (i+1) << "/" << files.size() << ") ";
				readAndCombine(files[i]);
			}
		}
		setShowHistograms(!_histCollection->Empty());
		if(downsampleTime)
		{
			std::cout << "Lowering time resolution..." << std::endl;
			_statCollection->LowerTimeResolution(timeSize);
		}

		if(downsampleFreq)
		{
			std::cout << "Lowering frequency resolution..." << std::endl;
			_statCollection->LowerFrequencyResolution(freqSize);
		}

		std::cout << "Integrating baseline statistics to one channel..." << std::endl;
		_statCollection->IntegrateBaselinesToOneChannel();
		
		std::cout << "Regridding time statistics..." << std::endl;
		_statCollection->RegridTime();
		
		std::cout << "Copying statistics..." << std::endl;
		_fullStats = new StatisticsCollection(*_statCollection);
		
		std::cout << "Integrating time statistics to one channel..." << std::endl;
		_statCollection->IntegrateTimeToOneChannel();
		
		std::cout << "Opening statistics panel..." << std::endl;
		_isOpen = true;
	}
}

void AOQPlotWindow::onStatusChange(const std::string &newStatus)
{
	_statusBar.pop();
	_statusBar.push(newStatus);
}

void AOQPlotWindow::Save(const AOQPlotWindow::PlotSavingData& data)
{
	const std::string& prefix = data.filenamePrefix;
	QualityTablesFormatter::StatisticKind kind = data.statisticKind;
	
	std::cout << "Saving " << prefix << "-antennas.pdf...\n";
	_antennaePlotPage.SavePdf(prefix+"-antennas.pdf", kind);
	
	std::cout << "Saving " << prefix << "-baselines.pdf...\n";
	_baselinePlotPage.SavePdf(prefix+"-baselines.pdf", kind);
	
	std::cout << "Saving " << prefix << "-baselinelengths.pdf...\n";
	_bLengthPlotPage.SavePdf(prefix+"-baselinelengths.pdf", kind);
	
	std::cout << "Saving " << prefix << "-timefrequency.pdf...\n";
	_timeFrequencyPlotPage.SavePdf(prefix+"-timefrequency.pdf", kind);
	
	std::cout << "Saving " << prefix << "-time.pdf...\n";
	_timePlotPage.SavePdf(prefix+"-time.pdf", kind);
	
	std::cout << "Saving " << prefix << "-frequency.pdf...\n";
	_frequencyPlotPage.SavePdf(prefix+"-frequency.pdf", kind);
}
