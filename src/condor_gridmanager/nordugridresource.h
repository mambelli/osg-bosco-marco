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

#ifndef NORDUGRIDRESOURCE_H
#define NORDUGRIDRESOURCE_H

#include "condor_common.h"
#include "../condor_daemon_core.V6/condor_daemon_core.h"

#include "nordugridjob.h"
#include "baseresource.h"

#define ACQUIRE_DONE		0
#define ACQUIRE_QUEUED		1
#define ACQUIRE_FAILED		2

class NordugridJob;
class NordugridResource;

class NordugridResource : public BaseResource
{
 public:

	NordugridResource( const char *resource_name );
	~NordugridResource();

	bool IsEmpty();

	void Reconfig();
	void RegisterJob( NordugridJob *job );
	void UnregisterJob( NordugridJob *job );

	int AcquireConnection( NordugridJob *job, ftp_lite_server *&server );
	void ReleaseConnection( NordugridJob *job );

	static NordugridResource *FindOrCreateResource( const char *resource_name );

 private:
	bool OpenConnection();
	bool CloseConnection();

	static HashTable <HashKey, NordugridResource *> ResourcesByName;

	List<NordugridJob> *registeredJobs;
	NordugridJob *connectionUser;
	List<NordugridJob> *connectionWaiters;
	ftp_lite_server *ftpServer;
};

#endif