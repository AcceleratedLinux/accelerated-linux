/*
 * statusacct.h
 *
 * accounting module for GNU Gatekeeper that send it's output to the status port. 
 *
 * Copyright (c) 2005, Jan Willamowius
 *
 * This work is published under the GNU Public License (GPL)
 * see file COPYING for details.
 * We also explicitely grant the right to link this code
 * with the OpenH323 library.
 *
 * $Log: statusacct.h,v $
 * Revision 1.2  2006/04/14 13:56:19  willamowius
 * call failover code merged
 *
 * Revision 1.1.1.1  2005/11/21 20:19:59  willamowius
 *
 *
 * Revision 1.4  2005/11/15 19:52:56  jan
 * Michal v1 (works, but on in routed, not proxy mode)
 *
 * Revision 1.1  2005/08/28 18:05:55  willamowius
 * new accounting module StatusAcct
 *
 *
 */

#ifndef __STATUSACCT_H
#define __STATUSACCT_H "@(#) $Id: statusacct.h,v 1.2 2006/04/14 13:56:19 willamowius Exp $"

#include "gkacct.h"


/** Accounting module for the status port.
	It sends accounting call start/stop/update/connect events
	to the status port.
*/
class StatusAcct : public GkAcctLogger
{
public:
	enum Constants
	{
		/// events recognized by this module
		StatusAcctEvents = AcctStart | AcctStop | AcctUpdate | AcctConnect
	};

	StatusAcct( 
		/// name from Gatekeeper::Acct section
		const char* moduleName,
		/// config section name to be used with an instance of this module,
		/// pass NULL to use a default section (named "moduleName")
		const char* cfgSecName = NULL
		);
		
	/// Destroy the accounting logger
	virtual ~StatusAcct();

	/// overriden from GkAcctLogger
	virtual Status Log(
		AcctEvent evt,
		const callptr& call
		);

	/// overriden from GkAcctLogger
	virtual PString EscapeAcctParam(const PString& param) const;
		
private:
	StatusAcct();
	/* No copy constructor allowed */
	StatusAcct(const StatusAcct&);
	/* No operator= allowed */
	StatusAcct& operator=(const StatusAcct&);

private:
	/// parametrized string for the call start event
	PString m_startEvent;
	/// parametrized string for the call stop (disconnect) event
	PString m_stopEvent;
	/// parametrized string for the call update event
	PString m_updateEvent;
	/// parametrized string for the call connect event
	PString m_connectEvent;
	/// timestamp formatting string
	PString m_timestampFormat;
};

#endif /* __STATUSACCT_H */
