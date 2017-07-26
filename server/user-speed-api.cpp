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


static map <string, string> read_storage_application (FILE *in)
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
	
	map <string, string> memo;
	
	for (auto user: object) {
		string username = user.first;
		string application = user.second.get <string> ();
		memo.insert (pair <string, string> (username, application));
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
	unsigned int speed_order;
	double manual_score;
	bool manual_score_available;
	unsigned int recommendation_order;
	string application;
public:
	User (string a_host, string a_username, double a_speed, string a_application) {
		host = a_host;
		username = a_username;
		speed = a_speed;
		application  = a_application;
	};
	bool operator < (const User &right) const {
		return right.speed < speed;
	};
};


class bySpeed {
public:
	bool operator () (const User &left, const User &right) const {
		return right.speed < left.speed;
	};
};


static set <string> get_hosts (string hosts_s)
{
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
	return hosts;
}


class UserException: public exception {
	/* Nothing. */
};


static string get_host (const picojson::value &user)
{
	if (! user.is <picojson::object> ()) {
		throw (UserException {});
	}
	auto properties = user.get <picojson::object> ();
	if (properties.find (string {"host"}) == properties.end ()) {
		throw (UserException {});
	}
	auto object = properties.at (string {"host"});
	if (! object.is <string> ()) {
		throw (UserException {});
	}
	return object.get <string> ();
}


static string get_username (const picojson::value &user)
{
	if (! user.is <picojson::object> ()) {
		throw (UserException {});
	}
	auto properties = user.get <picojson::object> ();
	if (properties.find (string {"username"}) == properties.end ()) {
		throw (UserException {});
	}
	auto object = properties.at (string {"username"});
	if (! object.is <string> ()) {
		throw (UserException {});
	}
	return object.get <string> ();
}


static double get_manual_score (const picojson::value &user)
{
	if (! user.is <picojson::object> ()) {
		throw (UserException {});
	}
	auto properties = user.get <picojson::object> ();
	if (properties.find (string {"score"}) == properties.end ()) {
		throw (UserException {});
	}
	auto object = properties.at (string {"score"});
	if (! object.is <double> ()) {
		throw (UserException {});
	}
	return object.get <double> ();
}


static map <string, double> get_manual_score (string score_s)
{
	map <string, double> score;

	picojson::value json_value;
	string error = picojson::parse (json_value, score_s);
	if (! error.empty ()) {
		cerr << error << endl;
		return map <string, double> {}; /* Silent error. */
	}
	if (! json_value.is <picojson::array> ()) {
		return map <string, double> {}; /* Silent error. */
	}

	vector <picojson::value> users = json_value.get <picojson::array> ();
	
	for (auto user: users) {
		try {
			string host = get_host (user);
			string user_name = get_username (user);
			double manual_score = get_manual_score (user);
			score.insert (pair <string, double> {user_name + string {"@"} + host, manual_score});
		} catch (UserException e) {
			/* Do nothing. */
		}
	}
	return score;
}


class byScore {
public:
	bool operator () (const User &left, const User &right) const {
		if (left.manual_score < 2.0) {
			if (right.manual_score < 2.0) {
				return
					right.manual_score < left.manual_score? true:
					right.manual_score == left.manual_score? false:
					right.speed < left.speed;
			} else {
				return false;
			}
		} else {
			if (right.manual_score < 2.0) {
				return true;
			} else {
				return right.speed * pow (10, right.manual_score) < left.speed * pow (10, left.manual_score);
			}
		}
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


int main (int argc, char **argv)
{
	string hosts_s = http_get (string {"https://raw.githubusercontent.com/distsn/follow-recommendation/master/hosts.txt"});
	set <string> hosts = get_hosts (hosts_s);
	
	vector <User> users;

	for (auto host: hosts) {
		map <string, double> speeds;
		map <string, string> applications;
		{
			const string storage_filename = string {"/var/lib/distsn/user-speed/"} + host;
			FILE * storage_file_in = fopen (storage_filename.c_str (), "r");
			if (storage_file_in != nullptr) {
				speeds = read_storage (storage_file_in);
				fclose (storage_file_in);
			}
		}
		{
			const string storage_filename = string {"/var/lib/distsn/user-application/"} + host;
			FILE * storage_file_in = fopen (storage_filename.c_str (), "r");
			if (storage_file_in != nullptr) {
				applications = read_storage_application (storage_file_in);
				fclose (storage_file_in);
			}
		}
		for (auto i: speeds) {
			string username = i.first;
			double speed = i.second;
			string application;
			if (applications.find (username) != applications.end ()) {
				application = applications.at (username);
			}
			User user {host, username, speed, application};
			users.push_back (user);
		}
	}
	
	{
		vector <User> active_users;
		for (auto i: users) {
			if (1.0 <= i.speed * 60 * 60 * 24 * 7) {
				active_users.push_back (i);
			}
		}
		users = active_users;
	}

	string score_s = http_get (string {"https://raw.githubusercontent.com/distsn/follow-recommendation/master/manual-score.txt"});
	map <string, double> score = get_manual_score (score_s);

	for (auto &user: users) {
		string key = user.username + string {"@"} + user.host;
		if (score.find (key) == score.end ()) {
			user.manual_score = 2.0;
			user.manual_score_available = false;
		} else {
			user.manual_score = score.at (key);
			user.manual_score_available = true;
		}
	}

	sort (users.begin (), users.end (), byScore {});

	for (unsigned int cn = 0; cn < users.size (); cn ++) {
		users.at (cn).recommendation_order = cn;
	}

	sort (users.begin (), users.end (), bySpeed {});
	
	for (unsigned int cn = 0; cn < users.size (); cn ++) {
		users.at (cn).speed_order = cn;
	}

	cout << "Content-type: application/json" << endl << endl;

	cout << "[";
	
	for (unsigned int cn = 0; cn < users.size () && cn < 10000; cn ++) {
		auto user = users.at (cn);
		if (cn != 0) {
			cout << ",";
		}
		cout
			<< "{"
			<< "\"host\":\"" << user.host << "\","
			<< "\"username\":\"" << escape_json (user.username) << "\","
			<< "\"speed\":" << scientific << user.speed << ","
			<< "\"speed_order\":" << user.speed_order << ","
			<< "\"manual_score\":" << user.manual_score << ","
			<< "\"manual_score_available\":" << (user.manual_score_available? "true": "false") << ","
			<< "\"recommendation_order\":" << user.recommendation_order << ","
			<< "\"application\":\"" << escape_json (user.application) << "\""
			<< "}";
	}

	cout << "]";

	return 0;
}

