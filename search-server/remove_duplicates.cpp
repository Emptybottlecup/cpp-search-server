#include "remove_duplicates.h"
#include <vector>

void RemoveDuplicates(SearchServer& search_server){
 std::vector<std::pair<int, std::vector<string>>> id;
   for(auto ids : search_server){
       vector<string> words;
       for(auto word : search_server.GetWordFrequencies(ids)) {
           words.push_back(word.first);
       }
       id.push_back({ids,words});
   }
   
   for(int i = 0; i < id.size(); ++i) {
       for(auto i_2 = i+1; i_2 < id.size(); ++i_2) {
           if(id[i_2].first > id[i].first && id[i].second == id[i_2].second) {
               cout << "Found duplicate document id " << id[i_2].first << endl;
                   search_server.RemoveDocument(id[i_2].first);
           }
       }
   }

}