#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include "picojson.h"


using namespace std;


static int writer (char * data, size_t size, size_t nmemb, std::string * writerData)
{
	if (writerData == nullptr) {
		return 0;
	}
	writerData->append (data, size * nmemb);
	return size * nmemb;
}


class HttpException: public exception {
	/* Nothing */
};


static string http_get (string url)
{
	CURL *curl;
	CURLcode res;
	curl_global_init (CURL_GLOBAL_ALL);

	curl = curl_easy_init ();
	if (! curl) {
		throw (HttpException {});
	}
	curl_easy_setopt (curl, CURLOPT_URL, url.c_str ());
	string reply_1;
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, writer);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, & reply_1);
	res = curl_easy_perform (curl);
	curl_easy_cleanup (curl);
	if (res != CURLE_OK) {
		throw (HttpException {});
	}
	return reply_1;
}


static map <string, double> read_storage (FILE *in)
{
	string s;
	for (; ; ) {
		if (feof (in)) {
			break;
		}
		char b [1024];
		fgets (b, 1024, in);
		s += string {b};
	}
	picojson::value json_value;
	picojson::parse (json_value, s);
	auto object = json_value.get <picojson::object> ();
	
	map <string, double> memo;
	
	for (auto user: object) {
		string username = user.first;
		double speed = user.second.get <double> ();
		memo.insert (pair <string, double> (username, speed));
	}
	
	return memo;
}


static bool valid_host_name (string host)
{
	return 0 < host.size () && host.at (0) != '#';
}


class User {
public:
	string host;
	string username;
	double speed;
public:
	User (string a_host, string a_username, double a_speed) {
		host = a_host;
		username = a_username;
		speed = a_speed;
	};
	bool operator < (const User &right) const {
		return right.speed < speed;
	};
};


int main (int argc, char **argv)
{
	string hosts_s = http_get (string {"https://raw.githubusercontent.com/distsn/follow-recommendation/master/hosts.txt"});
	set <string> hosts;
	string host;
	for (char c: hosts_s) {
		if (c == '\n') {
			if (valid_host_name (host)) {
				hosts.insert (host);
			}
			host.clear ();
		} else {
			host.push_back (c);
		}
	}
	if (valid_host_name (host)) {
		hosts.insert (host);
	}
	
	vector <User> users;

	for (auto host: hosts) {
		const string storage_filename = string {"/var/lib/distsn/user-speed/"} + host;
		FILE * storage_file_in = fopen (storage_filename.c_str (), "r");
		if (storage_file_in != nullptr) {
			map <string, double> memo = read_storage (storage_file_in);
			fclose (storage_file_in);
			for (auto i: memo) {
				User user {host, i.first, i.second};
				users.push_back (user);
			}
		}
	}

	sort (users.begin (), users.end ());

	cout << "Content-type: text/html" << endl << endl;

	cout << "<ol>" << endl;
	
	for (unsigned int cn = 0; cn < 10000 && cn < users.size (); cn ++) {
		auto user = users.at (cn);
		cout
			<< "<li>"
			<< "<a href=\"" << "https://" << user.host << "/@" << user.username << "\" target=\"_blank\">"
			<< user.username << "@" << user.host
			<< "</a> "
			<< (user.speed * static_cast <double> (60 * 60 * 24))
			<< "</li>" << endl;
	}

	cout << "</ol>" << endl;

	return 0;
}

