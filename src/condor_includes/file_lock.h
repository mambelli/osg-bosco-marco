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

#ifndef FILE_LOCK_H
#define FILE_LOCK_H

#include "condor_constants.h"
#include <stdio.h>	// for FILE

typedef enum { READ_LOCK, WRITE_LOCK, UN_LOCK } LOCK_TYPE;

#if defined(__cplusplus)
extern "C" {
#endif

/* Lock/unlock a file using the OS's file locking mechanism.
 *
 * lock_file_plain() doesn't call any Condor code. It can be used anywhere
 *   in Condor without fear of recursion.
 *
 * lock_file() makes calls to low-level Condor code like param() and
 *   dprintf() to provide additional functionality (like optionally
 *   ignoring locking failures on NFS).
 */

int lock_file( int fd, LOCK_TYPE lock_type, BOOLEAN do_block );
int lock_file_plain( int fd, LOCK_TYPE lock_type, BOOLEAN do_block );

#if defined(__cplusplus)
}		/* End of extern "C" declaration */
#endif

#if defined(__cplusplus)

	// C++ wrapper for lock_file.  Note that the constructor takes 
	// the path to the file, which must be supplied. This class
	// will attempt to use a mutex instead of a filesystem lock unless
	// the config file says otherwise.  We do this because (a) filesystem
	// locks are often broken over NFS, and (b) filesystem locks are often
	// not FIFO, so some lock waiters can starve.  -Todd Tannenbaum 7/2007
class FileLockBase
{
  public:
	FileLockBase( void );
	virtual ~FileLockBase(void);

		// Read only access functions
	bool isBlocking(void) const			// whether or not operations will block
		{ return m_blocking; };
	bool isLocked(void) const			// false if state == UN_LOCK
		{ return ( UN_LOCK != m_state ); };
	bool isUnlocked(void) const			// true if state == UN_LOCK
		{ return ( UN_LOCK == m_state ); };
	LOCK_TYPE	getState(void) const	// cur state {READ,WRITE,UN}_LOCK
		{ return m_state; };
	virtual bool isFakeLock(void) const = 0;	// true if this is a fake lock

		// Control functions
	virtual bool obtain( LOCK_TYPE t ) = 0;		// get a lock
	virtual bool release(void) = 0;				// release a lock
	void SetFdFp( int fd, FILE *fp = NULL )
		{ SetFdFpFile( fd, fp, NULL ); };
	virtual void SetFdFpFile( int fd, FILE *fp, const char *file ) = 0;

		// Set lock mode
	void setBlocking( bool val )		// set blocking TRUE or FALSE
		{ m_blocking = val; };
	
		// Read only access functions
	virtual void display(void) const = 0;
	const char *getStateString( LOCK_TYPE state ) const;

	// An internal static list of lock objects associated with files
	// is walked and each lock object is told to update the timestamp
	// on its lockfile.  This makes it easy for daemoncore to
	// periodically update the timestamps on the lock files regardless
	// of the daemon using it.
	static void	updateAllLockTimestamps(void);

	// when the object is created or the lock file changed with
	// SetFdFpFile(), this is invoked on the filename to ensure it has
	// a current timestamp.  This prevents things like linux's
	// tmpwatch from deleting a valid lock file.
	virtual void updateLockTimestamp(void) { };


  protected:
	bool		m_blocking;				// Whether or not to block
	LOCK_TYPE	m_state;				// Type of lock we are holding

  private:
	// When the lock object is instantiated, it keeps track of itself in
	// a statically available master list--but only if it has a valid path
	// which can also be adjusted by SetFdFpFile()()
	void		recordExistence(void);
	void		eraseExistence(void);

	// used for the static linked list of all valid locks.
	struct FileLockEntry
	{
		FileLockBase	*fl;
		FileLockEntry	*next;
	};

	// Keep track of all valid FileLock objects allocated
	// anywhere. This allows daemoncore to ask them to touch the atime
	// of their lock files so programs like tmpwatch under linux don't
	// get rid of in use lock files. A constraint is that a filelock
	// is only recorded into this list if it has a non-null filename
	// pointer associated with it.
	// WARNING: I could not use a structure like the HashTable here
	// because this file is included with {read,write}_user_log.h and
	// distributed with libcondorapi. Using HashTable would have
	// caused many dependancies to be brought in header file-wise and
	// things will get very ugly.
	static FileLockEntry *m_all_locks;

};	// FileLockBase


// Real file lock class
class FileLock : public FileLockBase
{
  public:
	FileLock( int fd, FILE *fp = NULL, const char* path = NULL );
	FileLock( const char* path );
	~FileLock(void);

		// Not a fake lock
	bool isFakeLock(void) const { return false; };

		// Set the file descriptor, pointer, and file (which may not be NULL)
	void SetFdFpFile( int fd, FILE *fp, const char *file );

		// Control functions
	bool obtain( LOCK_TYPE t );		// get a lock
	bool release(void);				// release a lock

		// Read only access functions
	virtual void display(void) const;

	void updateLockTimestamp(void);

  private:

	unsigned int m_id;		// the ID number for this object
	int			 m_fd;		// File descriptor to deal with
	FILE		*m_fp;
	char		*m_path;		// Path to the file being locked, or NULL
	int			 m_use_kernel_mutex;	// -1=unitialized,0=false,1=true

	//
	// Private methods
	//
	void		Reset( void );

	// Windows specific, actually
	int			lockViaMutex( LOCK_TYPE type );
# ifdef WIN32
	HANDLE		m_debug_win32_mutex;
# endif
};

class FakeFileLock : public FileLockBase
{
  public:
	FakeFileLock( void ) : FileLockBase( ) { };
	FakeFileLock( const char* /*path*/ ) : FileLockBase( ) { };
	~FakeFileLock( void ) { };

		// Is a fake lock
	bool isFakeLock(void) const { return true; };

		// Set the file descriptor  & pointer
	void SetFdFpFile( int /*fd*/, FILE * /*fp*/, const char * /*file*/ )
		{ };

		// Read only access functions
	void display(void) const
		{ };

		// Control functions
	bool obtain( LOCK_TYPE t )			// obtain the lock
		{ m_state = t; return true; };
	bool release( void )				// release the lock
		{ m_state = UN_LOCK; return true; };
};

#endif	/* cpluscplus */

#endif