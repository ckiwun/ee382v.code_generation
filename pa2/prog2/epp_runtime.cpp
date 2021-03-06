#include "epp_runtime.h"
#include <iostream>
#include <vector>
#include <map>

using namespace std;

static map<int,map<int,int>> count;
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
	visited[loopId] = true;
	std::map<int,int> path_count;
	count[loopId] = path_count;
}

void inc_path_reg(int loopId, int val) {
	//this method should be instrumented at chord edges
	r[loopId]+=val;
}

void finalize_path_reg(int loopId) {
	count[loopId][r[loopId]]++;
	r[loopId]=0;
}

void update_route(int loopId,int pathId,char* test) {
	string str_test(test);
	cout << "[TESTING!!!] " << loopId << " " << pathId << " " << str_test << endl;
}

void dump_path_regs() {
	cout << "LoopID  PathID  PathInfo" << endl;
	for(auto loop:count)
	for(auto path:loop.second) {
		cout << loop.first << "       " << path.first << "       " << path.second << endl;
	}
}

