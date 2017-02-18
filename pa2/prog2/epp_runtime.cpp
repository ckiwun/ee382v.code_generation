#include "epp_runtime.h"
#include <iostream>
#include <vector>
#include <map>

using namespace std;

#define VEC_SIZE 1000

static map<int,vector<int>> count;
static map<int,int> r;

void init_path_reg(int loopId) {
	//this method should be instrumented at first nonphi of each for loop
	//assume each loop only call this one time
	cout << "initial" << endl;
	vector<int> path_count(VEC_SIZE);
	count[loopId] = path_count;
	r[loopId] = 0;
}

void inc_path_reg(int loopId, int val) {
	//this method should be instrumented at chord edges
	cout << "increment" << endl;
	r[loopId]+=val;
}

void finalize_path_reg(int loopId) {
	cout << "finalize" << endl;
	count[loopId][r[loopId]]++;
	r[loopId]=0;
}

void dump_path_regs() {
	cout << "dump" << endl;
	for(auto obj:count) {
		cout << "Number " << obj.first << ", ";
		auto pref = "";
		for(auto it:obj.second) {
			cout << pref << it;
			pref = ", ";
		}
		cout << endl;
	}
}

