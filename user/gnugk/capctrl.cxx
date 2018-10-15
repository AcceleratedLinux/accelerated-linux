/*
 * capctrl.cxx
 *
 * Module for accoutning per IP/H.323 ID/CLI/prefix inbound call volume
 *
 * $Id: capctrl.cxx,v 1.3 2006/12/13 15:34:22 zvision Exp $
 *
 * Copyright (c) 2006, Michal Zygmuntowicz
 *
 * This work is published under the GNU Public License (GPL)
 * see file COPYING for details.
 * We also explicitely grant the right to link this code
 * with the OpenH323 library.
 */
#if defined(_WIN32) && (_MSC_VER <= 1200)
#pragma warning(disable:4284)
#endif

#include <ptlib.h>
#include <ptlib/ipsock.h>
#include <h225.h>
#include <h323pdu.h>
#include "h323util.h"
#include "RasTbl.h"
#include "RasPDU.h"
#include "sigmsg.h"
#include "gkacct.h"
#include "gkauth.h"
#include "capctrl.h"

namespace {
// greater operators for sorting route lists

struct IpRule_greater : public std::binary_function<CapacityControl::IpCallVolume, CapacityControl::IpCallVolume, bool> {

	bool operator()(const CapacityControl::IpCallVolume &e1, const CapacityControl::IpCallVolume &e2) const 
	{
		int diff;
		if (e1.first.IsAny()) {
			if (!e2.first.IsAny())
				return false;
		} else {
			if (e2.first.IsAny())
				return true;
			diff = e1.first.Compare(e2.first);
			if (diff != 0)
				return diff > 0;
		}
		return false;
	}
};

struct H323IdRule_greater : public std::binary_function<CapacityControl::H323IdCallVolume, CapacityControl::H323IdCallVolume, bool> {

	bool operator()(const CapacityControl::H323IdCallVolume &e1, const CapacityControl::H323IdCallVolume &e2) const 
	{
		return H323GetAliasAddressString(e1.first) > H323GetAliasAddressString(e2.first);
	}
};

struct CLIRule_greater : public std::binary_function<CapacityControl::CLICallVolume, CapacityControl::CLICallVolume, bool> {

	bool operator()(const CapacityControl::CLICallVolume &e1, const CapacityControl::CLICallVolume &e2) const 
	{
		return e1.first.compare(e2.first) > 0;
	}
};

} // end of anonymous namespace

CapacityControl::InboundCallVolume::InboundCallVolume()
	: m_maxVolume(0), m_currentVolume(0)
{
}

CapacityControl::InboundCallVolume::~InboundCallVolume()
{
}

PString CapacityControl::InboundCallVolume::AsString() const
{
	return PString("pfx: ") + (m_prefix.empty() ? "*" : m_prefix.c_str())
		+ ", vol (cur/max): " + PString(m_currentVolume) + "/" + PString(m_maxVolume);
}

bool CapacityControl::InboundCallVolume::operator==(const InboundCallVolume &obj) const
{
	return m_prefix == obj.m_prefix;
}

bool CapacityControl::InboundIPCallVolume::operator==(const InboundIPCallVolume &obj) const
{
	return m_sourceAddress == obj.m_sourceAddress && ((InboundCallVolume&)*this) == ((InboundCallVolume&)obj);
}

bool CapacityControl::InboundH323IdCallVolume::operator==(const InboundH323IdCallVolume &obj) const
{
	return m_sourceH323Id == obj.m_sourceH323Id && ((InboundCallVolume&)*this) == ((InboundCallVolume&)obj);
}

bool CapacityControl::InboundCLICallVolume::operator==(const InboundCLICallVolume &obj) const
{
	return m_sourceCLI == obj.m_sourceCLI && ((InboundCallVolume&)*this) == ((InboundCallVolume&)obj);
}

CapacityControl::CapacityControl(
	) : Singleton<CapacityControl>("CapacityControl")
{
	LoadConfig();
}

void CapacityControl::LoadConfig()
{
	IpCallVolumes ipCallVolumes;
	H323IdCallVolumes h323IdCallVolumes;
	CLICallVolumes cliCallVolumes;

	PConfig* cfg = GkConfig();
	const PString cfgSec("CapacityControl");
	
	unsigned ipRules = 0, h323IdRules = 0, cliRules = 0;
	IpCallVolumes::iterator ipRule = ipCallVolumes.end();
	H323IdCallVolumes::iterator h323IdRule  = h323IdCallVolumes.end();
	CLICallVolumes::iterator cliRule  = cliCallVolumes.end();
	
	const PStringToString kv = cfg->GetAllKeyValues(cfgSec);
	for (PINDEX i = 0; i < kv.GetSize(); ++i) {
		PString key = kv.GetKeyAt(i);

		if (key[0] == '%') {
			const PINDEX sepIndex = key.Find('%', 1);
			if (sepIndex != P_MAX_INDEX)
				key = key.Mid(sepIndex + 1).Trim();
		}

		// on Unix multiple entries for the same key are concatenated
		// to one string and separated by a new line
		PStringArray dataLines = kv.GetDataAt(i).Tokenise("\n", FALSE);
		for (PINDEX d = 0; d < dataLines.GetSize(); d++) {
			PString data = dataLines[d];
			InboundCallVolume *rule = NULL;
			bool newIpRule = false, newH323IdRule = false, newCLIRule = false;

			// check the rule type (ip/h323id/cli)
			if (key.Find("ip:") == 0) {
				const PString ip = key.Mid(3).Trim();
			
				NetworkAddress addr;
				if (!(ip == "*" || ip == "any"))
					addr = NetworkAddress(ip);

				ipCallVolumes.resize(ipCallVolumes.size() + 1);
				ipRule = ipCallVolumes.end() - 1;
				ipRule->first = addr;
				ipRule->second.m_sourceAddress = addr;
				newIpRule = true;
	
				rule = &(ipRule->second);
			} else if (key.Find("h323id:") == 0) {
				H225_AliasAddress alias;
				const PString h323id = key.Mid(7).Trim();
				H323SetAliasAddress(h323id, alias, H225_AliasAddress::e_h323_ID);

				h323IdCallVolumes.resize(h323IdCallVolumes.size() + 1);
				h323IdRule = h323IdCallVolumes.end() - 1;
				h323IdRule->first = alias;
				h323IdRule->second.m_sourceH323Id = alias;
				newH323IdRule = true;
				
				rule = &(h323IdRule->second);
			} else if (key.Find("cli:") == 0) {
				const PString cli = key.Mid(4).Trim();

				cliCallVolumes.resize(cliCallVolumes.size() + 1);
				cliRule = cliCallVolumes.end() - 1;
				cliRule->first = string((const char*)cli);
				cliRule->second.m_sourceCLI = string((const char*)cli);
				newCLIRule = true;
				
				rule = &(cliRule->second);
			} else {
				PTRACE(1, "CAPCTRL\tUknown CapacityControl rule: " << key << '=' 
					<< kv.GetDataAt(i)
					);
				continue;
			}

			PStringArray tokens(data.Tokenise(" \t", FALSE));
			if (tokens.GetSize() < 1) {
				PTRACE(1, "CAPCTRL\tInvalid CapacityControl rule syntax: " << key << '=' 
					<< kv.GetDataAt(i)
					);
				if (newIpRule)
					ipCallVolumes.erase(ipRule);
				else if (newH323IdRule)
					h323IdCallVolumes.erase(h323IdRule);
				else if (newCLIRule)
					cliCallVolumes.erase(cliRule);
				continue;
			}
	
			unsigned tno = 0;
			if (tokens.GetSize() >= 2)
				rule->m_prefix = string((const char*)(tokens[tno++]));
			rule->m_maxVolume = tokens[tno++].AsUnsigned();
			
			if (newIpRule)
				++ipRules;
			else if (newH323IdRule)
				++h323IdRules;
			else if (newCLIRule)
				++cliRules;
		} /* for (d) */
	} /* for (i) */

	// sort rules by IP network mask length	
	std::stable_sort(ipCallVolumes.begin(), ipCallVolumes.end(), IpRule_greater());
	std::stable_sort(h323IdCallVolumes.begin(), h323IdCallVolumes.end(), H323IdRule_greater());
	std::stable_sort(cliCallVolumes.begin(), cliCallVolumes.end(), CLIRule_greater());

	// update route entries that have not changed
	for (unsigned i = 0; i < ipCallVolumes.size(); ++i) {
		IpCallVolumes::iterator rule = m_ipCallVolumes.begin();
		while (rule != m_ipCallVolumes.end()) {
			rule = find(
				rule, m_ipCallVolumes.end(), ipCallVolumes[i]
				);
			if (rule == m_ipCallVolumes.end())
				break;
			if (rule->second == ipCallVolumes[i].second) {
				ipCallVolumes[i].second.m_currentVolume = rule->second.m_currentVolume;
				ipCallVolumes[i].second.m_calls = rule->second.m_calls;
			}
			++rule;
		}
	}

	for (unsigned i = 0; i < h323IdCallVolumes.size(); ++i) {
		H323IdCallVolumes::iterator rule = m_h323IdCallVolumes.begin();
		while (rule != m_h323IdCallVolumes.end()) {
			rule = find(
				rule, m_h323IdCallVolumes.end(), h323IdCallVolumes[i]
				);
			if (rule == m_h323IdCallVolumes.end())
				break;
			if (rule->second == h323IdCallVolumes[i].second) {
				h323IdCallVolumes[i].second.m_currentVolume = rule->second.m_currentVolume;
				h323IdCallVolumes[i].second.m_calls = rule->second.m_calls;
			}
			++rule;
		}
	}

	for (unsigned i = 0; i < cliCallVolumes.size(); ++i) {
		CLICallVolumes::iterator rule = m_cliCallVolumes.begin();
		while (rule != m_cliCallVolumes.end()) {
			rule = find(
				rule, m_cliCallVolumes.end(), cliCallVolumes[i]
				);
			if (rule == m_cliCallVolumes.end())
				break;
			if (rule->second == cliCallVolumes[i].second) {
				cliCallVolumes[i].second.m_currentVolume = rule->second.m_currentVolume;
				cliCallVolumes[i].second.m_calls = rule->second.m_calls;
			}
			++rule;
		}
	}
	
	m_ipCallVolumes.clear();
	m_h323IdCallVolumes.clear();
	m_cliCallVolumes.clear();

	m_ipCallVolumes = ipCallVolumes;
	m_h323IdCallVolumes = h323IdCallVolumes;
	m_cliCallVolumes = cliCallVolumes;

	PTRACE(5, "CAPCTRL\t" << ipRules << " IP rules loaded");
#if PTRACING
	if (PTrace::CanTrace(6)) {
		ostream &strm = PTrace::Begin(6, __FILE__, __LINE__);
		strm << "Per IP call volume rules:" << endl;
		for (unsigned i = 0; i < m_ipCallVolumes.size(); ++i) {
			strm << "\tsrc " << m_ipCallVolumes[i].first.AsString() << ":" << endl;
			strm << "\t\t" << m_ipCallVolumes[i].second.AsString() << endl;
		}
		PTrace::End(strm);
	}
#endif

	PTRACE(5, "CAPCTRL\t" << h323IdRules << " H.323 ID rules loaded");
#if PTRACING
	if (PTrace::CanTrace(6)) {
		ostream &strm = PTrace::Begin(6, __FILE__, __LINE__);
		strm << "Per H.323 ID call volume rules:" << endl;
		for (unsigned i = 0; i < m_h323IdCallVolumes.size(); i++) {
			strm << "\tsrc " << H323GetAliasAddressString(m_h323IdCallVolumes[i].first) << ":" << endl;
			strm << "\t\t" << m_h323IdCallVolumes[i].second.AsString() << endl;
		}
		PTrace::End(strm);
	}
#endif

	PTRACE(5, "CAPCTRL\t" << cliRules << " CLI rules loaded");
#if PTRACING
	if (PTrace::CanTrace(6)) {
		ostream &strm = PTrace::Begin(6, __FILE__, __LINE__);
		strm << "Per CLI call volume rules:" << endl;
		for (unsigned i = 0; i < m_cliCallVolumes.size(); i++) {
			strm << "\tsrc " << m_cliCallVolumes[i].first << ":" << endl;
			strm << "\t\t" << m_cliCallVolumes[i].second.AsString() << endl;
		}
		PTrace::End(strm);
	}
#endif
}

CapacityControl::IpCallVolumes::iterator CapacityControl::FindByIp(
	const NetworkAddress &srcIp,
	const PString &calledStationId
	)
{
	unsigned netmaskLen = 0;
	PINDEX matchLen = P_MAX_INDEX;

	const IpCallVolumes::iterator ipEnd = m_ipCallVolumes.end();
	IpCallVolumes::iterator bestIpMatch = ipEnd, i = m_ipCallVolumes.begin();
	while (i != ipEnd) {
		if (bestIpMatch != ipEnd && i->first.GetNetmaskLen() < netmaskLen)
			break;
		if (i->first.IsAny() || srcIp << i->first) {
			PINDEX offset, len;
			if (i->second.m_prefix.empty())
				len = 0;
			else if (!calledStationId.FindRegEx(
					PRegularExpression(i->second.m_prefix.c_str(), PRegularExpression::Extended), offset, len))
				len = P_MAX_INDEX;
			if (len != P_MAX_INDEX && (matchLen == P_MAX_INDEX || len > matchLen)) {
				bestIpMatch = i;
				netmaskLen = i->first.GetNetmaskLen();
				matchLen = len;
			}
		}
		++i;
	}
	
	return bestIpMatch;
}

CapacityControl::H323IdCallVolumes::iterator CapacityControl::FindByH323Id(
	const PString &h323Id,
	const PString &calledStationId
	)
{
	PINDEX matchLen = P_MAX_INDEX;
	const H323IdCallVolumes::iterator h323IdEnd = m_h323IdCallVolumes.end();
	H323IdCallVolumes::iterator bestH323IdMatch = h323IdEnd, i = m_h323IdCallVolumes.begin();
	while (i != h323IdEnd) {
		if (h323Id.IsEmpty())
			break;
		PString alias = H323GetAliasAddressString(i->first);
		if (bestH323IdMatch != h323IdEnd && alias != h323Id)
			break;
		if (alias == h323Id) {
			PINDEX offset, len;
			if (i->second.m_prefix.empty())
				len = 0;
			else if (!calledStationId.FindRegEx(PRegularExpression(i->second.m_prefix.c_str(), PRegularExpression::Extended), offset, len))
				len = P_MAX_INDEX;
			if (len != P_MAX_INDEX && (matchLen == P_MAX_INDEX || len > matchLen)) {
				bestH323IdMatch = i;
				matchLen = len;
			}
		}
		++i;
	}
	
	return bestH323IdMatch;
}

CapacityControl::CLICallVolumes::iterator CapacityControl::FindByCli(
	const std::string &cli,
	const PString &calledStationId
	)
{
	PINDEX matchLen = P_MAX_INDEX;
	const CLICallVolumes::iterator cliEnd = m_cliCallVolumes.end();
	CLICallVolumes::iterator bestCliMatch = cliEnd, i = m_cliCallVolumes.begin();
	while (i != cliEnd) {
		if (cli.empty())
			break;
		if (bestCliMatch != cliEnd && i->first != cli)
			break;
		if (i->first == cli) {
			PINDEX offset, len;
			if (i->second.m_prefix.empty())
				len = 0;
			else if (!calledStationId.FindRegEx(PRegularExpression(i->second.m_prefix.c_str(), PRegularExpression::Extended), offset, len))
				len = P_MAX_INDEX;
			if (len != P_MAX_INDEX && (matchLen == P_MAX_INDEX || len > matchLen)) {
				bestCliMatch = i;
				matchLen = len;
			}
		}
		++i;
	}
	
	return bestCliMatch;
}

void CapacityControl::LogCall(
	const NetworkAddress &srcIp,
	const PString &srcAlias,
	const std::string &srcCli,
	const PString &calledStationId,
	PINDEX callNumber,
	bool callStart
	)
{
	IpCallVolumes::iterator bestIpMatch = FindByIp(srcIp, calledStationId);
	if (bestIpMatch != m_ipCallVolumes.end()) {
		PTRACE(5, "CAPCTRL\tCall #" << callNumber
			<< " to " << calledStationId << " matched IP rule " << bestIpMatch->first.AsString()
			<< "\t" << bestIpMatch->second.AsString()
			);
		PWaitAndSignal lock(m_updateMutex);
		if (callStart) {
			++(bestIpMatch->second.m_currentVolume);
			bestIpMatch->second.m_calls.push_back(callNumber);
		} else {
			if (find(bestIpMatch->second.m_calls.begin(), bestIpMatch->second.m_calls.end(), 
					callNumber) != bestIpMatch->second.m_calls.end()) {
				--(bestIpMatch->second.m_currentVolume);
				bestIpMatch->second.m_calls.remove(callNumber);
			}
		}
		return;
	}

	H323IdCallVolumes::iterator bestH323IdMatch = FindByH323Id(srcAlias, calledStationId);
	if (bestH323IdMatch != m_h323IdCallVolumes.end()) {
		PTRACE(5, "CAPCTRL\tCall #" << callNumber
			<< " to " << calledStationId << " matched H323.ID rule " << H323GetAliasAddressString(bestH323IdMatch->first)
			<< "\t" << bestH323IdMatch->second.AsString()
			);
		PWaitAndSignal lock(m_updateMutex);
		if (callStart) {
			++(bestH323IdMatch->second.m_currentVolume);
			bestH323IdMatch->second.m_calls.push_back(callNumber);
		} else {
			if (find(bestH323IdMatch->second.m_calls.begin(), bestH323IdMatch->second.m_calls.end(), 
					callNumber) != bestH323IdMatch->second.m_calls.end()) {
				--(bestH323IdMatch->second.m_currentVolume);
				bestH323IdMatch->second.m_calls.remove(callNumber);
			}
		}
		return;
	}

	CLICallVolumes::iterator bestCliMatch = FindByCli(srcCli, calledStationId);
	if (bestCliMatch != m_cliCallVolumes.end()) {
		PTRACE(5, "CAPCTRL\tCall #" << callNumber
			<< " to " << calledStationId << " matched CLI rule " << bestCliMatch->first
			<< "\t" << bestCliMatch->second.AsString()
			);
		PWaitAndSignal lock(m_updateMutex);
		if (callStart) {
			++(bestCliMatch->second.m_currentVolume);
			bestCliMatch->second.m_calls.push_back(callNumber);
		} else {
			if (find(bestCliMatch->second.m_calls.begin(), bestCliMatch->second.m_calls.end(), 
					callNumber) != bestCliMatch->second.m_calls.end()) {
				--(bestCliMatch->second.m_currentVolume);
				bestCliMatch->second.m_calls.remove(callNumber);
			}
		}
		return;
	}
}

bool CapacityControl::CheckCall(
	const NetworkAddress &srcIp,
	const PString &srcAlias,
	const std::string &srcCli,
	const PString &calledStationId
	)
{
	IpCallVolumes::iterator bestIpMatch = FindByIp(srcIp, calledStationId);
	if (bestIpMatch != m_ipCallVolumes.end()) {
		PTRACE(5, "CAPCTRL\tCall from IP " << srcIp.AsString()
			<< " to " << calledStationId << " matched IP rule " << bestIpMatch->first.AsString()
			<< "\t" << bestIpMatch->second.AsString()
			);
		PWaitAndSignal lock(m_updateMutex);
		return bestIpMatch->second.m_currentVolume < bestIpMatch->second.m_maxVolume;
	}

	H323IdCallVolumes::iterator bestH323IdMatch = FindByH323Id(srcAlias, calledStationId);
	if (bestH323IdMatch != m_h323IdCallVolumes.end()) {
		PTRACE(5, "CAPCTRL\tCall to " << calledStationId << " matched H323.ID rule "
			<< H323GetAliasAddressString(bestH323IdMatch->first)
			<< "\t" << bestH323IdMatch->second.AsString()
			);
		PWaitAndSignal lock(m_updateMutex);
		return bestH323IdMatch->second.m_currentVolume < bestH323IdMatch->second.m_maxVolume;
	}

	CLICallVolumes::iterator bestCliMatch = FindByCli(srcCli, calledStationId);
	if (bestCliMatch != m_cliCallVolumes.end()) {
		PTRACE(5, "CAPCTRL\tCall to " << calledStationId << " matched CLI rule "
			<< bestCliMatch->first << "\t" << bestCliMatch->second.AsString()
			);
		PWaitAndSignal lock(m_updateMutex);
		return bestCliMatch->second.m_currentVolume < bestCliMatch->second.m_maxVolume;
	}
	
	return true;
}

namespace {

class CapCtrlAcct : public GkAcctLogger
{
public:
	enum Constants {
		/// events recognized by this module
		CapCtrlAcctEvents = AcctStart | AcctStop
	};
	
	/// Create a logger that updates information about inbound traffic
	CapCtrlAcct( 
		/// name from Gatekeeper::Acct section
		const char* moduleName
		);
		
	/** Log accounting event.
	
		@return
		Status of this logging operation (see #Status enum#)
	*/
	virtual Status Log( 
		AcctEvent evt, /// accounting event to log
		const callptr& call /// additional data for the event
		);

private:
	/* No copy constructor allowed */
	CapCtrlAcct(const CapCtrlAcct&);
	/* No operator= allowed */
	CapCtrlAcct& operator=(const CapCtrlAcct&);
	
private:
	CapacityControl *m_capacityControl;
};

} // end of anonymous namespace

CapCtrlAcct::CapCtrlAcct(
	const char* moduleName
	) : GkAcctLogger(moduleName), m_capacityControl(CapacityControl::Instance())
{
	SetSupportedEvents(CapCtrlAcctEvents);
}

GkAcctLogger::Status CapCtrlAcct::Log(
	GkAcctLogger::AcctEvent evt, 
	const callptr &call
	)
{
	if ((evt & GetEnabledEvents() & GetSupportedEvents()) == 0)
		return Next;
	if (!call) {
		PTRACE(1, "GKACCT\t" << GetName() << " - missing call info for event " << evt);
		return Fail;
	}
		
	PIPSocket::Address addr;
	WORD port;
	call->GetSrcSignalAddr(addr, port);

	PString h323Id = GetBestAliasAddressString(
		call->GetSourceAddress(), true, AliasAddressTagMask(H225_AliasAddress::e_h323_ID)
		);
	if (h323Id.IsEmpty() && call->GetCallingParty())
		h323Id = GetBestAliasAddressString(
			call->GetCallingParty()->GetAliases(), true,
			AliasAddressTagMask(H225_AliasAddress::e_h323_ID)
			);

	std::string cli((const char*)(GetCallingStationId(call)));

	m_capacityControl->LogCall(addr, h323Id, cli, GetCalledStationId(call), call->GetCallNumber(), evt == AcctStart);

	return Ok;
}

namespace {

/// Authenticator module for controlling inbound traffic
/// To be used together with CapacityControl accounting module
class CapCtrlAuth : public GkAuthenticator
{
public:
	enum SupportedChecks {
		CapCtrlAuthMiscChecks = e_Setup | e_SetupUnreg
	};
	
	/// build authenticator reading settings from the config
	CapCtrlAuth(
		/// name for this authenticator and for the config section to read settings from
		const char* authName,
		/// RAS check events supported by this module
		unsigned supportedRasChecks = 0,
		/// Misc check events supported by this module
		unsigned supportedMiscChecks = CapCtrlAuthMiscChecks
		);
	
	/** Authenticate using data from Q.931 Setup message.
	
		@return:
		#GkAuthenticator::Status enum# with the result of authentication.
	*/
	virtual int Check(
		/// Q.931/H.225 Setup message to be authenticated
		SetupMsg &setup,
		/// authorization data (call duration limit, reject reason, ...)
		SetupAuthData& authData
		);

private:
	CapCtrlAuth();
	CapCtrlAuth(const CapCtrlAuth&);
	CapCtrlAuth& operator=(const CapCtrlAuth&);

private:
	CapacityControl *m_capacityControl;
};

} // end of anonymous namespace

CapCtrlAuth::CapCtrlAuth(
	const char* authName,
	unsigned supportedRasChecks,
	unsigned supportedMiscChecks
	) 
	: 
	GkAuthenticator(authName, supportedRasChecks, supportedMiscChecks), 
	m_capacityControl(CapacityControl::Instance())
{
}

int CapCtrlAuth::Check(
	/// Q.931/H.225 Setup message to be authenticated
	SetupMsg &setup,
	/// authorization data (call duration limit, reject reason, ...)
	SetupAuthData& authData
	)
{
	PString h323Id;
	PIPSocket::Address addr;
	setup.GetPeerAddr(addr);

	if (authData.m_call) {
		h323Id = GetBestAliasAddressString(
			authData.m_call->GetSourceAddress(), true, AliasAddressTagMask(H225_AliasAddress::e_h323_ID)
			);
		if (h323Id.IsEmpty() && authData.m_call->GetCallingParty())
			h323Id = GetBestAliasAddressString(
				authData.m_call->GetCallingParty()->GetAliases(), true,
				AliasAddressTagMask(H225_AliasAddress::e_h323_ID)
				);
	}
	
	if (!m_capacityControl->CheckCall(
			addr, h323Id, (const char*)(authData.m_callingStationId),
			authData.m_calledStationId)) {
		authData.m_rejectCause = Q931::NoCircuitChannelAvailable;
		return e_fail;
	} else
		return e_ok;
}

namespace {
GkAcctLoggerCreator<CapCtrlAcct> CapCtrlAcctLoggerCreator("CapacityControl");
GkAuthCreator<CapCtrlAuth> CapCtrlAuthCreator("CapacityControl");
}
