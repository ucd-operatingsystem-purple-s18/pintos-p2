
PINTOS PROJECT2

Johnathan Becker
Nate Terry

ucd-operatingsystem-purple-s18

====================================================
//Install Brian's preferred editor
pintos@os-dev:~$ sudo apt-get install geany

====================================================
To run the test file:
	pintos@os-dev:~/pintos/src/userprog$ make clean
	pintos@os-dev:~/pintos/src/userprog$ make check VERBOSE=1

====================================================
To run a single test:
	pintos@os-dev:~/pintos/src/userprog$ pintos run test-args-single


====================================================
To view the txt file with the test results:
	pintos@os-dev:~/pintos/src/userprog$ gedit makecheck-dump.txt 

====================================================
Peter Gibbs File
Automatically make and mount the disk in Pintos
/pintos/src/userprog
	pintos-p2-rebuild_disk_sh.sh

====================================================
Rename Peter's file"
	pintos-p2-rebuild-disk.sh

====================================================
Grant yourself access to it:
	pintos@os-dev:~/pintos/src/userprog$ chmod +x pintos-p2-rebuild-disk.sh

====================================================
NOW - there is a fault in reading Carriage Return characters between its creation (in windows?) and here in Linux.
We have to remove those Carriage Return characters.

	pintos@os-dev:~/pintos/src/userprog$ sed -i -e 's/\r$//' pintos-p2-rebuild-disk.sh 

====================================================
Run it:
	pintos@os-dev:~/pintos/src/userprog$ ./pintos-p2-rebuild-disk.sh 

It seemed to work just fine

====================================================
Note, initially, when we ran a single test with:
	pintos@os-dev:~/pintos/src/userprog$ pintos run test-args-none
	
One of the many return lines that we got was:
	Kernel PANIC at ../../filesys/filesys.c:22 in filesys_init(): No file system device found, can't initialize file system.

After downloading, renaming, chmod-ing, and fixing that Carriage Return character, then running a single test appeared to work fine.
====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================
3/14/18
/home/pintos/pintos/src/userprog/process.c

process_wait (tid_t child_tid UNUSED) 
{
  while (true) {} //added
  return -1;
}
====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================

====================================================
