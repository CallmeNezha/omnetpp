//==========================================================================
//  FSUTILS.H - part of
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//
//  Declaration of the following classes:
//    TOmnetApp:  abstract base class for simulation applications
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2004 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __FSUTILS_H
#define __FSUTILS_H

#include "util.h"
#include "opp_string.h"

/**
 * Utility function to split a file path into directory and file name parts.
 */
void splitFileName(const char *pathname, opp_string& dir, opp_string& fnameonly);

/**
 * Utility class for temporary change of directory. Changes back to
 * original dir when goes out of scope.
 */
class PushDir
{
  private:
    opp_string olddir;
  public:
    PushDir(const char *changetodir);
    ~PushDir();
};

/**
 * Utility function: dynamically loads a DLL (Windows) or shared object
 * (.so, on Unix systems) and registers the module types, channel types,
 * networks etc. in it. The file name should be given without extension;
 * ".dll" or ".so" will be appended, depending on the platform.
 *
 * On failure, cException will be thrown.
 */
void loadExtensionLibrary(const char *lib);

#endif

