
// Note: this code is a work-in-progress.  This version: 2:00PM PST 11/14/15

// I intend that the prototypes and behaviors of these functions be
// identical to those of their glibc counterparts, with three minor
// exceptions:

//   *  open(...) returns the handle of a file rather than the
//      descriptor of an open file.

//   *  pread(...) and pwrite(...) take
//      the handle of a file rather than the descriptor of an open file
//      as their respective first arguments.

// Recall that openfiles are to files what stringstreams are to strings. 

// In our case, we have no need to create an open file from the file
// in question.  


// Josef Pfeiffer's bbfs.c includes
//#include "params.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
//#include <fuse.h>  
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

// Here we include
#include </usr/include/linux/fs.h>  // needed for compilation on vagrant
#include <sys/stat.h>  // this has our official definition of stat
#include <dirent.h>    // this has our official definition of dirent
#include <errno.h>
#include <cassert>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <iostream>
#include "my_stubs.H"
#include <time.h>
#include <pwd.h>
#include <grp.h>

using namespace std;

#define cdbg cerr << "\nLn " << dec <<  __LINE__ << " of "  << __FUNCTION__ << ": "
//<< " by " << report() 

void show_stat( struct stat& root );  // prototype
void initialize();                    // prototype
ino_t find_ino( string path );        // prototype
// The above should go into my_stubs.H

string cwd;   // to hold name of current working directory but not used yet.

struct dirent_frame {
  dirent the_dirent;
  char overflow[NAME_MAX]; // to catch overflow from one-char name field.
};


class File {
public:
  //inode metadata;                     // for use by all files
  struct stat metadata;                 // for use by all files
  // one or the other of the following (data and frame) should be empty.
  string data;                          // for use by regular files
  vector<dirent_frame> dentries;        // for use by directories
  //list<dirent_frame> dentries;        // it'd be cleaner to make this a list
};


class Ilist {
public:
  int count;
  map<ino_t,File> entry;
  int next() {
    static int count = 2;  // ino counter stats at 2.
    return count++;
  }
} ilist; 


void initialize() { // now called from main() but could be called from
                    // Ilist constructor (I think).
  my_mkdir("/", 0700);
  char tmp[PATH_MAX];
  getcwd(tmp,998);
  cwd = string(tmp);  
  // cwd now hold the path to the initial current working directory.
  // But, we don't yet use it.
}

void show_stat( struct stat& root ) {
  // Displays the entries in a stat in same formats that Pfeiffer uses.
  cerr << "SHOW\n";
  cerr << "st_dev     = " << dec << root.st_dev     << endl;  
  cerr << "st_ino     = "        << root.st_ino     << endl;  
  cerr << "st_mode    = " << oct << root.st_mode    << endl;  
  cerr << "st_nlink   = "        << root.st_nlink   << endl;  
  cerr << "st_uid     = " << dec << root.st_uid     << endl;  
  cerr << "st_gid     = " << dec << root.st_gid     << endl;  
  cerr << "st_rdev    = "        << root.st_rdev    << endl;  
  cerr << "st_size    = "        << root.st_size    << endl;  
  cerr << "st_blksize = "        << root.st_blksize << endl;  
  cerr << "st_blocks  = "        << root.st_blocks  << endl;  
  cerr << "st_atime   = " << hex << root.st_atime   << endl;  
  cerr << "st_mtime   = " << hex << root.st_mtime   << endl;  
  cerr << "st_ctime   = " << hex << root.st_ctime   << endl;  
}; 


inline  // a simple utility for splitting strings at a find-pattern.
vector<string>
split(const string s, const string pat ) {
  string c;
  vector<string> v;
  int i = 0;
  for (;;) {
    int t = s.find(pat,i);
    int j = ( t == string::npos ) ? s.size() : t;
    string c = s.substr(i,j-i);
    v.push_back(c);
    i = j+pat.size();
    if ( t == string::npos ) return v;
  }
}

inline  // joins vector of strings separating via a given pattern.
string
join( const vector<string> v, const string pat, int start=0, int end=-1 ) {
  if ( end < 0 ) end += v.size();
  //  assert ( start < v.size() );   // should throw an exception.
  if ( start >= v.size() ) return "";
  string s = v[start++];
  for ( int i = start; i <= end; ++i )  s += pat + v[i];
  return s;
}

const int ok = 0;
const int err = -1;
// Note that for pointer and ino_t return values, 0 indicates an error.


// lstat is called at line #95 of bbfs.c
int my_lstat( const char* path, struct stat *statbuf ) {
  //cdbg << "lstat has been called and is calling find_ino on " << path << endl;
  ino_t fh = find_ino(path);
  //cdbg << "fh = " << fh << endl;
  int retstat;
  if ( fh == 0 ) {
    retstat = err;
  } else {
    retstat = my_fstat(fh,statbuf);
  }  
  return retstat;
}  

// called at line #125 of bbfs.c
int my_readlink( const char *path, char *link, size_t size ) {
  return err;  
}  

// called at line #168 of bbfs.c.  See line #151.
int my_mknod( const char *path, mode_t mode, dev_t dev ) {
  cdbg <<"mknod been called with " << path << " " << oct << mode 
       << " " << dev <<endl;
  // Returns error code only.  It should make a file and install it
  // into ilist and instrall a dentry for it in the parent directory.

  // To later get the handle for that inode use find_ino().  To get a
  // pointer to that file use find_file.  IMHO, we should use it to
  // creat both directories and regular files, but the man page leaves
  // out directories.  So, I've not used it, and it's not been tested.

  // This code has been copied over from my_mkdir.
  int the_ino = ilist.next();     // Get and record this directory's ino
  cdbg << "the_ino =" << the_ino << endl;
  struct stat& md = ilist.entry[the_ino].metadata;   // Create this file
  mode_t old_umask = umask(0);  // sets umask to 0 and returns old value
  umask(old_umask);                       // restores umask to old value
  md.st_dev     = dev;                /* ID of device containing file */
  md.st_ino     = the_ino;                            /* inode number */
  md.st_mode    = mode & ~ old_umask;          /* type and protection */
  md.st_nlink   = 1;                          /* number of hard links */
  md.st_uid     = geteuid();                      /* user ID of owner */
  md.st_gid     = getegid();              /* group ID of owning group */
  md.st_rdev    = 0;                   /* device ID (if special file) */
  md.st_size    = 0;       /* size in bytes for reg-files and symlinks*/
  md.st_blksize = 4096;              /* blocksize for file system I/O */
  md.st_blocks  = 0;               /* number of 512B blocks allocated */
  md.st_atime   = time(0);                     /* time of last access */
  md.st_mtime   = time(0);               /* time of last modification */
  md.st_ctime   = time(0);              /* time of last status change */

  // The man page on STAT(2) is an excellent resource for configuring
  // a stat, e.g., http://man7.org/linux/man-pages/man2/stat.2.html

  // Now we push a dentry for this file onto the parent directory.
  // except for root directory
  vector<string> v = split( path, "/" );  // split the path
  string the_name = v.back();
  v.pop_back();

  // Create, configure, and install a dentry for this new directory's
  // in its parent directory.  But, omit this the first time, since
  // the root directory has no parent.
  cdbg << "the_ino =" << the_ino << endl;
  ino_t parent_ino = find_ino(join(v,"/"));  
  static bool first_time = true;  // static vars get initialized once
  if ( first_time ) {                    
    first_time = false;
  } else {    // Omit this on first invocation, i.e., when making root
    ino_t fh = find_ino(join(v,"/"));
    dirent_frame df;                    
    df.the_dirent.d_ino = the_ino; 
    strcpy(df.the_dirent.d_name, the_name.c_str());  
    //cdbg << "Inserting dentry for " << the_ino << " w name " \
           << the_name << " into directory " << join(v,"/");
    ilist.entry[fh].dentries.push_back(df);  // push df onto the back of parent
  }


  return ok;  
}  


// called at line #186 of bbfs.c
int my_mkdir( const char *path, mode_t mode ) {
  // returns an error code.
  cdbg << "mkdir has been called with path \"" << path<< "\" and mode " \
       << oct <<  mode << endl;

  // Now we create and configure this directory's metadata (inode/struct).

  int the_ino = ilist.next();     // Get and record this directory's ino
  struct stat& md = ilist.entry[the_ino].metadata;   // Create this file
  mode_t old_umask = umask(0);  // sets umask to 0 and returns old value
  umask(old_umask);                       // restores umask to old value
  md.st_dev     = 100;                /* ID of device containing file */
  md.st_ino     = the_ino;                            /* inode number */
  md.st_mode    = S_IFDIR | ( mode & ~ old_umask);      /* protection */
  md.st_nlink   = 1;                          /* number of hard links */
  md.st_uid     = geteuid();                      /* user ID of owner */
  md.st_gid     = getegid();              /* group ID of owning group */
  md.st_rdev    = 0;                   /* device ID (if special file) */
  md.st_size    = 0;       /* size in bytes for reg-files and symlinks*/
  md.st_blksize = 4096;              /* blocksize for file system I/O */
  md.st_blocks  = 0;               /* number of 512B blocks allocated */
  md.st_atime   = time(0);                     /* time of last access */
  md.st_mtime   = time(0);               /* time of last modification */
  md.st_ctime   = time(0);              /* time of last status change */

  // The man page on STAT(2) is an excellent resource for configuring
  // a stat, e.g., http://man7.org/linux/man-pages/man2/stat.2.html


  // Now we push a dentry for this directory onto the parent directory.
  // except for root directory
  vector<string> v = split( path, "/" );  // split the path
  string the_name = v.back();
  v.pop_back();

  // Create, configure, and install a dentry for this new directory's
  // in its parent directory.  But, omit this the first time, since
  // the root directory has no parent.
  cdbg << "the_ino =" << the_ino << endl;
  ino_t parent_ino = find_ino(join(v,"/"));  
  static bool first_time = true;  // static vars get initialized once
  if ( first_time ) {                    
    first_time = false;
  } else {    // Omit this on first invocation, i.e., when making root
    ino_t fh = find_ino(join(v,"/"));
    dirent_frame df;                    
    df.the_dirent.d_ino = the_ino; 
    strcpy(df.the_dirent.d_name, the_name.c_str());  
    //cdbg << "Inserting dentry for " << the_ino << " w name " \
           << the_name << " into directory " << join(v,"/");
    ilist.entry[fh].dentries.push_back(df);  // push df onto the back of parent
  }

  // Install dentries for . and .. within this new directory
  dirent_frame df;

  strcpy(df.the_dirent.d_name, ".");
  df.the_dirent.d_ino  = the_ino;  
  ilist.entry[the_ino].dentries.push_back(df);

  strcpy(df.the_dirent.d_name, "..");
  df.the_dirent.d_ino  = parent_ino;  // needs to be adjusted for root directory
  ilist.entry[the_ino].dentries.push_back(df);

  return ok;
}  // my_mkdir

// called at line #203 of bbfs.cg
int my_unlink( const char *path ) {
  return err;   
}  

// called at line #220 of bbfs.c
int my_rmdir( const char *path ) {
  return err;   
}  

// called at line #241 of bbfs.c
int my_symlink(const char *path, const char *link) {
  return err;  
}  

// called at line #261 of bbfs.c
int my_rename( const char *path, const char *newpath ) {
  return err;  
}  

// called at line #279 of bbfs.c
int my_link(const char *path, const char *newpath) {
  vector<string> v = split(string(newpath),"/");
  string tail = v.back();
  string dirpath = join(v, "/");
  ino_t fh = find_ino(path);
  if ( ! S_ISDIR( ilist.entry[fh].metadata.st_mode ) ) {
    errno = EPERM; 
    return err; 
  }
  ino_t fh2 = find_ino(newpath);
  if ( ! S_ISDIR( ilist.entry[fh2].metadata.st_mode ) ) {
    errno = EPERM; 
    return err; 
  }
  ++ ilist.entry[fh].metadata.st_nlink;
  // check for overflow.
  dirent d;
  d.d_ino = fh;
  strcpy( d.d_name, tail.c_str() );
  return ok;  
}  

// called at line #296 of bbfs.c
int my_chmod(const char *path, mode_t mode) {
  return err;  
}  

// called at line #314 of bbfs.c
int my_chown(const char *path, uid_t uid, gid_t gid) {
  return err;  
}  

// called at line #331 of bbfs.c
int my_truncate(const char *path, off_t newsize) {
  return err;  
}  

// called at line #349 of bbfs.c
int my_utime(const char *path, struct utimbuf *ubuf) {
  return err;  
}  

// called at line #376 of bbfs.c.  Returns file handle not a file descriptor
int my_open( const char *path, int flags ) {
  // Write a function called "lookup" that takes a full path and the
  // handle of a directory and returns the handle of the inode having
  // that name.  To do so, split full path.  Then starting with the
  // handle of the root directory, look up each segment in the
  // directory whose handle you currently have.  Use a linear search
  // that consists of successive calls to readdir() to find the
  // corresponding directory entry, which has type struct dirent.
  // Return its d_fileno, unless there's an error and then return -1.

  // FINISH THIS
  // static filecount = 0;
  // ino_t fh = find_ino(path);  // 
  // if ( fh >= 0 && ) {
  //   return fh;
  // } else if ( flags & CREAT ) {
  //   // create a new inode with ino_t filecount++;
  // }

}  

// called at line #411 of bbfs.c  Note that our firt arg is an fh not an fd
int my_pread( int fh, char *buf, size_t size, off_t offset ) {
  return err;
}  

// called at line #439 of bbfs.c  Note that our firt arg is an fh not an fd
int my_pwrite( int fh, const char *buf, size_t size, off_t offset ) {
  return err;
}  

// called at line #463 of bbfs.c
int my_statvfs(const char *fpath, struct statvfs *statv) {
  return err;  
}  

// called at line #530 of bbfs.c
int my_close( int fh ) {
  return err;
}  

// called at line #553 of bbfs.c
int my_fdatasync( ino_t fh ) {
  return err;  
}  

// called at line #556 of bbfs.c
int my_fsync( ino_t fh ) {
  return err;  
}  

// called at line #575 of bbfs.c
int my_lsetxattr( const char *fpath, const char *name, const char *value, size_t size, int flags ) {
  return err;  
}  

// called at line #592 of bbfs.c
int my_lgetxattr( const char *fpath, const char *name, char *value, size_t size, int flags ) {
  return err;  
}  

// called at line #613 of bbfs.c
int my_llistxattr( const char *path, char *list, size_t size ) {
  return err;  
}  

// called at line #634 of bbfs.c
int my_lremovexattr( const char *path, const char *name ) {
  return err;  
}  


// called at line #826 of bbfs.c
int my_access( const char *fpath, int mask ) {
  return err;  
}  

// called at line #856 of bbfs.c
int my_creat( const char *fpath, mode_t mode ) {
  // we can create a file by using the right flags to open
  return err;  
}  

// called at line #887 of bbfs.c
int my_ftruncate( ino_t fh, off_t offset ) {
  return err;  
}  

// called at line #921 of bbfs.c
int my_fstat( ino_t fh, struct stat* statbuf ) {

  //cdbg << "my_fstat has been called on " << fh << " " << long(statbuf) << endl;
  if ( ilist.entry.count(fh) == 0 ) {
    errno = ENOENT;
    return err;
  }  
  *statbuf = ilist.entry[fh].metadata;
  // cdbg << "and here's *statbuf for " << endl;
  show_stat(*statbuf);
  return ok;
}  


// Here are my helper functions ==================================

// In the code below  opendir is defined in terms of find_ino,
// which is defined in terms of lookup,
// which is defined in terms of opendir and readdir.
// The header file takes care of that circularity.

// It doesn't say so anywhere that I can find, but a MY_DIR has to be
// similar to an open file.  However, rather than creating a stream of
// bytes (chars) from the file, it creates a stream of directory
// entries.  The key difference is how much you increment the offset
// counter each time you move to the next item.

// Note that at line #742 of bbfs.c, Professor Pfeiffer converts a
// file handle into a DIR* via "(uintptr_t) fh."  But, his file handles
// are indices of byte streams, while our are the addresses of inodes.


MY_DIR* fopendir( ino_t fh ) {  // not exported
  if ( ! S_ISDIR( ilist.entry[fh].metadata.st_mode ) ) {
    return 0;  // null pointer indicates err
  }
  MY_DIR * tmp = new MY_DIR;
  // Initialize both members of tmp*
  tmp->fh    = fh;
  tmp->index = 0;
  return tmp;
}


// called at lines #707 and #726 of bbfs.c
dirent* my_readdir( MY_DIR* dirp ) {  
  vector<dirent_frame> v = ilist.entry[dirp->fh].dentries;
  //list<dirent_frame> v = ilist.entry[dirp->fh].dentries;
  int tmp = dirp->index++;  // post increment dirp*'s index.
  return tmp == v.size() ? 0 : & v[tmp].the_dirent;
}


// called at line #742 of bbfs.c
int my_closedir( MY_DIR* dirp ) {
  delete dirp;  
}


ino_t lookup( string name, ino_t fh ) {
  // Searches for and returns ino of file of a given name within the 
  // directory given by fh.  This function will be used by find_ino().

  MY_DIR* dirp = fopendir( fh ); 
  // fopendir() will return 0 if fh isn't the handle of a directory.
  if ( ! dirp ) return 0;                   // And so will lookup().
  while ( dirent* dp = my_readdir(dirp) ) {  
    //cdbg << dp->d_name << " =? " <<name<< " then return " << dp->d_ino << endl;
    if ( string(dp->d_name) == name ) { // comparision of C++ strings
      my_closedir(dirp);
      return dp->d_ino;  
    } 
  }
  my_closedir(dirp);  // close MY_DIR asap, to reset internal data
  return 0;  // name-not-found
}  


ino_t find_ino( string path ) {

  //cdbg << "find_ino() has been called with path \"" << path << "\"\n"; 
  vector<string> v = split( path, "/" );  // The members of v are "segments."
  // Here are the exact details concerning path resolution:
  // http://manpages.ubuntu.com/manpages/lucid/en/man7/path_resolution.7.html

  // The plan: if the initial segment is not null prepend the full
  // pathname for the current working directory to path and start
  // over.  Process out segments of the form"." or ".." (along with
  // its predecessor).  Delete any other null segments.  Among other
  // things, we want the path name "/" to designate the root
  // directory.

  if ( v[0] != "" ) path = cwd + "/" + path;  // cwd == "" for now
  ino_t fh = 2;    // I've read that 2 is the ino of a filesystem's 
                   // root directory.  
  for ( auto it : v ) {
    if ( it == "" ) {
       continue;                  // ignore null segments.
    } else if ( ilist.entry.count(fh) == 0 ) {  // no such directory
      cdbg << "no such entry as " << it << " in " << path << endl;
      errno = ENOENT;
      fh = 0;
      break;
    } else if ( S_ISDIR(ilist.entry[fh].metadata.st_mode) ) {
      cdbg << "lookup(" << it << "," << fh << ") yields ...";
      fh = lookup( it, fh );
      cerr << "yields ... this value " << fh << endl;
    } else {
      cdbg << "In " << path <<  ", " << it << " is not a directory \n";
      errno = ENOTDIR;
      fh = 0;
      break;   
    }
    //cdbg << v[i] << " " << fh << endl;
  } 
  // cdbg << "and is returning " << fh << endl;
  return fh;
} 


File* find_file( ino_t ino ) { // could improve readability of code
  return & ilist.entry[ino];
}


File* find_file( string s ) {  // could improve readability of code
  return find_file(find_ino(s));
}


// called at line #659 of bbfs.c
MY_DIR * my_opendir( char path[PATH_MAX] ) {
  return fopendir( find_ino( path ) ); 
  // Note that fopendir checks whether its input is the handle of a
  // directory.
}


int ls(string s) {
  // diagnostic tool to see what's in a directory
  ino_t fh = find_ino(s.c_str());
  // cdbg << "ls has been called on \"" << s << "\" and ilist has " 
  // << ilist.entry.size() << " entries\n";
  File f = ilist.entry[fh];
  if ( ! S_ISDIR(f.metadata.st_mode) ) {
    errno = ENOTDIR;
    //cdbg << "ilist entry " << fh << " isn't a directory: mode = " 
    //     << oct << f.metadata.st_mode << endl;
    show_stat( f.metadata);
    return err;
  }
  vector<struct dirent_frame> v = f.dentries;
  //cdbg << "The directory (ino" << fh << ") has " << v.size() << " entries:\n";
  for ( auto it : v ) {
    struct dirent d = it.the_dirent;
    cout << d.d_ino << " " << d.d_name << endl;
  }
}

// ===========================================


template< typename T1, typename T2>   // An idea
string pickle(map<T1,T2> m) {
  typename map<T1,T2>::iterator it;
  string s;
  stringstream ss(s);
  for ( auto it : m ) {
    // This requires some kind of separation/terminaton symbol at the end of each.
    ss << it.first;  
    ss << it.second;
  }  
}  

  
// MY_DIR is typedef'd in my_stubs.H.  It's essentially an iterator for 
// directories.

// Per this GNU document (http://www.delorie.com/gnu/docs/dirent/dirent.5.html): 
// "The dirent structure is defined below.
// struct dirent {
//   long           d_ino;
//   off_t          d_off;
//   unsigned short d_reclen;
//   char d_name[1];
// };
// The field d_ino is a number which is unique for each file in the
// file system. The field d_off represents an offset of that directory
// entry in the actual file system directory. The field d_name is the
// beginning of the character array giving the name of the directory
// entry. This name is null terminated and may have at most NAME_MAX
// characters in addition to the null terminator. This results in file
// system independent directory entries being variable-length
// entities. The value of d_reclen is the record length of this
// entry. This length is defined to be the number of bytes between the
// beginning of the current entry and the next one, adjusted so that
// the next entry will start on a long boundary."

// But, per the relevant GNU manual pages on dirent,
// (http://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html),
// it may have many additional fields and even d_reclen is not
// required.  So far as I can tell, we don't need d_off and d_reclen
// but may need to maintain them for compatibility with FUSE, etc.

// Per a creator of FUSE (Millos Szeredit) regarding the offset field of dirents:
// http at fuse.996288.n3.nabble.com/issues-in-readdir-and-fuse-fill-dir-t-filler-td10397.html:

// "For directories it's completely irrelevant.  It can go forward,
// backward or jump around randomly.  Ancient unix systems allowed
// directories to be read as regular files (i.e. cat would work) and the
// offset was really an offset into this file.

// But now filesystems are free to implement directories in any way and
// the "offset" is not necessarily an offset any more, just a opaque
// cookie, unique for each directory entry within the directory,
// indicating the position within the directory stream.

// Thanks,
// Miklos "

// IMHO, this validates our use of maps for directories (thp).

// INODES: There is a good definition of struct inode at
// http://www.makelinux.net/books/lkd2/ch12lev1sec6.  But
// we are not using them.  Rather we are using struct stat
// for the metadata of a file.


// Here are details on lstat() and struct stat from
// http://linux.die.net/man/2/stat:

// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
// These functions return information about a file. No permissions are
// required on the file itself, but-in the case of stat() and lstat()
// - execute (search) permission is required on all of the directories
// in path that lead to the file.

// stat() stats the file pointed to by path and fills in buf.

// lstat() is identical to stat(), except that if path is a symbolic
// link, then the link itself is stat-ed, not the file that it refers
// to.

// fstat() is identical to stat(), except that the file to be stat-ed
// is specified by the file descriptor fd.

// All of these system calls return a stat structure, which contains
// the following fields:

//     struct stat {
//         dev_t     st_dev;     /* ID of device containing file */
//         ino_t     st_ino;     /* inode number */
//         mode_t    st_mode;    /* protection */
//         nlink_t   st_nlink;   /* number of hard links */
//         uid_t     st_uid;     /* user ID of owner */
//         gid_t     st_gid;     /* group ID of owner */
//         dev_t     st_rdev;    /* device ID (if special file) */
//         off_t     st_size;    /* total size, in bytes */
//         blksize_t st_blksize; /* blocksize for file system I/O */
//         blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
//         time_t    st_atime;   /* time of last access */
//         time_t    st_mtime;   /* time of last modification */
//         time_t    st_ctime;   /* time of last status change */
//     };

// The st_dev field describes the device on which this file
// resides. (The major(3) and minor(3) macros may be useful to
// decompose the device ID in this field.)

// The st_rdev field describes the device that this file (inode) represents.

// The st_size field gives the size of the file (if it is a regular
// file or a symbolic link) in bytes. The size of a symbolic link is
// the length of the pathname it contains, without a terminating null
// byte.

// The st_blocks field indicates the number of blocks allocated to the
// file, 512-byte units. (This may be smaller than st_size/512 when
// the file has holes.)

// The st_blksize field gives the "preferred" blocksize for efficient
// file system I/O. (Writing to a file in smaller chunks may cause an
// inefficient read-modify-rewrite.)

// Not all of the Linux file systems implement all of the time
// fields. Some file system types allow mounting in such a way that
// file and/or directory accesses do not cause an update of the
// st_atime field. (See noatime, nodiratime, and relatime in mount(8),
// and related information in mount(2).) In addition, st_atime is not
// updated if a file is opened with the O_NOATIME; see open(2).

// The field st_atime is changed by file accesses, for example, by
// execve(2), mknod(2), pipe(2), utime(2) and read(2) (of more than
// zero bytes). Other routines, like mmap(2), may or may not update
// st_atime.

// The field st_mtime is changed by file modifications, for example,
// by mknod(2), truncate(2), utime(2) and write(2) (of more than zero
// bytes). Moreover, st_mtime of a directory is changed by the
// creation or deletion of files in that directory. The st_mtime field
// is not changed for changes in owner, group, hard link count, or
// mode.

// The field st_ctime is changed by writing or by setting inode
// information (i.e., owner, group, link count, mode, etc.).

// The following POSIX macros are defined to check the file type using
// the st_mode field:

//     S_ISREG(m)

//     is it a regular file?

//     S_ISDIR(m)

//     directory?

//     S_ISCHR(m)

//     character device?

//     S_ISBLK(m)

//     block device?

//     S_ISFIFO(m)

//     FIFO (named pipe)?

//     S_ISLNK(m)

//     symbolic link? (Not in POSIX.1-1996.)

//     S_ISSOCK(m)

//     socket? (Not in POSIX.1-1996.) 
// The following flags are defined for the st_mode field:
// The set-group-ID bit (S_ISGID) has several special uses. For a
// directory it indicates that BSD semantics is to be used for that
// directory: files created there inherit their group ID from the
// directory, not from the effective group ID of the creating process,
// and directories created there will also get the S_ISGID bit
// set. For a file that does not have the group execution bit
// (S_IXGRP) set, the set-group-ID bit indicates mandatory file/record
// locking.

// The sticky bit (S_ISVTX) on a directory means that a file in that
// directory can be renamed or deleted only by the owner of the file,
// by the owner of the directory, and by a privileged process.
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



// The following gives information on configuring the root directory:
// "The root directory has a '.' and a '..' entry in it, and the inode
// number for each is the same. Traditionally, the inode number is 2;
// it still is on MacOS X, Linux, Solaris. So, when you do 'cd /; cd
// ..', you end up at the same location.

// In the 1980s, there was a system called Newcastle Connection that
// treated networked computers as being above the root of your local
// computer. Thus, on such a machine, you would type:

// cd /../othermachine/path/to/interesting/place

// to change directory to a remote file system.

// (You can find the paper via a Google search of 'Newcastle
// Connection' - the URL is intractable.)"
// See http://superuser.com/questions/186184/the-parent-of-the-root-directory

     // struct stat {
     //     unsigned int  st_dev;      /* ID of device containing file */
     //     unsigned int  st_ino;      /* inode number */
     //     mode_t        st_mode;     /* type and protection of file */
     //     unsigned int  st_nlink;    /* number of hard links */
     //     unsigned int  st_uid;      /* user ID of owner */
     //     unsigned int  st_gid;      /* group ID of owner */
     //     unsigned int  st_rdev;     /* device type (if inode device) */
     //     unsigned long st_size;     /* total size, in bytes */
     //     unsigned long st_blksize;  /* blocksize for filesystem I/O */
     //     unsigned long st_blocks;   /* number of 512B blocks allocated */
     //     time_t        st_atime;    /* time of last access */
     //     time_t        st_mtime;    /* time of last modification */
     //     time_t        st_ctime;    /* time of last change */
     // };
// Note that this shows the real types of all inode components except
// mode_t and time_t.
// for moredetails see: 
//   http://manpages.ubuntu.com/manpages/hardy/man2/stat.2.html

// Under development: Function to format ls lines in format of "ls -d"
int describe_file( string it, struct stat st) {
  //string stream ccout;
  string filename = it;    

  //struct stat st;       // "struct stat" because stat() is defined
  if ( my_lstat( filename.c_str(), &st ) != 0 ) {
    cerr << "Cannot stat file " << filename 
         << ": " << strerror(errno) << endl;
    return -1;
  }

  cout << ( (S_ISDIR(st.st_mode) != 0) ? 'd' : '-' )
       << ( (st.st_mode & S_IRUSR) ? 'r' : '-' )
       << ( (st.st_mode & S_IWUSR) ? 'w' : '-' )
       << ( (st.st_mode & S_IXUSR) ? 'x' : '-' )
       << ( (st.st_mode & S_IRGRP) ? 'r' : '-' )
       << ( (st.st_mode & S_IWGRP) ? 'w' : '-' )
       << ( (st.st_mode & S_IXGRP) ? 'x' : '-' )
       << ( (st.st_mode & S_IROTH) ? 'r' : '-' )
       << ( (st.st_mode & S_IWOTH) ? 'w' : '-' )
       << ( (st.st_mode & S_IXOTH) ? 'x' : '-' )
  ;

  char date[64];
  strftime( date, 15, "%b %d %H:%M  ", localtime( &st.st_mtime ) );

  printf( 
    "%2i %7s %7s %8ld %8s ",          // format string
    st.st_nlink,                      // number of links
    getpwuid(st.st_uid)->pw_name,     // password name
    getgrgid(st.st_gid)->gr_name,     // group name
    st.st_size,                       // size of file
    date                              // time of last modification
  );
    
}


int main() {
  // The place for testing of functions.
  int cwd = 2;  // ino of current working directory;
  cdbg << "Now we call initialize()" << endl;
  initialize();
  show_stat( ilist.entry[2].metadata );  // all looks good here.
  cdbg << "Now we call lstat on \"/\" and &mystat" << endl;
  struct stat mystat;
  my_lstat("/",&mystat);
  cdbg << "Now we call ls on \"/\"" << endl;
  ls("/");
  cdbg << "Now we call mkdir on \"/junk\" and 0700" << endl;
  my_mkdir("/junk", 0700);
  cdbg << "Now we call ls on \"/\"" << endl;
  ls("/");

  cdbg << "now we call ls on \"/junk\"" << endl;
  //  cout << endl;
  ls("/junk");
  
  cdbg << "Now we call mkdir on \"/junk/stuff\" and 0700" << endl;
  my_mkdir("/junk/stuff", 700);
  cdbg << "now we call ls on \"/junk\"" << endl;
  ls("/junk");
  cdbg << "now we call ls on \"/junk/stuff\"" << endl;
  ls("/junk/stuff");

  cout << endl;
  describe_file( "/junk", mystat);
  cout << endl;
}

