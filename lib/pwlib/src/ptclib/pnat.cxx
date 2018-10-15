
/*
 * pnat.cxx
 *
 * NAT Strategy support for Portable Windows Library.
 *
 * Virteos is a Trade Mark of ISVO (Asia) Pte Ltd.
 *
 * Copyright (c) 2004 ISVO (Asia) Pte Ltd. All Rights Reserved.
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
 *
 * The Original Code is derived from and used in conjunction with the 
 * OpenH323 Project (www.openh323.org/)
 *
 * The Initial Developer of the Original Code is ISVO (Asia) Pte Ltd.
 *
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: pnat.cxx,v $
 * Revision 1.3  2005/11/30 12:47:41  csoutheren
 * Removed tabs, reformatted some code, and changed tags for Doxygen
 *
 * Revision 1.2  2005/07/13 11:15:26  csoutheren
 * Backported NAT abstraction files from isvo branch
 *
 * Revision 1.1.2.1  2005/04/25 13:24:55  shorne
 * Initial version
 *
 *
*/

#include <ptlib.h>
#include <ptclib/pnat.h>

PNatStrategy::PNatStrategy()
{

}

PNatStrategy::~PNatStrategy()
{

}

void PNatStrategy::AddMethod(PNatMethod * method)
{
  natlist.Append(method);
}

PNatMethod * PNatStrategy::GetMethod()
{
  for (PINDEX i=0; i < natlist.GetSize(); i++) {
       PNatMethod * meth = (PNatMethod *)natlist.GetAt(i);

     if (meth->IsAvailable())
       return meth;
  }

  return NULL;
}

void PNatStrategy::SetPortRanges(
      WORD portBase, WORD portMax, WORD portPairBase, WORD portPairMax)
{
  for (PINDEX i=0; i < natlist.GetSize(); i++) {
       PNatMethod * meth = (PNatMethod *)natlist.GetAt(i);

     meth->SetPortRanges(portBase,portMax,portPairBase,portPairMax);
  }
}


///////////////////////////////////////////////////////////////////////

PNatMethod::PNatMethod()
{

}

PNatMethod::~PNatMethod()
{

}

void PNatMethod::SetPortRanges(WORD portBase, WORD portMax, WORD portPairBase, WORD portPairMax) 
{
  singlePortInfo.mutex.Wait();

  singlePortInfo.basePort = portBase;
  if (portBase == 0)
    singlePortInfo.maxPort = 0;
  else if (portMax == 0)
    singlePortInfo.maxPort = (WORD)(singlePortInfo.basePort+99);
  else if (portMax < portBase)
    singlePortInfo.maxPort = portBase;
  else
    singlePortInfo.maxPort = portMax;

  singlePortInfo.currentPort = singlePortInfo.basePort;

  singlePortInfo.mutex.Signal();

  pairedPortInfo.mutex.Wait();

  pairedPortInfo.basePort = (WORD)((portPairBase+1)&0xfffe);
  if (portPairBase == 0) {
    pairedPortInfo.basePort = 0;
    pairedPortInfo.maxPort = 0;
  }
  else if (portPairMax == 0)
    pairedPortInfo.maxPort = (WORD)(pairedPortInfo.basePort+99);
  else if (portPairMax < portPairBase)
    pairedPortInfo.maxPort = portPairBase;
  else
    pairedPortInfo.maxPort = portPairMax;

  pairedPortInfo.currentPort = pairedPortInfo.basePort;

  pairedPortInfo.mutex.Signal();
}
