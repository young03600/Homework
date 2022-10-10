#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <fstream>
#include <time.h>
#include <string>
#include <algorithm>
#include <vector>
#include <regex>
using namespace std;

void *match(void *);

struct Arg{
    int index;
    int num;
    Arg(int _num, int _index) {
        num = _num;
        index = _index;
    }
};

vector<vector<size_t>> positions;
vector<string> dnaVec;
vector<string> searchDna;
int eachThread;
pthread_mutex_t lock;
void print();

int main(int argc, char *argv[]) {
   
    ifstream ifs(argv[1]);
    if(!ifs)
        throw "File not found\n";
    
    string temp;
    while(getline(ifs, temp)) {
        if(isdigit(temp[0])) break;
        dnaVec.push_back(temp);
    }
    eachThread = stoi(temp);
    while(getline(ifs, temp))
        searchDna.push_back(temp);
#ifdef _DEBUG
    cout << dnaVec[0].size() << '\n';
    cout << eachThread << '\n';
    for(const string &str : searchDna)
        cout << str << '\n';
    for(int i = 0; i < dnaVec.size(); ++i) {
        for(int j = 0; j < searchDna.size(); ++j) {
            cout << searchDna[j] << '\n';
            size_t pos = dnaVec[i].find(searchDna[j]);
            if(pos == string::npos) { cout << "not found\n"; continue; };
            while(pos != string::npos) {
                cout << pos << ' ';
                pos = dnaVec[i].find(searchDna[j], pos + 1);
            }
            cout << '\n';
        }
    }
#endif
    const int dnaNum = dnaVec.size();
    int numThread = searchDna.size() * eachThread;
    
    positions.resize(searchDna.size(), vector<size_t>());
    vector<pthread_t> tid(numThread);
    vector<pthread_attr_t> attr(numThread);

    for(int i = 0; i < numThread; ++i)
        pthread_attr_init(&attr[i]);

    if(pthread_mutex_init(&lock, nullptr) != 0) {
        cerr << "mutex init failed\n";
        exit(-1);
    }    

    clock_t start, end;
    double cpu_time_used;

    start = clock();
    for(int i = 0; i < dnaNum; ++i) {  //10
        int count = 0;
        int num = 0;
        for(int j = 0; j < numThread; ++j) {
            if(pthread_create(&tid[j], &attr[j], match, (void *)new Arg(num, count)) != 0) {
                cerr << "Create thread error\n";
                exit(-1);
            }
            ++count;
            if(count == eachThread) {
                ++num;
                count = 0;
            } 
        }
        for(int j = 0; j < numThread; ++j)
            pthread_join(tid[j], nullptr);
        
        print();
    }
    end = clock();
    
    cpu_time_used = ((double(end - start)) / CLOCKS_PER_SEC) * 1000;
    cout << "CPU time: " << cpu_time_used << " ms\n";

    pthread_mutex_destroy(&lock);
}

void *match(void *arguments) {

    int index = static_cast<Arg *>(arguments)->index;
    int num = static_cast<Arg *>(arguments)->num;
    
    string regExpr;
    if(pthread_mutex_lock(&lock) != 0) {
        cerr << "Lock failed\n";
        exit(-1);
    }
    string searching = searchDna[num];
    if(pthread_mutex_unlock(&lock) != 0) {
        cerr << "Unlock failed\n";
        exit(-1);
    }
    for(size_t i = 0; i < searching.size(); ++i)
        switch(searching[i]) {
            case 'A':
            case 'T':
            case 'C':
            case 'G':
                regExpr += searching[i];
                break;
            case '?':
                regExpr += ".";
                break;
            case '{':
                regExpr += "[";
                break;
            case '}':
                regExpr += "]";
                break;
            default:
                break;
        }
    regex reg(regExpr);

    if(pthread_mutex_lock(&lock) != 0) {
        cerr << "Lock failed\n";
        exit(-1);
    }
    const int n = dnaVec.size();
    if(pthread_mutex_unlock(&lock) != 0) {
        cerr << "Unlock failed\n";
        exit(-1);
    }
    for(int i = 0; i < n; ++i) {
        vector<size_t> positions;
        
        if(pthread_mutex_lock(&lock) != 0) {
            cerr << "Lock failed\n";
            exit(-1);
        }
        size_t range = dnaVec[i].size() / eachThread;      
        size_t start_search = range * index;
        start_search = (start_search == 0 ? start_search : start_search - searching.size());           
        string sub = dnaVec[i].substr(start_search, range);

        if(pthread_mutex_unlock(&lock) != 0) {
            cerr << "Unlock failed\n";
            exit(-1);
        }
        cout << '[' <<  "Tid=" << pthread_self() << ']' << " serach " << searching << " at "
            << start_search << ' ' << sub.substr(0, 8) << '\n';
        
        sregex_iterator next(sub.begin(), sub.end(), reg);
        sregex_iterator end;
        size_t pos, cumulate = 0;
        while(next != end) {
            pos = next->position() + cumulate;
            positions.push_back(pos + start_search);
            cumulate += next->position() + 1;
            next = sregex_iterator(sub.cbegin() + cumulate, sub.cend(), reg);
        }

        if(pthread_mutex_lock(&lock) != 0)                               
            cerr << "Luck failed\n";
        const int idx = (2 * num + index) / eachThread;
        for(auto & pos : positions)
            ::positions[idx].push_back(pos); // global vector
        if(pthread_mutex_unlock(&lock) != 0)                               
            cerr << "Unlock failed\n";
    }
    
    return nullptr;
}

void print() {

    cout << "\nSearching result:\n";
    for(size_t i = 0; i < searchDna.size(); ++i) {
        cout << '[' << searchDna[i] << "] ";
        if(positions[i].empty()) { cout << "Not found\n"; continue; }
        sort(positions[i].begin(), positions[i].end());
        positions[i].erase(unique(positions[i].begin(), positions[i].end()), positions[i].end());
        for(auto &x : positions[i])
            cout << x << ' ';
        cout << '\n';
        positions[i].clear();
    }
    cout << '\n';
}
