#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
using namespace std;

size_t num_thread;
size_t num_resource;


bool safe(int, vector<vector<int>> &, vector<vector<int>> &, vector<int> &, const vector<int> & = vector<int>());
bool resource_request(vector<pair<vector<int>, char>>::iterator &it, vector<pair<vector<int>, char>> &request,
	vector<vector<int>> &allocation, vector<int> &available, vector<vector<int>> &need, vector<pair<vector<int>, char>> &q);

vector<size_t> safe_sequence;

int main(int agrc, char *argv[]) {

	ifstream ifs(argv[1]);
	if (!ifs) {
		cout << "File not found\n";
		return -1;
	}

	vector<int> available;
	vector<vector<int>> max, allocation;
	vector<pair<vector<int>, char>> request;
	vector<pair<vector<int>, char>> q;


	string temp;
	int val;
	while (getline(ifs, temp)) {
		if (temp == "#MAX") break;
		istringstream iss(temp);
		while (iss >> val)
			available.push_back(val);
	}

	while (getline(ifs, temp)) {
		if (temp == "#ALLOCATION") break;
		istringstream iss(temp);
		vector<int> v;
		while (iss >> val)
			v.push_back(val);
		max.push_back(v);
	}

	while (getline(ifs, temp)) {
		if (temp == "#REQUEST") break;
		istringstream iss(temp);
		vector<int> v;
		while (iss >> val)
			v.push_back(val);
		allocation.push_back(v);
	}

	while (getline(ifs, temp)) {
		istringstream iss(temp);
		string s;
		vector<int> v;
		while (iss >> s) {
			if (isalpha(s[0])) break;
			v.push_back(stoi(s));
		}
		request.push_back(make_pair(v, s[0]));
	}

	num_thread = allocation.size();
	num_resource = available.size();
	
	vector<vector<int>> need;
	for (size_t i = 0; i < num_thread; ++i) {
		vector<int> v; v.push_back(allocation[i][0]);
		for (size_t j = 1; j <= num_resource; ++j)
			v.push_back(max[i][j] - allocation[i][j]);
		need.push_back(v);
	}

#ifdef _DEBUG

	cout << "num_thread: " << num_thread << '\n';
	cout << "num_resource: " << num_resource << '\n';

	cout << "available:\n";
	for (auto &x : available)
		cout << x << ' ';
	cout << '\n';

	cout << "max:\n";
	for (auto &v : max) {
		for (auto &x : v)
			cout << x << ' ';
		cout << '\n';
	}

	cout << "allocation:\n";
	for (auto &v : allocation) {
		for (auto &x : v)
			cout << x << ' ';
		cout << '\n';
	}

	cout << "request:\n";
	for (auto &x : request) {
		for (auto &val : x.first)
			cout << val << ' ';
		cout << x.second << '\n';
	}

	cout << "need:\n";
	for (auto &v : need) {
		for (auto &x : v)
			cout << x << ' ';
		cout << '\n';
	}
#endif

	if (!safe(0, allocation, need, available)) {
		cout << "Initial state: not safe\n";
		return 0;
	}


	//vector<int> work(available);
	for (auto it = request.begin(); it != request.end();) {
		
		if (it->second == 'r') {
			size_t i;
			for (i = 1; i <= num_resource; ++i)
				if (allocation[it->first[0]][i] < it->first[i]) break;
			if (i == num_resource + 1) {
				cout << '(';
				for (size_t j = 0; j <= num_resource; ++j) {
					cout << it->first[j];
					if (j != num_resource) cout << ", ";
				}
				cout << "): RELEASE granted, AVAIABLE=(";
				for (size_t j = 1; j <= num_resource; ++j) {
					available[j - 1] += it->first[j];
					cout << available[j - 1];
					allocation[it->first[0]][j] -= it->first[j];
					need[it->first[0]][j] += it->first[j];
					if (j != num_resource) cout << ", ";
				}
				cout << ")\n";
			}
			else {
				cout << '(';
				for (size_t j = 0; j <= num_resource; ++j) {
					cout << it->first[j];
					if (j != num_resource) cout << ", ";
				}
				cout << "): ";
				
				cout << "release resource greater than the gid " << it->first[0] << " allocation, request aborted\n";
				//it = request.erase(it); // abort this request
				
				//continue;
			}
			

			for (auto q_iter = q.begin(); q_iter != q.end();) {

				if (resource_request(q_iter, request, allocation, available, need, q)) {  // granted
					if (!safe(1, allocation, need, available, q_iter->first))
						++q_iter;  // next request in the waiting queue
					else
						q_iter = q.erase(q_iter); // request successfully, pop from the queue
				}
				else { // not granted
					if (!q.empty()) {
						cout << "gid " << q_iter->first[0] << " waits for other threads releasing resources\n";
						++q_iter; // next request in the waiting queue
					}
				}
			}
			
			it = request.erase(it);
			continue;
		}

		//request type == allocation
		if (resource_request(it, request, allocation, available, need, q))  // granted
			safe(1, allocation, need, available, it->first);

		// if this is the last request check all the request in the waiting queue
		if (it == request.end() - 1) {
			cout << "This is the last request, so check all the requests in the waiting queue\n";
			cout << "If not granted, direcly abort those request\n";
			for (auto q_iter = q.begin(); q_iter != q.end();) {
				if(resource_request(q_iter, request, allocation, available, need, q))
					safe(1, allocation, need, available, q_iter->first);
				q_iter = q.erase(q_iter);
			}

			if (!q.empty()) {
				cerr << "not empty()\n";
			}
		}

		it = request.erase(it);

#ifdef _DEBUG
		cout << "available:\n";
		for (auto &x : available)
			cout << x << ' ';
		cout << '\n';

		cout << "allocation:\n";
		for (auto &v : allocation) {
			for (auto &x : v)
				cout << x << ' ';
			cout << '\n';
		}

		cout << "need:\n";
		for (auto &v : need) {
			for (auto &x : v)
				cout << x << ' ';
			cout << '\n';
		}
#endif

	}
}

bool safe(int mode, vector<vector<int>> &allocation, vector<vector<int>> &need, vector<int> &available, const vector<int> &request) {
	
	vector<int> work(available);
	vector<bool> finish(num_thread, false);


	if(!safe_sequence.empty()) safe_sequence.clear();

	if (mode) {
		cout << '(';
		for (size_t i = 0; i <= num_resource; ++i) {
			cout << request[i];
			if (i != num_resource) cout << ", ";
		}
		cout << "): WORK=(";
		for (size_t i = 0; i < num_resource; ++i) {
			cout << work[i];
			if (i != num_resource - 1) cout << ", ";
		}
		cout << ")\n";
	}


	bool found = false;
	int num_found = 0;
	while (num_found < num_thread) {
		for (size_t i = 0; i < num_thread; ++i) {
			if (finish[i]) continue;

			size_t j;
			for (j = 1; j <= num_resource; ++j)
				if (need[i][j] > work[j - 1])
					break;

			if (j == num_resource + 1) {
				for (size_t k = 1; k <= num_resource; ++k)
					work[k - 1] += allocation[i][k];

				if (mode) {
					cout << "(";
					for (size_t k = 0; k <= num_resource; ++k) {
						cout << request[k];
						if (k != num_resource) cout << ", ";
					}
					cout << "): ";

					cout << "gid " << i << " finish, WORK=(";
					for (size_t k = 0; k < num_resource; ++k) {
						cout << work[k];
						if (k != num_resource - 1) cout << ", ";
					}
					cout << ")\n";
				}

				finish[i] = true;
				++num_found;
				found = true;

				safe_sequence.push_back(i);
				i = 0;
			}
		}


		if (!found) {
			if (mode) {
				cout << '(';
				for (size_t i = 0; i <= num_resource; ++i) {
					cout << request[i];
					if (i != num_resource) cout << ", ";
				}
				cout << "): Not SAFE, restored the old state, ";

				for (size_t i = 1; i <= num_resource; ++i) {
					available[i - 1] += request[i];
					allocation[request[0]][i] -= request[i];
					need[request[0]][i] += request[i];
				}

				cout << "AVAILABLE=(";
				for (size_t i = 0; i < num_resource; ++i) {
					cout << available[i];
					if (i != num_resource - 1) cout << ", ";
				}
				cout << ")\n";
			}
			return false;
		}
	}

	if (mode) {
		cout << '(';
		for (size_t i = 0; i <= num_resource; ++i) {	
			cout << request[i];
			if (i != num_resource) cout << ", ";
		}
		cout << ") request granted, safe sequence=[";
	}
	else
		cout << "Initial state: safe, safe sequence = [";
	for (size_t i = 0; i < safe_sequence.size(); ++i) {
		cout << safe_sequence[i];
		if (i != safe_sequence.size() - 1) cout << ", ";
	}
	cout << "]\n";


	return true;
}

bool resource_request(vector<pair<vector<int>, char>>::iterator &it, vector<pair<vector<int>, char>> &request,
	vector<vector<int>> &allocation, vector<int> &available, vector<vector<int>> &need, vector<pair<vector<int>, char>> &q) {
	
	size_t i;
	for (i = 1; i <= num_resource; ++i)
		if (it->first[i] > need[it->first[0]][i])
			break;

	if (i == num_resource + 1) {
		cout << "(";
		for (size_t j = 0; j <= num_resource; ++j) {
			cout << it->first[j];
			if (j != num_resource) cout << ", ";
		}
		cout << "): Need OK\n";

		size_t j;
		for (j = 1; j <= num_resource; ++j)
			if (it->first[j] > available[j - 1])  //request > available
				break;
		if (j == num_resource + 1) {

			cout << '(';
			for (size_t k = 0; k <= num_resource; ++k) {
				cout << it->first[k];
				if (k != num_resource) cout << ", ";
			}
			cout << "): AVAILABLE OK\n";

			for (size_t k = 1; k <= num_resource; ++k) {
				available[k - 1] -= it->first[k];
				allocation[it->first[0]][k] += it->first[k];
				need[it->first[0]][k] -= it->first[k];
			}
		}
		//need ok but available not ok
		else if (j != num_resource + 1 && request.size() == 1) {
			cout << '(';
			for (size_t k = 0; k <= num_resource; ++k) {
				cout << it->first[k];
				if (k != num_resource) cout << ", ";
			}
			cout << "): AVAILABLE not ok, dircetly abort this request\n";
			return false;
		}
		else{
			q.push_back(*it);
			
			cout << '(';
			for (size_t k = 0; k <= num_resource; ++k) {
				cout << it->first[k];
				if (k != num_resource) cout << ", ";
			}
			cout << "): AVAILABLE not ok, gid " << it->first[0] << " waits for other threads releasing resources\n";
		
			return false;
		}
	}
	// need not ok
	else {
		cout << "(";
		for (size_t k = 0; k <= num_resource; ++k) {
			cout << it->first[k];
			if (k != num_resource) cout << ", ";
		}
		cout << "): NEED error, request aborted\n";
		//it = request.erase(it);
		
		return false;
	}

	return true;
}
