#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server){
   for(auto g : search_server.GetWordsIds()){
       if((g.second).size() > 1){
           bool flag = true;
           for(auto k : g.second){
               if(flag == false){
                   cout << "Found duplicate document id " << k << endl;
                   search_server.RemoveDocument(k);
               }
               flag = false;
           }
       }
   }

}