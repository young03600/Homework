#include <iostream>
#include <cstdlib>
#include <sys/types.h> 
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string> 
using namespace std;

enum class Directions { 
    None,
    Up, 
    Down, 
    Left, 
    Right, 
    UpLeft, 
    UpRight, 
    DownLeft, 
    DownRight
};

struct stru_shm {
    int guessX, guessY;
    int answerX, answerY;
    int guessRound;
    bool correct;
    bool parentAccess;
    bool childAccess;
    Directions dir;
    //stru_shm() : correct(false), guessRound(5) {}
};


int main(int argc, char *argv[]) {

    const unsigned int seed = atoi(argv[1]);
    if(seed < 0 ||seed > 100) {
       cerr << "the random seed value must be greater than or equal to 0 and less than or equal to 100\n";
       exit(1);
    }
    cout << argv[0] << '\n';
    srand(seed);

    const string name("shm");
    int fd = shm_open(name.c_str(), O_RDWR | O_CREAT, 0666);
    if(fd == -1)
       throw "shm_open failed\n"; 
    
    if(ftruncate(fd, sizeof(struct stru_shm)) == -1)
       throw "ftruncate failed\n";
  
    struct stru_shm *shmptr = (stru_shm *)mmap(NULL, 
                                               sizeof(struct stru_shm), 
                                               PROT_READ | PROT_WRITE, 
                                               MAP_SHARED, 
                                               fd,
                                               0);
   
    if(shmptr == MAP_FAILED)
       throw "MAP_FAILED\n";
   
    shmptr->guessRound = 5;
    shmptr->correct = false;
    shmptr->childAccess = false;
    shmptr->parentAccess = false;
    shmptr->dir= Directions::None;
    
    pid_t pid = fork();
    if(pid == -1) {
       cerr << "crate child process failed.\n";
       exit(1);
    }
    else if(pid == 0) {
        srand(seed+1);
        int answerX = rand() % 10, answerY = rand() % 10;
        
        cout << "[" << getpid() << "  Child process]: OK. Set entry {1, j} successfully" << '\n';
        shmptr->parentAccess = true;
        while(!shmptr->correct && shmptr->guessRound) {    
            if(shmptr->childAccess) {
                //cout << "shmptr->childAccess " <<  shmptr->guessRound << '\n';
                if(shmptr->guessRound == 1 && ( shmptr->guessX != answerX || shmptr->guessY != answerY ) ) {
                    cout << '[' << getpid() << "  Child Process]: Miss, you lose" << endl;
                    --shmptr->guessRound;
                    shmptr->answerX = answerX; shmptr->answerY = answerY;
                    exit(0);
                }
                if(shmptr->guessX == answerX && shmptr->guessY == answerY) {
                    cout << '[' << getpid() << "  Child Process]: You got the target\n";
                    shmptr->correct = true;
                    exit(0);
                }
                // guess 5 5  ans 6 4
                if(shmptr->guessX > answerX && shmptr->guessY > answerY) {
                    //cout << shmptr->guessX << ' ' << shmptr->guessY << '\n';
                    cout << '[' << getpid() << "  Child Process]: Miss, Up Left\n";
                    shmptr->dir = Directions::UpLeft;
                }
                else if(shmptr->guessX > answerX && shmptr->guessY < answerY) {
                    //cout << shmptr->guessX << ' ' << shmptr->guessY << '\n';
                    cout << '[' << getpid() << "  Child Process]: Miss, Down Left\n";
                    shmptr->dir = Directions::DownLeft;
                }
                else if(shmptr->guessX < answerX && shmptr->guessY > answerY) {
                    //cout << shmptr->guessX << ' ' << shmptr->guessY << '\n';
                    cout << '[' << getpid() << "  Child Process]: Miss, Up Right\n";
                    shmptr->dir = Directions::UpRight;
                }
                else if(shmptr->guessX < answerX && shmptr->guessY < answerY) {
                    //cout << shmptr->guessX << ' ' << shmptr->guessY << '\n';
                    cout << '[' << getpid() << "  Child Process]: Miss, Down Right\n";
                    shmptr->dir = Directions::DownRight;
                }
                else if(shmptr->guessY == answerY && shmptr->guessX < answerX) {
                    //cout << shmptr->guessX << ' ' << shmptr->guessY << '\n';
                    cout << '[' << getpid() << "  Child Process]: Miss, Right\n";
                    shmptr->dir = Directions::Right;
                }
                else if(shmptr->guessY == answerY && shmptr->guessX > answerX) {
                    //cout << shmptr->guessX << ' ' << shmptr->guessY << '\n';
                    cout << '[' << getpid() << "  Child Process]: Miss, Left\n";
                    shmptr->dir = Directions::Left;
                }
                else if(shmptr->guessX == answerX && shmptr->guessY < answerY) {
                    //cout << shmptr->guessX << ' ' << shmptr->guessY << '\n';
                    cout << '[' << getpid() << "  Child Process]: Miss, Down\n";
                    shmptr->dir = Directions::Down;
                }
                else if(shmptr->guessX == answerX && shmptr->guessY > answerY) {
                    //cout << shmptr->guessX << ' ' << shmptr->guessY << '\n';
                    cout << '[' << getpid() << "  Child Process]: Miss, Up\n";
                    shmptr->dir = Directions::Up;
                }
                --shmptr->guessRound;
                //cout << "guess round: " << shmptr->guessRound << '\n';
                shmptr->parentAccess = true;
                shmptr->childAccess = false;
            }
        }
        //exit(0);
    }else { 
        cout << "[" << getpid() << " Parent process]: " << "Creat a child " << pid << '\n';
        int boundedXUpper = 10, boundedXLower = 0;
        int boundedYUpper = 10, boundedYLower = 0;
        while(!shmptr->correct && shmptr->guessRound) {
            if(shmptr->parentAccess) {
                //cout << "BoundedXLower: " << boundedXLower << " boundedXUpper: " << boundedXUpper << " boundedYLower: " << boundedYLower << " boundedYUpper: " << boundedYUpper << '\n';
                
                switch(shmptr->dir) {
                    
                    case Directions::Up:
                        boundedYUpper = shmptr->guessY;
                        shmptr->guessY = rand() % boundedYUpper;
                        break;
                    case Directions::Down:
                        boundedYLower = shmptr->guessY;
                        shmptr->guessY = boundedYLower + ( ( rand() % ( boundedYUpper - boundedYLower - 1 ) ) + 1 );
                        break;
                    case Directions::Left:
                       boundedXUpper = shmptr->guessX;
                       shmptr->guessX = rand() % boundedXUpper;
                       break;
                    case Directions::Right:
                       boundedXLower = shmptr->guessX;
                       shmptr->guessX = boundedXLower + ( ( rand() % ( boundedXUpper - boundedXLower - 1 ) ) + 1 );
                       break;
                    case Directions::UpLeft:
                       boundedXUpper = shmptr->guessX;
                       boundedYUpper = shmptr->guessY;
                       shmptr->guessX = rand() % boundedXUpper;
                       shmptr->guessY = rand() % boundedYUpper;
                       break;
                    case Directions::UpRight:
                       boundedXLower = shmptr->guessX;
                       boundedYUpper = shmptr->guessY;
                       shmptr->guessX = boundedXLower + ( ( rand() % ( boundedXUpper - boundedXLower - 1 ) ) + 1 );
                       shmptr->guessY = rand() % boundedYUpper;
                       break;
                    case Directions::DownLeft:
                       boundedXUpper = shmptr->guessX;
                       boundedYLower = shmptr->guessY;
                       shmptr->guessX = rand() % boundedXUpper;
                       shmptr->guessY = boundedYLower + ( ( rand() % ( boundedYUpper - boundedYLower - 1 ) ) + 1 );
                       break;
                    case Directions::DownRight:
                       boundedXLower = shmptr->guessX;
                       boundedYLower = shmptr->guessY;
                       shmptr->guessX = boundedXLower + ( ( rand() % ( boundedXUpper - boundedXLower - 1 ) ) + 1);
                       shmptr->guessY = boundedYLower + ( ( rand() % ( boundedYUpper - boundedYLower - 1 ) ) + 1);
                       break;
                    case Directions::None:
                       shmptr->guessX = rand() % boundedXUpper;
                       shmptr->guessY = rand() % boundedYUpper;
                       shmptr->childAccess = true;
                       break;
                    defalut:
                       break;
                }
                cout << '[' << getpid() << " Parent Process]: Guess: " << '[' << shmptr->guessX << ',' << shmptr->guessY << ']' << '\n';
                //cout << "BoundedXLower: " << boundedXLower << " boundedXUpper: " << boundedXUpper << " boundedYLower: " << boundedYLower << " boundedYUpper: " << boundedYUpper << '\n';
                shmptr->childAccess = true;
                shmptr->parentAccess = false;
            }
        }
        if(!shmptr->correct)
            cout << '[' << getpid() << " Parent Process]: Target: [" << shmptr->answerX << ',' << shmptr->answerY << "]\n";
    }
  
    shm_unlink(name.c_str());
    return 0;
}

