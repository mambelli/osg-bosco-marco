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

#ifndef VMGAHP_ERROR_CODES_H_INCLUDE
#define VMGAHP_ERROR_CODES_H_INCLUDE

// COMMON 
#define VMGAHP_ERR_VM_NOT_FOUND "VMGAHP_ERR_VM_NOT_FOUND"
#define VMGAHP_ERR_INTERNAL "VMGAHP_ERR_INTERNAL"
#define VMGAHP_ERR_VM_INVALID_OPERATION "VMGAHP_ERR_VM_INVALID_OPERATION"
#define VMGAHP_ERR_CRITICAL "VMGAHP_ERR_CRITICAL"
#define VMGAHP_ERR_CANNOT_CREATE_ISO_FILE "VMGAHP_ERR_CANNOT_CREATE_ISO_FILE"
#define VMGAHP_ERR_CANNOT_READ_CDROM_FILE "VMGAHP_ERR_CANNOT_READ_CDROM_FILE"
#define VMGAHP_ERR_CANNOT_CREATE_ARG_FILE "VMGAHP_ERR_CANNOT_CREATE_ARG_FILE"

// VM START
#define VMGAHP_ERR_NO_JOBCLASSAD_INFO "VMGAHP_ERR_NO_JOBCLASSAD_INFO"
#define VMGAHP_ERR_NO_SUPPORTED_VM_TYPE "VMGAHP_ERR_NO_SUPPORTED_VM_TYPE"
#define VMGAHP_ERR_VM_NOT_ENOUGH_MEMORY "VMGAHP_ERR_VM_NOT_ENOUGH_MEMORY"
#define VMGAHP_ERR_XEN_INVALID_GUEST_KERNEL "VMGAHP_ERR_XEN_INVALID_GUEST_KERNEL"
#define VMGAHP_ERR_XEN_VBD_CONNECT_ERROR "VMGAHP_ERR_XEN_VBD_CONNECT_ERROR"
#define VMGAHP_ERR_XEN_CONFIG_FILE_ERROR "VMGAHP_ERR_XEN_CONFIG_FILE_ERROR"

// VM CHECKPOINT
#define VMGAHP_ERR_VM_CANNOT_CREATE_CKPT_FILES "VMGAHP_ERR_VM_CANNOT_CREATE_CKPT_FILES"
#define VMGAHP_ERR_VM_NO_SUPPORT_CHECKPOINT "VMGAHP_ERR_VM_NO_SUPPORT_CHECKPOINT"

// VM RESUME/SUSPEND
#define VMGAHP_ERR_VM_INVALID_SUSPEND_FILE "VMGAHP_ERR_VM_INVALID_SUSPEND_FILE"
#define VMGAHP_ERR_VM_NO_SUPPORT_SUSPEND "VMGAHP_ERR_VM_NO_SUPPORT_SUSPEND"

// Job ClassAd error
#define VMGAHP_ERR_JOBCLASSAD_NO_VM_MEMORY_PARAM "VMGAHP_ERR_JOBCLASSAD_NO_VM_MEMORY_PARAM"
#define VMGAHP_ERR_JOBCLASSAD_TOO_MUCH_MEMORY_REQUEST "VMGAHP_ERR_JOBCLASSAD_TOO_MUCH_MEMORY_REQUEST"
#define VMGAHP_ERR_JOBCLASSAD_MISMATCHED_NETWORKING "VMGAHP_ERR_JOBCLASSAD_MISMATCHED_NETWORKING"
#define VMGAHP_ERR_JOBCLASSAD_MISMATCHED_NETWORKING_TYPE "VMGAHP_ERR_JOBCLASSAD_MISMATCHED_NETWORKING_TYPE"
#define VMGAHP_ERR_JOBCLASSAD_MISMATCHED_HARDWARE_VT "VMGAHP_ERR_JOBCLASSAD_MISMATCHED_HARDWARE_VT"

// Error for Xen
#define VMGAHP_ERR_JOBCLASSAD_XEN_NO_KERNEL_PARAM "VMGAHP_ERR_JOBCLASSAD_XEN_NO_KERNEL_PARAM"
#define VMGAHP_ERR_JOBCLASSAD_XEN_NO_ROOT_DEVICE_PARAM "VMGAHP_ERR_JOBCLASSAD_XEN_NO_ROOT_DEVICE_PARAM"
#define VMGAHP_ERR_JOBCLASSAD_XEN_NO_DISK_PARAM "VMGAHP_ERR_JOBCLASSAD_XEN_NO_DISK_PARAM"
#define VMGAHP_ERR_JOBCLASSAD_XEN_INVALID_DISK_PARAM "VMGAHP_ERR_JOBCLASSAD_XEN_INVALID_DISK_PARAM"
#define VMGAHP_ERR_JOBCLASSAD_XEN_KERNEL_NOT_FOUND "VMGAHP_ERR_JOBCLASSAD_XEN_KERNEL_NOT_FOUND"
#define VMGAHP_ERR_JOBCLASSAD_XEN_INITRD_NOT_FOUND "VMGAHP_ERR_JOBCLASSAD_XEN_INITRD_NOT_FOUND"
#define VMGAHP_ERR_JOBCLASSAD_XEN_NO_CDROM_DEVICE "VMGAHP_ERR_JOBCLASSAD_XEN_NO_CDROM_DEVICE"
#define VMGAHP_ERR_JOBCLASSAD_XEN_MISMATCHED_CHECKPOINT "VMGAHP_ERR_JOBCLASSAD_XEN_MISMATCHED_CHECKPOINT"

// Error for VMware
#define VMGAHP_ERR_JOBCLASSAD_NO_VMWARE_DIR_PARAM "VMGAHP_ERR_JOBCLASSAD_NO_VMWARE_DIR_PARAM"
#define VMGAHP_ERR_JOBCLASSAD_NO_VMWARE_VMX_PARAM "VMGAHP_ERR_JOBCLASSAD_NO_VMWARE_VMX_PARAM"
#define VMGAHP_ERR_JOBCLASSAD_NO_VMWARE_VMDK_PARAM "VMGAHP_ERR_JOBCLASSAD_NO_VMWARE_VMDK_PARAM"
#define VMGAHP_ERR_JOBCLASSAD_VMWARE_VMX_NOT_FOUND "VMGAHP_ERR_JOBCLASSAD_VMWARE_VMX_NOT_FOUND"
#define VMGAHP_ERR_JOBCLASSAD_VMWARE_VMX_ERROR "VMGAHP_ERR_JOBCLASSAD_VMWARE_VMX_ERROR"
#define VMGAHP_ERR_JOBCLASSAD_VMWARE_NO_CDROM_DEVICE "VMGAHP_ERR_JOBCLASSAD_VMWARE_NO_CDROM_DEVICE"

#endif
