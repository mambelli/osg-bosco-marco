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

#ifndef CONDOR_QMGR_JOB_UPDATER_H
#define CONDOR_QMGR_JOB_UPDATER_H

#include "condor_common.h"
#include "condor_attributes.h"
#include "condor_classad.h"
#include "condor_io.h"
#include "../condor_daemon_core.V6/condor_daemon_core.h"


// What kind of update to the job queue are we performing?
typedef enum { 
	U_NONE = 0,
	U_PERIODIC,
	U_TERMINATE,
	U_HOLD,
	U_REMOVE,
	U_REQUEUE,
	U_EVICT
} update_t;


class QmgrJobUpdater : public Service
{
public:
	QmgrJobUpdater( ClassAd* job_ad, const char* schedd_addr );
	~QmgrJobUpdater();

	void startUpdateTimer( void );

		/** Connect to the job queue and update all relevent
			attributes of the job class ad.  This checks our job
			classad to find any dirty attributes, and compares them
			against the lists of attribute names we care about.  
			@param type What kind of update we want to do
			@return true on success, false on failure
		*/
	bool updateJob( update_t type );

		/** Connect to the job queue and update one attribute.
			WARNING: This method is BAD NEWS for schedd scalability.
			We make a whole new qmgmt connection for *every* attribute
			we update.  Worse yet, we use this method to service a
			pseudo syscall from the user job (pseudo_set_job_attr), so
			the schedd can be held hostage by user-jobs that call this
			syscall repeatedly.  :(
		*/
	bool updateAttr( const char *name, const char *expr );

		/** Add the given attribute to our list of attributes we
			should watch for changes and update.  The type specifies
			if it's a special attribute that should only be updated on
			a given kind of event.  If not specified, the type
			defaults to 0, which means the given attribute should be
			added to the list of common attributes that should always
			be updated.
			@param attr Attribute we want to add
			@param type What kind of updates does this attribute
			   matter for?
		    @return true if added, false if it was already there
		*/
	bool watchAttribute( const char* attr, update_t type = U_NONE );

private:

		/** Initialize our StringLists for attributes we want to keep
			updated in the job queue itself
		*/ 
	void initJobQueueAttrLists( void );

		/** Timer handler which just calls updateJobInQueue with the
			right arguments so we do a periodic update.
		*/
	void periodicUpdateQ( void );


		/** Update a specific attribute from our job ad into the
			queue.  This checks the type of the given ExprTree and
			calls the appropriate SetAttribute* function to do the
			work.  This function assumes you're in the middle of a
			qmgmt operation, i.e., that you've called ConnectQ().
			@param The ExprTree you want to update in the job queue 
			@return success or failure to set the attribute
		 */
	bool updateExprTree( ExprTree* tree );

		/// Pointers to lists of attribute names we care about

		/** Attributes that should go in the job queue regardless of
			what action we're taking with this job. */
	StringList* common_job_queue_attrs;
		/// Attributes specific to certain kinds of updates
	StringList* hold_job_queue_attrs;
	StringList* evict_job_queue_attrs;
	StringList* remove_job_queue_attrs;
	StringList* requeue_job_queue_attrs;
	StringList* terminate_job_queue_attrs;

	ClassAd* job_ad;
	char* schedd_addr;

	int cluster;
	int proc;

	int q_update_tid;
};	


#endif /* CONDOR_QMGR_JOB_UPDATER_H */