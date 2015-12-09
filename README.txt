Compilation in vagrant environment that we set up previously works (not sure about macbooks, there seems to be some library issues).

1. Run 'vagrant up' command from the vagrant directory on your host system 

(If you don't have the vagrant directory or it was deleted repeat set up process
in Anthony's note on Piazza, week 9/27)

(following commands are run in CentOS virtual machine)

2. Run 'usermod -a -G fuse vagrant'

3. Open terminal and add following lines to .bashrc and restart terminal after

alias g++14="g++ -std=c++11 -pthread"
alias setupcpp="source /opt/rh/devtoolset-2/enable"

4. cd /vagrant/myfs (myfs should be the git repo)

5. Run 'setupcpp'

6. Compile with 'g++14 my_stubs.cc'


General Todo
179
-revision and turn in of new midterm report Nov. 30 (Isaac)
-everyone add to the project notebook/make it look complete (All)
-ask about project ‘write up’ in discussion tomorrow (All)
-code your portion of the high priority for minimum grade in presentation (All)

Todo for functions

High priority — Finish by Wednesday Dec. 2 

my_rename (move)	Ryan
my_link             Allyn
my_unlink 		Allyn
my_create 		Ben
my_open			Andrew
my_close		Andrew
my_pread		Isaac
my_pwrite		Isaac

(Do any other functions you have time for; be sure to tell the group
which ones you are doing if you do so)

Medium priority

my_access 
my_rmdir
my_chown
my_chmod    Allyn

Low priority

my_readlink
my_symlink
my_utime
my_fdatasync
my_fsync
my_ftruncate
my_truncate
my_statvfs

Very low priority

my_lsetattr
my_lgetattr
my_llistxattr
my_lremovexattr
