#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <vector>
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


static string get_username (const picojson::value &toot)
{
	if (! toot.is <picojson::object> ()) {
		throw (TootException {});
	}
	auto properties = toot.get <picojson::object> ();
	if (properties.find (string {"account"}) == properties.end ()) {
		throw (TootException {});
	}
	auto account = properties.at (string {"account"});
	if (! account.is <picojson::object> ()) {
		throw (TootException {});
	}
	auto account_map = account.get <picojson::object> ();
	if (account_map.find (string {"username"}) == account_map.end ()) {
		throw (TootException {});
	}
	auto username = account_map.at (string {"username"});
	if (! username.is <string> ()) {
		throw (TootException {});
	}
	auto username_s = username.get <string> ();
	return username_s;
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


static void write_storage (FILE *out, map <string, double> memo)
{
	fprintf (out, "{");
	for (auto user: memo) {
		string username = user.first;
		double speed = user.second;
		fprintf (out, "\"%s\":%e,", username.c_str (), speed);
	}
	fprintf (out, "}");
}


static void for_host (string host)
{
	/* Apply forgetting rate to memo. */
	const string storage_filename = string {"/var/lib/distsn/user-speed/"} + host;
	map <string, double> memo;

	FILE * storage_file_in = fopen (storage_filename.c_str (), "r");
	if (storage_file_in != nullptr) {
		memo = read_storage (storage_file_in);
		fclose (storage_file_in);
	}

	const double forgetting_rate = 0.125;

	for (auto &user_memo: memo) {
		user_memo.second *= (1.0 - forgetting_rate);
	}

	/* Save memo. */
	{
		FILE * storage_file_out = fopen (storage_filename.c_str (), "w");
		if (storage_file_out != nullptr) {
			write_storage (storage_file_out, memo);
			fclose (storage_file_out);
		}
	}

	/* Get timeline. */
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

	double duration = top_time - bottom_time;
	if (! (1.0 < duration && duration < 60 * 60 * 24 * 365)) {
		throw (HostException {});
	}

	map <string, unsigned int> occupancy;

	for (auto toot: toots) {
		string username = get_username (toot);
		if (occupancy.find (username) == occupancy.end ()) {
			occupancy.insert (pair <string, unsigned int> (username, 1));
		} else {
			occupancy.at (username) ++;
		}
	}
	
	/* Update memo. */
	for (auto user_occupancy: occupancy) {
		double speed = static_cast <double> (user_occupancy.second) / duration;
		auto user_memo = memo.find (user_occupancy.first);
		if (user_memo == memo.end ()) {
			memo.insert (pair <string, double> (user_occupancy.first, speed));
		} else {
			memo.at (user_occupancy.first) += forgetting_rate * speed;
		}
	}
	
	/* Save memo again. */
	{
		FILE * storage_file_out = fopen (storage_filename.c_str (), "w");
		if (storage_file_out != nullptr) {
			write_storage (storage_file_out, memo);
			fclose (storage_file_out);
		}
	}
}


static bool valid_host_name (string host)
{
	return 0 < host.size () && host.at (0) != '#';
}


int main (int argc, char **argv)
{
	string hosts_s = http_get (string {"https://raw.githubusercontent.com/distsn/follow-recommendation/master/hosts.txt"});
	vector <string>	hosts;
	string host;
	for (char c: hosts_s) {
		if (c == '\n') {
			if (valid_host_name (host)) {
				hosts.push_back (host);
			}
			host.clear ();
		} else {
			host.push_back (c);
		}
	}
	if (valid_host_name (host)) {
		hosts.push_back (host);
	}

	for (auto host: hosts) {
		try {
			for_host (string {host});
		} catch (HttpException e) {
			/* Nothing. */
		} catch (HostException e) {
			/* Nothing. */
		}
	}

	return 0;
}

