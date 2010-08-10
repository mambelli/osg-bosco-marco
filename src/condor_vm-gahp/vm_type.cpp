/***************************************************************
 *
 * Copyright (C) 1990-2007, Condor Team, Computer Sciences Department,
 * University of Wisconsin-Madison, WI.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You may
 * obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************/

#include "condor_common.h"
#include "condor_config.h"
#include "condor_string.h"
#include "string_list.h"
#include "condor_attributes.h"
#include "condor_classad.h"
#include "condor_arglist.h"
#include "MyString.h"
#include "util_lib_proto.h"
#include "setenv.h"
#include "vmgahp_common.h"
#include "vmgahp.h"
#include "vmgahp_error_codes.h"
#include "condor_vm_universe_types.h"
#include "vm_type.h"
#include "condor_mkstemp.h"

extern VMGahp *vmgahp;

VMType::VMType(const char* prog_for_script, const char* scriptname, const char* workingpath, ClassAd *ad)
{
	m_vm_id = vmgahp->getNewVMId();
	m_prog_for_script = prog_for_script;
	m_scriptname = scriptname;
	m_workingpath = workingpath;
	m_classAd = *ad;
	m_vm_pid = 0;
	m_vm_mem = 0;
	m_vm_networking = false;
	m_vm_checkpoint = false;
	m_vm_no_output_vm = false; 
	m_vm_hardware_vt = false; 
	m_vm_transfer_cdrom_files = false;
	m_local_iso = false;
	m_has_iso = false;
	m_is_soft_suspended = false;
	m_self_shutdown = false;
	m_is_checkpointed = false;
	m_status = VM_STOPPED;
	m_cpu_time = 0;
	m_delete_working_files = false;
	m_vcpus = 1;

	vmprintf(D_FULLDEBUG, "Constructed VM_Type.\n");

	// Use the script program to create a configuration file for VM ?
	m_use_script_to_create_config = 
		param_boolean("USE_SCRIPT_TO_CREATE_CONFIG", false);

	// Create initially transfered file list
	createInitialFileList();

}

VMType::~VMType()
{
	if( m_vm_pid > 0 && daemonCore ) {
		daemonCore->Send_Signal(m_vm_pid, SIGKILL);
	}
	m_vm_id = 0;
	m_vm_pid = 0;
	m_status = VM_STOPPED;
	m_cpu_time = 0;

	if( m_delete_working_files && !m_is_checkpointed ) {
		if( m_workingpath.IsEmpty() == false ) {
			// We will delete all files in the working directory.
			Directory working_dir(m_workingpath.Value(), PRIV_USER);
			working_dir.Remove_Entire_Directory();
		}
	}
}

bool 
VMType::parseCommonParamFromClassAd(bool is_root /*false*/)
{
	// Read common parameters for vm
	
	m_result_msg = "";
	// Read the amount of memory for VM
	if( m_classAd.LookupInteger( ATTR_JOB_VM_MEMORY, m_vm_mem) != 1 ) {
		vmprintf(D_ALWAYS, "%s cannot be found in job classAd\n", 
				ATTR_JOB_VM_MEMORY);
		m_result_msg = VMGAHP_ERR_JOBCLASSAD_NO_VM_MEMORY_PARAM;
		return false;
	}else {
		// Requested memory should not be larger than the maximum allowed memory
		if( ( m_vm_mem <= 0 ) || 
				(m_vm_mem > vmgahp->m_gahp_config->m_vm_max_memory) ) {
			m_result_msg = VMGAHP_ERR_JOBCLASSAD_TOO_MUCH_MEMORY_REQUEST;
			return false;
		}
	}
	vmprintf(D_FULLDEBUG, "Memory: %d\n", m_vm_mem);

	vmprintf(D_FULLDEBUG, "Looking up number of vcpus.\n");
	// Read the number of vcpus
	if( m_classAd.LookupInteger( ATTR_JOB_VM_VCPUS, m_vcpus) != 1) {
	  vmprintf(D_FULLDEBUG, "No VCPUS defined or VCPUS definition is bad.\n");
	}
	if(m_vcpus < 1) m_vcpus = 1;
	vmprintf(D_FULLDEBUG, "Setting up %d CPUS\n", m_vcpus);

	// Read parameter for networking
	m_vm_networking = false;
	m_classAd.LookupBool( ATTR_JOB_VM_NETWORKING, m_vm_networking);

	if( m_vm_networking && 
			(vmgahp->m_gahp_config->m_vm_networking == false ) ) {
		vmprintf(D_ALWAYS, "A job requests networking but " 
				"the gahp server doesn't support networking\n");
		m_result_msg = VMGAHP_ERR_JOBCLASSAD_MISMATCHED_NETWORKING;
		return false;
	}

	if( m_vm_networking ) {
		// Read parameter for networking types
		if( m_classAd.LookupString( ATTR_JOB_VM_NETWORKING_TYPE, 
					m_vm_networking_type) == 1 ) {
			// vm_networking_type is defined

			// change string to lowercase
			m_vm_networking_type.trim();
			m_vm_networking_type.lower_case();
			if( vmgahp->m_gahp_config->m_vm_networking_types.contains(m_vm_networking_type.Value()) == false ) {
				vmprintf(D_ALWAYS, "Networking type(%s) is not supported by "
						"this gahp server\n", m_vm_networking_type.Value());
				m_result_msg = VMGAHP_ERR_JOBCLASSAD_MISMATCHED_NETWORKING_TYPE;
				return false;
			}
		}else {
			// vm_networking_type is undefined
			if( vmgahp->m_gahp_config->m_vm_default_networking_type.IsEmpty() == false ) {
				m_vm_networking_type = vmgahp->m_gahp_config->m_vm_default_networking_type;
			}else {
				m_vm_networking_type = "nat";
			}
		}
		if(m_classAd.LookupString( ATTR_JOB_VM_MACADDR, m_vm_job_mac) != 1)
		  {
		    vmprintf(D_FULLDEBUG, "MAC address was not defined.\n");
		  }
	}

	// Read parameter for checkpoint
	m_vm_checkpoint = false;
	m_classAd.LookupBool(ATTR_JOB_VM_CHECKPOINT, m_vm_checkpoint);

	if( VMType::createVMName(&m_classAd, m_vm_name) == false ) {
		m_result_msg = VMGAHP_ERR_CRITICAL;
		return false;
	}

	// Insert vm_name to classAd for future use
	m_classAd.Assign("VMPARAM_VM_NAME", m_vm_name); 

	// Read the parameter of hardware VT
	m_vm_hardware_vt = false; 
	m_classAd.LookupBool(ATTR_JOB_VM_HARDWARE_VT, m_vm_hardware_vt);
	if( m_vm_hardware_vt && 
			(vmgahp->m_gahp_config->m_vm_hardware_vt == false ) ) {
		vmprintf(D_ALWAYS, "A job requests hardware virtualization but " 
				"the vmgahp server doesn't support hardware VT\n");
		m_result_msg = VMGAHP_ERR_JOBCLASSAD_MISMATCHED_HARDWARE_VT;
		return false;
	}

	// Read the parameter of vm_no_output_vm
	// This parameter is an experiment parameter
	// When this parameter is TRUE, Condor will not transfer 
	// all files for VM back to a job user.
	// This parameter would be used if a job user uses explict method 
	// to get output files from VM. 
	// For example, if a job user uses a ftp program 
	// to send output files inside VM to his/her dedicated machine for ouput, 
	// the job user doesn't want to get modified VM disk files back. 
	// So the job user can use this parameter
	m_vm_no_output_vm = false; 
	m_classAd.LookupBool(VMPARAM_NO_OUTPUT_VM, m_vm_no_output_vm);

	// Read CDROM files from Job classAd
	m_vm_transfer_cdrom_files = false;
	m_classAd.LookupBool(VMPARAM_TRANSFER_CDROM_FILES, m_vm_transfer_cdrom_files);

	MyString cdrom_files;
	if( m_classAd.LookupString(VMPARAM_CDROM_FILES, cdrom_files) == 1 ) {
		cdrom_files.trim();
	}

	m_iso_file = "";
	m_local_iso = false;
	m_has_iso = false;
	m_vm_cdrom_files.clearAll();

	if( cdrom_files.IsEmpty() == false ) {
		StringList cd_file_list(NULL, ",");
		cd_file_list.initializeFromString(cdrom_files.Value());
	
		if( cd_file_list.isEmpty() == false ) {
			// check if cdrom files are readable by user
			const char *fname = NULL;
			cd_file_list.rewind();
			while( (fname = cd_file_list.next()) != NULL ) {
				MyString tmp_fullname;
				isTransferedFile(fname, tmp_fullname);

				// check if this file is readable
				if( check_vm_read_access_file(tmp_fullname.Value(), 
							false) == false ) {
					vmprintf(D_ALWAYS, "file(%s) for CDROM cannot "
							"be read\n", tmp_fullname.Value());
					m_result_msg = VMGAHP_ERR_CANNOT_READ_CDROM_FILE;
					return false;
				}
			}

			m_vm_cdrom_files.create_union(cd_file_list, false);

			// If cd_file_list has an iso file, 
			// we don't need to create a ISO file.
			// We will just use the iso file directly.
			if( cd_file_list.number() == 1 ) {
				cd_file_list.rewind();

				const char *file_name = cd_file_list.next();
				if( has_suffix(file_name, ".iso") ) {
					// check if this file was transferred.
					MyString tmp_fullname;
					if( isTransferedFile(file_name, tmp_fullname) ) {
						// this file was transferred.
						m_local_iso = true;
					}else {
						// this file is not transferred.
						m_local_iso = false;
					}

					m_iso_file = tmp_fullname;
					m_has_iso = true;
				}
			}
		}
	}

	m_classad_arg = "";
	ArgList arglist;
	MyString error_msg;
	if(!arglist.GetArgsStringV1or2Raw(&m_classAd, &m_classad_arg, &error_msg)) {
		m_classad_arg = "";
	}

	if( m_classad_arg.IsEmpty() == false ) {
		if( m_has_iso ) {
			vmprintf(D_ALWAYS, "A job user defined both an ISO file and "
					"'Argument' in a job description file. But 'Argument' "
					"cannot be used with an iso file together\n");
			m_result_msg = VMGAHP_ERR_CANNOT_CREATE_ARG_FILE;
			return false;
		}

		if( cdrom_files.find(VM_UNIV_ARGUMENT_FILE, 0) >= 0 ) {
			vmprintf(D_ALWAYS, "A file with the same filename '%s' "
					"is already in '%s'\n", VM_UNIV_ARGUMENT_FILE, 
					VMPARAM_CDROM_FILES);
			m_result_msg = VMGAHP_ERR_CANNOT_CREATE_ARG_FILE;
			return false;
		}

		// Create a file for arguments
		FILE *argfile_fp = safe_fopen_wrapper(VM_UNIV_ARGUMENT_FILE, "w");
		if( !argfile_fp ) {
			vmprintf(D_ALWAYS, "failed to safe_fopen_wrapper the file "
					"for arguments: safe_fopen_wrapper(%s) returns %s\n", 
					VM_UNIV_ARGUMENT_FILE, strerror(errno));
			m_result_msg = VMGAHP_ERR_CANNOT_CREATE_ARG_FILE;
			return false;
		}
		if( fprintf(argfile_fp, "%s", m_classad_arg.Value()) < 0) {
			fclose(argfile_fp);
			unlink(VM_UNIV_ARGUMENT_FILE);
			vmprintf(D_ALWAYS, "failed to fprintf in CreateConfigFile(%s:%s)\n",
					VM_UNIV_ARGUMENT_FILE, strerror(errno));
			m_result_msg = VMGAHP_ERR_CANNOT_CREATE_ARG_FILE;
			return false;
		}
		fclose(argfile_fp);

		m_arg_file.sprintf("%s%c%s", m_workingpath.Value(), 
				DIR_DELIM_CHAR, VM_UNIV_ARGUMENT_FILE);
		// Add arg file to cdrom list
		m_vm_cdrom_files.append(m_arg_file.Value());
	}
	return true;
}

vm_status 
VMType::getVMStatus(void)
{
	return m_status;
}

void 
VMType::setVMStatus(vm_status status)
{
	m_status = status;
}

void 
VMType::setLastStatus(const char *result_msg)
{
	m_last_status_time.getTime();
	m_last_status_result = result_msg;
}

void
VMType::createInitialFileList()
{
	MyString intermediate_files;
	StringList intermediate_file_list(NULL, ",");
	MyString input_files;
	StringList input_file_list(NULL, ",");

	m_initial_working_files.clearAll();
	m_transfer_intermediate_files.clearAll();
	m_transfer_input_files.clearAll();

	// Create m_initial_working_files
	find_all_files_in_dir(m_workingpath.Value(), m_initial_working_files, true);

	// Read Intermediate files from Job classAd
	m_classAd.LookupString( ATTR_TRANSFER_INTERMEDIATE_FILES, intermediate_files);
	if( intermediate_files.IsEmpty() == false ) {
		intermediate_file_list.initializeFromString(intermediate_files.Value());
	}

	// Read Input files from Job classAd
	m_classAd.LookupString( ATTR_TRANSFER_INPUT_FILES, input_files);
	if( input_files.IsEmpty() == false ) {
		input_file_list.initializeFromString(input_files.Value());
	}

	// Create m_transfer_intermediate_files and m_transfer_input_files with fullpath.

	const char *tmp_file = NULL;
	m_initial_working_files.rewind();
	while( (tmp_file = m_initial_working_files.next()) != NULL ) {
		// Create m_transfer_intermediate_files
		if( filelist_contains_file(tmp_file, 
					&intermediate_file_list, true) ) {
			m_transfer_intermediate_files.append(tmp_file);
		}
		// Create m_transfer_input_files
		if( filelist_contains_file(tmp_file, 
							&input_file_list, true) ) {
			m_transfer_input_files.append(tmp_file);
		}
	}
}

void
VMType::deleteNonTransferredFiles()
{
	// Rebuild m_initial_working_files
	m_initial_working_files.clearAll();

	// Find all files in working directory
	find_all_files_in_dir(m_workingpath.Value(), m_initial_working_files, true);

	const char *tmp_file = NULL;
	m_initial_working_files.rewind();
	while( (tmp_file = m_initial_working_files.next()) != NULL ) {
		if( m_transfer_input_files.contains(tmp_file) == false ) {
			// This file was created after starting a job
			unlink(tmp_file);
			m_initial_working_files.deleteCurrent();
		}
	}
	m_transfer_intermediate_files.clearAll();
}

// Create a name representing a virtual machine
// Usually this name is used in vm config file
bool 
VMType::createVMName(ClassAd *ad, MyString& vmname)
{
	if( !ad ) {
		return false;
	}

	if( create_name_for_VM(ad, vmname) == false ) {
		vmprintf(D_ALWAYS, "Cannot make the name of VM\n");
		return false;
	}
	return true;
}

// Create a temporary file in working directory
// If suffix is given, the temporary file will have the suffix
// The last six characters of template_string must be XXXXXX.
// outname is the full path name of a created file;
bool
VMType::createTempFile(const char *template_string, const char *suffix, MyString &outname)
{
	MyString tmp_config_name;
	tmp_config_name.sprintf("%s%c%s",m_workingpath.Value(), 
			DIR_DELIM_CHAR, template_string);

	char *config_name = strdup(tmp_config_name.Value() );
	ASSERT(config_name);

	int config_fd = -1;
	config_fd = condor_mkstemp( config_name );
	if( config_fd < 0 ) {
		vmprintf(D_ALWAYS, "condor_mkstemp(%s) returned %d, '%s' (errno %d) "
				"in VMType::createTempFile()\n", config_name, config_fd,
				strerror(errno), errno );
		free(config_name);
		return false;
	}
	close(config_fd);

	outname = config_name;

	if( suffix ) {
		tmp_config_name.sprintf("%s%s",config_name, suffix);

		if( rename(config_name, tmp_config_name.Value()) < 0 ) {
			vmprintf(D_ALWAYS, "Cannot rename the temporary config file(%s), '%s' (errno %d) in "
					"VMType::createTempFile()\n", tmp_config_name.Value(), 
					strerror(errno), errno );
			free(config_name);
			return false;
		}
		outname = tmp_config_name;
	}
	free(config_name);
	return true;
}

bool
VMType::createISOConfigAndName(StringList *cd_files, MyString &isoconf, MyString &isofile)
{
	if( !cd_files || cd_files->isEmpty() ) {
		return false;
	}

	isoconf = "";
	isofile = "";

	MyString tmp_config;
	if( createTempFile("isoXXXXXX", ".config", tmp_config) == false ) {
		vmprintf(D_ALWAYS, "Temporary config file for ISO cannot be created\n");
		return false;
	}

	FILE *config_fp = safe_fopen_wrapper(tmp_config.Value(), "w");
	if( !config_fp ) {
		vmprintf(D_ALWAYS, "failed to safe_fopen_wrapper ISO config file "
				": safe_fopen_wrapper(%s) returns %s\n", 
				tmp_config.Value(), strerror(errno));
		return false;
	}

	cd_files->rewind();
	const char* tmp_file = NULL;
	while( (tmp_file = cd_files->next() ) != NULL ) {
		MyString tmp_fullname;

		// if a file was transferred, make full path with working directory.
		isTransferedFile(tmp_file, tmp_fullname);

		// check if this file is readable
		if( check_vm_read_access_file(tmp_fullname.Value()) == false ) {
			vmprintf(D_ALWAYS, "file(%s) for CDROM cannot be read\n", 
					tmp_fullname.Value());
			fclose(config_fp);
			unlink(tmp_config.Value());
			return false;
		}
		if( fprintf(config_fp, "%s\n", tmp_fullname.Value()) < 0 ) {
			fclose(config_fp);
			unlink(tmp_config.Value());
			vmprintf(D_ALWAYS, "failed to fprintf in createISOConfigAndName(%s:%s)\n",
					tmp_config.Value(), strerror(errno));
			return false;
		}
	}
	fclose(config_fp);

	// Make the name of ISO image
	MyString iso_name(tmp_config);
	iso_name.replaceString(".config", ".iso");

	isoconf = tmp_config;
	isofile = iso_name;
	return true;
}

bool
VMType::createISO()
{
	vmprintf(D_FULLDEBUG, "Inside VMType::createISO\n");

	m_iso_file = "";

	if( m_scriptname.IsEmpty() || m_vm_cdrom_files.isEmpty() ) {
		return false;
	}

	MyString tmp_config;
	MyString tmp_file;
	if( createISOConfigAndName(&m_vm_cdrom_files, tmp_config, 
				tmp_file) == false ) {
		return false;
	}

	ArgList systemcmd;
	if( m_prog_for_script.IsEmpty() == false ) {
		systemcmd.AppendArg(m_prog_for_script);
	}
	systemcmd.AppendArg(m_scriptname);
	systemcmd.AppendArg("createiso");
	systemcmd.AppendArg(tmp_config);
	systemcmd.AppendArg(tmp_file);

	bool need_root = false;
	if( !strcasecmp(m_vmtype.Value(), CONDOR_VM_UNIVERSE_XEN) ) {
		need_root = true;
	}

	int result = systemCommand(systemcmd, need_root);
	if( result != 0 ) {
		return false;
	}

#if defined(LINUX)	
	// To avoid lazy-write behavior to disk
	sync();
#endif

	unlink(tmp_config.Value());
	m_iso_file = tmp_file;
	m_local_iso = true;

	// Insert the name of created iso file to classAd for future use
	m_classAd.Assign("VMPARAM_ISO_NAME", condor_basename(m_iso_file.Value()));
	return true;
}

// check if a file was transferred.
// if so, fullname will have full path in working directory.
// Otherwise, fullname will be same to file_name
bool 
VMType::isTransferedFile(const char* file_name, MyString& fullname) 
{
	if( !file_name || m_initial_working_files.isEmpty() ) {
		return false;
	}

	// check if this file was transferred.
	MyString tmp_fullname;
	if( filelist_contains_file(file_name,
				&m_initial_working_files, true) ) {
		// this file was transferred.
		// make full path with workingdir
		tmp_fullname.sprintf("%s%c%s", m_workingpath.Value(), 
				DIR_DELIM_CHAR, condor_basename(file_name));
		fullname = tmp_fullname;
		return true;
	}else {
		// this file is not transferred
		if( fullpath(file_name) == false ) {
			vmprintf(D_ALWAYS, "Warning: The file(%s) doesn't have "
					"full path even though it is not "
					"transferred\n", file_name);
		}
		fullname = file_name;
		return false;
	}
	return false;
}

bool
VMType::createConfigUsingScript(const char* configfile)
{
	vmprintf(D_FULLDEBUG, "Inside VMType::createConfigUsingScript\n");

	if( !configfile || m_scriptname.IsEmpty() ) {
		return false;
	}

	// Set temporary environments for script program
	MyString name;
	MyString value;
	StringList name_list;

	ExprTree* expr = NULL;
	ExprTree* L_expr;
	ExprTree* R_expr;

	m_classAd.ResetExpr();
	while( (expr = m_classAd.NextExpr()) != NULL ) {
		L_expr = expr->LArg();
		L_expr->PrintToStr(name);

		R_expr = expr->RArg();
		R_expr->PrintToStr(value);

		if( !strncasecmp( name.Value(), "JobVM", strlen("JobVM") ) ||
			!strncasecmp( name.Value(), "VMPARAM", strlen("VMPARAM") )) {

			name_list.append(name.Value());
			SetEnv(name.Value(), value.Value());
		}
	}

	ArgList systemcmd;
	if( m_prog_for_script.IsEmpty() == false ) {
		systemcmd.AppendArg(m_prog_for_script);
	}
	systemcmd.AppendArg(m_scriptname);
	systemcmd.AppendArg("createconfig");
	systemcmd.AppendArg(configfile);

	bool need_root = false;
	if( !strcasecmp(m_vmtype.Value(), CONDOR_VM_UNIVERSE_XEN) ) {
		need_root = true;
	}

	int result = systemCommand(systemcmd, need_root);

	// UnSet temporary environments for script program
	const char *tmp_name = NULL;
	name_list.rewind();
	while( (tmp_name = name_list.next()) != NULL ) {
		UnsetEnv(tmp_name);
	}

	if( result != 0 ) {
		vmprintf(D_ALWAYS, "Failed to create Configuration file('%s') using "
				"script program('%s')\n", configfile, 
				m_scriptname.Value());
		return false;
	}
	return true;
}
