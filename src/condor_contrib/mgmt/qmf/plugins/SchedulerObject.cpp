/*
 * Copyright 2009-2011 Red Hat, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "condor_common.h"
#include "condor_config.h"

#include "SchedulerObject.h"

#include "ArgsSchedulerSubmitJob.h"
#include "ArgsSchedulerSetJobAttribute.h"
#include "ArgsSchedulerHoldJob.h"
#include "ArgsSchedulerReleaseJob.h"
#include "ArgsSchedulerRemoveJob.h"

#ifndef WIN32
	#include "stdint.h"
#endif

#include "PoolUtils.h"
#include "JobUtils.h"
#include "Utils.h"

#include "condor_attributes.h"

#include "condor_debug.h"

#include "condor_qmgr.h"

#include "MgmtConversionMacros.h"

#ifndef READ_ONLY_SCHEDULER_OBJECT
// Global Scheduler object, used for needReschedule
#include "../condor_schedd.V6/scheduler.h"
extern Scheduler scheduler;
extern char * Name;
extern bool qmgmt_all_users_trusted;
#endif

using namespace com::redhat::grid;
using namespace qmf::com::redhat::grid;
using namespace qpid::management;
using namespace qpid::types;


SchedulerObject::SchedulerObject(qpid::management::ManagementAgent *agent,
					const char* _name)
{
	mgmtObject = new qmf::com::redhat::grid::Scheduler(agent, this);

	// By default the scheduler will be persistent.
	bool _lifetime = param_boolean("QMF_IS_PERSISTENT", true);
	agent->addObject(mgmtObject, _name, _lifetime);
}


SchedulerObject::~SchedulerObject()
{
	if (mgmtObject) {
		mgmtObject->resourceDestroy();
	}
}


void
SchedulerObject::update(const ClassAd &ad)
{
	MGMT_DECLARATIONS;

	mgmtObject->set_Pool(GetPoolName());

	STRING(CondorPlatform);
	STRING(CondorVersion);
	TIME_INTEGER(DaemonStartTime);
	TIME_INTEGER(JobQueueBirthdate);
	STRING(Machine);
	INTEGER(MaxJobsRunning);
	INTEGER(MonitorSelfAge);
	DOUBLE(MonitorSelfCPUUsage);
	DOUBLE(MonitorSelfImageSize);
	INTEGER(MonitorSelfRegisteredSocketCount);
	INTEGER(MonitorSelfResidentSetSize);
	TIME_INTEGER(MonitorSelfTime);
	STRING(MyAddress);
		//TIME_INTEGER(MyCurrentTime);
	STRING(Name);
	INTEGER(NumUsers);
	STRING(MyAddress);
	INTEGER(TotalHeldJobs);
	INTEGER(TotalIdleJobs);
	INTEGER(TotalJobAds);
	INTEGER(TotalRemovedJobs);
	INTEGER(TotalRunningJobs);

    INTEGER(WindowedStatWidth);
    INTEGER(UpdateInterval);

    INTEGER(JobsSubmitted);
    DOUBLE(JobSubmissionRate);
    INTEGER(JobsCompleted);
    DOUBLE(JobCompletionRate);
    INTEGER(JobsExited);
    INTEGER(ShadowExceptions);
    INTEGER(JobsSubmittedCum);
    INTEGER(JobsCompletedCum);
    INTEGER(JobsExitedCum);
    INTEGER(ShadowExceptionsCum);
    INTEGER(JobsStartedCum);
    INTEGER(JobsStarted);
    DOUBLE(JobStartRate);
    DOUBLE(MeanTimeToStartCum);
    DOUBLE(MeanRunningTimeCum);
    INTEGER64(SumTimeToStartCum);
    INTEGER64(SumRunningTimeCum);
    DOUBLE(MeanTimeToStart);
    DOUBLE(MeanRunningTime);

	mgmtObject->set_System(mgmtObject->get_Machine());

	// debug
	if (DebugFlags & D_FULLDEBUG) {
		const_cast<ClassAd*>(&ad)->dPrint(D_FULLDEBUG|D_NOHEADER);
	}
}


#ifndef READ_ONLY_SCHEDULER_OBJECT
Manageable::status_t
SchedulerObject::Submit(Variant::Map &jobAdMap, std::string &id, std::string &text)
{
	int cluster;
	int proc;

	// our mandatory set of attributes for a submit
	const char* required[] = {
				ATTR_JOB_CMD,
				ATTR_REQUIREMENTS,
				ATTR_OWNER,
				ATTR_JOB_IWD,
				NULL
				};

		// 1. Create transaction
	BeginTransaction();

		// 2. Create cluster
	if (-1 == (cluster = NewCluster())) {
		AbortTransaction();
		text = "Failed to create new cluster";
		return STATUS_USER + 1;
	}

		// 3. Create proc
	if (-1 == (proc = NewProc(cluster))) {
		AbortTransaction();
		text = "Failed to create new proc";
		return STATUS_USER + 2;
	}

		// 4. Submit job ad

		// Schema: (vanilla job)
		// Schedd demands - Owner, JobUniverse
		// To run - JobStatus, Requirements

		// Schedd excepts if no Owner
		// Schedd prunes on startup if no Owner or JobUniverse
		// Schedd won't run job without JobStatus
		// Job cannot match without Requirements
		// Shadow rejects jobs without an Iwd
		// Shadow: Job has no CondorVersion, assuming pre version 6.3.3
		// Shadow: Unix Vanilla job is pre version 6.3.3, setting 'TransferFiles = "NEVER"'
		// Starter won't run job without Cmd
		// Starter needs a valid Owner (local account name) if not using nobody
		// condor_q requires ClusterId (int), ProcId (int), QDate (int), RemoteUserCpu (float), JobStatus (int), JobPrio (int), ImageSize (int), Owner (str) and Cmd (str)

		// Schema: (vm job)

	ClassAd ad;
	int universe;

	if (!PopulateAdFromVariantMap(jobAdMap, ad, text)) {
		AbortTransaction();
		return STATUS_USER + 3;
	}

	std::string missing;
	if (!CheckRequiredAttrs(ad, required, missing)) {
		AbortTransaction();
		text = "Job ad is missing required attributes: " + missing;
		return STATUS_USER + 4;
	}

    // ShouldTransferFiles - unset by default, must be set
    ad.Assign(ATTR_SHOULD_TRANSFER_FILES, "NO");

		// EARLY SET: These attribute are set early so the incoming ad
		// has a change to override them.
	::SetAttribute(cluster, proc, ATTR_JOB_STATUS, "1"); // 1 = idle

		// Junk that condor_q wants, but really shouldn't be necessary
	::SetAttribute(cluster, proc, ATTR_JOB_REMOTE_USER_CPU, "0.0"); // float
	::SetAttribute(cluster, proc, ATTR_JOB_PRIO, "0");              // int
	::SetAttribute(cluster, proc, ATTR_IMAGE_SIZE, "0");            // int

	if (!ad.LookupInteger(ATTR_JOB_UNIVERSE, universe)) {
		char* uni_str = param("DEFAULT_UNIVERSE");
		if (!uni_str) {
			universe = CONDOR_UNIVERSE_VANILLA;
		}
		else {
			universe = CondorUniverseNumber(uni_str);
		}
		::SetAttributeInt(cluster, proc, ATTR_JOB_UNIVERSE, universe );
	}
	// more stuff - without these our idle stats are whack
	if ( universe != CONDOR_UNIVERSE_MPI && universe != CONDOR_UNIVERSE_PVM ) {
		::SetAttribute(cluster, proc, ATTR_MAX_HOSTS, "1");              // int
		::SetAttribute(cluster, proc, ATTR_MIN_HOSTS, "1");            // int
	}
	::SetAttribute(cluster, proc, ATTR_CURRENT_HOSTS, "0"); // int

	ExprTree *expr;
	const char *name;
	std::string value;
	ad.ResetExpr();
	while (ad.NextExpr(name,expr)) {

			// All these extra lookups are horrible. They have to
			// be there because the ClassAd may have multiple
			// copies of the same attribute name! This means that
			// the last attribute with a given name will set the
			// value, but the last attribute tends to be the
			// attribute with the oldest (wrong) value. How
			// annoying is that!
		if (!(expr = ad.Lookup(name))) {
			dprintf(D_ALWAYS, "Failed to lookup %s\n", name);

			AbortTransaction();
			text = "Failed to parse job ad attribute";
			return STATUS_USER + 6;
		}

        value = ExprTreeToString(expr);
        ::SetAttribute(cluster, proc, name, value.c_str());
	}

		// LATE SET: These attributes are set late, after the incoming
		// ad, so they override whatever the incoming ad set.
	char buf[22]; // 22 is max size for an id, 2^32 + . + 2^32 + \0
	snprintf(buf, 22, "%d", cluster);
	::SetAttribute(cluster, proc, ATTR_CLUSTER_ID, buf);
	snprintf(buf, 22, "%d", proc);
	::SetAttribute(cluster, proc, ATTR_PROC_ID, buf);
	snprintf(buf, 22, "%d", time(NULL));
	::SetAttribute(cluster, proc, ATTR_Q_DATE, buf);

		// Could check for some invalid attributes, e.g
		//  JobUniverse <= CONDOR_UNIVERSE_MIN or >= CONDOR_UNIVERSE_MAX

		// 5. Commit transaction
	CommitTransaction();


		// 6. Reschedule
	scheduler.needReschedule();


		// 7. Return identifier
		// TODO: dag ids?
	MyString tmp;
	tmp.sprintf("%s#%d.%d", Name, cluster, proc);
	id = tmp.Value();

	return STATUS_OK;
}

Manageable::status_t
SchedulerObject::SetAttribute(std::string key,
							  std::string name,
							  std::string value,
							  std::string &text)
{
	PROC_ID id = getProcByString(key.c_str());
	if (id.cluster < 0 || id.proc < 0) {
		dprintf(D_FULLDEBUG, "SetAttribute: Failed to parse id: %s\n", key.c_str());
		text = "Invalid Id";
		return STATUS_USER + 0;
	}

	if (IsSubmissionChange(name.c_str())) {
		text = "Changes to submission name not allowed";
		return STATUS_USER + 1;
	}

	if (IsKeyword(name.c_str())) {
		text = "Attribute name is reserved: " + name;
		return STATUS_USER + 2;
	}

	if (!IsValidAttributeName(name,text)) {
		return STATUS_USER + 3;
	}

		// All values are strings in the eyes of
		// ::SetAttribute. Their type is inferred when read from
		// the ClassAd log. It is important that the incoming
		// value is properly quoted to differentiate between EXPR
		// and STRING.
	if (::SetAttribute(id.cluster,
					   id.proc,
					   name.c_str(),
					   value.c_str())) {
		text = "Failed to set attribute " + name + " to " + value;
		return STATUS_USER + 4;
	}

	return STATUS_OK;
}


Manageable::status_t
SchedulerObject::Hold(std::string key, std::string &reason, std::string &text)
{
	PROC_ID id = getProcByString(key.c_str());
	if (id.cluster < 0 || id.proc < 0) {
		dprintf(D_FULLDEBUG, "Hold: Failed to parse id: %s\n", key.c_str());
		text = "Invalid Id";
		return STATUS_USER + 0;
	}

	if (!holdJob(id.cluster,
				 id.proc,
				 reason.c_str(),
				 true, // Always perform this action within a transaction
				 true, // Always notify the shadow of the hold
				 false, // Do not email the user about this action
				 false, // Do not email admin about this action
				 false // This is not a system related (internal) hold
				 )) {
		text = "Failed to hold job";
		return STATUS_USER + 1;
	}

	return STATUS_OK;
}


Manageable::status_t
SchedulerObject::Release(std::string key, std::string &reason, std::string &text)
{
	PROC_ID id = getProcByString(key.c_str());
	if (id.cluster < 0 || id.proc < 0) {
		dprintf(D_FULLDEBUG, "Release: Failed to parse id: %s\n", key.c_str());
		text = "Invalid Id";
		return STATUS_USER + 0;
	}

	if (!releaseJob(id.cluster,
					id.proc,
					reason.c_str(),
					true, // Always perform this action within a transaction
					false, // Do not email the user about this action
					false // Do not email admin about this action
					)) {
		text = "Failed to release job";
		return STATUS_USER + 1;
	}

	return STATUS_OK;
}


Manageable::status_t
SchedulerObject::Remove(std::string key, std::string &reason, std::string &text)
{
	PROC_ID id = getProcByString(key.c_str());
	if (id.cluster < 0 || id.proc < 0) {
		dprintf(D_FULLDEBUG, "Remove: Failed to parse id: %s\n", key.c_str());
		text = "Invalid Id";
		return STATUS_USER + 0;
	}

	if (!abortJob(id.cluster,
				  id.proc,
				  reason.c_str(),
				  true // Always perform within a transaction
				  )) {
		text = "Failed to remove job";
		return STATUS_USER + 1;
	}

	return STATUS_OK;
}

Manageable::status_t
SchedulerObject::Suspend(std::string key, std::string &reason, std::string &text)
{
	PROC_ID id = getProcByString(key.c_str());
	if (id.cluster < 0 || id.proc < 0) {
		dprintf(D_FULLDEBUG, "Suspend: Failed to parse id: %s\n", key.c_str());
		text = "Invalid Id";
		return STATUS_USER + 0;
	}

	scheduler.enqueueActOnJobMyself(id,JA_SUSPEND_JOBS,true);

	return STATUS_OK;
}

Manageable::status_t
SchedulerObject::Continue(std::string key, std::string &reason, std::string &text)
{
	PROC_ID id = getProcByString(key.c_str());
	if (id.cluster < 0 || id.proc < 0) {
		dprintf(D_FULLDEBUG, "Continue: Failed to parse id: %s\n", key.c_str());
		text = "Invalid Id";
		return STATUS_USER + 0;
	}

	scheduler.enqueueActOnJobMyself(id,JA_CONTINUE_JOBS,true);

	return STATUS_OK;
}
#endif

qpid::management::ManagementObject *
SchedulerObject::GetManagementObject(void) const
{
	return mgmtObject;
}


Manageable::status_t
SchedulerObject::ManagementMethod(uint32_t methodId,
								  Args &args,
								  std::string &text)
{
#ifndef READ_ONLY_SCHEDULER_OBJECT
	Manageable::status_t result = STATUS_OK;
	bool orig_qaut = qmgmt_all_users_trusted;
	qmgmt_all_users_trusted = true;

	switch (methodId) {
	case qmf::com::redhat::grid::Scheduler::METHOD_ECHO:
		if (!param_boolean("QMF_MANAGEMENT_METHOD_ECHO", false)) return STATUS_NOT_IMPLEMENTED;

		break;
	case qmf::com::redhat::grid::Scheduler::METHOD_SUBMITJOB:
		result = Submit(((ArgsSchedulerSubmitJob &) args).i_Ad,
					  ((ArgsSchedulerSubmitJob &) args).o_Id,
					  text);
		break;
	case qmf::com::redhat::grid::Scheduler::METHOD_SETJOBATTRIBUTE:
		result = SetAttribute(((ArgsSchedulerSetJobAttribute &) args).i_Id,
							((ArgsSchedulerSetJobAttribute &) args).i_Name,
							((ArgsSchedulerSetJobAttribute &) args).i_Value,
							text);
		break;
	case qmf::com::redhat::grid::Scheduler::METHOD_HOLDJOB:
		result = Hold(((ArgsSchedulerHoldJob &) args).i_Id,
					((ArgsSchedulerHoldJob &) args).i_Reason,
					text);
		break;
	case qmf::com::redhat::grid::Scheduler::METHOD_RELEASEJOB:
		result = Release(((ArgsSchedulerReleaseJob &) args).i_Id,
					   ((ArgsSchedulerReleaseJob &) args).i_Reason,
					   text);
		break;
	case qmf::com::redhat::grid::Scheduler::METHOD_REMOVEJOB:
		result = Remove(((ArgsSchedulerRemoveJob &) args).i_Id,
					  ((ArgsSchedulerRemoveJob &) args).i_Reason,
					  text);
		break;
	case qmf::com::redhat::grid::Scheduler::METHOD_SUSPENDJOB:
		result = Suspend(((ArgsSchedulerRemoveJob &) args).i_Id,
					  ((ArgsSchedulerRemoveJob &) args).i_Reason,
					  text);
		break;
	case qmf::com::redhat::grid::Scheduler::METHOD_CONTINUEJOB:
		result = Continue(((ArgsSchedulerRemoveJob &) args).i_Id,
					  ((ArgsSchedulerRemoveJob &) args).i_Reason,
					  text);
		break;
	default:
		result = STATUS_NOT_IMPLEMENTED;
	}

	qmgmt_all_users_trusted = orig_qaut;
	return result;
#else
	return STATUS_NOT_IMPLEMENTED;
#endif
}

bool
SchedulerObject::AuthorizeMethod(uint32_t methodId, Args& args, const std::string& userId) {
	dprintf(D_FULLDEBUG, "AuthorizeMethod: checking '%s'\n", userId.c_str());
	if (0 == userId.compare("cumin")) {
		return true;
	}
	return false;
}
