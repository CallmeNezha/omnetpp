//=========================================================================
//
// CPARSIMSEGMENT.H - part of
//                          OMNeT++
//           Discrete System Simulation in C++
//
//   Written by:  Andras Varga, 2003
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2001 Eric Wu
  Monash University, Dept. of Electrical and Computer Systems Eng.
  Melbourne, Australia

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/


#ifndef __CPARSIMSEGMENT_H__
#define __CPARSIMSEGMENT_H__

#include "defs.h"

// forward declarations:
class cSimulation;
class cParsimSynchronizer;
class cParsimCommunications;
class cCommBuffer;


/**
 * Represents one partition in a parallel simulation. Knows about
 * partitions and the links between this partition and its neighbours.
 *
 * Interconnections are stored not directly inside this object,
 * but in cProxyGate's of cPlaceHolderModule's. The remote address fields
 * of cProxyGate's are filled out here in the connectRemoteGates() method.
 *
 * This layer also handles generic (synchronization-independent part of)
 * communication with other partitions:
 *
 * - cMessages outgoing from this partition flow through here
 *   (processOutgoingMessage()), and
 *
 * - incoming messages are processed at here by processReceivedBuffer()
 *   (invoked from the synchronization layer, cParsimSynchronizer)
 *
 * This layer is communication library independent (i.e it contains
 * no MPI, PVM, etc. calls) -- it builds on the abstraction layer provided
 * by cParsimCommunications.
 *
 * @ingroup Parsim
 */
class cParsimPartition
{
  protected:
    cSimulation *sim;
    cParsimCommunications *comm;
    cParsimSynchronizer *synch;
    bool debug;

  protected:
    // internal: fills in remote gate addresses of all cProxyGate's in the current partition
    void connectRemoteGates();

  public:
    /**
     * Constructor.
     */
    cParsimPartition();

    /**
     * Virtual destructor.
     */
    virtual ~cParsimPartition();

    /**
     * Pass cParsimPartition the objects it has to cooperate with.
     */
    void setContext(cSimulation *sim, cParsimCommunications *comm, cParsimSynchronizer *synch);

    /**
     * Called at the beginning of a simulation run. Fills in remote gate addresses
     * of all cProxyGate's in the current partition.
     */
    void startRun();

    /**
     * Called at the end of a simulation run.
     */
    void endRun();

    /**
     * A hook called from cProxyGate::deliver() when an outgoing cMessage
     * arrives at partition boundary. We just pass it up to the synchronization
     * layer (see similar method in cParsimSynchronizer).
     */
    virtual void processOutgoingMessage(cMessage *msg, int procId, int moduleId, int gateId, void *data);

    /**
     * Process messages coming from other partitions. This method is called from
     * the synchronization layer (see cParsimSynchronizer), after it has
     * processed all tags that it understands (namely, cMessages
     * (tag=TAG_CMESSAGE) and all tags used by the synchronization protocol).
     */
    virtual void processReceivedBuffer(cCommBuffer *buffer, int tag, int sourceProcId);

    /**
     * Process cMessages received from other partitions. This method is called from
     * the synchronization layer (see cParsimSynchronizer) when it received
     * a message from other partitions. This method checks that the destination
     * module/gate still exists, sets the source module/gate to the appropriate
     * placeholder module, and inserts the message into the FES.
     */
    virtual void processReceivedMessage(cMessage *msg, int destModuleId, int destGateId, int sourceProcId);

    /**
     * Called when a cTerminationException occurs (i.e. the simulation is
     * about to be finished normally), this methods notifies other partitions
     * about the exception.
     */
    virtual void broadcastTerminationException(cTerminationException *e);

    /**
     * Called when a cException occurs (i.e. the simulation is about to be
     * stopped with an error), this methods notifies other partitions
     * about the exception.
     */
    virtual void broadcastException(cException *e);
};

#endif

