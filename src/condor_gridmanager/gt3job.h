/***************************Copyright-DO-NOT-REMOVE-THIS-LINE**
 * CONDOR Copyright Notice
 *
 * See LICENSE.TXT for additional notices and disclaimers.
 *
 * Copyright (c)1990-1998 CONDOR Team, Computer Sciences Department, 
 * University of Wisconsin-Madison, Madison, WI.  All Rights Reserved.  
 * No use of the CONDOR Software Program Source Code is authorized 
 * without the express consent of the CONDOR Team.  For more information 
 * contact: CONDOR Team, Attention: Professor Miron Livny, 
 * 7367 Computer Sciences, 1210 W. Dayton St., Madison, WI 53706-1685, 
 * (608) 262-0856 or miron@cs.wisc.edu.
 *
 * U.S. Government Rights Restrictions: Use, duplication, or disclosure 
 * by the U.S. Government is subject to restrictions as set forth in 
 * subparagraph (c)(1)(ii) of The Rights in Technical Data and Computer 
 * Software clause at DFARS 252.227-7013 or subparagraphs (c)(1) and 
 * (2) of Commercial Computer Software-Restricted Rights at 48 CFR 
 * 52.227-19, as applicable, CONDOR Team, Attention: Professor Miron 
 * Livny, 7367 Computer Sciences, 1210 W. Dayton St., Madison, 
 * WI 53706-1685, (608) 262-0856 or miron@cs.wisc.edu.
****************************Copyright-DO-NOT-REMOVE-THIS-LINE**/

#ifndef GT3JOB_H
#define GT3JOB_H

#include "condor_common.h"
#include "condor_classad.h"
#include "MyString.h"
#include "globus_utils.h"
#include "classad_hashtable.h"

#include "proxymanager.h"
#include "basejob.h"
#include "gt3resource.h"
#include "gahp-client.h"

#define JM_COMMIT_TIMEOUT	600

class GT3Resource;

/////////////////////from gridmanager.h
class GT3Job;
extern HashTable <HashKey, GT3Job *> GT3JobsByContact;

const char *gt3JobId( const char *contact );
void gramCallbackHandler( void *user_arg, char *job_contact, int state,
						  int errorcode );

void GT3JobInit();
void GT3JobReconfig();
bool GT3JobAdMustExpand( const ClassAd *jobad );
BaseJob *GT3JobCreate( ClassAd *jobad );
extern const char *GT3JobAdConst;
///////////////////////////////////////

class GT3Job : public BaseJob
{
 public:

	GT3Job( ClassAd *classad );

	~GT3Job();

	void Reconfig();
	int doEvaluateState();
	void NotifyResourceDown();
	void NotifyResourceUp();
	void UpdateGlobusState( int new_state, int new_error_code );
	void GramCallback( int new_state, int new_error_code );
	bool GetCallbacks();
	void ClearCallbacks();
	BaseResource *GetResource();

	static int probeInterval;
	static int submitInterval;
	static int restartInterval;
	static int gahpCallTimeout;
	static int maxConnectFailures;
	static int outputWaitGrowthTimeout;

	static void setProbeInterval( int new_interval )
		{ probeInterval = new_interval; }
	static void setSubmitInterval( int new_interval )
		{ submitInterval = new_interval; }
	static void setRestartInterval( int new_interval )
		{ restartInterval = new_interval; }
	static void setGahpCallTimeout( int new_timeout )
		{ gahpCallTimeout = new_timeout; }
	static void setConnectFailureRetry( int count )
		{ maxConnectFailures = count; }

	// New variables
	bool resourceDown;
	bool resourceStateKnown;
	int gmState;
	int globusState;
	int globusStateErrorCode;
	int globusStateBeforeFailure;
	int callbackGlobusState;
	int callbackGlobusStateErrorCode;
	bool resourcePingPending;
	bool jmUnreachable;
	bool jmDown;
	GT3Resource *myResource;
	time_t lastProbeTime;
	bool probeNow;
	time_t enteredCurrentGmState;
	time_t enteredCurrentGlobusState;
	time_t lastSubmitAttempt;
	int numSubmitAttempts;
	int submitFailureCode;
	int lastRestartReason;
	time_t lastRestartAttempt;
	int numRestartAttempts;
	int numRestartAttemptsThisSubmit;
	time_t jmProxyExpireTime;
	time_t outputWaitLastGrowth;
	int outputWaitOutputSize;
	int outputWaitErrorSize;
	// HACK!
	bool retryStdioSize;
	char *resourceManagerString;
	bool useGridJobMonitor;

		// TODO should query these from GahpClient when needed
	char *gassServerUrl;
	char *gramCallbackContact;


	Proxy *myProxy;
	GahpClient *gahp;

	MyString *buildSubmitRSL();
	void DeleteOutput();

	char *jobContact;
		// If we're in the middle of a globus call that requires an RSL,
		// the RSL is stored here (so that we don't have to reconstruct the
		// RSL every time we test the call for completion). It should be
		// freed and reset to NULL once the call completes.
	MyString *RSL;
	MyString errorString;
	char *localOutput;
	char *localError;
	bool streamOutput;
	bool streamError;
	bool stageOutput;
	bool stageError;
	int globusError;

	bool restartingJM;
	time_t restartWhen;

	int numGlobusSubmits;

 protected:
	bool callbackRegistered;
	int connect_failure_counter;
	bool AllowTransition( int new_state, int old_state );
};

#endif
