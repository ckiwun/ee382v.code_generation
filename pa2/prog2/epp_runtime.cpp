#include "epp_runtime.h"
#include <iostream>
#include <vector>
#include <map>

using namespace std;

static map<int,vector<int>> count;
static map<int,int> r;
static map<int,bool> visited;
static int test = 0;

void printTest(int loopId, int whatever) {
	cout << "Loop ID from external: " << loopId << endl;
}

void init_path_reg(int loopId) {
	//this method should be instrumented at first nonphi of each for loop
	//assume each loop only call this one time
	r[loopId] = 0;
	if(visited[loopId]) return;
	cout << "initial" << endl;
	visited[loopId] = true;
	vector<int> path_count(8);
	count[loopId] = path_count;
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
	cout << count.size() << endl;
	for(auto obj:count) {
		cout << "[Loop #" << obj.first << "]" << endl;
		unsigned path_count = 0;
		for(auto it:obj.second) {
			cout << "[Path #" << path_count << "] " << it << endl;
			path_count++;
		}
	}
}

