 學號: 1083520       姓名:謝宏陽 
How to compile:      g++ ./prog4.cpp –o prog4 
How to run program:  ./prog4 <path of input file> 
 
a. 基本功能 
1. Safety algorithm 
2. Banker’s algorithm 
 
程式的設計理念: 
- Data儲存方式:  
  將request以外給定vector以二維vector儲存  
  request則使用pair進行存取, 以得知某個request為哪種型態(‘a’, ‘r’)  
 
- Safety algorithm:  
  同課本所述, 以某時間OS下snapshot給定的available vector, 由上至下檢查,  
  是否有thread or process的need vector data皆小於available, 如果有這樣一個thread or process則在對應的bool vector finish做紀錄.  
  如果都無法找到則會處於deadlock狀態, 代表為unsafe, 直接結束程式, 不繼續實作resource request.  
 
- Resource-Request Algorithm:  
  這部分分成兩種可能 1. 型態為 a type  2. 型態為 r type  
  1. a type 進行resource request algorithm, 如果為granted, 則對allocation, available, need vector進行調整,  
     如果不為granted, 則檢查request是否有大於該thread or process的need vector. 如果大於, 則予以捨棄.  
     如果request小於need, 但大於目前的available, 則必須進入FCFS的queue等待, 直到有thread or process release資源, 並且作後續檢查.  
  2. r type, 直接檢查要release的資源是否有大於allocation vector, 如果有, 則直接捨棄. 如果沒有, 則對allocation, available, need進行調整  
 
- 最後, 如果最後一個request被執行完後, 仍然有request在waiting queue裡, 一個一個確認是否可以被granted,  
  如果被granted, 則依照resource-request algorithm執行, 否則, 直接將該request abort掉 
