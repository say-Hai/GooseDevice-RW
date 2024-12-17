//
// Created by ubuntu on 8/17/24.
//

#ifndef SERVER_SQL_CONN_H
#define SERVER_SQL_CONN_H


#include <mysql.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "RFID.h"
#include "Logger.h"
#include "Defines.h"

using namespace std;

class SQL_Conn {

public:
	MYSQL *mysql;
	MYSQL_STMT *stmt;
public:
	SQL_Conn();

	~SQL_Conn();

	bool connect();

	bool insert_EnterOrExit_Data(string user_id, string device_id, string epc,
								 string start_time, string end_time, string direction, double weight = 0);

	bool insert_originRFID_Data(string user_id, string device_id, string ant_id, string epc, string start_time);

	bool insert_originWeight(string user_id, string device_id, string time, double weight);

	bool insert_weight_change_record(string user_id, string device_id, string epc, string time, double weight = 0);

	vector<double> select_WeightInTime(string user_id, string device_id, string start_time, string end_time);

	bool updateWeight(double weight, string epc);

	unordered_map<string, string> select_epc_isNest();

	bool insert_alarm_information(string device_id, string device_name, string alarm_notice,
								  string alarm_time, string house_id, int process_state);

	int select_LatestWeight(string epc);
};


#endif //SERVER_SQL_CONN_H
