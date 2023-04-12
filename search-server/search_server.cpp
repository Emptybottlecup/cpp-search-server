#include "search_server.h"
#include <numeric>

void SearchServer::AddDocument(int document_id, string_view document, DocumentStatus status, const vector<int>& ratings) {
       if ((document_id < 0) || (documents_.count(document_id) > 0)) {
            throw invalid_argument("Invalid document_id"s);
        }
        const auto words = SplitIntoWordsNoStop(document);
        set<string_view> words_;
        const double inv_word_count = 1.0 / words.size();
        for (auto word : words) {
            all_words.push_back(string(word));
            word_to_document_freqs_[string_view(all_words.back())][document_id] += inv_word_count;
            document_id_word_freqs_[document_id][string_view(all_words.back())] += inv_word_count;
            words_.emplace(string_view(all_words.back()));
        }
        
    (words_ids[words_]).insert(document_id);
        
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
        document_ids_.insert(document_id);
}
 
vector<Document> SearchServer::FindTopDocuments(string_view raw_query, DocumentStatus status) const {
        return FindTopDocuments(execution::seq,raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}
 
vector<Document> SearchServer::FindTopDocuments(string_view raw_query) const {
        return FindTopDocuments(execution::seq,raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
        return documents_.size();
}
 
const std::set<int>::iterator SearchServer::begin() const {
        return document_ids_.begin();
}
 
const std::set<int>::iterator SearchServer::end() const {
        return document_ids_.end();
}
 
const map<set<string_view> , set<int>>& SearchServer::GetWordsIds(){
    return words_ids;
}
 
const std::map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const{
    static map<string_view, double> empty_dictionary{};
    if (document_id_word_freqs_.count(document_id) != 0){
    return document_id_word_freqs_.at(document_id);}
 
     return empty_dictionary;
}
 
void SearchServer::RemoveDocument(int document_id) {
 
    for(auto word : GetWordFrequencies(document_id)) {
        word_to_document_freqs_.at(word.first).erase(document_id);
        if(word_to_document_freqs_.at(word.first).empty()) {
            word_to_document_freqs_.erase(word.first);
        }
    }
    document_ids_.erase(document_id);
    document_id_word_freqs_.erase(document_id);
    documents_.erase(document_id);
}
 
void SearchServer::RemoveDocument(const std::execution::sequenced_policy&, int document_id) {
        return RemoveDocument(document_id);
    }
 
void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id) {
 
    vector<string_view> to_delete(document_id_word_freqs_.at(document_id).size());
    transform(execution::par,document_id_word_freqs_.at(document_id).begin(),document_id_word_freqs_.at(document_id).end(),to_delete.begin() , [](auto word_freq) {
        return word_freq.first;
    });
 
 for_each(execution::par, to_delete.begin(),to_delete.end(), [this,document_id](auto word) {
      word_to_document_freqs_.at(word).erase(document_id);
 });
 
    document_ids_.erase(document_id);
    document_id_word_freqs_.erase(document_id);
    documents_.erase(document_id);
}
 
tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(string_view raw_query, int document_id) const {
    bool flag = true;
        const auto query = ParseQuery(raw_query, flag);
 
        vector<string_view> matched_words;
    for (auto word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                return { matched_words, documents_.at(document_id).status };
            }
        }
        for (auto word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        return { matched_words, documents_.at(document_id).status };
}
 
tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, string_view raw_query, int document_id) const {
        return SearchServer::MatchDocument(raw_query,document_id);
}
 
tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&, string_view raw_query, int document_id) const {
      bool flag = false;
        const auto query = ParseQuery(raw_query,flag);
        vector<string_view> matched_words(query.plus_words.size());
 
    bool answer = any_of(execution::par, query.minus_words.begin(),query.minus_words.end(),[this,document_id](string_view minus_word) {
    return word_to_document_freqs_.at(minus_word).count(document_id);   
    });
 
    if(answer) {
        matched_words.clear();
        return { matched_words, documents_.at(document_id).status };
    }
 
        auto last = copy_if(execution::par, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(), [this, document_id, &matched_words]( string_view word) {
            if (word_to_document_freqs_.count(word) == 0) {
               return false;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                return true;
            }
            return false;
            });
 
    sort(matched_words.begin(),last);
    auto del = unique(matched_words.begin(),last);
    matched_words.erase(del,matched_words.end());
    return { matched_words, documents_.at(document_id).status };

}
 
bool SearchServer::IsStopWord(string_view word) const {
        return stop_words_.count(word) > 0;
}
 
bool SearchServer::IsValidWord(string_view word) {
        // A valid word must not contain special characters
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
}
 
vector<string_view> SearchServer::SplitIntoWordsNoStop(string_view text) const {
        vector<string_view> words;
        for (auto word : SplitIntoWords(text)) {
            if (!IsValidWord(word)) {
                throw invalid_argument("Word "s + std::string(word) + " is invalid");
            }
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
}
 
int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
 
        return rating_sum / static_cast<int>(ratings.size());
}
 
SearchServer::QueryWord SearchServer::ParseQueryWord(string_view text) const {
        if (text.empty()) {
            throw invalid_argument("Query word "s + std::string(text) + " empty"s);
        }
        string_view word = text;
        bool is_minus = false;
        if (word[0] == '-') {
            is_minus = true;
            word = word.substr(1);
        }
        if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
            throw invalid_argument("Query word is invalid");
        }
 
        return { word, is_minus, IsStopWord(word) };
}
 
SearchServer::Query SearchServer::ParseQuery(string_view text, const bool& flag) const {
        Query result;
        for (auto word : SplitIntoWords(text)) {
            const auto query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    result.minus_words.push_back(query_word.data);
                }
                else {
                    result.plus_words.push_back(query_word.data);
                }
            }
        }
 
        if(flag) {
            sort(result.minus_words.begin(), result.minus_words.end());
            sort(result.plus_words.begin(),result.plus_words.end());
            auto last_1 = std::unique(result.minus_words.begin(), result.minus_words.end());
            auto last_2 = std::unique(result.plus_words.begin(), result.plus_words.end());
            result.minus_words.erase(last_1,result.minus_words.end());
            result.plus_words.erase(last_2,result.plus_words.end());
    }
 
    return result;
}
 
double SearchServer::ComputeWordInverseDocumentFreq(string_view word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}
 
void PrintDocument(const Document& document) {
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s << endl;
}
 
void PrintMatchDocumentResult(int document_id, vector<string_view> words, DocumentStatus status) {
    cout << "{ "s
        << "document_id = "s << document_id << ", "s
        << "status = "s << static_cast<int>(status) << ", "s
        << "words ="s;
    for (auto word : words) {
        cout << ' ' << word;
    }
    cout << "}"s << endl;
}
 
void AddDocument(SearchServer& search_server, int document_id, string_view document, DocumentStatus status,
    const vector<int>& ratings) {
    try {
        search_server.AddDocument(document_id, document, status, ratings);
    }
    catch (const invalid_argument& e) {
        cout << "Ошибка добавления документа "s << document_id << ": "s << e.what() << endl;
    }
}
 
void FindTopDocuments(const SearchServer& search_server, string_view raw_query) {
    cout << "Результаты поиска по запросу: "s << raw_query << endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    }
    catch (const invalid_argument& e) {
        cout << "Ошибка поиска: "s << e.what() << endl;
    }
}
 
void MatchDocuments(const SearchServer& search_server, string_view query) {
    try {
        cout << "Матчинг документов по запросу: "s << query << endl;
        for (auto i : search_server) {
            const auto [words, status] = search_server.MatchDocument(query, i);
            PrintMatchDocumentResult(i, words, status);
        }
    }
    catch (const invalid_argument& e) {
        cout << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << endl;
    }
}