//////////////////////////////////////////////////////////////////
//
// gk.h gatekeeper process
//
// This work is published under the GNU Public License (GPL)
// see file COPYING for details.
// We also explicitely grant the right to link this code
// with the OpenH323 library.
//
//////////////////////////////////////////////////////////////////


#ifndef GK_H
#define GK_H "@(#) $Id: gk.h,v 1.27 2007/03/21 22:30:16 willamowius Exp $"

#include "version.h"
#include <ptlib/pprocess.h>


class GkTimer;
class Gatekeeper : public PProcess 
{
	PCLASSINFO(Gatekeeper, PProcess)
 public:
	Gatekeeper
		(const char * manuf = "GNU", 
		 const char * name = "Gatekeeper", 
		 WORD majorVersion = GNUGK_MAJOR_VERSION,
		 WORD minorVersion = GNUGK_MINOR_VERSION,
		 CodeStatus status = GNUGK_BUILD_TYPE,
		 WORD buildNumber = GNUGK_BUILD_NUMBER);

	virtual void Main();

#if PTRACING
	enum RotationIntervals {
		Hourly,
		Daily,
		Weekly,
		Monthly,
		RotationIntervalMax
	};
	
	static bool SetLogFilename(
		const PString& filename
		);
		
	static bool RotateLogFile();
	static bool ReopenLogFile();	
	static void CloseLogFile();

	static void EnableLogFileRotation(
		bool enable = true
		);

	/** Rotate the log file, saving old file contents to a different
	    file and starting with a new one. This is a callback function
	    called when the rotation timer expires.
	*/
	static void RotateOnTimer(
		GkTimer* timer /// timer object that triggered rotation
		);
#endif // PTRACING

 protected:
	/** returns the template string for which the cmommand line is parsed */
	virtual const PString GetArgumentsParseString() const;

	/**@name Initialization 
	 * A sequence of virtual initialization methods is called from #Main#
	 * before the fun starts. 
	 * Each one takes the already parsed command line arguments (so you can
	 * depend the behavior on them). Later -- after #InitConfig# -- you can 
	 * also use #Toolkit::Config()# to decide different things.
	 * Every method may return #FALSE# to abort #Main# and end the program.
	 */
	//@{

	/** installs the signal handlers; First called init method. */
	virtual BOOL InitHandlers(const PArgList &args);

	/** factory for the static toolkit; Called after #InitHandlers#.  */
	virtual BOOL InitToolkit(const PArgList &args);

	/** factory for the static Config in Toolkit; Called after #InitToolkit# */
	virtual BOOL InitConfig(const PArgList &args);

	/** initiates logging and tracing; Called after #InitConfig# */
	virtual BOOL InitLogging(const PArgList &args);

	/** print the available command-line-options **/
	void PrintOpts(void);

	/** Set a new user and group (ownership) for the GK process.
		The group that will be set is the user's default group.
	*/
	virtual bool SetUserAndGroup(const PString &username);

	//@}

private:
#if PTRACING
	/// parse rotation interval from the config
	static void GetRotateInterval(
		PConfig& cfg, /// the config
		const PString& section /// name of the config section to check
		);
#endif

private:
#if PTRACING
	/// rotate file after the specified period of time (if >= 0)
	static int m_rotateInterval;
	/// a minute when the interval based rotation should occur
	static int m_rotateMinute;
	/// an hour when the interval based rotation should occur
	static int m_rotateHour;
	/// day of the month (or of the week) for the interval based rotation
	static int m_rotateDay;
	/// timer for rotation events
	static GkTimer* m_rotateTimer;
	/// gatekeeper log file
	static PTextFile* m_logFile;
	/// filename for the logfile
	static PFilePath m_logFilename;
	/// atomic log file operations (rotation, closing)
	static PMutex m_logFileMutex;
	/// human readable names for rotation intervals
	static const char* const m_intervalNames[];
#endif // PTRACING
};

#endif // GK_H
