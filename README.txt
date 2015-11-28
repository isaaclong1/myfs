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
