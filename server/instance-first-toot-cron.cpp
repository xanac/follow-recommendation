#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <set>
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


static string get_id (const picojson::value &toot)
{
	if (! toot.is <picojson::object> ()) {
		throw (TootException {});
	}
	auto properties = toot.get <picojson::object> ();
	if (properties.find (string {"id"}) == properties.end ()) {
		throw (TootException {});
	}
	auto id_object = properties.at (string {"id"});
	if (! id_object.is <double> ()) {
		throw (TootException {});
	}
	double id_double = id_object.get <double> ();
	stringstream s;
	s << static_cast <unsigned int> (id_double);
	return s.str ();
}


static string get_url (const picojson::value &toot)
{
	if (! toot.is <picojson::object> ()) {
		throw (TootException {});
	}
	auto properties = toot.get <picojson::object> ();
	if (properties.find (string {"url"}) == properties.end ()) {
		throw (TootException {});
	}
	auto url_object = properties.at (string {"url"});
	if (! url_object.is <string> ()) {
		throw (TootException {});
	}
	return url_object.get <string> ();
}


class Host {
public:
	string domain;
	time_t first_toot_time;
	string first_toot_url;
	string title;
public:
	Host (string a_domain, time_t a_time, string a_url, string a_title) {
		domain = a_domain;
		first_toot_time = a_time;
		first_toot_url = a_url;
		title = a_title;
	};
};


class byFresh {
public:
	bool operator () (const Host &left, const Host &right) const {
		return right.first_toot_time < left.first_toot_time;
	};
};


static string escape_json (string in)
{
	string out;
	for (auto c: in) {
		if (c == '\n') {
			out += string {"\\n"};
		} else if (c == '"'){
			out += string {"\\\""};
		} else if (c == '\\'){
			out += string {"\\\\"};
		} else {
			out.push_back (c);
		}
	}
	return out;
}


static void write_storage (FILE *out, vector <Host> hosts)
{
	fprintf (out, "[");
	for (unsigned int cn = 0; cn < hosts.size (); cn ++) {
		if (0 < cn) {
			fprintf (out, ",");
		}
		Host host = hosts.at (cn);
		ostringstream time_s;
		time_s << host.first_toot_time;
		fprintf (out, "{\"domain\":\"%s\",\"first_toot_time\":\"%s\",\"first_toot_url\":\"%s\",\"title\":\"%s\"}",
			host.domain.c_str (), time_s.str ().c_str (), host.first_toot_url.c_str (), escape_json (host.title).c_str ());
	}
	fprintf (out, "]");
}


static vector <picojson::value> get_timeline (string host)
{
	vector <picojson::value> timeline;

	string query
		= string {"https://"}
		+ host
		+ string {"/api/v1/timelines/public?local=true&max_id=20"};
	string reply = http_get (query);

	picojson::value json_value;
	string error = picojson::parse (json_value, reply);
	if (! error.empty ()) {
		throw (HostException {});
	}
	if (! json_value.is <picojson::array> ()) {
		throw (HostException {});
	}

	vector <picojson::value> toots = json_value.get <picojson::array> ();
	return toots;
}


static string get_host_title (string domain)
{
	string reply = http_get (string {"https://"} + domain + string {"/api/v1/instance"});

	picojson::value json_value;
	string error = picojson::parse (json_value, reply);
	if (! error.empty ()) {
		throw (HostException {});
	}
	if (! json_value.is <picojson::object> ()) {
		throw (HostException {});
	}
	auto properties = json_value.get <picojson::object> ();
	if (properties.find (string {"title"}) == properties.end ()) {
		throw (HostException {});
	}
	auto title_object = properties.at (string {"title"});
	if (! title_object.is <string> ()) {
		throw (HostException {});
	}
	return title_object.get <string> ();
}


static Host for_host (string domain)
{
	/* Get timeline. */
	vector <picojson::value> toots = get_timeline (domain);
	if (toots.size () < 1) {
		throw (HostException {});
	}

	const picojson::value &bottom_toot = toots.back ();
	time_t bottom_time;
	string bottom_url;
	try {
		bottom_time = get_time (bottom_toot);
		bottom_url = get_url (bottom_toot);
	} catch (TootException e) {
		throw (HostException {});
	}

	string title;
	try {
		title = get_host_title (domain);
	} catch (HostException e) {
		/* Do nothing. */
	}

	return Host {domain, bottom_time, bottom_url, title};
}


static bool valid_host_name (string host)
{
	return 0 < host.size () && host.at (0) != '#';
}


static set <string> get_domains ()
{
	string domains_s = http_get (string {"https://raw.githubusercontent.com/distsn/follow-recommendation/master/hosts.txt"});
	set <string> domains;
	string domain;
	for (char c: domains_s) {
		if (c == '\n') {
			if (valid_host_name (domain)) {
				domains.insert (domain);
			}
			domain.clear ();
		} else {
			domain.push_back (c);
		}
	}
	if (valid_host_name (domain)) {
		domains.insert (domain);
	}
	return domains;
}


int main (int argc, char **argv)
{
	set <string> domains = get_domains ();

	const string storage_filename = string {"/var/lib/distsn/instance-first-toot/instance-first-toot.json"};

	vector <Host> hosts;

	for (auto domain: domains) {
		try {
			Host host = for_host (string {domain});
			hosts.push_back (host);
		} catch (HttpException e) {
			/* Nothing. */
		} catch (HostException e) {
			/* Nothing. */
		}
	}

	sort (hosts.begin (), hosts.end (), byFresh {});

	FILE * storage_file_out = fopen (storage_filename.c_str (), "w");
	if (storage_file_out != nullptr) {
		write_storage (storage_file_out, hosts);
		fclose (storage_file_out);
	}

	return 0;
}

