//==========================================================================
//   FILEMGRS.CC - header for
//                             OMNeT++
//            Discrete System Simulation in C++
//
//  Members of the following classes:
//     cFileOutputVectorManager
//     cFileOutputScalarManager
//     cFileSnapshotManager
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2004 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "defs.h"

#include <assert.h>
#include <string.h>
#include <fstream>
#include "cenvir.h"
#include "omnetapp.h"
#include "csimul.h"
#include "cmodule.h"
#include "cstat.h"
#include "macros.h"
#include "filemgrs.h"

using std::ostream;
using std::ofstream;
using std::ios;


Register_Class(cFileOutputVectorManager);

#define CHECK(fprintf)    if (fprintf<0) throw new cException(eOUTVECT)

// helper function
static void createFileName(opp_string& fname, int run_no, const char *configentry, const char *defaultval)
{
    // get file name from ini file
    char section[16];
    sprintf(section,"Run %d",run_no);

    fname = ev.config()->getAsString2(section,"General",configentry,defaultval);
    ev.app->processFileName(fname);
}


cFileOutputVectorManager::cFileOutputVectorManager()
{
    nextid = 0;
    f = NULL;
}

cFileOutputVectorManager::~cFileOutputVectorManager()
{
    closeFile();
}

void cFileOutputVectorManager::openFile()
{
    f = fopen(fname.c_str(),"a");
    if (f==NULL)
        throw new cException("Cannot open output file `%s'",fname.c_str());
}

void cFileOutputVectorManager::closeFile()
{
    if (f)
    {
        fclose(f);
        f = NULL;
    }
}

void cFileOutputVectorManager::initVector(sVectorData *vp)
{
    if (!f)
    {
        openFile();
        if (!f) return;
    }

    CHECK(fprintf(f,"vector %ld  \"%s\"  \"%s\"  %d\n",
                  vp->id, vp->modulename.c_str(), vp->vectorname.c_str(), vp->tuple));
    vp->initialised = true;
}

void cFileOutputVectorManager::startRun()
{
    // clean up file from previous runs
    closeFile();
    createFileName(fname, simulation.runNumber(), "output-vector-file", "omnetpp.vec");
    remove(fname.c_str());
}

void cFileOutputVectorManager::endRun()
{
    closeFile();
}

void *cFileOutputVectorManager::registerVector(const char *modulename, const char *vectorname, int tuple)
{
    sVectorData *vp = createVectorData();
    vp->tuple = tuple;
    vp->id = nextid++;
    vp->initialised = false;
    vp->modulename = modulename;
    vp->vectorname = vectorname;

    ev.app->getOutVectorConfig(simulation.runNumber(), modulename, vectorname,
                               vp->enabled, vp->starttime, vp->stoptime);
    return vp;
}

cFileOutputVectorManager::sVectorData *cFileOutputVectorManager::createVectorData()
{
    return new sVectorData;
}

void cFileOutputVectorManager::deregisterVector(void *vectorhandle)
{
    sVectorData *vp = (sVectorData *)vectorhandle;
    delete vp;
}

bool cFileOutputVectorManager::record(void *vectorhandle, simtime_t t, double value)
{
    sVectorData *vp = (sVectorData *)vectorhandle;

    if (!vp->enabled)
        return false;

    if (t>=vp->starttime && (vp->stoptime==0.0 || t<=vp->stoptime))
    {
        if (!vp->initialised)
            initVector(vp);
        assert(f!=NULL);
        CHECK(fprintf(f,"%ld\t%.9g\t%.9g\n",vp->id, t, value));
        return true;
    }
    return false;
}

bool cFileOutputVectorManager::record(void *vectorhandle, simtime_t t, double value1, double value2)
{
    sVectorData *vp = (sVectorData *)vectorhandle;

    if (!vp->enabled)
        return false;

    if (t>=vp->starttime && (vp->stoptime==0.0 || t<=vp->stoptime))
    {
        if (!vp->initialised)
            initVector(vp);
        assert(f!=NULL);
        CHECK(fprintf(f,"%ld\t%.9g\t%.9g\t%.9g\n",vp->id, t, value1, value2));
        return true;
    }
    return false;
}

const char *cFileOutputVectorManager::fileName() const
{
    return fname.c_str();
}


//=================================================================

Register_Class(cFileOutputScalarManager);

#define CHECK(fprintf)    if (fprintf<0) throw new cException(eOUTVECT)

cFileOutputScalarManager::cFileOutputScalarManager()
{
    f = NULL;
}

cFileOutputScalarManager::~cFileOutputScalarManager()
{
    closeFile();
}

void cFileOutputScalarManager::openFile()
{
    f = fopen(fname.c_str(),"a");
    if (f==NULL)
        throw new cException("Cannot open output file `%s'",fname.c_str());
}

void cFileOutputScalarManager::closeFile()
{
    if (f)
    {
        fclose(f);
        f = NULL;
    }
}

void cFileOutputScalarManager::startRun()
{
    // clean up file from previous runs
    closeFile();
    createFileName(fname, simulation.runNumber(), "output-scalar-file", "omnetpp.sca");
    initialized = false;
}

void cFileOutputScalarManager::endRun()
{
    closeFile();
}

void cFileOutputScalarManager::init()
{
    if (!f)
    {
        openFile();
        if (!f) return;
    }

    if (!initialized)
    {
        initialized = true;
        fprintf(f,"run %d \"%s\"\n", simulation.runNumber(), simulation.networkType()->name());
    }
}

void cFileOutputScalarManager::recordScalar(cModule *module, const char *name, double value)
{
    if (!initialized)
        init();

    if (!f) return;

    fprintf(f,"scalar \"%s\" \t\"%s\" \t%.9g\n", module->fullPath(), name? name : "(null)", value);
}

void cFileOutputScalarManager::recordScalar(cModule *module, const char *name, const char *text)
{
    if (!initialized)
        init();

    if (!f) return;

    fprintf(f,"scalar \"%s\" \t\"%s\" \t\"%s\"\n", module->fullPath(), name? name : "(null)", text);
}

void cFileOutputScalarManager::recordScalar(cModule *module, const char *name, cStatistic *stats)
{
    if (!initialized)
        init();

    if (!f) return;

    fprintf(f,"statistics \"%s\" \t\"%s\" \t\"%s\"\n", module->fullPath(), name? name : "(null)", stats->className());
    stats->saveToFile( f );
}

const char *cFileOutputScalarManager::fileName() const
{
    return fname.c_str();
}

//=================================================================

Register_Class(cFileSnapshotManager);

cFileSnapshotManager::cFileSnapshotManager()
{
}

cFileSnapshotManager::~cFileSnapshotManager()
{
}

void cFileSnapshotManager::startRun()
{
    // clean up file from previous runs
    createFileName(fname, simulation.runNumber(), "snapshot-file", "omnetpp.sna");
    remove(fname.c_str());
}

void cFileSnapshotManager::endRun()
{
}

ostream *cFileSnapshotManager::getStreamForSnapshot()
{
    ostream *os = new ofstream(fname.c_str(), ios::out|ios::app);
    return os;
}

void cFileSnapshotManager::releaseStreamForSnapshot(std::ostream *os)
{
    delete os;
}

const char *cFileSnapshotManager::fileName() const
{
    return fname.c_str();
}

