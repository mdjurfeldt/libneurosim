/*
 *  connection_generator.cpp
 *
 *  This file is part of libneurosim.
 *
 *  Copyright (C) 2016 INCF
 *
 *  libneurosim is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libneurosim is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "connection_generator_V2_0.h"

#include <iostream>
#include <sstream> 
#include <fstream>
#include <stdexcept>
#include <map>
#include <ltdl.h>

static const char E_NO_THREADS[] = "Context doesn't support threads.";

void
CGEN::V2_0::Context::initLock (Lock* lock)
{
  (void) lock;
  throw std::logic_error (E_NO_THREADS);
}


void
CGEN::V2_0::Context::lock (Lock* lock)
{
  (void) lock;
  throw std::logic_error (E_NO_THREADS);
}


void
CGEN::V2_0::Context::unlock (Lock* lock)
{
  (void) lock;
  throw std::logic_error (E_NO_THREADS);
}


void
CGEN::V2_0::Context::destroyLock (Lock* lock)
{
  (void) lock;
  throw std::logic_error (E_NO_THREADS);
}


void
CGEN::V2_0::Context::barrier ()
{
  throw std::logic_error (E_NO_THREADS);
}


#if 0
/**
 * Default implementation of size ()
 */
int
CGEN::V2_0::ConnectionGenerator::size ()
{
  start ();
  int i, j;
  double* vals = new double[arity ()];
  int n;
  for (n = 0; next (i, j, vals); ++n);
  delete[] vals;
  return n;
}
#endif

#ifdef CONNECTION_GENERATOR_DEBUG
/**
 * Default implementation of size ()
 */
class DummyGenerator : public ConnectionGenerator {
public:
  int arity () { return 2; }
  void setMask (std::vector<Mask>&, int) { }
  void start () { }
  bool next (int&, int&, double*) { return false; }
};

ConnectionGenerator* makeDummyConnectionGenerator ()
{
  return new DummyGenerator ();
}

#endif

// The following implementation is INCOMPLETE

static lt_dlhandle
loadLibrary (std::string library)
{
  static bool dlInitialized;
  if (!dlInitialized)
    {
      if (lt_dlinit ())
	{
	  //*fixme* Add proper error handling
	  std::cerr << "Couldn't initialize libltdl" << std::endl;
	  abort ();
	}
      dlInitialized = true;
    }
  lt_dlhandle dl = lt_dlopen (library.c_str ());

  if (!dl)
    {
      //*fixme* Add proper error handling
      std::cerr << "Couldn't load library " << library << std::endl;
      abort ();
    }

  return dl;
}

struct CGImplementation {
  CGImplementation (CGEN::V2_0::ParseCGFunc pcg,
		    CGEN::V2_0::ParseCGFunc pcgFile, 
		    CGEN::V2_0::ParseCGCFunc pcgc,
		    CGEN::V2_0::ParseCGCFunc pcgcFile)
    : CGFromXML (pcg),
      CGFromXMLFile (pcgFile),
      CGCFromXML (pcgc),
      CGCFromXMLFile (pcgcFile){ }
  CGImplementation () { }
  CGEN::V2_0::ParseCGFunc CGFromXML;
  CGEN::V2_0::ParseCGFunc CGFromXMLFile;
  CGEN::V2_0::ParseCGCFunc CGCFromXML;
  CGEN::V2_0::ParseCGCFunc CGCFromXMLFile;
};

typedef std::map<std::string, CGImplementation> tagRegistryT;

static tagRegistryT tagRegistry;

typedef std::map<std::string, CGImplementation> libraryRegistryT;

static libraryRegistryT libraryRegistry;

void
CGEN::V2_0::ConnectionGenerator::selectCGImplementation (std::string tag,
							 std::string library)
{
  loadLibrary (library);

  if (libraryRegistry.find (library) == libraryRegistry.end ())
    {
      //*fixme* Add proper error handling
      std::cerr << "Library " << library << " not registered" << std::endl;
      abort ();
    }
  tagRegistry.insert (std::make_pair(tag, libraryRegistry[library]));
}

CGEN::V2_0::ConnectionGenerator*
CGEN::V2_0::ConnectionGenerator::fromXML (std::string xml)
{
  std::stringstream xmlStream(xml);
  std::string tag = ParseXML(xmlStream);
  if (tag.empty())
    {
      //*fixme* Add proper error handling
      std::cerr << "fromXML: no tag found in the given xml string" << std::endl;
      abort ();
    }

  tagRegistryT::iterator pos = tagRegistry.find (tag);
  if (pos == tagRegistry.end ())
    {
      //*fixme* Add proper error handling
      std::cerr << "fromXML: implementation for tag " << tag << " not selected"
		<< std::endl;
      abort ();
    }
  ParseCGFunc parseCG = pos->second.CGFromXML;
  return parseCG (xml);
}

CGEN::V2_0::ConnectionGenerator*
CGEN::V2_0::ConnectionGenerator::fromXMLFile (std::string fname)
{
  std::ifstream xmlFile(fname.c_str());
  if (!xmlFile)
    {
      //*fixme* Add proper error handling
      std::cerr << "fromXML: file '" << fname << "' not found" << std::endl;
      abort ();
    }

  std::stringstream xmlStream;
  xmlStream << xmlFile.rdbuf();
  xmlFile.close();
  std::string tag = ParseXML(xmlStream);
  if (tag.empty())
    {
      //*fixme* Add proper error handling
      std::cerr << "fromXML: no tag found in the given xml file" << std::endl;
      abort ();
    }

  tagRegistryT::iterator pos = tagRegistry.find (tag);
  if (pos == tagRegistry.end ())
    {
      //*fixme* Add proper error handling
      std::cerr << "fromXMLFile: implementation for tag " << tag
		<< " not selected" << std::endl;
      abort ();
    }
  ParseCGFunc parseCG = pos->second.CGFromXMLFile;
  return parseCG (fname);
}

CGEN::V2_0::ConnectionGeneratorClosure*
CGEN::V2_0::ConnectionGeneratorClosure::fromXML (std::string xml)
{
  (void) xml;
  return 0;
}

CGEN::V2_0::ConnectionGeneratorClosure*
CGEN::V2_0::ConnectionGeneratorClosure::fromXMLFile (std::string fname)
{
  (void) fname;
  return 0;
}

std::string
CGEN::V2_0::ConnectionGenerator::ParseXML(std::stringstream& xmlStream)
{
  std::string token;

  while (xmlStream >> token)
    {
      if (token.substr(0, 1) == "<")
        {
          bool is_starttag = token.substr(0, 5) == "<?xml";
          bool is_comment = token.substr(0, 4) == "<!--";
          if (!is_starttag && !is_comment)
            {
              size_t len = token.size();
              if (token.substr(len - 1, len) == ">")
                  return token.substr(1, len - 2); // skip the closing >
              else
                  return token.substr(1, std::string::npos);
            }
        }
    }

  return "";
}

void
CGEN::V2_0::registerConnectionGeneratorLibrary (std::string library,
						ParseCGFunc pcg,
						ParseCGFunc pcgFile,
						ParseCGCFunc pcgc,
						ParseCGCFunc pcgcFile)
{
  libraryRegistry.insert (std::make_pair (library,
					  CGImplementation (pcg,
							    pcgFile,
							    pcgc,
							    pcgcFile)));
}
