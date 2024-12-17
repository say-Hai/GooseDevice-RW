#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <utility>
#include<map>
#include <unordered_map>
#include <iostream>
#include "Defines.h"
#include <deque>
#include "Logger.h"

using namespace std;

int getWeightItem(vector<double>& Wc, string epc);

void removeLessTwoKG(vector<double>& Wc);
//中值滤波
void median_filter(std::vector<double>& data, int kernel_size);
//平均滤波
void moving_average_filter(std::vector<double>& data, int window_size);
//找到最大平稳区间
void find_max_interval_data(std::vector<double>& weight);
// 计算数组的平均值并四舍五入为整型
int calculate_average_and_round(const std::vector<double>& arr);