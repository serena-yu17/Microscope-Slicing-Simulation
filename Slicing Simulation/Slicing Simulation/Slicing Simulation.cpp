// Slicing Simulation.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"	
using namespace std;

using DataSet = vector<pair<unsigned, unsigned>>;

namespace std
{
	template<>
	struct hash<pair<unsigned, unsigned>>
	{
		size_t operator()(pair<unsigned, unsigned> const& pair) const
		{
			return (pair.first << 16) | pair.second;
		}
	};
}

//unit: 0.1 um
const unsigned increment = 3;
//mutex mut;
random_device rd{};
mt19937 mt{ rd() };
gamma_distribution<> dista, distb;
const int batch = 5000;



inline unsigned sliceRadius(unsigned radiusSq, unsigned radius, unsigned distance);
void cut(unsigned radiusSq, unsigned radius, unsigned distance, DataSet* dict);
void cutBall(unsigned radius, DataSet* dict);
void populateDB(DataSet* summary, const char* conninfo);
string stringFromDict(vector<pair<unsigned, unsigned>>& summary, size_t begin);


inline unsigned sliceRadius(unsigned radiusSq, unsigned radius, unsigned distance)
{
	unsigned distCentre = radius - distance;
	if (radius < distance)
		distCentre = distance - radius;
	unsigned distanceSq = distCentre * distCentre;
	if (radiusSq > distanceSq)
		return (unsigned)round(sqrt(radiusSq - distanceSq));
}

void cut(unsigned radiusSq, unsigned radius, unsigned distance, DataSet* dset)
{
	unsigned sliceR = sliceRadius(radiusSq, radius, distance);
	dset->push_back(make_pair(round(sliceR), round(radius)));
}

void cutBall(unsigned radius, DataSet* dset)
{
	unsigned radiusSq = radius * radius;
	unsigned distance = increment;
	while (distance < radius * 2)
	{
		cut(radiusSq, radius, distance, dset);
		distance += increment;
	}
}

void processBall(DataSet* dset)
{
	for (unsigned radius = increment; radius < 500; radius++)
		cutBall(radius, dset);
}

int main()
{
	DataSet* data = new DataSet();
	cout << "Input database connection string. Example:\nhost=localhost port=5432 dbname=slice user=postgres password=psword1\n";
	string conninfo;
	cin >> conninfo;
	processBall(data);
	if (data->size())
		populateDB(data, conninfo.c_str());
	getchar();
}

void populateDB(DataSet* summary, const char* conninfo)
{
	char *errmsg = NULL;
	PGconn     *conn;
	PGresult   *res;
	conn = PQconnectdb(conninfo);
	if (PQstatus(conn) != CONNECTION_OK)
	{
		cout << PQerrorMessage(conn) << '\n';
		return;
	}


	res = PQexec(conn, "begin;");
	PQclear(res);
	res = PQexec(conn, "TRUNCATE table slice;");
	PQclear(res);
	cout << PQerrorMessage(conn) << '\n';
	for (size_t size = 0; size < summary->size(); size += batch)
	{
		cout << "Inserting on " << size << '\n';
		string strSummary = stringFromDict(*summary, size);
		res = PQexec(conn, "COPY slice(slice, sample) FROM STDIN;");
		PQclear(res);
		cout << PQerrorMessage(conn) << '\n';
		int copyStatus = PQputCopyData(conn, strSummary.c_str(), strSummary.size());
		if (copyStatus == -1)
			cout << PQerrorMessage(conn) << '\n';
		int endStatus = PQputCopyEnd(conn, errmsg);
		if (endStatus == -1)
			cout << PQerrorMessage(conn) << '\n';
		if (errmsg)
			cout << errmsg << '\n';
	}
	res = PQexec(conn, "commit;");
	PQclear(res);
	cout << PQerrorMessage(conn) << '\n';
	cout << "Done.";
	PQfinish(conn);
}

string stringFromDict(vector<pair<unsigned, unsigned>>& summary, size_t begin)
{
	string strDict;
	for (size_t i = begin; i < begin + batch && i < summary.size(); i++)
	{
		strDict.append(to_string(summary[i].first));
		strDict.push_back('\t');
		strDict.append(to_string(summary[i].second));
		strDict.push_back('\n');
	}
	strDict.pop_back();
	return strDict;
}