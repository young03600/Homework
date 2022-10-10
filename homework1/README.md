Prog. #1 Proc. Generation & Communication 
姓名: 謝宏陽 學號:1083520 
 
How to compile:      g++ ./prog1.cpp –o prog1 -lrt 
How to run program:  ./prog1 <seed number>  
 
a. 基本功能 
1. 父行程從命令列讀入整數參數，作為亂數種子。整數範圍0～100 
2. 亂數相關函式正確使用。亂數種子只設定一次 
3. 父行程能用fork()產生一個子行程。過程中只有一個子行程 
4. 行程只能用單一塊POSIX shared memory來互相傳遞正確參數給另一個行程 
5. 子行程能正確計算比較結果 
6. 行程之間可以正確同步控制執行計算流程 
7. 行程可以正確使用getpid() 

程式的設計理念: 
為方便說明將程式中的structure寫在這裡 
 
struct stru_shm{ 
 int guessX, guessY;                //存放Parent猜測的entry {i, j} 
 int answerX, answerY;              //存放Child所設定的entry{i, j} 
 
 int guessRound;                    //剩餘猜測次數 
 bool correct;                      //最終結果 猜對or猜錯 
 bool parentAccess;                 //是否允許Parent存取資料 
 bool childAccess;                  //是否允許Child存取資料 
 
 Directions dir;                    //根據Child所給方向調整猜測方向 
};  
 
在程式一開始呼叫POSIX API提供的function shm_open(), 
建立一個shared-memory的物件. 並呼叫ftruncate()設定這個物件的大小,  
在這裡的物件大小即是程式中宣告的struct stru_shm的大小. 
最後呼叫mmap()建立包含shared-memory物件的memory-mapped file, 
並使用mmap()回傳的pointer提供Parent Process 及 Child Process存取
shared-memory物件裡的資料. 
 
如何進行判斷: 
- Parent Process猜測完後會將{i, j}存入 guessX, guessY，傳送給Child Process進行判斷後, 修改Directions dir裡面的方向, 從而告知Parent Process應該往哪個方向猜測. 
- Parent得知錯誤後, 會依照Directions dir裡的方向修改{i, j}的bound. 

如何進行同步: 
- Parent 及 Child 在while迴圈持續檢查structure裡的bool變數ParentAccess 及 ChildAccess, 
  查看他們是否有權可以存取shared-memory物件裡的資料, 直到遇到終止條件. 

- 當ParentAccess為true, 則表示Parent可以進行猜測動作, 將猜測值{i, j}存入shared-memory物件, 並將ParentAccess設成false, ChildAccess設成true, 
  代表此時Parent不能再繼續進行猜測動作直到Child判斷完成, 而Child則可以到shared-memory裡存取猜測值{i, j}判斷正確與否. 

- 當ChildAccess為true, 則表示Parent已經進行猜測, 並由Child進行斷. 
  Child判斷完後會print出是否正確, 如果正確, 則結束程式, 如果錯誤則告訴Parent應該猜測的方向, 並將ChildAccess設為false, ParentAccess設成true, 
  此時表示允許Parent進行下一次的猜測, 並且停止Child到shared-memory存取猜測值{i, j}, 直到Parent進行完下一次的猜測.  
  
- 迴圈終止條件為structure裡的int guessRound 及 bool correct. 
  當guessRound為0 或 Parent猜中時, 必須離開迴圈結束程式, 否則持續檢查Parent 及 Child 該進行的動作, Parent 繼續猜測, Child繼續判斷正確與否.
