#include <cstdio>
#include <cstdlib>
#include <tinyxml2.h>
#include <curl/curl.h>
#include <iostream>


using namespace std;
using namespace tinyxml2;


int main (int argc, char **argv)
{
	if (1 < argc) {
		FILE *in = fopen (argv [1], "r");
		if (in == nullptr) {
			cerr << "File " << argv [1] << "is not found." << endl;
			exit (1);
		}
		char host [1024];
		for (; ; ) {
			if (feof (in)) {
				break;
			}
			fgets (host, 1024, in);
			if (0 < strlen (host) && host [0] != '#' && host [0] != '\n') {
				if (host [strlen (host) - 1] == '\n') {
					host [strlen (host) - 1] = '\0';
				}
				cout << host << endl;
			}
		}
	} else {
		cout << "distsn-user-speed hosts.txt" << endl;
	}
	return 0;
}

