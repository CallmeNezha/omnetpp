//=========================================================================
//
// CFILECOMM.CC - part of
//                          OMNeT++
//           Discrete System Simulation in C++
//
//   Written by:  Andras Varga, 2003
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 2004 Andras Varga
  Monash University, Dept. of Electrical and Computer Systems Eng.
  Melbourne, Australia

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "cexception.h"
#include "cfilecomm.h"
#include "cfilecommbuffer.h"
#include "macros.h"
#include "cenvir.h"
#include "cconfig.h"
#include "parsimutil.h"

Register_Class(cFileCommunications);


//
// First, some stuff to hide platform dependent details
// of getting a directory listing...
//
#if defined(_WIN32) && !defined(__CYGWIN32__)
#include <io.h>
#include <direct.h>
#define usleep(x) _sleep((x)/1000)
#define stat _stat
#define mkdir(x,y) _mkdir(x)
static long _handle;
static struct _finddata_t _fdata;
static char _dir[_MAX_FNAME];
static char _tmpfname[_MAX_FNAME];
const char *findFirstFile(const char *mask)
{
    _handle = _findfirst(mask, &_fdata);
    if (_handle<0) {_findclose(_handle); return NULL;}
    strcpy(_dir,mask);
    char *s = _dir + strlen(_dir);
    while (--s>=_dir)
        if (*s=='/' || *s=='\\')
            {*(s+1)='\0'; break;}
    strcpy(_tmpfname,_dir);
    strcat(_tmpfname,_fdata.name);
    return _tmpfname;
}
const char *findNextFile()
{
    int done=_findnext(_handle, &_fdata);
    if (done) {_findclose(_handle); return NULL;}
    strcpy(_tmpfname,_dir);
    strcat(_tmpfname,_fdata.name);
    return _tmpfname;
}
void findCleanup()
{
}
#else
#include <unistd.h>  // usleep()
#include <glob.h>
glob_t globdata;
int globpos;
const char *findFirstFile(const char *mask)
{
    if (glob(mask, 0, NULL, &globdata)!=0)
        return NULL;
    globpos = 0;
    return globdata.gl_pathv[globpos++];
}
const char *findNextFile()
{
    return globdata.gl_pathv[globpos++];
}
void findCleanup()
{
    globfree(&globdata);
}
#endif


//----------------

cFileCommunications::cFileCommunications()
{
    commDirPrefix = ev.config()->getAsString("General", "parsim-filecommunications-prefix", "comm/");
    readDirPrefix = ev.config()->getAsString("General", "parsim-filecommunications-read-prefix", "comm/read/");
    preserveReadFiles = ev.config()->getAsBool("General", "parsim-filecommunications-preserve-read", false);

    seqNum = 0;
}

cFileCommunications::~cFileCommunications()
{
}

void cFileCommunications::init()
{
    // get numPartitions and myProcId from "-p" command-line option
    getProcIdFromCommandLineArgs(myProcId, numPartitions, "cFileCommunications");
    ev.printf("cFileCommunications: started as process %d out of %d.\n", myProcId, numPartitions);

    // We cannot check here that the communications directory is empty, because
    // other partitions may have already sent messages to us...
}

void cFileCommunications::shutdown()
{
}

int cFileCommunications::getNumPartitions()
{
    return numPartitions;
}

int cFileCommunications::getProcId()
{
    return myProcId;
}

cCommBuffer *cFileCommunications::createCommBuffer()
{
    return new cFileCommBuffer();
}

void cFileCommunications::recycleCommBuffer(cCommBuffer *buffer)
{
    delete buffer;
}

void cFileCommunications::send(cCommBuffer *buffer, int tag, int destination)
{
    cFileCommBuffer *b = (cFileCommBuffer *)buffer;

    // to prevent concurrency problems, first create the file as .tmp,
    // then rename it to .msg
    char fname[100], fname2[100];
    sprintf(fname,"%s#%.6d-s%d-d%d-t%d.tmp", commDirPrefix.buffer(), seqNum++, myProcId, destination, tag);

    // create
    FILE *f = fopen(fname,"wb");
    if (!f)
        throw new cException("cFileCommunications: cannot open %s for write: %s", fname, strerror(errno));
    if (fwrite(b->getBuffer(), b->getMessageSize(), 1, f)<1)
        throw new cException("cFileCommunications: cannot write %s: %s", fname, strerror(errno));
    if (fclose(f)!=0)
        throw new cException("cFileCommunications: cannot close %s after writing: %s", fname, strerror(errno));

    // rename
    strcpy(fname2,fname);
    strcpy(fname2+strlen(fname2)-4, ".msg");
    if (rename(fname, fname2)!=0)
        throw new cException("cFileCommunications: cannot rename %s to %s: %s", fname, fname2, strerror(errno));
}

bool cFileCommunications::receiveBlocking(int filtTag, cCommBuffer *buffer, int& receivedTag, int& sourceProcId)
{
    while (!receiveNonblocking(filtTag, buffer, receivedTag, sourceProcId))
    {
        if (ev.idle())
            return false;
        usleep(100000); // be nice and polite: wait 0.1s
    }
    return true;
}

bool cFileCommunications::receiveNonblocking(int filtTag, cCommBuffer *buffer, int& receivedTag, int& sourceProcId)
{
    cFileCommBuffer *b = (cFileCommBuffer *)buffer;
    b->reset();

    char fmask[100];
    char fname2[100];
    if (filtTag==PARSIM_ANY_TAG)
        sprintf(fmask,"%s#*-s*-d%d-t*.msg", commDirPrefix.buffer(), myProcId);
    else
        sprintf(fmask,"%s#*-s*-d%d-t%d.msg", commDirPrefix.buffer(), myProcId, filtTag);

    bool ret = false;
    const char *fname = findFirstFile(fmask);
    if (fname)
    {
        ret = true;

        // parse fname
        const char *s = strstr(fname, "-s");
        sourceProcId = atol(s+2);
        const char *t = strstr(fname, "-t");
        receivedTag = atol(t+2);
        //const char *n = strstr(fname, "#");
        //int seqNum = atol(n+2);

        //DBG: printf("%d: filecomm: found %s -- src=%d, tag=%d\n",getProcId(),fname,sourceProcId,receivedTag);

        // read data
        struct stat statbuf;
        if (stat(fname, &statbuf)!=0)
            throw new cException("cFileCommunications: cannot stat() file %s: ", fname, strerror(errno));
        int len = statbuf.st_size;
        b->allocateAtLeast(len);
        FILE *f = fopen(fname,"rb");
        if (!f)
        {
            // try a bit harder. On Windows, first fopen() sometimes fails with
            // "permission denied".
            for (int i=0; i<20; i++)
            {
                usleep(500000); // wait 0.1s
                ev.printf("cFileCommunications: retrying to open file %s (previous attempt failed)\n", fname);
                f = fopen(fname,"rb");
                if (f) break;
            }
        }
        if (!f)
            throw new cException("cFileCommunications: cannot open existing file %s for read: %s", fname, strerror(errno));
        if (fread(b->getBuffer(), len, 1, f)==0)
            // FIXME condition always fires. why?
            //throw new cException("cFileCommunications: cannot read existing file %s: %s", fname, strerror(errno));
            ;
        fclose(f);
        b->setMessageSize(len);

        if (preserveReadFiles)
        {
            // move file to 'read' directory
            //
            // BEWARE: for mysterious reasons, it appears that there cannot be more
            // than about 19800 files in a directory. When that point is reached,
            // an exception is thrown somewhere inside the standard C library, which
            // materializes itself in OMNeT++ as an "Error: (null)" message...
            // Strangely, this can be reproduced in both Linux and Windows.
            //
            strcpy(fname2, readDirPrefix.buffer());
            strcat(fname2, fname + strlen(commDirPrefix.buffer()));
            if (rename(fname, fname2)!=0)
                throw new cException("cFileCommunications: cannot rename %s to %s: %s", fname, fname2, strerror(errno));
        }
        else
        {
            // delete file
            if (unlink(fname)!=0)
                throw new cException("cFileCommunications: cannot delete file %s: %s", fname, strerror(errno));
        }
    }
    findCleanup();
    //DBG: printf("%d: filecomm: nothing found matching %s\n",getProcId(),fmask);
    return ret;
}



