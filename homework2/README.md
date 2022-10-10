 
學號: 1083520       姓名:謝宏陽 
How to compile:      g++ ./prog2.cpp –o prog2 -lpthread 
How to run program:  ./prog2 <path of input file> 
 
a. 基本功能 
1. 主執行緒能正確產生子執行緒 
2. 主執行緒與子執行緒可以達成下面要求 
3. 檔案只能讀一次，搜尋工作必須平均分配在所產生的執行緒 
4. 字元序列範圍是大寫英文字母{A,C,G,T} 
5. 目標序列最長為10K個字元，搜尋序列最長為32個字元。總共有2個搜尋序列。 
6. n 為 2。（也就是，程式必須同時執行4個搜尋的子執行緒） 
7. 子執行緒能用傳遞正確結果給主執行緒 
8. 子執行緒能自行用函式而不是透過主執行緒共用的變數拿到自己pthread_t 的 tid。 
9. 主執行緒能正確印出CPU 時間，以毫秒為單位。 

b.進階功能 
1. 搜尋序列總共有2～5個搜尋序列。n在2～4之間 
2. 在搜尋序列中，可以使用 “?”表示一個萬用字元，例如：CC?TT，表示收尋以下四種序列：CCATT, CCCTT, CCGTT, CCTTT 
3. 在搜尋序列中，可以限定某個字元的範圍，字元的範圍以{}表示，例如：CC{A,C}TT，表示要搜尋CCATT及CCCTT。 
 
 
程式的設計理念: 
- 使用3個global vector存取input string  
  以便threads可以共享這些data。  
  // input sequences  
  vector<string> dnaVec;  
  // input search sequences  
  vector<string> searchDna;  
  // all occurrence positions for each input search sequences  
  vector<vector<size_t>> positions;  

- 由於需要多個threads同時執行Match function, 可能同時存取這些global variables, 為了避免data race, 宣告 pthread_mutex_t lock, 以便使用pthread_mutex_lock() 
  
- 只要thread跑到critical section(存取global variable)的地方就執行pthread_mutex_lock()結束後執行pthread_mutex_unlock()。 

- Match function使用c++11支持的regex進行收尋及配對字串將每個input search string轉換成對應的regular expression 之後進行配對。 

- 由於每個input sequence必須分成n等分, 分割區塊之間必須要要讓下一個thread在收尋的時候將收尋範圍往前數input search sequence的size, 避免在分割區塊之後分割區間造成沒有收尋到的問題。 
