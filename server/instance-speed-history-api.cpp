#include <cstdio>
#include <cstdlib>
#include <string>


using namespace std;


static bool valid_character (char c)
{
	return ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || (c == '-') || (c == '.');
}


static bool valid_filename (string filename)
{
	if (filename.size () < 1) {
		return false;
	}
	if (filename.at (0) == '.') {
		return false;
	}
	for (auto c: filename) {
		if (! valid_character (c)) {
			return false;
		}
	}
	return true;
}


int main (int argc, char ** argv)
{
	if (argc < 2) {
		return 1;
	}
	string filename = string {argv [1]};
	if (! valid_filename (filename)) {
		return 2;
	}
	const string base = string {"/var/lib/distsn/instance-speed-cron"};
	const string path = base + string {"/"} + filename;
	FILE *in = fopen (path.c_str (), "r");
	if (in == nullptr) {
		return 3;
	}
	printf ("Content-type: text/csv\n\n");
	char b [1024];
	for (; ; ) {
		if (feof (in)) {
			break;
		}
		fgets (b, 1024, in);
		printf ("%s", b);
	}
}


