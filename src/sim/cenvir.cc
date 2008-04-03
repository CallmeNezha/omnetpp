//=========================================================================
//  CENVIR.CC - part of
//
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//  Author: Andras Varga
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2005 Andras Varga

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include "cenvir.h"

cEnvir *evPtr;


/*
 * The std::streambuf class used by cEnvir's ostream. It redirects writes to
 * cEnvir::sputn(s,n). Flush is done at the end of each line, meanwhile
 * writes are buffered in a stringbuf.
 */
template <class E, class T = std::char_traits<E> >
class basic_evbuf : public std::basic_stringbuf<E,T>
{
  public:
    basic_evbuf() {}
    // gcc>=3.4 needs either this-> or std::basic_stringbuf<E,T>:: in front of pptr()/pbase()
    // note: this method is needed because pptr() and pbase() are protected
    bool isempty() {return this->pptr()==this->pbase();}
  protected:
    virtual int sync()  {
        ev.sputn(this->pbase(), this->pptr()-this->pbase());
        setp(this->pbase(),this->epptr());
        return 0;
    }
    virtual std::streamsize xsputn(const E *s, std::streamsize n) {
        std::streamsize r = std::basic_stringbuf<E,T>::xsputn(s,n);
        for(;n>0;n--,s++)
            if (*s=='\n')
               {sync();break;}
        return r;
    }
};

typedef basic_evbuf<char> evbuf;

//----

cEnvir::cEnvir() : out(new evbuf())
{
    disable_tracing = false;
    debug_on_errors = false;
    suppress_notifications = false; //FIXME set to true when not needed!
}

cEnvir::~cEnvir()
{
}

// note: exploits the fact that evbuf does sync() on "\n"'s
void cEnvir::flushLastLine()
{
    evbuf *buf = (evbuf *)out.rdbuf();
    if (!buf->isempty())
        buf->sputn("\n",1);
}

//
// Temp buffer for vararg functions below.
// Note: using a static buffer reduces stack usage of activity() modules;
// it also makes the following functions non-reentrant, but we don't need
// them to be reentrant anyway.
//
#define ENVIR_TEXTBUF_LEN 1024
static char buffer[ENVIR_TEXTBUF_LEN];

void cEnvir::printfmsg(const char *fmt,...)
{
    va_list va;
    va_start(va, fmt);
    vsprintf(buffer, fmt, va);  //FIXME use vsnprintf
    va_end(va);

    putsmsg(buffer);
}

int cEnvir::printf(const char *fmt,...)
{
    if (disable_tracing)
        return 0;

    va_list va;
    va_start(va, fmt);
    int len = vsprintf(buffer, fmt, va);  //FIXME use vsnprintf
    va_end(va);

    sputn(buffer, len);
    return len;
}

bool cEnvir::askYesNo(const char *fmt,...)
{
    va_list va;
    va_start(va, fmt);
    vsprintf(buffer, fmt, va);  //FIXME use vsnprintf
    va_end(va);

    return askyesno(buffer);
}


