//==========================================================================
//   CPLACEHOLDERMOD.CC  -  header for
//                             OMNeT++
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2003 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "cplaceholdermod.h"
#include "cproxygate.h"


cPlaceHolderModule::cPlaceHolderModule(const cPlaceHolderModule& mod) :
  cModule(NULL,NULL)
{
    setName(mod.name());
    operator=(mod);
}

cPlaceHolderModule::cPlaceHolderModule(const char *name, cModule *parentmod) :
  cModule(name, parentmod)
{
}

cPlaceHolderModule::~cPlaceHolderModule()
{
}

cPlaceHolderModule& cPlaceHolderModule::operator=(const cPlaceHolderModule& mod)
{
    if (this==&mod) return *this;
    cModule::operator=( mod );
    return *this;
}


void cPlaceHolderModule::arrived(cMessage *msg,int n,simtime_t t)
{
    throw new cException(this, "internal error: arrived() called");
}

bool cPlaceHolderModule::callInitialize(int stage)
{
    // do nothing
    return false;
}

void cPlaceHolderModule::callFinish()
{
    // do nothing
}

void cPlaceHolderModule::scheduleStart(simtime_t t)
{
    // do nothing
}

void cPlaceHolderModule::deleteModule()
{
    // adjust gates that were directed here
    for (int i=0; i<gates(); i++)
    {
            cGate *g = gate(i);
            if (g && g->toGate() && g->toGate()->fromGate()==g)
               g->toGate()->setFrom( NULL );
            if (g && g->fromGate() && g->fromGate()->toGate()==g)
               g->fromGate()->setTo( NULL );
    }

    // delete module
    simulation.deleteModule( id() );
}

cGate *cPlaceHolderModule::createGateObject(const char *gname, char tp)
{
    if (tp=='I')
        return new cProxyGate(gname,tp);
    else
        return cModule::createGateObject(gname,tp);
}

//----------------------------------------
// as usual: tribute to smart linkers
#include "cfilecomm.h"
#include "cnamedpipecomm.h"
//#include "cmpicomm.h"
#include "cnosynchronization.h"
#include "cnullmessageprot.h"
#include "cidealsimulationprot.h"
#include <stdio.h>
void parsim_dummy()
{
    cFileCommunications fc;
    cNamedPipeCommunications npc;
    //cMPICommunications mc;
    cNoSynchronization ns;
    cNullMessageProtocol np;
    cIdealSimulationProtocol ip;
    // prevent "unused variable" warnings:
    printf("%p%p%p%p%p",&fc,&npc,&ns,&np,&ip);
}
