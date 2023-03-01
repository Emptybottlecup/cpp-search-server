#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server){
   for(auto ids : search_server.GetWordsIds()){
       if((ids.second).size() > 1){
           bool flag = true;
           for(auto id : ids.second){
               if(flag == false){
                   cout << "Found duplicate document id " << id << endl;
                   search_server.RemoveDocument(id);
               }
               flag = false;
           }
       }
   }

}