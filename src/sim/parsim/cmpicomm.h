//=========================================================================
//
// CMPICOMM.H - part of
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


#include "parsim/cparsimcomm.h"

#ifndef __CMPICOMM_H__
#define __CMPICOMM_H__


/**
 * MPI implementation of the basic parallel simulation communications layer.
 *
 * @ingroup Parsim
 */
class cMPICommunications : public cParsimCommunications
{
  protected:
    cCommBuffer *recycledBuffer;
    int numPartitions;
    int myRank;

  public:
    /**
     * Constructor.
     */
    cMPICommunications();

    /**
     * Destructor calls MPI_Finalize()
     */
    virtual ~cMPICommunications();

    /** @name Redefined methods from cParsimCommunications */
    //@{
    /**
     * Init the library.
     */
    virtual void init();

    /**
     * Shutdown the communications library.
     */
    virtual void shutdown();

    /**
     * Returns total number of partitions.
     */
    virtual int getNumPartitions();

    /**
     * Returns the MPI rank of this partition.
     */
    virtual int getProcId();

    /**
     * Creates an empty buffer of type cMPICommBuffer.
     */
    virtual cCommBuffer *createCommBuffer();

    /**
     * Recycle communication buffer after use.
     */
    virtual void recycleCommBuffer(cCommBuffer *buffer);

    /**
     * Sends packed data with given tag to destination.
     */
    virtual void send(cCommBuffer *buffer, int tag, int destination);

    /**
     * Sends packed data with given tag to all partitions.
     */
    virtual void broadcast(cCommBuffer *buffer, int tag);

    /**
     * Receives packed data with given tag from given destination.
     */
    virtual void receiveBlocking(cCommBuffer *buffer, int& receivedTag, int& sourceProcId);

    /**
     * Receives packed data with given tag from given destination.
     * Call is non-blocking -- it returns true if something has been
     * received, false otherwise.
     */
    virtual bool receiveNonblocking(cCommBuffer *buffer,  int& receivedTag, int& sourceProcId);

    /**
     * Blocks until all partitions call the same method.
     */
    virtual void synchronize();
    //@}
};

#endif


