#ifndef AOFLAGGER_IMAGE_ASSERTER_H
#define AOFLAGGER_IMAGE_ASSERTER_H

#include <string>
#include <stdexcept>
#include <sstream>

#include "../../structures/image2d.h"
#include "equalsasserter.h"

class ImageAsserter {
	public:
		static void AssertEqual(const Image2DCPtr &actual, const Image2DCPtr &expected, const std::string &str)
		{
			if(actual->Width() != expected->Width())
				throw std::runtime_error("Width of images do not match");
			if(actual->Height() != expected->Height())
				throw std::runtime_error("Height of images do not match");
			
			std::stringstream s;
			s << "ImageAsserter::AssertEqual failed for test '" << str << "': ";
			
			size_t errCount = 0;
			for(size_t y=0;y<actual->Height();++y)
			{
				for(size_t x=0;x<actual->Width();++x)
				{
					if(!EqualsAsserter::IsAlmostEqual(actual->Value(x, y), expected->Value(x, y)))
					{
						if(errCount < 25)
						{
							if(errCount != 0) s << ", ";
							s
							<< "sample (" << x << ',' << y << "), expected " << expected->Value(x,y)
							<< ", actual " << actual->Value(x,y);
						} else if(errCount == 25)
						{
							s << ", ...";
						}
						++errCount;
					}
				}
			}
			if(errCount != 0)
			{
				s << ". " << errCount << " errors.";
				throw std::runtime_error(s.str());
			}
		}
		
		static void AssertConstant(const Image2DCPtr &actual, const num_t &expected, const std::string &str)
		{
			std::stringstream s;
			s << "ImageAsserter::AssertConstant failed for test '" << str << "': ";
			
			size_t errCount = 0;
			for(size_t y=0;y<actual->Height();++y)
			{
				for(size_t x=0;x<actual->Width();++x)
				{
					if(!EqualsAsserter::IsAlmostEqual(actual->Value(x, y), expected))
					{
						if(errCount < 25)
						{
							if(errCount != 0) s << ", ";
							s
							<< "sample (" << x << ',' << y << "), expected " << expected
							<< ", actual " << actual->Value(x,y);
						} else if(errCount == 25)
						{
							s << ", ...";
						}
						++errCount;
					}
				}
			}
			if(errCount != 0)
			{
				s << ". " << errCount << " errors.";
				throw std::runtime_error(s.str());
			}
		}
		
		static void AssertFinite(const Image2DCPtr &actual, const std::string &str)
		{
			std::stringstream s;
			s << "ImageAsserter::AssertFinite failed for test '" << str << "': ";
			
			size_t errCount = 0;
			for(size_t y=0;y<actual->Height();++y)
			{
				for(size_t x=0;x<actual->Width();++x)
				{
					if(!std::isfinite(actual->Value(x, y)))
					{
						if(errCount < 25)
						{
							if(errCount != 0) s << ", ";
							s << "sample (" << x << ',' << y << ") was not finite (" << actual->Value(x,y) << ")";
						} else if(errCount == 25)
						{
							s << ", ...";
						}
						++errCount;
					}
				}
			}
			if(errCount != 0)
			{
				s << ". " << errCount << " errors.";
				throw std::runtime_error(s.str());
			}
		}
};

#endif
