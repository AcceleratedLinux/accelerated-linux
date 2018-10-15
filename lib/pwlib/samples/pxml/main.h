/*
 * main.h
 *
 * PWLib application header file for PxmlTest
 *
 * Copyright 2002 David Iodice.
 *
 * $Log: main.h,v $
 * Revision 1.1  2002/03/07 01:56:56  robertj
 * Added XML sample/test program.
 *
 */

#ifndef _PxmlTest_MAIN_H
#define _PxmlTest_MAIN_H


#include <ptclib/pxml.h>


class PxmlTest : public PProcess
{
  PCLASSINFO(PxmlTest, PProcess)

  public:
    PxmlTest();
    void Main();
};

#endif  // _PxmlTest_MAIN_H


// End of File ///////////////////////////////////////////////////////////////
