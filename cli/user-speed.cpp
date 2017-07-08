#include <cstdio>
#include <cstdlib>
#include <tinyxml2.h>
#include <curl/curl.h>
#include <iostream>
#include "picojson.h"


using namespace std;
using namespace tinyxml2;


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


class HostException: public exception {
	/* Nothing */
};


class TootException: public exception {
	/* Nothing */
};


static time_t str2time (string s)
{
	struct tm tm;
	strptime (s.c_str (), "%Y-%m-%dT%H:%M:%S", & tm);
	return mktime (& tm);
}


static time_t get_time (const picojson::value &toot)
{
	if (! toot.is <picojson::object> ()) {
		throw (TootException {});
	}
	auto properties = toot.get <picojson::object> ();
	if (properties.find (string {"created_at"}) == properties.end ()) {
		throw (TootException {});
	}
	auto time_object = properties.at (string {"created_at"});
	if (! time_object.is <string> ()) {
		throw (TootException {});
	}
	auto time_s = time_object.get <string> ();
	return str2time (time_s);
}


static string get_user_id (const picojson::value &toot)
{
	return string {};
}


static void for_host (string host)
{
	string reply_1 = http_get (string {"https://"} + host + string {"/api/v1/timelines/public?local=true&limit=40"});

	picojson::value json_value;
	string error = picojson::parse (json_value, reply_1);
	if (! error.empty ()) {
		throw (HostException {});
	}
	if (! json_value.is <picojson::array> ()) {
		throw (HostException {});
	}
	const picojson::array & toots = json_value.get <picojson::array> ();
	if (toots.size () != 40) {
		throw (HostException {});
	}
	const picojson::value &top_toot = toots.at (0);
	const picojson::value &bottom_toot = toots.at (toots.size () - 1);
	time_t top_time;
	time_t bottom_time;
	try {
		top_time = get_time (top_toot);
		bottom_time = get_time (bottom_toot);
	} catch (TootException e) {
		throw (HostException {});
	}
	cout << host << " " << top_time - bottom_time << endl;
}


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
				try {
					for_host (string {host});
				} catch (HttpException e) {
					/* Nothing. */
				} catch (HostException e) {
					/* Nothing. */
				}
			}
		}
	} else {
		cout << "distsn-user-speed hosts.txt" << endl;
	}
	return 0;
}

