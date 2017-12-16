/*
* FPM
* Alireza Naimi
* March, 2015
*/

#if __WIN32__
#define stat64 _stat64
#endif

#include "fn.h"
#include <fcntl.h>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <iomanip>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <time.h>
#include <vector>
#include <mutex>
#include <future>
#include <stdio.h>
#include <sys/types.h>
#include <fstream>
#include "tz.h"
#include "rec.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <boost/signals2/signal.hpp>

#define _XOPEN_SOURCE
#define LF 0x0a
#define CR 0x0D
#define OUT_BUFF_SIZE 512

using namespace std;
using namespace boost::posix_time;

db::db(){
	concurentThreadsSupported = std::thread::hardware_concurrency();
}

// Initilize variables and objects
db::structProgrs db::progrs = {};
ullong db::length = 0;
bool db::lock = false;
db::structMapf db::mapf = {};
bool db::export_csv = false;
bool db::export_shp = true;
bool db::export_separateDT = true;
bool db::multithreaded_console_IO = true;
bool db::applyDST = true;
time_t tmt = {};
int thr = {};
struct tm tm_tmp = {};
bool withinrng = true;
bool yearIncluded = true;

struct tm db::Get_fr() { return tm_fr; }
struct tm db::Get_to() { return tm_fr; }
time_t db::Get_fr_timet() { return _tm_fr_it; }
time_t db::Get_to_timet() { return _tm_to_it; }
struct tm db::tm_fr_min(){ return _tm_fr_min; }
struct tm db::tm_to_max(){ return _tm_to_max; }
time_t db::dur(){ return _dur; }
struct tm _tmtmp = {};

// Decrypt the input data
void db::char2tm(const char *ca_dt, time_t * t)
{
	_tmtmp.tm_year = -1900 + (ca_dt[0] - 48) * 1000 + (ca_dt[1] - 48) * 100 + (ca_dt[2] - 48) * 10 + (ca_dt[3] - 48);
	_tmtmp.tm_mon = (ca_dt[5] - 48) * 10 + (ca_dt[6] - 48) - 1;
	_tmtmp.tm_mday = (ca_dt[8] - 48) * 10 + (ca_dt[9] - 48);
	_tmtmp.tm_hour = (ca_dt[11] - 48) * 10 + (ca_dt[12] - 48);
	_tmtmp.tm_min = (ca_dt[14] - 48) * 10 + (ca_dt[15] - 48);
	_tmtmp.tm_sec = (ca_dt[17] - 48) * 10 + (ca_dt[18] - 48);
	_tmtmp.tm_isdst = 0;

	timezone = 0; _timezone = 0;
    *t = mktime(&_tmtmp);
}

void db::set_fr(struct tm v)
{
	tm_fr = v;
	_tm_fr_it = mktime(&v);
	_dur = std::difftime(_tm_to_it, _tm_fr_it);
}

void db::set_to(struct tm v)
{
	tm_to = v;
	_tm_to_it = mktime(&v);
	_dur = std::difftime(_tm_to_it, _tm_fr_it);
}

// Reset all objects
void db::reset(){
	progrs.cnt = 0;
	iperv = 0;
	progrs.ib = 0;
	progrs.cnt_line_wrote = 0;
	progrs.cnt_line_read = 0;
}

std::string itos(int i){
	std::stringstream ss;
	ss << i;
	return ss.str();
}

void ali_timer(std::function<void(void)> func, unsigned int interval)
{
	std::thread([func, interval]() {
		while (true)
		{
			func();
			std::this_thread::sleep_for(std::chrono::milliseconds(interval));
		}
	}).detach();
}

int db::Computation(int * numbers, size_t size) {
	int answer = 0;
	for (size_t k = 0; k < size; k += 2) {
		answer += numbers[k];
	}
	for (size_t k = 1; k < size; k += 2) {
		answer += numbers[k] * 2;
	}
	return answer;
}

int db::testwithCpp(char * name, int N) {
	int answer = 0;
	ifstream in(name, ios::binary);
	vector<char> numbers(512);
	for (int t = 0; t < N; ++t) {
		int size = 0;
		in.read(reinterpret_cast<char *>(&size), sizeof(char));
		numbers.resize(size);
	}
	in.close();
	return answer;
}

int char2int(const char *c, int max_dig)
{
	int ret = 0, dec = 0;
	for (int i = 0; i < max_dig; i++)
	{
		if (c[i] >= 48 && c[i] < 58)
		{
			ret *= 10;
			ret += (c[i] - 48);
		}
	}

	return ret;
}

// sort cmp
void db::sortdb()
{
	std::sort(this->allRec.begin(), this->allRec.end(), before_key());
}

void db::chkinc_once(bool DoW[], bool MoY[], bool ToD[]){
	for (int i = 0; i < 7; i++)
	{
		if (!DoW[i]) AllDoWChecked = false;
	}

	for (int i = 0; i < 12; i++)
	{
		if (!MoY[i]) AllMoYChecked = false;
	}

	for (int i = 0; i < 24; i++)
	{
		if (!ToD[i]) AllToDChecked = false;
	}
}

void db::chkinc(bool * inc, rec * r)
{
	withinrng = true;
	tm_tmp = r->get_loc();
	yearIncluded = true;
	for (int i : Yremoved)
		if (i - 1900 == tm_tmp.tm_year) yearIncluded = false;

	if (DoW[tm_tmp.tm_wday] && MoY[tm_tmp.tm_mon] && ToD[tm_tmp.tm_hour] && withinrng && yearIncluded)
		*inc = true;
	else
		*inc = false;
}

string db::get_month_str() { return monthsNames[month]; }

mnth mnth_str2enum(string mnth) {
	return December;
}

void db::set_month(string m) {
	try
	{
		month = mnth_str2enum(m);
	}
	catch (int e)
	{
		cout << "Value of month is invalid" << endl;
	}
}

void db::set_month(mnth monthvalue) {
	try
	{
		month = monthvalue;
	}

	catch (int e)
	{
		cout << "Value of month is invalid" << endl;
	}
}

void db::hdlCMDMSG(string str){
	cout << str;
	fstream o;
	o.open("fpm.log", fstream::out | fstream::app | fstream::ate);
	o << str;
	o.close();
}

const std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
	return buf;
}

int main_wrap(int argc, char *argv[])
{
	db db2;
	return db2.maino(argc, argv);
}


// Find boundaries in the database 
void db::find_minmax_fr_to(){
	if (allRec.size() < 3) return;
	time_t min = allRec[2].t,
		max = allRec[2].t;

	time_t sec_min3 = 2000000000;
	time_t sec_max3 = 0;
	time_t sec = 0;

	for (rec i : allRec)
	{
		sec = i.t;
		if (sec > 0){
			if (sec_min3 > sec && sec > 0)
			{
				sec_min3 = sec;
				min = i.t;
			}
			if (sec_max3 < sec)
			{
				sec_max3 = sec;
				max = i.t;
			}
		}
	}

	_tm_fr_min = *localtime(&min);
	_tm_to_max = *localtime(&max);

	string strMin = std::asctime(&_tm_fr_min);
	string strMax = std::asctime(&_tm_to_max);
	SigMSG("min date/time: " + strMin + "max date/time: " + strMax);
}

// main application
int db::main(int argc, char* argv[]) {

	db db2;
	db2.multithreaded_console_IO = false;
	db2.SigMSG.connect(bind(&db::hdlCMDMSG, &db2, _1));
		
	db2.split_by = 0;
	char * c_in = {};
	char * c_out = (char *)"filtered";
	char * fr = (char *)"1900-01-01 00:00:00";
	char * to = (char *)"3000-01-01 00:00:00";

	bool DoW[7] = { true }; std::fill_n(DoW, 7, true);
	bool MoY[12] = { true }; std::fill_n(MoY, 12, true);// MoY[0] = true;
	bool ToD[24] = { true }; std::fill_n(ToD, 24, true); //ToD[2] = true;

	if (argc < 3) { // Check the value of argc. If not enough parameters have been passed, inform user and exit.
		std::cout << "Usage: " << endl
			<< "fpm.exe -f <infile> -o <outdir> -d <day of week [Su,Mo,...,Sa]> -m <month of year [0,1,..,12]> -t <time of day [0,1,...,24]>" << endl
			<< "" << endl
			<< "Example:" << endl
			<< "fpm.exe -f c:\\201201.csv -o c:\\ali.csv -d 1,1,1,1,1,1,1 -m 1,1,1,1,1,1,1,1,1,1,1,1 -t 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1" << endl;
		db2.SigMSG("Usage... \n");

		exit(0);
	}
	else {

		char* myOutPath = argv[0];
		string str("");
		for (int i = 1; i < argc; i++) {
			str = string(argv[i]);
			if (str == "-f") {
				c_in = argv[i + 1];
			}
			else if (str == "-o") {
				c_out = argv[i + 1];
			}
			else if (str == "-d") {
				char * d = argv[i + 1];
				int cnt = 0;
				for (int j = 0; j < strlen(d); j++)
					if (isdigit(d[j]) && cnt < 7)
					{
						if (d[j] == '0') DoW[cnt] = false; else DoW[cnt] = true;
						cnt++;
					}
			}
			else if (str == "-m") {
				char * d = argv[i + 1];
				int cnt = 0;
				for (int j = 0; j < strlen(d); j++)
					if (isdigit(d[j]) && cnt < 12)
					{
						if (d[j] == '0') MoY[cnt] = false; else MoY[cnt] = true;
						cnt++;
					}
			}
			else if (str == "-t") {
				char * d = argv[i + 1];
				int cnt = 0;
				for (int j = 0; j < strlen(d); j++)
					if (isdigit(d[j]) && cnt < 24)
					{
						if (d[j] == '0') ToD[cnt] = false; else ToD[cnt] = true;
						cnt++;
					}
			}
			else if (str == "-fr") {
				fr = argv[i + 1];
			}
			else if (str == "-to") {
				to = argv[i + 1];
			}
			else if (str == "-s") {
				db2.split_by = std::stoi(argv[i + 1]);
			}
		}


		time_t tfr = {};
		time_t tto = {};
		struct tm tmfr = {};
		struct tm tmto = {};

		tmfr.tm_year = 100;
		tmto.tm_year = 200;

		db2.char2tm(fr, &tfr);
		db2.char2tm(to, &tto);

		string strtmp(c_in);
		db2.str_in = strtmp;

		string strtmp2(c_out);
		db2.str_out = strtmp2;

		std::copy(std::begin(DoW), std::end(DoW), std::begin(db2.DoW));
		std::copy(std::begin(MoY), std::end(MoY), std::begin(db2.MoY));
		std::copy(std::begin(ToD), std::end(ToD), std::begin(db2.ToD));
		db2.Set_fr(tmfr);
		db2.Set_to(tmto);

		db2.export_csv = true;
		db2.export_shp = true;
		db2.process2();

		db2.SigMSG(std::to_string(db2.allRec.size()) + " features imported.\n");

		db2._w2shp();
		db2.SigMSG(currentDateTime() + " done.\n");
	}

	return 0;
}
