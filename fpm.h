/*
* FPM
* Alireza Naimi
* March, 2015
*/

#ifndef FN_H
#define FN_H

#ifdef ALI_EXPORT
#define ALI_API __declspec(dllexport)
#else
#define ALI_API __declspec(dllimport)
#endif

#include "rec.h"
#include <sstream>
#include <vector>
#include <ctime>
#include <iostream>
#include <cstdio>
#include <string>
#include <functional>
#include <signal.h>
#include <atomic>
#include <boost/signals2.hpp>

using namespace std;

enum mnth {
	January, February, March, April, May, June,
	July, August, September, October, November, December
};

enum days{ Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday };

static const char *monthsNames[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
static const char * dayNames[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

void ali_timer(std::function<void(void)>, unsigned int);


typedef unsigned long long ullong;

class db
{
public:

	boost::signals2::signal<void()> sigImported;
	boost::signals2::signal<void()> SigExported;
	boost::signals2::signal<void(string)> SigMSG;
	db();
	vector <rec> allRec;
	unsigned concurentThreadsSupported;
	static bool multithreaded_console_IO;

	int preprocess_o(char *);
	void process_o(bool DoW[], bool MoY[], bool ToD[], struct tm tm_fr, struct tm tm_to);
	void process();
	int preprocess();
	void process2();
	int split_by;

	bool DoW[7];
	bool MoY[12];
	bool ToD[24];
	vector <int> Yremoved;

	struct structProgrs{
		std::atomic<ullong> cnt;
		std::atomic<ullong> ib;
		string cur;
		string progrs_txt;
		std::atomic<double> progrs_per;
		std::atomic<ullong> cnt_line_read;
		std::atomic<ullong> cnt_line_wrote;
		std::atomic<ullong> total_shp_files;
		std::atomic<ullong> total_shp_recs;
		std::atomic<ullong> total_shp_recs_done;
        long steps;
	};
	static structProgrs progrs;
	static ullong length;

	void Set_fr(struct tm v);
	struct tm Get_fr();

	void Set_to(struct tm v);
	struct tm Get_to();

	time_t Get_fr_timet();
	time_t Get_to_timet();


	struct tm tm_fr;
	struct tm tm_to;

	struct tm tm_fr_min();
	struct tm tm_to_max();

	time_t dur();

	void find_minmax_fr_to();

	struct structMapf{
		int x = 0;
		int y = 1;
		int segment = 2;
		int truckid = 3;
		int readdate = 4;
		int speed = 5;
		int heading = 6;
		int iid = 7;
	};
	static structMapf mapf;

	static bool export_csv;
	static bool export_shp;
	static bool export_separateDT;
	int sizl = 0;
	char *test[10];

	bool AllDoWChecked = true;
	bool AllMoYChecked = true;
	bool AllToDChecked = true;
	int readdb();
	char * read_wholefile();

	void reset();
	void char2tm(const char *ca_dt, time_t *);
	mnth get_month() { return month; }
	string get_month_str();
	void set_month(string);
	void set_month(mnth);

	char delim = ',';
	char delimbs = '/';
	char delimsc = ':';

	int w2shp();
	int _w2shp();
	int _w2shp_n2(string fn, vector<unsigned long> lst);

	int _w2shp_o();
	int _w2shp_o(string fn, long fr, long to);

	std::string str_in;
	std::string str_out;
        
    void chkinc(bool * inc, rec * r);
	static bool lock;
	void hdlCMDMSG(string);

	// int maino(int, char**);
	int maino(int argc, char* argv[]);
	void get_file_size();
	static bool applyDST;

private:
	time_t _tm_fr_it;
	time_t _tm_to_it;
	struct tm _tm_fr_min;
	struct tm _tm_to_max;
	time_t _dur;
	rec r;
	mnth month;
	char *ctmpp;
	int st_dt = 0;
	int iperv = 0;
	inline void filrec(char * rowp, bool * inc);

	void split_row(char *row, char* a[], char** b);
	void split_row2(char *row, char* a[], char** b);
	void split_row_s(char *row, string(&s)[10]);
	void chkinc_once(bool DoW[], bool MoY[], bool ToD[]);

	void find_start_datetime(int & st_dt, int & sizl, const char *ctmpp);
	int testwithCpp(char * name, int N);
	int doSomeComputation(int * numbers, size_t size);

	int ifs();
	void sortdb();
	void fill_tz();
	void fill_tz_(size_t beg, size_t end);
};

std::string itos(int);
const std::string currentDateTime();

#endif


