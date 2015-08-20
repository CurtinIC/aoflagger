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
#ifndef DIRECTBASELINEREADER_H
#define DIRECTBASELINEREADER_H

#include <map>
#include <vector>
#include <stdexcept>

#include "antennainfo.h"
#include "baselinereader.h"
#include "image2d.h"
#include "mask2d.h"
#include "measurementset.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class DirectBaselineReader : public BaselineReader {
	public:
		explicit DirectBaselineReader(const std::string &msFile);
		~DirectBaselineReader();

		virtual void PerformReadRequests();
		virtual void PerformFlagWriteRequests();
		virtual void PerformDataWriteTask(std::vector<Image2DCPtr> /*_realImages*/, std::vector<Image2DCPtr> /*_imaginaryImages*/, int /*antenna1*/, int /*antenna2*/, int /*spectralWindow*/, unsigned /*sequenceId*/)
		{
			throw std::runtime_error("The direct baseline reader can not write data back to file: use the indirect reader");
		}
		std::vector<UVW> ReadUVW(unsigned antenna1, unsigned antenna2, unsigned spectralWindow, unsigned sequenceId);
		void ShowStatistics();
	private:
		class BaselineCacheIndex
		{
		public:
			BaselineCacheIndex() { }
			BaselineCacheIndex(const BaselineCacheIndex &source) :
				antenna1(source.antenna1), antenna2(source.antenna2),
				spectralWindow(source.spectralWindow), sequenceId(source.sequenceId)
			{
			}
			void operator=(const BaselineCacheIndex &source)
			{
				antenna1 = source.antenna1;
				antenna2 = source.antenna2;
				spectralWindow = source.spectralWindow;
				sequenceId = source.sequenceId;
			}
			bool operator==(const BaselineCacheIndex &rhs) const
			{
				return
					antenna1 == rhs.antenna1 &&
					antenna2 == rhs.antenna2 &&
					spectralWindow == rhs.spectralWindow &&
					sequenceId == rhs.sequenceId;
			}
			bool operator<(const BaselineCacheIndex &rhs) const
			{
				if(antenna1 < rhs.antenna1)
					return true;
				else if(antenna1 == rhs.antenna1)
				{
					if(antenna2 < rhs.antenna2)
						return true;
					else if(antenna2 == rhs.antenna2)
					{
						if(spectralWindow < rhs.spectralWindow)
							return true;
						else if(spectralWindow == rhs.spectralWindow)
							return sequenceId < rhs.sequenceId;
					}
				}
				return false;
			}
			
			int antenna1, antenna2, spectralWindow, sequenceId;
		};
		class BaselineCacheValue {
			public:
			std::vector<size_t> rows;
			BaselineCacheValue() : rows()
			{ }
			BaselineCacheValue(const BaselineCacheValue &source) : rows(source.rows)
			{ }
			void operator=(const BaselineCacheValue &source)
			{
				rows = source.rows;
			}
		};
		
		void initBaselineCache();
		
		void addRequestRows(ReadRequest request, size_t requestIndex, std::vector<std::pair<size_t, size_t> > &rows);
		void addRequestRows(FlagWriteRequest request, size_t requestIndex, std::vector<std::pair<size_t, size_t> > &rows);
		void addRowToBaselineCache(int antenna1, int antenna2, int spectralWindow, int sequenceId, size_t row);
		void readUVWData();

		void readTimeData(size_t requestIndex, size_t xOffset, int frequencyCount, const casacore::Array<casacore::Complex> data, const casacore::Array<casacore::Complex> *model);
		void readTimeFlags(size_t requestIndex, size_t xOffset, int frequencyCount, const casacore::Array<bool> flag);
		void readWeights(size_t requestIndex, size_t xOffset, int frequencyCount, const casacore::Array<float> weight);

		std::map<BaselineCacheIndex, BaselineCacheValue> _baselineCache;
};

#endif // DIRECTBASELINEREADER_H
