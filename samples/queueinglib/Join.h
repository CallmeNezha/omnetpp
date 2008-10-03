//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2008 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifndef __QUEUEING_JOIN_H__
#define __QUEUEING_JOIN_H__

#include <list>
#include "QueueingDefs.h"
#include "Job.h"

namespace queueing {

/**
 * Merges sub-jobs generated by Split.
 */
class Join : public cSimpleModule
{
    protected:
        std::list<Job*> jobsHeld;
    public:
        Join();
        ~Join();
    protected:
        virtual void initialize();
        virtual void handleMessage(cMessage *msg);
};

}; // namespace

#endif
