//==========================================================================
//   UTIL.H - header for
//                             OMNeT++
//            Discrete System Simulation in C++
//
//
//  Utility functions
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2001 Andras Varga
  Technical University of Budapest, Dept. of Telecommunications,
  Stoczek u.2, H-1111 Budapest, Hungary.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __UTIL_H
#define __UTIL_H

#include <stdarg.h>  // for va_list
#include "defs.h"


/**
 * Number of random number generators.
 */
#define NUM_RANDOM_GENERATORS    32

#define INTRAND_MAX  0x7ffffffeL  /* = 2**31-2 FIXME */


//
// #defines provided for backwards compatibility.
// They may be removed in a future release!
//
#define myrandomize      opp_randomize
#define genk_myrandomize genk_opp_randomize
#define mystrdup         opp_strdup
#define mystrcpy         opp_strcpy
#define mystrcmp         opp_strcmp
#define mystrmatch       opp_strmatch
#define fastconcat       opp_concat

/**
 * @name Converting simulation time to and from string form.
 * @ingroup Functions
 */
//@{

/**
 * Convert a string to simtime_t. The string should have a format
 * similar to the one output by simtimeToStr() (like "1s 34ms").
 *
 * Returns -1 if the whole string cannot be interpreted as time.
 * Empty string (or only spaces+tabs) is also an error.
 * E.g. strtoSimtime("3s 600ms x") will return -1.
 */
SIM_API simtime_t strToSimtime(const char *str);

/**
 * Convert the beginning of a string to simtime_t. Similar to
 * strToSimtime(), only it processes the string as far as it
 * can be interpreted as simulation time. It sets the pointer
 * passed to the first character which cannot be interpreted
 * as part of the time string, or to the terminating zero.
 * Empty string is accepted as 0.0.
 * E.g. strToSimtime0("3s 600ms x") will return 3.6 and the
 * pointerstr will point to the character 'x'.
 */
SIM_API simtime_t strToSimtime0(const char *&str);

/**
 * Converts simulation time (passed as simtime_t) into a
 * string like "0.0120000 (12ms)". If no destination pointer
 * is given, uses a static buffer.
 */
SIM_API char *simtimeToStr(simtime_t t, char *dest=NULL);
//@}


/**
 * @name Random number generation.
 *
 * OMNeT++ has a built-in pseudorandom number generator that gives long int
 * (32-bit) values in the range 1...2^31-2, with a period length of 2^31-2.
 *
 * The generator is a linear congruential generator (LCG), and uses the method
 * x=(x * 75) mod (2^31-1). The testrand() method can be used
 * to check if the generator works correctly. Required hardware is exactly
 * 32-bit integer arithmetics.
 *
 * OMNeT++ provides several independent random number generators
 * (by default 32; this number is #defined as NUM_RANDOM_GENERATORS in
 * utils.h), identified by numbers. The generator number is usually the gen_nr
 * argument to functions beginning with genk_.
 *
 * Source: Raj Jain: The Art of Computer Systems Performance Analysis
 * (John Wiley & Sons, 1991), pages 441-444, 455.
 *
 * @ingroup Functions
 */
//@{

/**
 * Returns 1 if the random generator works OK. Keeps seed intact.
 * It works by checking the following: starting with x[0]=1,
 * x[10000]=1,043,618,065 must hold.
 */
SIM_API int testrand();

/**
 * Initialize random number generator 0 with a random value.
 */

SIM_API void opp_randomize();

/**
 * Returns current seed of generator 0.
 */
SIM_API long randseed();

/**
 * Sets seed of generator 0 and returns old seed value. Zero is not allowed as a seed.
 */
SIM_API long randseed(long seed);

/**
 * Produces random integer in the range 1...INTRAND_MAX using generator 0.
 */
SIM_API long intrand();

/**
 * Produces random integer in range 0...r-1 using generator 0.  (Assumes r &lt;&lt; INTRAND_MAX.)
 */
SIM_API long intrand(long r);

/**
 * Produces random double in range 0.0...1.0 using generator 0.
 */
inline  double dblrand();

/**
 * Initialize random number generator gen_nr with a random value.
 */
SIM_API void genk_opp_randomize(int gen_nr);

/**
 * Returns current seed of generator gen_nr.
 */
SIM_API long genk_randseed(int gen_nr);

/**
 * Sets seed of generator gen_nr and returns old seed value. Zero is not allowed as a seed.
 */
SIM_API long genk_randseed(int gen_nr, long seed);

/**
 * Produces random integer in the range 1...INTRAND_MAX using generator gen_nr.
 */
SIM_API long genk_intrand(int gen_nr);

/**
 * Produces random integer in range 0...r-1 using generator gen_nr. (Assumes r &lt;&lt; INTRAND_MAX.)
 */
SIM_API long genk_intrand(int gen_nr,long r);

/**
 * Produces random double in range 0.0...1.0 using generator gen_nr.
 */
inline  double genk_dblrand(int gen_nr);
//@}


/**
 * @name Distributions.
 *
 * Argument types and return value must be `double' so that they can be used
 * in NED files, and cPar 'F' and 'X' types.
 *
 * @ingroup Functions
 */
//@{

/**
 * Returns a random number with uniform distribution in the range [a,b).
 * Uses generator 0.
 */
SIM_API double uniform(double a, double b);

/**
 * Returns a random integer with uniform distribution in the range [a,b],
 * inclusive. (Note that the function can also return b!)
 * Uses generator 0.
 */
SIM_API double intuniform(double a, double b);

/**
 * Returns a random number with exponential distribution with the given mean
 * (with parameter 1/mean).
 * Uses generator 0.
 */
SIM_API double exponential(double mean);

/**
 * Returns a random number with normal distribution with the given mean and variance.
 * Uses generator 0.
 */
SIM_API double normal(double mean, double variance);

/**
 * Normal distribution truncated to nonnegative values.
 * Note that because it is implemented with a loop that discards
 * negative values until a nonnegative comes, the execution time
 * is not bounded (a large negative mean with much smaller variance
 * may result in many iterations and very long execution time).
 * Uses generator 0.
 */
SIM_API double truncnormal(double mean, double varianced);


/**
 * Same as uniform(), only uses random generator
 * gen_nr instead of generator 0.
 */
SIM_API double genk_uniform(double gen_nr, double a, double b);

/**
 * Same as intuniform(), only uses random generator
 * gen_nr instead of generator 0.
 */
SIM_API double genk_intuniform(double gen_nr, double a, double b);

/**
 * Same as exponential(), only uses random generator
 * gen_nr instead of generator 0.
 */
SIM_API double genk_exponential(double gen_nr, double p);

/**
 * Same as normal(), only uses random generator
 * gen_nr instead of generator 0.
 */
SIM_API double genk_normal(double gen_nr, double mean, double variance);

/**
 * Same as truncnormal(), only uses random generator
 * gen_nr instead of generator 0.
 */
SIM_API double genk_truncnormal(double gen_nr, double mean, double variance);
//@}


/**
 * @name Utility functions to support nedc-compiled expressions.
 * @ingroup Functions
 */
//@{

/**
 * Returns the minimum of a and b.
 */
SIM_API double min(double a, double b);

/**
 * Returns the maximum of a and b.
 */
SIM_API double max(double a, double b);

/**
 * Returns the boolean AND of a and b.
 * (Any nonzero number is treated as true.)
 */
SIM_API double bool_and(double a, double b);

/**
 * Returns the boolean OR of a and b.
 * (Any nonzero number is treated as true.)
 */
SIM_API double bool_or(double a, double b);

/**
 * Returns the boolean Exclusive OR of a and b.
 * (Any nonzero number is treated as true.)
 */
SIM_API double bool_xor(double a, double b);

/**
 * Returns the boolean negation of a.
 * (Any nonzero number is treated as true.)
 */
SIM_API double bool_not(double a);

/**
 * Returns the binary AND of a and b.
 * (a and b are converted to unsigned long for the operation.)
 */
SIM_API double bin_and(double a, double b);

/**
 * Returns the binary OR of a and b.
 * (a and b are converted to unsigned long for the operation.)
 */
SIM_API double bin_or(double a, double b);

/**
 * Returns the binary exclusive OR of a and b.
 * (a and b are converted to unsigned long for the operation.)
 */
SIM_API double bin_xor(double a, double b);

/**
 * Returns the bitwise negation (unary complement) of a.
 * (a is converted to unsigned long for the operation.)
 */
SIM_API double bin_compl(double a);

/**
 * Shifts a b bits to the left.
 * (a and b are converted to unsigned long for the operation.)
 */
SIM_API double shift_left(double a, double b);

/**
 * Shifts a b bits to the right.
 * (a and b are converted to unsigned long for the operation.)
 */
SIM_API double shift_right(double a, double b);
//@}


/**
 * @name Value-added string functions.
 *
 * These functions replace some of the <string.h> functions.
 * The difference is that they also accept NULL pointers as empty strings (""),
 * and also use operator new instead of malloc().
 * It is recommended to use these functions instead of the original
 * <string.h> functions.
 *
 * @ingroup Functions
 */
//@{

/**
 * Duplicates the string. If the pointer passed is NULL or points
 * to a null string (""), NULL is returned.
 */
SIM_API char *opp_strdup(const char *);

/**
 * Same as the standard strcpy() function, except that NULL pointers
 * in the second argument are treated like pointers to a null string ("").
 */
SIM_API char *opp_strcpy(char *,const char *);

/**
 * Same as the standard strcmp() function, except that NULL pointers
 * are treated like pointers to a null string ("").
 */
SIM_API int  opp_strcmp(const char *, const char *);

/**
 * Returns true if the two strings are identical up to the length of the
 * shorter one. NULL pointers are treated like pointers to a null string ("").
 */
SIM_API bool opp_strmatch(const char *, const char *);
//@}

/**
 * @name Miscellaneous functions.
 * @ingroup Functions
 */
//@{

/**
 * Concatentates up to four strings. Returns a pointer to a static buffer of length 256.
 */
SIM_API char *opp_concat(const char *s1, const char *s2, const char *s3=NULL, const char *s4=NULL);

/**
 * Creates a string like "component[35]" into buf, the first argument.
 */
SIM_API char *indexedname(char *buf, const char *name, int index);

/**
 * Returns the pointer passed as argument unchanged, except that if it was NULL,
 * it returns a pointer to a null string ("").
 */
inline const char *correct(const char *);

/**
 * Tests equality of two doubles, with the given precision.
 */
inline bool equal(double a, double b, double epsilon);
//@}

//
// INTERNAL: a restricted vsscanf implementation used by cStatistic::freadvarsf()
//
SIM_API int opp_vsscanf(const char *s, const char *fmt, va_list va);

/**
 * Error handling to be used in new classes. The functions call
 * simulation.error()/warning(). These functions were introduced so
 * that not every class that wants to issue an error message
 * needs to include "csimul.h" and half the simulation kernel with it.
 */
//@{

/**
 * Terminates the simulation with an error message.
 */
SIM_API void opp_error(int errcode,...);

/**
 * Same as function with the same name, but using custom message string.
 * To be called like printf().
 */
SIM_API void opp_error(const char *msg,...);

/**
 * Issues a warning. In a graphical user interface, this will cause
 * a message box to pop up with the given message, and the
 * user will be given a chance to continue or abort the simulation.
 */
SIM_API void opp_warning(int errcode,...);

/**
 * Same as function with the same name, but using custom message string.
 * To be called like printf().
 */
SIM_API void opp_warning(const char *msg,...);

/**
 * Print message and set error number.
 */
SIM_API void opp_terminate(int errcode,...);

/**
 * Same as function with the same name, but using custom message string.
 * To be called like printf().
 */
SIM_API void opp_terminate(const char *msg,...);
//@}

/**
 * Very simple string class. opp_string has only one data member,
 * a char* pointer. Allocation/deallocation of the contents takes place
 * via opp_strdup() and operator delete
 *
 * Recommended use: as class member, where otherwise the class members
 * would have to call opp_strdup() and delete for the char* member.
 *
 * @ingroup SimSupport
 */
class SIM_API opp_string
{
  private:
    char *str;

  public:
    /**
     * Constructor.
     */
    opp_string()               {str = 0;}

    /**
     * Constructor.
     */
    opp_string(const char *s)  {str = opp_strdup(s);}

    /**
     * Copy constructor.
     */
    opp_string(opp_string& s)  {str = opp_strdup(s.str);}

    /**
     * Destructor.
     */
    ~opp_string()              {delete str;}

    /**
     * Returns pointer to the string.
     */
    operator const char *() _CONST    {return str;}

    /**
     * Returns pointer to the internal buffer where the string is stored.
     * It is allowed to write into the string via this pointer, but the
     * length of the string should not be exceeded.
     */
    char *buffer() _CONST        {return str;}

    /**
     * Deletes the old value and opp_strdup()'s the new value
     * to create the object's own copy.
     */
    const char *operator=(const char *s)
                               {delete str;str=opp_strdup(s);return str;}

    /**
     * Assignment.
     */
    opp_string& operator=(_CONST opp_string& s)
                               {delete str;str=opp_strdup(s.str);return *this;}
};

//==========================================================================
//=== Implementation of utility functions:

inline bool equal(double a, double b, double epsilon)
{
   double d = a-b;
   return (d>=0.0 ? d : -d) < epsilon;
}

inline const char *correct(const char *s)
{
   return s ? s : "";
}

inline double dblrand()
{
   return (double)intrand() / (double)((unsigned long)INTRAND_MAX+1UL);
}

inline double genk_dblrand(int gen_nr)
{
   return (double)genk_intrand(gen_nr) / (double)((unsigned long)INTRAND_MAX+1UL);
}

#endif


