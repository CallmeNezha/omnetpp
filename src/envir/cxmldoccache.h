//==========================================================================
//   XMLDOCCACHE.H - header for
//                             OMNeT++
//            Discrete System Simulation in C++
//
//  Declaration of the following classes:
//    cXMLDocCache : reads and caches XML config files
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2004 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __XMLDOCCACHE_H
#define __XMLDOCCACHE_H

#include <map>
#include <string>
#include "defs.h"
#include "envdefs.h"
#include "cxmlelement.h"

/**
 * Reads and caches XML config files.
 */
class ENVIR_API cXMLDocCache : public cPolymorphic
{
  protected:
    typedef std::map<std::string,cXMLElement*> XMLDocMap;
    XMLDocMap cache;
    cXMLElement *parseDocument(const char *filename);

  public:
    /**
     * Constructor
     */
    cXMLDocCache();

    /**
     * Destructor
     */
    virtual ~cXMLDocCache();

    /**
     * Returns the given document.
     */
    virtual cXMLElement *getDocument(const char *filename);
};

#endif

