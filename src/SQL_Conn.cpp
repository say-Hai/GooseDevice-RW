#include <cstring>
#include "SQL_Conn.h"

Logger logger_sql("SQL.txt");

SQL_Conn::SQL_Conn() {}

SQL_Conn::~SQL_Conn() {
    if (stmt) {  // 检查 stmt 是否为空
        mysql_stmt_free_result(stmt);  // 释放 stmt 结果集
        mysql_stmt_close(stmt);  // 关闭 stmt
    }

    if (mysql) {  // 检查 mysql 是否为空
        mysql_close(mysql);  // 关闭 MySQL 连接
    }
}

bool SQL_Conn::connect() {
    mysql = mysql_init(nullptr);
    if (!mysql) {
        logger_sql.log("MySQL 初始化失败");
        return false;
    }

    if (!mysql_real_connect(mysql, MYSQL_IP, MYSQL_USR, MYSQL_PWD, MYSQL_DB, 0, nullptr, 0)) {
        logger_sql.log("数据库连接失败: " + std::string(mysql_error(mysql)));
        return false;
    }

    if (mysql_set_character_set(mysql, "utf8mb4") != 0) {
        logger_sql.log("设置字符集失败: " + std::string(mysql_error(mysql)));
        return false;
    }

    stmt = mysql_stmt_init(mysql);
    if (!stmt) {
        logger_sql.log("准备语句初始化失败: " + std::string(mysql_error(mysql)));
        return false;
    }

    logger_sql.log("数据库连接成功");
    return true;
}

bool SQL_Conn::insert_EnterOrExit_Data(string user_id, string device_id, string epc, string start_time, string end_time,
                                       string direction, double weight) {
    const char *insertQuery = "INSERT INTO scale_enter_exit_record (user_id, device_id, epc, start_timestamp, end_timestamp, direction, weight) VALUES(?, ?, ?, ?, ?, ?, ?)";

    // 预处理语句准备
    if (mysql_stmt_prepare(stmt, insertQuery, strlen(insertQuery)) != 0) {
        logger_sql.log("mysql_stmt_prepare() failed: " + std::string(mysql_stmt_error(stmt)));
        return false;
    }

    MYSQL_BIND params[7];
    memset(params, 0, sizeof(params));

    // 绑定 user_id
    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = (char *)user_id.c_str();
    params[0].buffer_length = user_id.length();

    // 绑定 device_id
    params[1].buffer_type = MYSQL_TYPE_STRING;
    params[1].buffer = (char *)device_id.c_str();
    params[1].buffer_length = device_id.length();

    // 绑定 epc
    params[2].buffer_type = MYSQL_TYPE_STRING;
    params[2].buffer = (char *)epc.c_str();
    params[2].buffer_length = epc.length();

    // 绑定 start_time
    params[3].buffer_type = MYSQL_TYPE_STRING;
    params[3].buffer = (char *)start_time.c_str();
    params[3].buffer_length = start_time.length();

    // 绑定 end_time
    params[4].buffer_type = MYSQL_TYPE_STRING;
    params[4].buffer = (char *)end_time.c_str();
    params[4].buffer_length = end_time.length();

    // 绑定 direction
    params[5].buffer_type = MYSQL_TYPE_STRING;
    params[5].buffer = (char *)direction.c_str();
    params[5].buffer_length = direction.length();

    // 绑定 weight
    params[6].buffer_type = MYSQL_TYPE_DOUBLE;
    params[6].buffer = &weight;
    params[6].buffer_length = sizeof(weight);

    // 绑定参数
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        logger_sql.log("mysql_stmt_bind_param() failed: " + std::string(mysql_stmt_error(stmt)));
        return false;
    }

    // 执行预处理语句
    if (mysql_stmt_execute(stmt) != 0) {
        logger_sql.log("mysql_stmt_execute() failed: " + std::string(mysql_stmt_error(stmt)));
        return false;
    }

    return true;
}

bool SQL_Conn::insert_originRFID_Data(string user_id, string device_id, string ant_id, string epc, string start_time) {
    const char *insertQuery = "INSERT INTO origin_rfid_record (user_id, device_id, epc, start_timestamp, ant_id) VALUES (?, ?, ?, ?, ?)";

    // 准备预处理语句
    if (mysql_stmt_prepare(stmt, insertQuery, strlen(insertQuery)) != 0) {
        logger_sql.log("mysql_stmt_prepare() failed: " + std::string(mysql_stmt_error(stmt)));
        return false;
    }

    MYSQL_BIND params[5];
    memset(params, 0, sizeof(params));

    // 绑定 user_id
    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = (char *)user_id.c_str();
    params[0].buffer_length = user_id.length();

    // 绑定 device_id
    params[1].buffer_type = MYSQL_TYPE_STRING;
    params[1].buffer = (char *)device_id.c_str();
    params[1].buffer_length = device_id.length();

    // 绑定 epc
    params[2].buffer_type = MYSQL_TYPE_STRING;
    params[2].buffer = (char *)epc.c_str();
    params[2].buffer_length = epc.length();

    // 绑定 start_time
    params[3].buffer_type = MYSQL_TYPE_STRING;
    params[3].buffer = (char *)start_time.c_str();
    params[3].buffer_length = start_time.length();

    // 绑定 ant_id
    params[4].buffer_type = MYSQL_TYPE_STRING;
    params[4].buffer = (char *)ant_id.c_str();
    params[4].buffer_length = ant_id.length();

    // 绑定参数
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        logger_sql.log("mysql_stmt_bind_param() failed: " + std::string(mysql_stmt_error(stmt)));
        return false;
    }

    // 执行预处理语句
    if (mysql_stmt_execute(stmt) != 0) {
        logger_sql.log("mysql_stmt_execute() failed: " + std::string(mysql_stmt_error(stmt)));
        return false;
    }

    return true;
}

bool SQL_Conn::insert_originWeight(string user_id, string device_id, string time, double weight) {
    const char *insertQuery = "INSERT INTO origin_weight_record (user_id, time, weight, device_id) VALUES (?, ?, ?, ?)";

    // 准备预处理语句
    if (mysql_stmt_prepare(stmt, insertQuery, strlen(insertQuery)) != 0) {
        logger_sql.log("mysql_stmt_prepare() failed: " + std::string(mysql_stmt_error(stmt)));
        return false;
    }

    MYSQL_BIND params[4];
    memset(params, 0, sizeof(params));

    // 绑定 user_id
    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = (char *)user_id.c_str();
    params[0].buffer_length = user_id.length();

    // 绑定 time
    params[1].buffer_type = MYSQL_TYPE_STRING;
    params[1].buffer = (char *)time.c_str();
    params[1].buffer_length = time.length();

    // 绑定 weight
    params[2].buffer_type = MYSQL_TYPE_DOUBLE;
    params[2].buffer = &weight;
    params[2].buffer_length = sizeof(weight);

    // 绑定 device_id
    params[3].buffer_type = MYSQL_TYPE_STRING;
    params[3].buffer = (char *)device_id.c_str();
    params[3].buffer_length = device_id.length();

    // 绑定参数
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        logger_sql.log("mysql_stmt_bind_param() failed: " + std::string(mysql_stmt_error(stmt)));
        return false;
    }

    // 执行预处理语句
    if (mysql_stmt_execute(stmt) != 0) {
        logger_sql.log("mysql_stmt_execute() failed: " + std::string(mysql_stmt_error(stmt)));
        return false;
    }

    return true;
}

bool SQL_Conn::insert_weight_change_record(string user_id, string device_id, string epc, string time, double weight) {
    const char *insertQuery = "INSERT INTO scale_weight_record (user_id, device_id, epc, start_timestamp, weight) VALUES (?, ?, ?, ?, ?)";

    // 准备预处理语句
    if (mysql_stmt_prepare(stmt, insertQuery, strlen(insertQuery)) != 0) {
        logger_sql.log("mysql_stmt_prepare() failed: " + std::string(mysql_stmt_error(stmt)));
        return false;
    }

    MYSQL_BIND params[5];
    memset(params, 0, sizeof(params));

    // 绑定 user_id
    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = (char *)user_id.c_str();
    params[0].buffer_length = user_id.length();

    // 绑定 device_id
    params[1].buffer_type = MYSQL_TYPE_STRING;
    params[1].buffer = (char *)device_id.c_str();
    params[1].buffer_length = device_id.length();

    // 绑定 epc
    params[2].buffer_type = MYSQL_TYPE_STRING;
    params[2].buffer = (char *)epc.c_str();
    params[2].buffer_length = epc.length();

    // 绑定 time
    params[3].buffer_type = MYSQL_TYPE_STRING;
    params[3].buffer = (char *)time.c_str();
    params[3].buffer_length = time.length();

    // 绑定 weight
    params[4].buffer_type = MYSQL_TYPE_DOUBLE;
    params[4].buffer = &weight;
    params[4].buffer_length = sizeof(weight);

    // 绑定参数
    if (mysql_stmt_bind_param(stmt, params) != 0) {
        logger_sql.log("mysql_stmt_bind_param() failed: " + std::string(mysql_stmt_error(stmt)));
        return false;
    }

    // 执行预处理语句
    if (mysql_stmt_execute(stmt) != 0) {
        logger_sql.log("mysql_stmt_execute() failed: " + std::string(mysql_stmt_error(stmt)));
        return false;
    }

    return true;
}

vector<double> SQL_Conn::select_WeightInTime(std::string user_id, std::string device_id, std::string start_time,
                                             std::string end_time) {
    vector<double> weights = {};
    const char *query = "SELECT weight FROM origin_weight_record WHERE user_id = ? AND device_id = ? AND time >= ? AND time <= ? LIMIT 60";

    // 准备预处理语句
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        logger_sql.log("mysql_stmt_prepare() failed: " + std::string(mysql_stmt_error(stmt)));
        return weights;
    }

    MYSQL_BIND params[4];
    memset(params, 0, sizeof(params));

    // 绑定 user_id
    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = (char *) user_id.c_str();
    params[0].buffer_length = user_id.length();

    // 绑定 device_id
    params[1].buffer_type = MYSQL_TYPE_STRING;
    params[1].buffer = (char *) device_id.c_str();
    params[1].buffer_length = device_id.length();

    // 绑定 start_time
    params[2].buffer_type = MYSQL_TYPE_STRING;
    params[2].buffer = (char *) start_time.c_str();
    params[2].buffer_length = start_time.length();

    // 绑定 end_time
    params[3].buffer_type = MYSQL_TYPE_STRING;
    params[3].buffer = (char *) end_time.c_str();
    params[3].buffer_length = end_time.length();

    // 绑定参数
    if (mysql_stmt_bind_param(stmt, params)) {
        logger_sql.log("mysql_stmt_bind_param() failed: " + std::string(mysql_stmt_error(stmt)));
        return weights;
    }

    // 执行预处理语句
    if (mysql_stmt_execute(stmt)) {
        logger_sql.log("mysql_stmt_execute() failed: " + std::string(mysql_stmt_error(stmt)));
        return weights;
    }

    // 获取结果元数据
    MYSQL_RES *resultMetadata = mysql_stmt_result_metadata(stmt);
    if (!resultMetadata) {
        logger_sql.log("mysql_stmt_result_metadata() failed: " + std::string(mysql_stmt_error(stmt)));
        return weights;
    }

    // 存储查询结果
    if (mysql_stmt_store_result(stmt)) {
        logger_sql.log("mysql_stmt_store_result() failed: " + std::string(mysql_stmt_error(stmt)));
        mysql_free_result(resultMetadata);
        return weights;
    }

    // 绑定结果变量
    double weight;
    MYSQL_BIND resultBind;
    memset(&resultBind, 0, sizeof(resultBind));
    resultBind.buffer_type = MYSQL_TYPE_DOUBLE;
    resultBind.buffer = &weight;

    // 绑定结果变量到预处理语句
    if (mysql_stmt_bind_result(stmt, &resultBind)) {
        logger_sql.log("mysql_stmt_bind_result() failed: " + std::string(mysql_stmt_error(stmt)));
        mysql_free_result(resultMetadata);
        return weights;
    }

    // 获取查询结果
    while (mysql_stmt_fetch(stmt) == 0) {
        weights.push_back(weight);
    }

    // 释放资源
    mysql_free_result(resultMetadata);
    return weights;
}










bool SQL_Conn::insert_alarm_information(string device_id, string device_name, string alarm_notice,
                                        string alarm_time, string house_id, int process_state) {
    const char *insertQuery = "insert into alarm_information (device_id, device_name, alarm_notice, alarm_time, house_id, process_state) values(?, ?, ?, ?, ?, ?)";
    mysql_stmt_prepare(stmt, insertQuery, strlen(insertQuery));
    MYSQL_BIND params[6];
    memset(params, 0, sizeof(params));
    // 绑定 device_id
    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = (char *) device_id.c_str();
    params[0].buffer_length = device_id.length();


    params[1].buffer_type = MYSQL_TYPE_STRING;
    params[1].buffer = (char *) device_name.c_str();
    params[1].buffer_length = device_name.length();


    params[2].buffer_type = MYSQL_TYPE_STRING;
    params[2].buffer = (char *) alarm_notice.c_str();
    params[2].buffer_length = alarm_notice.length();

    params[3].buffer_type = MYSQL_TYPE_STRING;
    params[3].buffer = (char *) alarm_time.c_str();
    params[3].buffer_length = alarm_time.length();

    params[4].buffer_type = MYSQL_TYPE_STRING;
    params[4].buffer = (char *) house_id.c_str();
    params[4].buffer_length = house_id.length();

    params[5].buffer_type = MYSQL_TYPE_LONG;
    params[5].buffer = &process_state;;
    if (mysql_stmt_bind_param(stmt, params)) {
        printf("mysql_stmt_bind_param\n");
        return false;
    }

    // 执行预处理语句
    int result = mysql_stmt_execute(stmt);
    if (result == 0) {
#ifdef DEBUG
        cout << "alarm 插入成功" << endl;
#endif // DEBUG
        return true;
    } else {
        const char *error = mysql_stmt_error(stmt);
#ifdef DEBUG
        cout << error << endl;
#endif // DEBUG

#ifdef LOGGEROUTPUT
        logger_sql.log(error);
#endif // LOGGEROUTPUT
    }
    return false;
}
int SQL_Conn::select_LatestWeight(std::string epc) {
    int weight = 0;
    const char *query = "SELECT weight FROM scale_weight_record WHERE epc = ? ORDER BY start_timestamp DESC LIMIT 1";

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        std::cerr << "mysql_stmt_prepare(), SELECT failed" << std::endl;
        return weight;
    }

    MYSQL_BIND param;
    memset(&param, 0, sizeof(param));

    // 绑定 epc 参数
    param.buffer_type = MYSQL_TYPE_STRING;
    param.buffer = (char *) epc.c_str();
    param.buffer_length = epc.length();

    if (mysql_stmt_bind_param(stmt, &param)) {
        std::cerr << "mysql_stmt_bind_param() failed" << std::endl;
        return weight;
    }

    // 执行预处理语句
    if (mysql_stmt_execute(stmt)) {
        std::cerr << "mysql_stmt_execute(), failed" << std::endl;
        return weight;
    }

    // 存储查询结果
    MYSQL_RES *resultMetadata = mysql_stmt_result_metadata(stmt);
    mysql_stmt_store_result(stmt);

    // 绑定结果变量
    MYSQL_BIND resultBind;
    memset(&resultBind, 0, sizeof(resultBind));
    resultBind.buffer_type = MYSQL_TYPE_LONG;
    resultBind.buffer = &weight;

    // 绑定结果
    if (mysql_stmt_bind_result(stmt, &resultBind) != 0) {
        std::cerr << "mysql_stmt_bind_result() failed" << std::endl;
        return weight;
    }

    // 获取结果
    if (mysql_stmt_fetch(stmt) != 0) {
        std::cerr << "mysql_stmt_fetch() failed" << std::endl;
        return weight;
    }

    return weight;
}

bool SQL_Conn::updateWeight(double weight, string epc) {
    const char *updateSQL = "UPDATE scale_enter_exit_record SET weight = ? WHERE epc = ? ORDER BY id DESC LIMIT 1";

    if (mysql_stmt_prepare(stmt, updateSQL, strlen(updateSQL)) != 0) {
        std::cerr << "无法准备预处理语句: " << mysql_stmt_error(stmt) << std::endl;
        return false;
    }

    MYSQL_BIND bindParams[2];
    memset(bindParams, 0, sizeof(bindParams));

    bindParams[0].buffer_type = MYSQL_TYPE_DOUBLE;
    bindParams[0].buffer = &weight;
    bindParams[0].buffer_length = sizeof(weight);

    // 绑定 epc
    bindParams[1].buffer_type = MYSQL_TYPE_STRING;
    bindParams[1].buffer = (char *) epc.c_str();
    bindParams[1].buffer_length = epc.length();

    if (mysql_stmt_bind_param(stmt, bindParams) != 0) {
        std::cerr << "无法绑定参数: " << mysql_stmt_error(stmt) << std::endl;
        return false;
    }

    int result = mysql_stmt_execute(stmt);
    if (result == 0) {
#ifdef DEBUG
        cout << "scale_enter_exit_record 更新成功" << endl;
#endif // DEBUG
        return true;
    } else {
        const char *error = mysql_stmt_error(stmt);
#ifdef DEBUG
        cout << error << endl;
#endif // DEBUG

#ifdef LOGGEROUTPUT
        logger_sql.log(error);
#endif // LOGGEROUTPUT
    }
    return false;
}

unordered_map<string, string> SQL_Conn::select_epc_isNest() {
    unordered_map<string, string> result;
    const char *query = "SELECT epc, enter_time FROM AtNest_record_View";
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        // 准备查询语句失败
        logger_sql.log("select_epc_isNest准备查询语句失败");
        return result;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        // 执行查询失败
        logger_sql.log("select_epc_isNest执行查询失败");
        return result;
    }


    MYSQL_RES *metadata = mysql_stmt_result_metadata(stmt);

    if (!metadata) {
        // 获取结果集元数据失败
        logger_sql.log("select_epc_isNest获取结果集元数据失败");
        return result; // 或者抛出异常
    }
    MYSQL_BIND resultBind[2];
    memset(&resultBind[0], 0, sizeof(resultBind[0]));
    char epc_buffer[1000];  // 存储查询结果的time字符串
    resultBind[0].buffer_type = MYSQL_TYPE_STRING;
    resultBind[0].buffer = (void *) epc_buffer;
    resultBind[0].buffer_length = sizeof(epc_buffer);

    memset(&resultBind[1], 0, sizeof(resultBind[1]));
    char time_buffer[1000];  // 存储查询结果的time字符串
    resultBind[1].buffer_type = MYSQL_TYPE_STRING;
    resultBind[1].buffer = (void *) time_buffer;
    resultBind[1].buffer_length = sizeof(time_buffer);


    if (mysql_stmt_bind_result(stmt, resultBind) != 0) {
        // 绑定结果集失败
        logger_sql.log("select_epc_isNest绑定结果失败：" + string(mysql_stmt_error(stmt)));
        return result;
    }

    while (mysql_stmt_fetch(stmt) == 0) {
        std::string epc = string(epc_buffer);
        std::string enter_time = string(time_buffer);
        result[epc] = enter_time;
    }

    return result;
}
