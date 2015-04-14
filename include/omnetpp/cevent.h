//==========================================================================
//   CEVENT.H  -  header for
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//  Author: Andras Varga
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2008 Andras Varga
  Copyright (C) 2006-2008 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __CEVENT_H
#define __CEVENT_H

#include "cownedobject.h"

NAMESPACE_BEGIN


/**
 * Represents an event in the discrete event simulator. When events are scheduled,
 * they are inserted into the future events set (FES) where they are (conceptually)
 * stored in timestamp (="arrival time") order. Events are removed from the FES one
 * by one, and their execute() methods are called. execute() should be overridden
 * in subclasses to carry out the actions associated with the event.
 *
 * If several events have identical timestamp values (arrival times), further fields
 * decide their ordering: first, scheduling priority and then, insertion order.
 *
 * Event objects (cEvent) are normally of little interest to the user. Instead, they
 * should utilize messages and packets (cMessage, cPacket). They are subclassed from
 * cEvent, and their execute() methods automatically delivers the message/packet
 * to the target module.
 *
 * @ingroup SimCore Internals
 */
class SIM_API cEvent : public cOwnedObject
{
    friend class cMessage;  //getArrivalTime()
    friend class cMessageHeap;
  private:
    simtime_t delivd;          //XXX rename ("arrivaltime" or "delivtime)! time of sending & delivery -- set internally
    short prior;               // priority -- used for scheduling events with equal timestamps
    int heapindex;             // used by cMessageHeap (-1 if not on heap; all other values, including negative ones, means "on the heap")
    unsigned long insertordr;  //XXX rename (add "e")! used by cMessageHeap
    eventnumber_t prev_event_num; // most recent event number when envir was notified about this event object (e.g. creating/cloning/sending/scheduling/deleting of this event object)

  public:
    // internal: returns the event number which scheduled this event object, or the event
    // number in which this event object was last executed (e.g. delivered to a module);
    // stored for recording into the event log file.
    eventnumber_t getPreviousEventNumber() const {return prev_event_num;}

    // internal: sets previousEventNumber.
    void setPreviousEventNumber(eventnumber_t num) {prev_event_num = num;}

    // internal: used by cMessageHeap.
    unsigned long getInsertOrder() const {return insertordr;}

    // internal: called by the simulation kernel to set the value returned
    // by the getArrivalTime() method
    void setArrivalTime(simtime_t t) {delivd = t;}

    // internal: used by the parallel simulation kernel.
    virtual int getSrcProcId() const {return -1;}

  public:
    /** @name Constructors, destructor, assignment */
    //@{

    /**
     * Copy constructor.
     */
    cEvent(const cEvent& event);

    /**
     * Constructor.
     */
    explicit cEvent(const char *name);

    /**
     * Destructor.
     */
    virtual ~cEvent();

    /**
     * Assignment operator. Duplication and the assignment operator work all right with cEvent.
     * The name member doesn't get copied; see cNamedObject's operator=() for more details.
     */
    cEvent& operator=(const cEvent& event);
    //@}

    /** @name Redefined cObject member functions. */
    //@{
    /**
     * Redefined to override return type. See cObject for more details.
     */
    virtual cEvent *dup() const = 0;

    /**
     * Produces a one-line description of the object's contents.
     * See cObject for more details.
     */
    virtual std::string info() const;

    /**
     * Produces a multi-line description of the object's contents.
     * See cObject for more details.
     */
    virtual std::string detailedInfo() const;

    /**
     * Calls v->visit(this) for each contained object.
     * See cObject for more details.
     */
    virtual void forEachChild(cVisitor *v);

    /**
     * Serializes the object into an MPI send buffer
     * Used by the simulation kernel for parallel execution.
     * See cObject for more details.
     */
    virtual void parsimPack(cCommBuffer *buffer) const;

    /**
     * Deserializes the object from an MPI receive buffer
     * Used by the simulation kernel for parallel execution.
     * See cObject for more details.
     */
    virtual void parsimUnpack(cCommBuffer *buffer);
    //@}

    /** @name Event attributes. */
    //@{

    /**
     * Sets the scheduling priority of this event. Scheduling priority is
     * used when the simulator inserts messages into the future events set
     * (FES), to order events with identical arrival time values.
     */
    void setSchedulingPriority(short p)  {prior=p;}

    /**
     * Returns the scheduling priority of this event.
     */
    short getSchedulingPriority() const  {return prior;}
    //@}

    /** @name Arrival information. */
    //@{

    /**
     * Returns true if this event is in the future events set (FES).
     */
    bool isScheduled() const  {return heapindex!=-1;}

    /**
     * Returns the simulation time this event object has been last scheduled for
     * (regardless whether it is currently scheduled), or zero if the event
     * hasn't been scheduled yet.
     */
    simtime_t_cref getArrivalTime() const  {return delivd;}

    /**
     * Return the object that this event will be delivered to or act upon,
     * or NULL if there is no such object. For messages and packets this will
     * be the destination module. This method is not used by the simulation
     * kernel for other than informational purposes, e.g. logging.
     *
     * @see cMessage::getArrivalModule()
     */
    virtual cObject *getTargetObject() const = 0;
    //@}

    /** @name Methods to be used by the simulation kernel and the scheduler. */
    //@{
    /**
     * A fast way (that is, faster than dynamic_cast) to determine whether this
     * event is a cMessage.
     */
    virtual bool isMessage() const {return false;}

    /**
     * Returns true if this event is stale. An event might go stale while
     * staying in the future events set (FES), for example due to its target
     * object being deleted. Stale events are discarded by the scheduler.
     */
    virtual bool isStale() {return false;}

    /**
     * This method performs the action associated with the event. When a
     * scheduled event makes it to the front of the FES, it is removed
     * from the FES and its execute() method is invoked. In cMessage,
     * execute() ends up calling the handleMessage() method of the destination
     * module, or switches to the coroutine of its activity() method.
     */
    virtual void execute() = 0;
    //@}
};

NAMESPACE_END

#endif
