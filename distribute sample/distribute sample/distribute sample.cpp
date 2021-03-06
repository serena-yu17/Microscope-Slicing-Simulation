// distribute sample.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <vector>
#include <string>

#include <boost/math/distributions/beta.hpp>
#include <libpq-fe.h>

#define bm boost::math

using namespace std;

const int increment = 3;
const int batch = 5000;
const int unit = 10;

void setProbability(vector<pair<int, double>>* prob, const bm::beta_distribution<double>&  dist)
{
	for (int radius = increment; radius < 100 * unit; radius++)
	{
		auto probability = bm::pdf(dist, (double)radius / (100 * unit));
		prob->push_back(make_pair(radius, probability));
	}
}

string strFromVector(vector<pair<int, double>>& prob)
{
	string str;
	for (auto& item : prob)
	{
		str.append(to_string(item.first));
		str.push_back('\t');
		str.append(to_string(item.second));
		str.push_back('\n');
	}
	return str;
}

void populateDB(vector<pair<int, double>>& prob, const char* conninfo)
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

	string strSummary = strFromVector(prob);

	res = PQexec(conn, "begin;");
	res = PQexec(conn, "TRUNCATE table sample;");
	cout << PQerrorMessage(conn) << '\n';
	for (size_t size = 0; size < prob.size(); size += batch)
	{
		cout << "Inserting on " << size << '\n';
		res = PQexec(conn, "COPY sample FROM STDIN;");
		cout << PQerrorMessage(conn) << '\n';
		int coptStatus = PQputCopyData(conn, strSummary.c_str(), strSummary.size());
		if (coptStatus == -1)
			cout << PQerrorMessage(conn) << '\n';
		int endStatus = PQputCopyEnd(conn, errmsg);
		if (endStatus == -1)
			cout << PQerrorMessage(conn) << '\n';
		if (errmsg)
			cout << errmsg << '\n';
	}
	res = PQexec(conn, "commit;");
	cout << PQerrorMessage(conn) << '\n';
	cout << "Done.";
	PQfinish(conn);
}

int main()
{
	cout << "Input database connection string. Example:\nhost=localhost port=5432 dbname=slice user=postgres password=psword1\n";
	string conninfo;
	cin >> conninfo;
	double a, b;
	cout << "Input beta factor a:\n";
	cin >> a;
	cout << "Input beta factor b:\n";
	cin >> b;
	vector<pair<int, double>> prob;
	const bm::beta_distribution<double> dist(5, 10);
	setProbability(&prob, dist);
	populateDB(prob, conninfo.c_str());
}

