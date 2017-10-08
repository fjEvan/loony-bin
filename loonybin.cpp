// 20171006  Jiao Feng (Evan)  loonybin
// a program managing loony.cpp 

#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <string>
using namespace std;

int createLoony(string, string[], int);  // function prototypes
int deadProcessInfo(string[], int[]);
void die(string, bool addsystem = true);  // addsystem: default value

int main(int argc, char **argv)
{
	int totalCount; // user specifies a number of loonies to be created
	int count = 0;  // used to count the initial and the last 5 loonies
	int loonyCount = 0;  // count how many loonies are created
	int deathCount = 0;  // count how many loonies terminated

	if(argc > 2)  // command line usage 
	{
		cerr << "Usage: " << argv[0] << " <number of executions>" << endl;
		exit(1);
	}
	else if(argc == 2) totalCount = atoi(argv[1]);  
	else               totalCount = 50;   // run 50 loonies by default
	
	if(totalCount < 5)	totalCount = 5;	  // create at least  5 loonies

	string cmd = "loony";  // let the forked child process run loony

	string name[5] = {"Ronaldo", "Messi", "Bale", "Hazard", "Ozil"};
	int pid[5];  // the two array indexes match with each other

	while(count < 5)  // create 5 initial loonies 
	{
		pid[count] = createLoony(cmd, name, count);

		count++;
		loonyCount++;  // loonyCount increments with each fork()
	}
	totalCount -= 5;  // totalCount - 5 after 5 fork()

	while(totalCount != 0)  // maintain 5 active loonies
	{
		int index = deadProcessInfo(name, pid);  // print information about the terminated process
		
		pid[index] = createLoony(cmd, name, index);  // create a loony, update the new pid
	
		totalCount--;   // totalCount decrements with each fork()
		loonyCount++;   
		deathCount++;   // get one dead process info in each iteration
	}

	while(count != 0)  // deal with the last 5 loonies
	{
		deadProcessInfo(name, pid);

		count--;
		deathCount++;
	}

	cout << "=== " << loonyCount << " processes started. ===" << endl;
	cout << "===  " << deathCount << " processes ended.  ===" << endl;	

	return 0;
}

int createLoony(string cmd, string name[5], int i)
{
	int temp = fork();  // get zero and the ID of the child process 
	int execOK = 0;   

	if(temp < 0)  die("Forking loonybin");  // fork() failed, return -1
 // child process, execute a new loony with the just erased name
	if(temp == 0)  
		execOK = execl(cmd.c_str(), cmd.c_str(), name[i].c_str(), NULL);
 // if execl() returns -1, execution failed
	if(execOK < 0)  die("Running loony on a new process");
		
	return temp;  // return child pid
}

int deadProcessInfo(string name[5], int pid[5])
{
	int i = 0;
	int status;
	int id = wait(&status);  // catch the termination of child process
  // wait() returns -1, error, exit
	if(id < 0)  die("Trying to learn about terminated process");  
	
	while(pid[i] - id != 0)  i++;  // find which process died, save i

	if(WIFEXITED(status))  // exit normally or abnormally
	{
		int exitCode = WEXITSTATUS(status);

		if(exitCode == 0) 
			cout << "=== " << name[i] << " (pid " << id 
				 << "): Normal exit ===" << endl;
		else  
			cout << "=== " << name[i] << " (pid " << id 
				 << "): Error exit code " << exitCode << " ===" << endl;
	}  		
	else if(WIFSIGNALED(status))   // be killed, report who and how
		cout << "=== " << name[i] << " (pid " << id << "): Signaled: " 
			 << strsignal(WTERMSIG(status)) << " ===" << endl;
	else 
		cout << "=== " << name[i] << " (pid " << id 
			 << "): Mysteriously vanished. ===" << endl;
	
	return i;  // return the index to know who got killed
}

 // report error and exit, swiped from loony.cpp
void die(string descr, bool addsystem)  // addsystem default value: true 
{
	cerr << descr;
	if(addsystem)  cerr << ": " << strerror(errno);
	cerr << endl;
	exit(2);
}

/* Reference: 
1. usage of WEXITSTATUS(), WTERMSIG(), strsignal()

   http://www.man7.org/linux/man-pages/man3/strsignal.3.html
   https://www.ibm.com/support/knowledgecenter/en/SSB23S_1.1.0.14/gtpc2/cpp_wait.html#cpp_wait

2. error reporting and strerror(errno)
   loony.cpp   http://sandbox.mc.edu/~bennet/cs422/asst/loony_cpp.html
*/











