/*!
 * \file F5cData.cc
 * \author Christian Eder
 * \brief This file includes the inlined methods of the classes \c CPair
 * and \c Label.
 * \details This file includes the inlined methods of the class \c CPair,
 * representing critical pairs in the F5C Algorithm, and the class \c Label,
 * representing the labels in the F5C Algorithm. In this file the handling of
 * these data types is defined.
 */

#include "mod2.h"

#ifdef HAVE_F5C
#include "kutil.h"
#include "structs.h"
#include "omalloc.h"
#include "polys.h"
#include "p_polys.h"
#include "ideals.h"
#include "febase.h"
#include "kstd1.h"
#include "khstd.h"
#include "kbuckets.h"
#include "weight.h"
#include "intvec.h"
#include "pInline1.h"
#include "f5c.h"
#include "F5cData.h"
#include "F5cLists.h"


// IMPLEMENTATIONS OF METHODS OF CLASS LABEL


// constructor / destructor of class Label
Label::Label(unsigned int* expVec) {
  m_pExpVec       = expVec;
  m_nShortExpVec  = computeShortExpVec(m_pExpVec);
}

Label::~Label() {
  delete &m_nShortExpVec;
  delete [] m_pExpVec;
}


// GETTER of class Label
unsigned int* Label::getExpVec() {
  return m_pExpVec;
}

unsigned long Label::getShortExpVec() {
  return m_nShortExpVec;
}


// SETTER of class Label


// MISC of class Label
unsigned long Label::computeShortExpVec(unsigned int* expVec) {
  //if (p == NULL) return 0;
  unsigned long ev = 0; // short exponent vector
  unsigned int n = BIT_SIZEOF_LONG / currRing->N; // number of bits per exp
  unsigned int m1; // highest bit which is filled with (n+1)
  unsigned int i = 0, j=1;

  if (n == 0) {
    if (currRing->N <2*BIT_SIZEOF_LONG) {
      n=1;
      m1=0;
    }
    else {
      for (; j<=(unsigned long) currRing->N; j++) {
        if (expVec[j] > 0) i++;
        if (i == BIT_SIZEOF_LONG) break;
      }
      if (i>0)
      ev = ~((unsigned long)0) >> ((unsigned long) (BIT_SIZEOF_LONG - i));
      return ev;
    }
  }
  else {
    m1 = (n+1)*(BIT_SIZEOF_LONG - n*currRing->N);
  }

  n++;
  while (i<m1) {
    ev |= getBitFields(expVec[j], i, n);
    i += n;
    j++;
  }

  n--;
  while (i<BIT_SIZEOF_LONG) {
    ev |= getBitFields(expVec[j], i, n);
    i += n;
    j++;
  }
  return ev;
}

unsigned long Label::getBitFields(unsigned int e, unsigned int s, unsigned int n) {
#define Sy_bit_L(x)     (((unsigned long)1L)<<(x))
  unsigned int i = 0;
  unsigned long  ev = 0L;
  assume(n > 0 && s < BIT_SIZEOF_LONG);
  do {
    assume(s+i < BIT_SIZEOF_LONG);
    if (e > (int) i) ev |= Sy_bit_L(s+i);
    else break;
    i++;
  }
  while (i < n);
  return ev;
}

// END IMPLEMENTATIONS OF METHODS OF CLASS LABEL

#endif
// HAVE_F5C