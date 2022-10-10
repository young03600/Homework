#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctime>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

using namespace std;

struct injectInfo {
    string name;
    string id;
    int numInject;
    injectInfo(string &&_name, string &&_id) : name(move(_name)), id(move(_id)), numInject(0) {}
};

void *request(void *);
void *receive(void *);
queue<injectInfo *> q;
injectInfo *injectSeat = nullptr;
bool robot_sleep = true;
pthread_mutex_t mutex, mutex2;
pthread_cond_t working, wait_inject;
vector<bool> injectDone;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Argument list is wrong\n";
        return -1;
    }

    size_t n = stoi(argv[1]);
    if (n < 10 || n > 20) {
        cerr << "Number of students < 10 or > 20\n";
        return -2;
    }

    int seed = stoi(argv[2]);
    if (seed < 0 || seed > 100) {
        cerr << "Random number must >= 0 and <= 100\n";
        return -3;
    }

    injectDone.resize(n, false);

    pthread_mutex_init(&mutex, nullptr);
    pthread_mutex_init(&mutex2, nullptr);
    pthread_cond_init(&working, nullptr);
    pthread_cond_init(&wait_inject, nullptr);

    srand(stoi(argv[2]));
    vector<pthread_t> tid(n + 1);

    pthread_create(&tid[0], nullptr, receive, nullptr);
    for (size_t i = 1; i <= n; ++i)
        pthread_create(&tid[i], nullptr, request,
        i < 10 ? new injectInfo("Student", "0" + to_string(i)) : new injectInfo("Student", to_string(i)));

    for (size_t i = 0; i <= n; ++i)
        pthread_join(tid[i], nullptr);

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex2);
    pthread_cond_destroy(&working);
    pthread_cond_destroy(&wait_inject);
}

void *receive(void *arg) {
    tm *time_info;
    time_t raw_time;
    tm _tm;

    bool done = false;
    while (!done) {
        pthread_mutex_lock(&mutex2);
        time(&raw_time);
        time_info = localtime_r(&raw_time, &_tm);
        robot_sleep = true;
        cout << time_info->tm_hour << ':' << time_info->tm_min << ':' << time_info->tm_sec << " Robot: Sleep\n";
        pthread_cond_wait(&working, &mutex2);
        pthread_mutex_unlock(&mutex2);

        do {
            //cout << "Robot: waked up waiting for sit_done\n";
            robot_sleep = false;
            while (!injectSeat);

            //cout << "Robot: injecting\n";

            string name = injectSeat->name;
            string id = injectSeat->id;

            time(&raw_time);
            time_info = localtime_r(&raw_time, &_tm);
            cout << time_info->tm_hour << ':' << time_info->tm_min << ':' << time_info->tm_sec << ' '
                << name << id << " entering to get a shot\n";

            sleep(2);

            ++injectSeat->numInject;
            if (injectSeat->numInject == 3) injectDone[stoi(id) - 1] = true;
            injectSeat = nullptr;

            time(&raw_time);
            time_info = localtime_r(&raw_time, &_tm);
            cout << time_info->tm_hour << ':' << time_info->tm_min << ':' << time_info->tm_sec << ' '
                << name << id << " is leaving\n";

        }while (!q.empty() || injectSeat);
        //pthread_mutex_unlock(&mutex2);

        //cout << "Robot: now the queue is empty\n";

        bool flag = true;
        for (size_t i = 0; i < injectDone.size(); ++i)
        if (!injectDone[i]) {
            flag = false;
            break;
        }
        if (flag) {
            done = true;
            time(&raw_time);
            time_info = localtime_r(&raw_time, &_tm);
            cout << time_info->tm_hour << ':' << time_info->tm_min << ':' << time_info->tm_sec 
                 << " Robot: all injection was completed\n";
        }
    }
  return nullptr;
}

void *request(void *arg) {
    injectInfo *studentInfo = static_cast<injectInfo *>(arg);
    tm *time_info;

    int queue_waiting_time = rand() % 11;
    while (studentInfo->numInject != 3) {
        time_t raw_time;
        tm _tm;

        sleep(queue_waiting_time);

        pthread_mutex_lock(&mutex);
        if (injectSeat || !q.empty()) {
            while (q.size() == 3) {
                int quit = rand() % 6 + 5;
                sleep(quit);
            }
            //pthread_mutex_lock(&mutex);
            q.push(studentInfo);
            int size = q.size();
            time(&raw_time);
            time_info = localtime_r(&raw_time, &_tm);
            cout << time_info->tm_hour << ':' << time_info->tm_min << ':' << time_info->tm_sec << ' '
                << q.back()->name << q.back()->id << " sitting in #" << size << '\n';
            pthread_mutex_unlock(&mutex);

            pthread_mutex_lock(&mutex2);
            while (injectSeat || q.front() != studentInfo)
                pthread_cond_wait(&wait_inject, &mutex2);
        }
        //if(q.empty() && !injectSeat) pthread_mutex_unlock(&mutex);
        if (!q.empty() && q.front() == studentInfo) {
            q.pop();
            pthread_mutex_unlock(&mutex2);
        }

        while (robot_sleep) pthread_cond_signal(&working);

        if (q.empty() && !injectSeat) pthread_mutex_unlock(&mutex);
        injectSeat = studentInfo;

        while (injectSeat);
        pthread_cond_signal(&wait_inject);

        //cout << "student " << studentInfo->id << " back to request " << "check q.size() is " << q.size() << '\n';

        queue_waiting_time = rand() % 21 + 10;
    }

    return nullptr;
}
