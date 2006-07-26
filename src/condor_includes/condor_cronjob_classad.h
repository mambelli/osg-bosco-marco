/***************************Copyright-DO-NOT-REMOVE-THIS-LINE**
  *
  * Condor Software Copyright Notice
  * Copyright (C) 1990-2006, Condor Team, Computer Sciences Department,
  * University of Wisconsin-Madison, WI.
  *
  * This source code is covered by the Condor Public License, which can
  * be found in the accompanying LICENSE.TXT file, or online at
  * www.condorproject.org.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  * AND THE UNIVERSITY OF WISCONSIN-MADISON "AS IS" AND ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  * WARRANTIES OF MERCHANTABILITY, OF SATISFACTORY QUALITY, AND FITNESS
  * FOR A PARTICULAR PURPOSE OR USE ARE DISCLAIMED. THE COPYRIGHT
  * HOLDERS AND CONTRIBUTORS AND THE UNIVERSITY OF WISCONSIN-MADISON
  * MAKE NO MAKE NO REPRESENTATION THAT THE SOFTWARE, MODIFICATIONS,
  * ENHANCEMENTS OR DERIVATIVE WORKS THEREOF, WILL NOT INFRINGE ANY
  * PATENT, COPYRIGHT, TRADEMARK, TRADE SECRET OR OTHER PROPRIETARY
  * RIGHT.
  *
  ****************************Copyright-DO-NOT-REMOVE-THIS-LINE**/
#ifndef _CONDOR_CRONJOB_CLASSAD_H
#define _CONDOR_CRONJOB_CLASSAD_H

#include "condor_cronjob.h"
#include "env.h"

// Define a "ClassAd" 'Cron' job
class ClassAdCronJob : public CronJobBase
{
  public:
	ClassAdCronJob( const char *mgrName, const char *jobName );
	virtual ~ClassAdCronJob( );
	int Initialize( void );

  private:
	virtual int ProcessOutput( const char *line );
	virtual int Publish( const char *name, ClassAd *ad ) = 0;

	ClassAd		*OutputAd;
	int			OutputAdCount;

	Env         classad_env;
	MyString	mgrNameUc;
};

#endif /* _CONDOR_CRONJOB_CLASSAD_H */