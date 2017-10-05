/*
 * This is the loony process that picks siblings at random and tries to kill
 * them.  Otherwise it sleeps, and after it's done this a while it dies.
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;

/*
 * Error exit.
 */
void die(string descr, bool addsystem = true)
{
	cerr << descr;
	if(addsystem) cerr << ": " << strerror(errno);
	cerr << endl;
	exit(2);
}

/*
 * This reads a status file from /proc and returns the parent pid from
 * the file, or -1 if it fails.
 */
pid_t extract_parent(string filename)
{
	// Mainly because the the file can disappear while we're
	// trying to read it, all sorts of bad stuff can happen.
	try {
		// Open the file.  If it fails, just figure the process exited
		// while we were reading the directory.
		ifstream pfile(filename.c_str());
		if(!pfile) return -1;

		// Read until we see PPid:, then the next thing will be the 
		// parent pid.
		string word;
		while(pfile >> word) {
			if(word == "PPid:") {
				// Next word should exist, and will be the 
				// parent pid.
				if(!(pfile >> word)) return -1;
				return atoi(word.c_str());
			}
		}
	} catch(...) {
	}

	return -1;
}

/*
 * This function builds and returns a vector of all sibling processes.
 */
vector<pid_t> siblings() 
{
	// Use the O/S calls to find our own pid and our parent's pid.
	pid_t mypid = getpid();
	pid_t mypar = getppid();

	// Make a string version, too
	ostringstream oss;
	oss << mypid;
	string mypid_s = oss.str();

	// Now go through all the processes listed in the system /proc 
	// directory and add our siblings to ret.
	vector<pid_t> ret;
	DIR *dir = opendir("/proc");
	if(dir == NULL) die("Reading /proc");
	while(1) {
		// Read the next directory entry, stop if that's all.
		struct dirent *de = readdir(dir);
		if(de == 0) break;

		// See if the file name looks like it's a process number.
		// We'll just see if the first character is a digit.
		// If not, just skip this file.
		if(!isdigit(de->d_name[0])) continue;

		// Skip ourselves.
		string name = de->d_name;
		if(name == mypid_s) continue;

		// Extract the parent pid, and skip it if it's not the
		// same as ours.
		if(extract_parent("/proc/"+name+"/status") != mypar) continue;

		// Add to the list
		ret.push_back(atoi(name.c_str()));
	}
	closedir(dir);
	
	return ret;
}

/*
 * Run this program with a string name on the command line which is used
 * in printouts.
 */
int main(int argc, char **argv)
{
	const char *name = argc > 1 ? argv[1] : "pgm";

	// List of signals to send.
	int siglist[] = { SIGINT, SIGQUIT, SIGTERM };

	// This keeps loonies from leaving core files when they die.  It does
	// this by setting the limit on the process' core file resource to zero.
	// This change effects only this process.  (It would also effect its
	// children if it created any.)
	struct rlimit none = { 0, 0 };
	setrlimit(RLIMIT_CORE, &none);

	// Make the loonies behave differently.
	srandom(time(0)+getpid());

	// Choose a liftime, ms.
	int life = random() % 6000 + 2000;

	while(1) {
		// How long before we kill something (ms)?
		int sleepy = random() % 2500 + 500;

		// If we're not going to live that long, go ahead and
		// croak now.
		if(sleepy > life) break;
		life -= sleepy;

		// Now I'm going to go to sleep for a while
		struct timespec ts = { sleepy / 1000, 1000000*(sleepy%1000) };

		nanosleep(&ts, 0);

		// Pick someone to kill.  If there isn't anyone, go back
		// to sleep.
		vector<pid_t> sibs = siblings();
		if(sibs.size() == 0) continue;
		pid_t victim = sibs[random() % sibs.size()];

		// Choose a signal to send.
		int bullet =
		    siglist[random() % ((sizeof siglist) / sizeof siglist[0])];

		cout << name << " throwing " << strsignal(bullet) << " at "
		     << (int)victim << endl;
		kill(victim, bullet);

		// See if it's time to do some other stupid thing.

		// May divide by zero.
		int z = 100 / (random() % 10); 

		// One chance in 10 to make an illegal memory reference.
		if(random() % 10 == 1) *(char *)19 = 71;
	}

	// If we actually made it this far, choose an exit code.
	// The calculation gives about 50% chance of a normal exit code
	// (zero), otherwise some exit code 1-10.
	int code = random() % 20;
	if(code > 10) code = 0;
	cout << name << " exiting code " << code << "." << endl;
	exit(code);
}
