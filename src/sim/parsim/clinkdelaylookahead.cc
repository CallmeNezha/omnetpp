//=========================================================================
//
// CLINKDELAYLOOKAHEAD.CC - part of
//                          OMNeT++
//           Discrete System Simulation in C++
//
//   Written by:  Andras Varga, 2003
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2003 Andras Varga
  Monash University, Dept. of Electrical and Computer Systems Eng.
  Melbourne, Australia

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/


#include "clinkdelaylookahead.h"
#include "csimul.h"
#include "cmessage.h"
#include "cenvir.h"
#include "cnullmessageprot.h"
#include "cparsimcomm.h"
#include "cparsimpartition.h"
#include "cplaceholdermod.h"
#include "cproxygate.h"
#include "cchannel.h"
#include "macros.h"


Register_Class(cLinkDelayLookahead);


cLinkDelayLookahead::cLinkDelayLookahead()
{
    numSeg = 0;
    segInfo = NULL;
}

cLinkDelayLookahead::~cLinkDelayLookahead()
{
    delete [] segInfo;
}

void cLinkDelayLookahead::startRun()
{
    ev << "starting Link Delay Lookahead...\n";

    delete [] segInfo;

    numSeg = comm->getNumPartitions();
    segInfo = new PartitionInfo[numSeg];
    int myProcId = comm->getProcId();

    // temporarily initialize everything to zero.
    int i;
    for (i=0; i<numSeg; i++)
        segInfo[i].minDelay = 0;

    // fill in minDelays
    ev << "  calculating minimum link delays..." << endl;
    for (int modId=0; modId<=sim->lastModuleId(); modId++)
    {
        cPlaceHolderModule *mod = dynamic_cast<cPlaceHolderModule *>(sim->module(modId));
        if (mod)
        {
            for (int gateId=0; gateId<mod->gates(); gateId++)
            {
                // if this is a properly connected proxygate, process it
                // FIXME: leave out gates from other cPlaceHolderModules!!!
                cGate *g = mod->gate(gateId);
                cProxyGate *pg  = dynamic_cast<cProxyGate *>(g);
                if (pg && pg->fromGate() && pg->getRemoteProcId()>=0)
                {
                    // check we have a delay on this link (it gives us lookahead)
                    cGate *fromg  = pg->fromGate();
                    cChannel *chan = fromg ? fromg->channel() : NULL;
                    cSimpleChannel *simplechan = dynamic_cast<cSimpleChannel *>(chan);
                    cPar *delaypar = simplechan ? simplechan->delay() : NULL;
                    double linkDelay = delaypar ? delaypar->doubleValue() : 0;
                    if (linkDelay<=0.0)
                        throw new cException("cLinkDelayLookahead: zero delay on link from gate `%s', no lookahead for parallel simulation", fromg->fullPath());

                    // store
                    int procId = pg->getRemoteProcId();
                    if (segInfo[procId].minDelay==0 || segInfo[procId].minDelay<linkDelay)
                        segInfo[procId].minDelay = linkDelay;
                }
            }
        }
    }

    for (i=0; i<numSeg; i++)
        if (i!=myProcId)
            ev << "    lookahead to procId=" << i << " is " << simtimeToStr(segInfo[i].minDelay) << endl;

    ev << "  setup done.\n";
}

void cLinkDelayLookahead::endRun()
{
    delete [] segInfo;
    segInfo = NULL;
}

double cLinkDelayLookahead::getCurrentLookahead(cMessage *, int procId, void *)
{
    return segInfo[procId].minDelay;
}

double cLinkDelayLookahead::getCurrentLookahead(int procId)
{
    return segInfo[procId].minDelay;
}



