//==========================================================================
//   CMESSAGE.H  -  header for
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//
//  Declaration of the following classes:
//    cMessage : message and event object
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2004 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __CMESSAGE_H
#define __CMESSAGE_H

#include <time.h>
#include "cobject.h"
#include "carray.h"
#include "cpar.h"
#include "csimul.h"

//=== classes mentioned:
class cPar;
class cGate;
class cChannel;
class cModule;
class cSimpleModule;
class cCompoundModule;
class cSimulation;
class cMessageHeap;


/**
 * Predefined message kind values (values for cMessage's kind(),
 * setKind() methods).
 *
 * Negative values are reserved for the \opp system and its
 * standard libraries. Zero and positive values can be freely used
 * by simulation models.
 */
enum eMessageKind
{
  MK_STARTER = -1,  /// Starter message. Used by scheduleStart().
  MK_TIMEOUT = -2,  /// Internal timeout message. Used by wait(), etc.
  MK_PACKET  = -3,  /// Network packet. Used by cPacket.
  MK_INFO    = -4,  /// Information packet. Used by cPacket.

  MK_PARSIM_BEGIN = -1000  /// values -1000...-2000 reserved for parallel simulation
};

//==========================================================================

/**
 * The message class in \opp. cMessage objects may represent events,
 * messages, packets (frames, cells, etc) or other entities in a simulation.
 *
 * Messages may be: scheduled (to arrive back at the same module at a later
 * time), cancelled, sent out on a gate, or sent directly to another module;
 * all via methods of cSimpleModule.
 *
 * cMessage can be assigned a name (a property inherited from cObject);
 * other attributes include message kind, length, priority,
 * error flag and time stamp. Arrival time and gate is also stored.
 * cMessage supports encapsulation, and messages may be cloned with the
 * dup() function. The control info field facilitates modelling communication
 * between protocol layers. The context pointer field makes it easier to
 * work with several timers (self-messages) at a time.
 * A useful method is isSelfMessage() which tells apart self-messages from
 * messages received from other modules.
 *
 * Further fields can be added to cMessage via message declarations (.msg files)
 * which are translated into C++ classes. An example message declaration:
 *
 * \code
 * message NetwPkt
 * {
 *    fields:
 *        int destAddr = -1; // destination address
 *        int srcAddr = -1;  // source address
 *        int ttl =  32;     // time to live
 * }
 * \endcode
 *
 * @ingroup SimCore
 */
class SIM_API cMessage : public cObject
{
    friend class cMessageHeap;

  private:
    int msgkind;               // message kind -- <0 reserved, 0>= user-defined meaning
    int prior;                 // priority -- used for scheduling msgs with equal times
    long len;                  // length of message -- used for bit errors and transm.delay
    bool error : 1;            // bit error occurred during transmission
    unsigned char refcount : 7;// reference count for encapsulated message (0: not encapsulated, max 127)
    unsigned char srcprocid;   // reserved for use by parallel execution: id of source partition
    cArray *parlistp;          // ptr to list of parameters
    cMessage *encapmsg;        // ptr to encapsulated msg
    cPolymorphic *ctrlp;       // ptr to "control info"
    void *contextptr;          // a stored pointer -- user-defined meaning, used with self-messages

    int frommod,fromgate;      // source module and gate IDs -- set internally
    int tomod,togate;          // dest. module and gate IDs -- set internally
    simtime_t created;         // creation time -- set be constructor
    simtime_t sent,delivd;     // time of sending & delivery -- set internally
    simtime_t tstamp;          // time stamp -- user-defined meaning

    int heapindex;             // used by cMessageHeap (-1 if not on heap)
    unsigned long insertordr;  // used by cMessageHeap

    // helper function
    void _createparlist();

    // global variables for statistics
    static long total_msgs;
    static long live_msgs;

  public:
    /** @name Constructors, destructor, assignment */
    //@{

    /**
     * Copy constructor.
     */
    cMessage(const cMessage& msg);

    /**
     * Constructor.
     */
    explicit cMessage(const char *name=NULL, int k=0, long len=0, int pri=0, bool err=false);

    /**
     * Destructor.
     */
    virtual ~cMessage();

    /**
     * Assignment operator. Duplication and the assignment operator work all right with cMessage.
     * The name member doesn't get copied; see cObject's operator=() for more details.
     */
    cMessage& operator=(const cMessage& msg);
    //@}

    /** @name Redefined cObject functions. */
    //@{

    /**
     * Creates and returns an exact copy of this object.
     * See cObject for more details.
     */
    virtual cObject *dup() const  {return new cMessage(*this);}

    /**
     * Produces a one-line description of object contents into the buffer passed as argument.
     * See cObject for more details.
     */
    virtual std::string info() const;

    /**
     * Calls v->visit(this) for each contained object.
     * See cObject for more details.
     */
    virtual void forEachChild(cVisitor *v);

    /**
     * Writes textual information about this object to the stream.
     * See cObject for more details.
     */
    virtual void writeContents(std::ostream& os);

    /**
     * Serializes the object into a PVM or MPI send buffer
     * Used by the simulation kernel for parallel execution.
     * See cObject for more details.
     */
    virtual void netPack(cCommBuffer *buffer);

    /**
     * Deserializes the object from a PVM or MPI receive buffer
     * Used by the simulation kernel for parallel execution.
     * See cObject for more details.
     */
    virtual void netUnpack(cCommBuffer *buffer);
    //@}

    /** @name Message attributes. */
    //@{

    /**
     * Sets message kind.  The message kind member is not used by OMNeT++,
     * it can be used freely by the user.
     */
    void setKind(int k)     {msgkind=k;}

    /**
     * Sets message priority.  The priority member is used when the simulator
     * inserts messages in the message queue (FES) to order messages
     * with identical arrival time values.
     */
    void setPriority(int p) {prior=p;}

    /**
     * Sets message length (bits). When the message is sent through a
     * channel, message length affects transmission delay
     * and the probability of setting the bit error flag.
     */
    void setLength(long l);

    /**
     * Changes message length by the given value (bits). This is useful for
     * modeling encapsulation/decapsulation. (See also encapsulate() and
     * decapsulate().)
     *
     * The value may be negative (message length may be decreased too).
     * If the resulting length would be negative, the method throws a cException.
     */
    void addLength(long delta);

    /**
     * Set bit error flag.
     */
    void setBitError(bool err) {error=err;}

    /**
     * Sets the message's time stamp to the current simulation time.
     */
    void setTimestamp() {tstamp=simulation.simTime();}

    /**
     * Directly sets the message's time stamp.
     */
    void setTimestamp(simtime_t t) {tstamp=t;}

    /**
     * Sets the context pointer. This pointer may store an arbitrary value.
     * It is useful when managing several timers (self-messages): when
     * scheduling the message one can set the context pointer to the data
     * structure the timer corresponds to (e.g. the buffer whose timeout
     * the message represents), so that when the self-message arrives it is
     * easier to identify where it belongs.
     */
    void setContextPointer(void *p) {contextptr=p;}

    /**
     * Attaches a "control info" structure (object) to the message.
     * This is most useful when passing packets between protocol layers
     * of a protocol stack: e.g. when sending down an IP datagram to Ethernet,
     * the attached "control info" can contain the destination MAC address.
     *
     * The "control info" object will be deleted when the message is deleted.
     * Only one "control info" structure can be attached (the second
     * setControlInfo() call throws an error).
     *
     * When the message is duplicated or copied, copies will have their
     * control info set to NULL because the cPolymorphic interface
     * doesn't define dup/copy operations.
     * The assignment operator doesn't change control info.
     */
    void setControlInfo(cPolymorphic *p);

    /**
     * Removes the "control info" structure (object) from the message
     * and returns its pointer. Returns NULL if there was no control info
     * in the message
     */
    cPolymorphic *removeControlInfo();

    /**
     * Returns message kind.
     */
    int  kind() const     {return msgkind;}

    /**
     * Returns message priority.
     */
    int  priority() const {return prior;}

    /**
     * Returns message length (bits).
     */
    long length() const   {return len;}

    /**
     * Returns true if bit error flag is set, false otherwise.
     */
    bool hasBitError() const {return error;}

    /**
     * Returns the message's time stamp.
     */
    simtime_t timestamp() const {return tstamp;}

    /**
     * INTERNAL: Used by cMessageHeap.
     */
    unsigned long insertOrder() const {return insertordr;}

    /**
     * Returns the context pointer.
     */
    void *contextPointer() const {return contextptr;}

    /**
     * Returns pointer to the attached "control info".
     */
    cPolymorphic *controlInfo() const {return ctrlp;}
    //@}

    /** @name Dynamically attaching objects. */
    //@{

    /**
     * Returns reference to the 'object list' of the message: a cArray
     * which is used to store parameter (cPar) objects and other objects
     * attached to the message.
     *
     * One can use either parList() combined with cArray methods,
     * or several convenience methods (addPar(), addObject(), par(), etc.)
     * to add, retrieve or remove cPars and other objects.
     *
     * <i>NOTE: using the object list has alternatives which may better
     * suit your needs. For more information, see class description for discussion
     * about message subclassing vs dynamically attached objects.</i>
     */
    cArray& parList()
        {if (!parlistp) _createparlist(); return *parlistp;}

    /**
     * Add a new, empty parameter (cPar object) with the given name
     * to the message's object list.
     *
     * <i>NOTE: This is a convenience function: one may use parList() and
     * cArray::add() instead. See also class description for discussion about
     * message subclassing vs dynamically attached objects.</i>
     *
     * @see parList()
     */
    cPar& addPar(const char *s)  {cPar *p=new cPar(s);parList().add(p);return *p;}

    /**
     * Add a parameter object to the message's object list.
     *
     * <i>NOTE: This is a convenience function: one may use parList() and
     * cArray::add() instead. See also class description for discussion about
     * message subclassing vs dynamically attached objects.</i>
     *
     * @see parList()
     */
    cPar& addPar(cPar *p)  {parList().add(p); return *p;}

    /**
     * DEPRECATED! Use addPar(cPar *p) instead.
     */
    cPar& addPar(cPar& p)  {parList().add(&p); return p;}

    /**
     * Returns the nth object in the message's object list, converting it to a cPar.
     * If the object doesn't exist or it cannot be cast to cPar (using dynamic_cast<>),
     * the method throws a cException.
     *
     * <i>NOTE: This is a convenience function: one may use parList() and
     * cArray::get() instead. See also class description for discussion about
     * message subclassing vs dynamically attached objects.</i>
     *
     * @see parList()
     */
    cPar& par(int n);

    /**
     * Returns the object with the given name in the message's object list,
     * converting it to a cPar.
     * If the object doesn't exist or it cannot be cast to cPar (using dynamic_cast<>),
     * the method throws a cException.
     *
     * <i>NOTE: This is a convenience function: one may use parList() and
     * cArray::get() instead. See also class description for discussion about
     * message subclassing vs dynamically attached objects.</i>
     *
     * @see parList()
     */
    cPar& par(const char *s);

    /**
     * Returns the index of the parameter with the given name in the message's
     * object list, or -1 if it could not be found.
     *
     * <i>NOTE: This is a convenience function: one may use parList() and
     * cArray::find() instead. See also class description for discussion about
     * message subclassing vs dynamically attached objects.</i>
     *
     * @see parList()
     */
    int findPar(const char *s) const;

    /**
     * Check if a parameter with the given name exists in the message's
     * object list.
     *
     * <i>NOTE: This is a convenience function: one may use parList() and
     * cArray::exist() instead. See also class description for discussion about
     * message subclassing vs dynamically attached objects.</i>
     *
     * @see parList()
     */
    bool hasPar(const char *s) const {return findPar(s)>=0;}

    /**
     * Add an object to the message's object list.
     *
     * <i>NOTE: This is a convenience function: one may use parList() and
     * cArray::add() instead. See also class description for discussion about
     * message subclassing vs dynamically attached objects.</i>
     *
     * @see parList()
     */
    cObject *addObject(cObject *p)  {parList().add(p); return p;}

    /**
     * Returns the object with the given name in the message's object list.
     * If the object is not found, it returns NULL.
     *
     * <i>NOTE: This is a convenience function: one may use parList() and
     * cArray::get() instead. See also class description for discussion about
     * message subclassing vs dynamically attached objects.</i>
     *
     * @see parList()
     */
    cObject *getObject(const char *s)  {return parList().get(s);}

    /**
     * Check if an object with the given name exists in the message's object list.
     *
     * <i>NOTE: This is a convenience function: one may use parList() and
     * cArray::exist() instead. See also class description for discussion about
     * message subclassing vs dynamically attached objects.</i>
     *
     * @see parList()
     */
    bool hasObject(const char *s)  {return !parlistp ? false : parlistp->find(s)>=0;}

    /**
     * Remove the object with the given name from the message's object list, and
     * return its pointer. If the object doesn't exist, NULL is returned.
     *
     * <i>NOTE: This is a convenience function: one may use parList() and
     * cArray::remove() instead. See also class description for discussion about
     * message subclassing vs dynamically attached objects.</i>
     *
     * @see parList()
     */
    cObject *removeObject(const char *s)  {return parList().remove(s);}

    /**
     * Remove the object with the given name from the message's object list, and
     * return its pointer. If the object doesn't exist, NULL is returned.
     *
     * <i>NOTE: This is a convenience function: one may use parList() and
     * cArray::remove() instead. See also class description for discussion about
     * message subclassing vs dynamically attached objects.</i>
     *
     * @see parList()
     */
    cObject *removeObject(cObject *p)  {return parList().remove(p);}
    //@}

    /** @name Message encapsulation. */
    //@{

    /**
     * Encapsulates msg in the message. msg->length()
     * will be added to the length of the message.
     */
    void encapsulate(cMessage *msg);

    /**
     * Decapsulates a message from the message object. The length of
     * the message will be decreased accordingly, except if it was zero.
     * If the length would become negative, cException is thrown.
     */
    cMessage *decapsulate();

    /**
     * Returns a pointer to the encapsulated message, or NULL.
     */
    cMessage *encapsulatedMsg() const {return encapmsg;}
    //@}

    /** @name Sending/arrival information. */
    //@{

    /**
     * Return true if message was posted by scheduleAt().
     */
    bool isSelfMessage() const {return togate==-1;}

    /**
     * Return true if message is among future events.
     */
    bool isScheduled() const {return heapindex!=-1;}

    /**
     * Returns pointers to the gate from which the message was sent and
     * on which gate it arrived. A NULL pointer is returned
     * for new (unsent) messages and messages sent via scheduleAt().
     */
    cGate *senderGate() const;

    /**
     * Returns pointers to the gate from which the message was sent and
     * on which gate it arrived. A NULL pointer is returned
     * for new (unsent) messages and messages sent via scheduleAt().
     */
    cGate *arrivalGate() const;

    /**
     * Returns sender module's index in the module vector or -1 if the
     * message hasn't been sent/scheduled yet.
     */
    int senderModuleId() const {return frommod;}

    /**
     * Returns index of gate sent through in the sender module or -1
     * if the message hasn't been sent/scheduled yet.
     */
    int senderGateId() const   {return fromgate;}

    /**
     * Returns receiver module's index in the module vector or -1 if
     * the message hasn't been sent/scheduled yet.
     */
    int arrivalModuleId() const {return tomod;}

    /**
     * Returns index of gate the message arrived on in the sender module
     * or -1 if the message hasn't sent/scheduled yet.
     */
    int arrivalGateId() const  {return togate;}

    /**
     * Returns time when the message was created.
     */
    simtime_t creationTime() const {return created;}

    /**
     * Returns time when the message was sent/scheduled or 0 if the message
     * hasn't been sent yet.
     */
    simtime_t sendingTime()  const {return sent;}

    /**
     * Returns time when the message arrived (or will arrive if it
     * is currently scheduled or is underway), or 0 if the message
     * hasn't been sent/scheduled yet.
     */
    simtime_t arrivalTime()  const {return delivd;}

    /**
     * Return true if the message arrived through gate g.
     */
    bool arrivedOn(int g) const {return g==togate;}

    /**
     * Return true if the message arrived through the gate
     * given with its name and index (if multiple gate).
     */
    bool arrivedOn(const char *s, int g=0);
    //@}

    /** @name Internally used methods. */
    //@{

    /**
     * Called internally by the simulation kernel as part of the send(),
     * scheduleAt() calls to set the parameters returned by the
     * senderModuleId(), senderGate(), sendingTime() methods.
     */
    virtual void setSentFrom(cModule *module, int gate, simtime_t t);

    /**
     * Called internally by the simulation kernel as part of processing
     * the send(), scheduleAt() calls to set the parameters returned
     * by the arrivalModuleId(), arrivalGate() methods.
     */
    virtual void setArrival(cModule *module, int gate);

    /**
     * Called internally by the simulation kernel as part of processing
     * the send(), scheduleAt() calls to set the parameters returned
     * by the arrivalModuleId(), arrivalGate(), arrivalTime() methods.
     */
    virtual void setArrival(cModule *module, int gate, simtime_t t);

    /**
     * Called internally by the simulation kernel to set the parameters
     * returned by the arrivalTime() method.
     */
    virtual void setArrivalTime(simtime_t t);

    /**
     * Used internally by the parallel simulation kernel.
     */
    void setSrcProcId(unsigned char procId) {srcprocid=procId;}

    /**
     * Used internally by the parallel simulation kernel.
     */
    unsigned char srcProcId() {return srcprocid;}
    //@}

    /** @name Miscellaneous. */
    //@{

    /**
     * Override to define a display string for the message. Display string
     * affects message appearance in Tkenv.
     *
     * This default implementation returns "".
     */
    virtual const char *displayString() const;

    /**
     * Static function that compares two messages by their delivery times,
     * then by their priorities. Usable as cQeueue CompareFunc.
     */
    static int cmpbydelivtime(cObject *one, cObject *other);

    /**
     * Static function that compares two messages by their priorities.
     * It can be used to sort messages in a priority queue.
     * Usable as cQeueue CompareFunc.
     */
    static int cmpbypriority(cObject *one, cObject *other);

    /** @name Statistics. */
    //@{
    /**
     * Returns the total number of messages created since the last
     * reset (reset is usually called my user interfaces at the beginning
     * of each simulation run). The counter is incremented by cMessage constructor.
     * Counter is <tt>signed</tt> to make it easier to detect if it overflows
     * during very long simulation runs.
     * May be useful for profiling or debugging memory leaks.
     */
    static long totalMessageCount() {return total_msgs;}

    /**
     * Returns the number of message objects that currently exist in the
     * program. The counter is incremented by cMessage constructor
     * and decremented by the destructor.
     * May be useful for profiling or debugging memory leaks caused by forgetting
     * to delete messages.
     */
    static long liveMessageCount() {return live_msgs;}

    /**
     * Reset counters used by totalMessageCount() and liveMessageCount().
     */
    static void resetMessageCounters()  {total_msgs=live_msgs=0;}
    //@}
};

#endif


