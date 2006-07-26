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

#ifndef _PIPE_WIN32_H
#define _PIPE_WIN32_H

// This class encalsulates our wonky pipe implementation on Windows. On
// Windows, DaemonCore pipes are monitored by the PID-watcher thread. The
// PipeEnd class (which has the ReadPipeEnd and WritePipeEnd subclasses)
// exposes three methods for use by the PID-watcher thread:
//    * pre_wait() - called before each call to WaitFormMultipleObjects, returns
//                   an Event to wait on
//    * post_wait() - called after WaitForMultipleObjects if Event handle is signaled
//    * set_unregistred() - called in response to a PidEntry's deallocate flag
//                          being set

#include <windows.h>

class PipeEnd {

public:
	PipeEnd(HANDLE handle, bool overlapped, bool nonblocking, int pipe_size);
	~PipeEnd();

	// retrieve the underlying pipe handle
	HANDLE get_handle() const { return m_handle; }

	// called from Register_Pipe
	void set_registered();

	// called from WatchPid to indicate that a PID-watcher
	// thread is using this object
	void set_watched();

	// called from either the PID-watcher thread when it sees
	// the deallocate flag or from cancel()
	void set_unregistered();

	// called from Cancel_Pipe. returns when the object is no
	// longer registered
	void cancel();

	// called from PID-watcher thread before WaitForMultipleObjects
	virtual HANDLE pre_wait() = 0;

	// called from PID-watcher thread after the overlapped I/O's event
	// object is signaled. returns true if the handler is ready to be
	// fired, false if not
	virtual bool post_wait() = 0;

	// checked from DaemonCore::Driver to ensure the pipe is ready
	// for I/O before calling the handler
	virtual bool io_ready() = 0;

protected:
	// the underlying pipe handle
	HANDLE m_handle;

	// the size of the underlying pipe buffer
	const int m_pipe_size;

	// properties of the pipe as specified in DaemonCore::Create_Pipe
	// (or DaemonCore::Inherit_Pipe)
	const bool m_overlapped;
	const bool m_nonblocking;

	// whether the pipe is registered with DaemonCore
	bool m_registered;

	// an event for waiting until this object is no longer
	// managed by a PID-watcher thread
	HANDLE m_watched_event;

	// objects used for overlapped I/O operations
	HANDLE m_event;
	OVERLAPPED m_overlapped_struct;

};

class ReadPipeEnd : public PipeEnd {

public:
	ReadPipeEnd(HANDLE h, bool o, bool n, int sz) :
	  PipeEnd(h, o, n, sz), m_async_io_state(IO_UNSTARTED), m_async_io_error(0) { }

	HANDLE pre_wait();
	bool post_wait();

	bool io_ready();

	int read(void* buffer, int len);
	
private:
	enum async_io_state_t {IO_UNSTARTED, IO_PENDING, IO_DONE};
	async_io_state_t m_async_io_state;

	char m_async_io_buf;
	DWORD m_async_io_error;

	int read_helper(void* buffer, int len);
};

class WritePipeEnd : public PipeEnd {

public:
	WritePipeEnd(HANDLE h, bool o, bool n, int sz) :
	  PipeEnd(h, o, n, sz), m_async_io_buf(NULL), m_async_io_error(0) { }

	HANDLE pre_wait();
	bool post_wait();

	bool io_ready();

	// don't let Close_Pipe close us if we have an outstanding async write
	bool needs_delayed_close() { return m_async_io_buf != NULL; }

	int write(const void* buffer, int len);

	// tries to finish the outstanding async operation
	// returns true on success, false if it would block
	// and the nonblocking param is true
	bool complete_async_write(bool nonblocking);

private:
	char* m_async_io_buf;
	int m_async_io_size;
	DWORD m_async_io_error;

	int async_write_helper();
};

#endif