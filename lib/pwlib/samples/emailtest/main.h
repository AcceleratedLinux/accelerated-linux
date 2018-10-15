/*
 * main.h
 *
 * PWLib application header file for emailtest
 *
 * Copyright (c) 2004 Post Increment
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: main.h,v $
 * Revision 1.1  2004/08/11 07:39:05  csoutheren
 * Initial version
 *
 */

#ifndef _Emailtest_MAIN_H
#define _Emailtest_MAIN_H




class Emailtest : public PProcess
{
  PCLASSINFO(Emailtest, PProcess)

  public:
    Emailtest();
    virtual void Main();
};


#endif  // _Emailtest_MAIN_H


// End of File ///////////////////////////////////////////////////////////////
