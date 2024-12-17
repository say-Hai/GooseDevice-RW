#include "WeightReckon.h"
#include "SQL_Conn.h"
extern Logger weightLog;



int getWeightItem(vector<double>& Wc, string epc) {
	unique_ptr<SQL_Conn> conn(new SQL_Conn);
	bool isConnect =  conn->connect();
	if(!isConnect) {
		return 0;
	}
	const int initLen = Wc.size();
	if(initLen == 0)
		return 0;
//滤波
	median_filter(Wc, MEDIANNUM);
	moving_average_filter(Wc, AVGNUM);
	const int filterLen = Wc.size();

//滤波后的元素最大值
	double max_value = *std::max_element(Wc.begin(), Wc.end());

//最长子区间
	find_max_interval_data(Wc);
	const int intervalLen = Wc.size();

	weightLog.log("WeightReckon初始、滤波、有效数据个数为： " + to_string(initLen) + " "
				+ to_string(filterLen) + " " + to_string(intervalLen));
//最长子区间的平均体重
	int curWeight = calculate_average_and_round(Wc);
//如果子区间长度为1或者和上次体重偏差过大，则取此次数据的最大值
	if(intervalLen == 1)
		return 0;
	return curWeight;
}

void removeLessTwoKG(vector<double> &Wc) {
	for (int i = 0; i < Wc.size(); ) {
		if (Wc[i] < 2000) {
			Wc.erase(Wc.begin() + i);
		} else {
			i++; // 只有在不删除元素时才递增索引
		}
	}
}

// 中值滤波函数，直接修改原数组
void median_filter(std::vector<double>& data, int kernel_size) {
	std::deque<double> window;
	std::vector<double> result(data.size());

	for (size_t i = 0; i < data.size(); ++i) {
		if (window.size() >= kernel_size) {
			window.pop_front();
		}
		window.push_back(data[i]);

		// 复制并排序窗口内数据来计算中值
		std::vector<double> sorted_window(window.begin(), window.end());
		std::sort(sorted_window.begin(), sorted_window.end());

		// 取中值
		result[i] = sorted_window[sorted_window.size() / 2];
	}

	// 将结果复制回原数组
	data = result;
}

// 移动平均滤波函数，直接修改原数组
void moving_average_filter(std::vector<double>& data, int window_size) {
	std::vector<double> result;
	double sum = 0.0;

	for (size_t i = 0; i < data.size(); ++i) {
		sum += data[i];

		if (i >= window_size) {
			sum -= data[i - window_size];
		}

		if (i >= window_size - 1) {
			result.push_back(sum / window_size);
		}
	}
	// 将结果缩短并复制回原数组
	data = std::vector<double>(result.begin(), result.end());
}

// 定义函数找到最大区间的数据子集
void find_max_interval_data(std::vector<double>& weight) {
	std::unordered_map<int, std::vector<double>> result_map;

	// 遍历 weight 列表
	for (size_t i = 0; i < weight.size(); ++i) {
		double current_value = weight[i];
		double lower_bound = current_value - 100;
		double upper_bound = current_value + 100;

		// 查找在 [lower_bound, upper_bound] 区间内的所有元素
		std::vector<double> interval_values;
		for (const double& w : weight) {
			if (lower_bound <= w && w <= upper_bound) {
				interval_values.push_back(w);
			}
		}
		result_map[i] = interval_values;
	}

	// 找到长度最多的区间
	int max_length = 0;
	std::vector<double> max_interval_data;

	for (const auto& pair : result_map) {
		if (pair.second.size() > max_length) {
			max_length = pair.second.size();
			max_interval_data = pair.second;
		}
	}
	weight = max_interval_data;
	// 返回包含最大区间元素的子数组
	return ;
}

// 计算数组的平均值并四舍五入为整型
int calculate_average_and_round(const std::vector<double>& arr) {
	// 如果数组为空，返回 0
	if (arr.empty()) {
		return 0;
	}

	// 计算数组的总和
	double sum = std::accumulate(arr.begin(), arr.end(), 0.0);

	// 计算平均值
	double average = sum / arr.size();

	// 四舍五入为整型
	return static_cast<int>(std::round(average));
}