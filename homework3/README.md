學號: 1083520       姓名:謝宏陽 
How to compile:      g++ ./prog3.cpp –o prog3 –lpthread -lrt 
How to run program:  ./prog3 <number of student> <random seed> 
 
a. 基本功能 
1. 從命令列讀入所有整數並能夠處理命令列輸入的各種錯誤 
2. 亂數的產生必須設定亂數種子。可以使用亂數相關函式，並且亂數種子只在main thread中設定一次 
3. 產生模擬每個人物的n +1個 thread 
4. 使用pthread API 中的 mutex 機制形成critical section來處理進入機器人辦公室的情況 
5. 使用pthread API 中的 mutex 機制形成critical section來處理辦公室走廊上等待的情況 
6. 執行 FCFS 的規則 
7. 由main thread 執行 join 讓程式結束 
 
程式的設計理念: 

- 設計一個structure injectInfo代表每位同學, 其中  
  int id        //學生編號  
  int numInject //已注射次數  
 
  pthread_mutex_t mutex, mutex2  
  //使用兩個mutex lock來實作處理進入機器人辦公室的情況 及 辦公室走廊上等待的情況 
 
  // working通知喚醒機器人, wait_inject代表辦公室已經有人, 必須等待pthread_cond_t working, wait_inject  
 
  // 模擬FCFS的走廊情況 queue<injectInfo *> q  
  // 初始化n位同學的狀態為false, 代表injectNum不為3 
  vector<bool> injectDone(n, false)  
 
- 在global宣告一個injectInfo *injectSeat, 代表機器人辦公室中的座位  
  初始injectSeat = nullptr, 代表目前沒人,  
  如果injectSeat不為nullptr代表目前辦公室有人  
 

- 模擬機器人的這個thread只有在有人的時候才醒著,  
  也就是辦公室座位有人 或是 代表走廊排隊的這個queue不為空 成立時  
  如果這兩個條件都不成立, 則讓機器人進入睡眠狀態, 當有學生到來, 而機器人為睡眠狀態, 這位學生就會透過pthread_cond_t的variable喚醒模擬機器人的這個thread  
 
- 使用queue<injectInfo *> q 來代表走廊情況 每一位的學生都是struct injectInfo的data type, injectNum用來記錄這位學生是否已經接種三次,  
  每次記錄由robot那端負責記錄, 並在每次幫同學注射完後, 檢查這位同學是否已經注射三次, 如果是, 則將同學id相對應於injectDone中的bool值設為true來表示已經完成,  
  如果injectDone都為true, 則結束程式 
  
如何完成走廊上一進一出的情況: 
- 使用pthread_cond_t variable wait_inject 
  只要一進入排隊就代表目前機器人辦公室中的座位有人,  
  因此呼叫pthread_cond_wait, 直到在機器人辦公室座位上那位同學完成注射後,  
  由那位離開辦公室的同學呼叫pthread_cond_signal(wait_injection),  
  在pthread_cond_wait的while迴圈會檢查目前自己是否為走廊上第一位同學, 如果是, 則進入辦公室進行注射, 如果不是, 則繼續在走廊上等待,  
  因此達到只有在列隊中第一位的同學能夠進入機器人辦公室進行注射  
 
- 初始等待時間, 注射秒數, 發現走廊座位滿了離開秒數, 每次注射完後再去排隊的時間, 皆使用sleep(int seconds)
