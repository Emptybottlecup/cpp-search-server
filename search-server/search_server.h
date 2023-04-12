#pragma once
#include "document.h"
#include "string_processing.h"
#include <execution>
#include "paginator.h"
#include <execution>
#include <map>
#include <cmath>
#include <deque>
#include "concurrent_map.h"
 
const int MAX_RESULT_DOCUMENT_COUNT = 5;
constexpr auto epsilon = 1e-6;
using namespace std;
 
class SearchServer {
public:
    template <typename StringContainer>
    SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words))  // Extract non-empty stop words
    {
        if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
            throw invalid_argument("Some of stop words are invalid"s);
        }
    }
 
   SearchServer(const string& stop_words_text)
        : SearchServer(SplitIntoWords(string_view(stop_words_text))){}

   SearchServer(string_view stop_words_text)
        : SearchServer(SplitIntoWords(stop_words_text)){}
 
    void AddDocument(int document_id, string_view document, DocumentStatus status, const vector<int>& ratings);
    
    template <typename DocumentPredicate,typename Policy>
vector<Document> FindTopDocuments(const Policy& policy,string_view raw_query, DocumentPredicate document_predicate) const;
    
    template <typename DocumentPredicate>
vector<Document> FindTopDocuments(string_view raw_query, DocumentPredicate document_predicate) const;
    
    vector<Document> FindTopDocuments(string_view raw_query, DocumentStatus status) const;
 
    vector<Document> FindTopDocuments(string_view raw_query) const;
    
    template<typename Policy>
    vector<Document> FindTopDocuments(const Policy& policy,string_view raw_query, DocumentStatus status) const;
    
    template<typename Policy>
    vector<Document> FindTopDocuments(const Policy& policy,string_view raw_query) const;
    
    
 
    int GetDocumentCount() const;
 
    const std::set<int>::iterator begin() const;
 
    const std::set<int>::iterator end() const;
 
    const std::map<string_view, double>& GetWordFrequencies(int document_id) const;
 
    void RemoveDocument(int document_id);
 
    void RemoveDocument(const std::execution::sequenced_policy&, int document_id);
 
    void RemoveDocument(const std::execution::parallel_policy&, int document_id);
    
    tuple<vector<string_view>, DocumentStatus> MatchDocument(string_view raw_query, int document_id) const;
 
    tuple<vector<string_view>, DocumentStatus> MatchDocument(const std::execution::sequenced_policy&, string_view raw_query, int document_id) const; 
 
    tuple<vector<string_view>, DocumentStatus> MatchDocument(const std::execution::parallel_policy&, string_view raw_query, int document_id) const;
    
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    const set<string,less<>> stop_words_;
    map<string_view, map<int, double>> word_to_document_freqs_;
    map<int, map<string_view, double>> document_id_word_freqs_;
    map<int, DocumentData> documents_;
    set<int> document_ids_;
    deque<string> all_words;
    bool IsStopWord(string_view word) const;
 
    static bool IsValidWord(string_view word);
 
    vector<string_view> SplitIntoWordsNoStop(string_view text) const;
 
    static int ComputeAverageRating(const vector<int>& ratings);
 
    struct QueryWord {
        string_view data;
        bool is_minus;
        bool is_stop;
    };
 
    QueryWord ParseQueryWord(string_view text) const;
 
    struct Query {
        vector<string_view> plus_words;
        vector<string_view> minus_words;
    };
 
    Query ParseQuery(string_view text, const bool& flag) const;
 
    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;
    
    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(execution::parallel_policy,const Query& query, DocumentPredicate document_predicate) const;
    
        template <typename DocumentPredicate>
        vector<Document> FindAllDocuments(execution::sequenced_policy,const Query& query, DocumentPredicate document_predicate) const;
 
    // Existence required
    double ComputeWordInverseDocumentFreq(string_view word) const;
 
};
 
void PrintDocument(const Document& document);
 
void PrintMatchDocumentResult(int document_id, vector<string_view> words, DocumentStatus status);
 
void AddDocument(SearchServer& search_server, int document_id, string_view document, DocumentStatus status,
    const vector<int>& ratings);
 
void FindTopDocuments(const SearchServer& search_server, string_view raw_query);
 
 
 template <typename DocumentPredicate,typename Policy>
vector<Document> SearchServer::FindTopDocuments(const Policy& policy,string_view raw_query, DocumentPredicate document_predicate) const {
        const auto query = ParseQuery(raw_query, true);
 
        auto matched_documents = FindAllDocuments(policy,query, document_predicate);
 
        sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
            if (abs(lhs.relevance - rhs.relevance) < epsilon) {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
        });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
 
        return matched_documents;
}

template<typename Policy>
vector<Document> SearchServer::FindTopDocuments(const Policy& policy,string_view raw_query, DocumentStatus status) const {
        return FindTopDocuments(policy,raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

template<typename Policy>
vector<Document> SearchServer::FindTopDocuments(const Policy& policy,string_view raw_query) const {
        return FindTopDocuments(policy,raw_query, DocumentStatus::ACTUAL);
}

 template <typename DocumentPredicate>
vector<Document> SearchServer::FindTopDocuments(string_view raw_query, DocumentPredicate document_predicate) const { 
    return FindTopDocuments(execution::seq, raw_query, document_predicate);
}

template <typename DocumentPredicate>
vector<Document> SearchServer::FindAllDocuments(execution::sequenced_policy,const Query& query, DocumentPredicate document_predicate) const {
        map<int, double> document_to_relevance;
        for (auto word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }
 
        for (auto word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }
 
        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
}

template <typename DocumentPredicate>
vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
return FindAllDocuments(execution::seq,query, document_predicate);
}

template <typename DocumentPredicate>
vector<Document> SearchServer::FindAllDocuments(execution::parallel_policy,const Query& query, DocumentPredicate document_predicate) const {
        ConcurrentMap<int,double> mapa(documents_.size());
        map<int, double> document_to_relevance;
        for_each(execution::par,query.plus_words.begin(),query.plus_words.end(),[&](auto& word) {
            if (word_to_document_freqs_.count(word) != 0) {
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    mapa[document_id].ref_to_value += term_freq * inverse_document_freq;
                }
            }
            }
        });
 
        for_each (execution::par,query.minus_words.begin(),query.minus_words.end(),[&](auto& word) {
            if (word_to_document_freqs_.count(word) != 0) {
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                mapa.erase(document_id);
            }}
        });
        vector<Document> matched_documents;
        document_to_relevance = move(mapa.BuildOrdinaryMap());
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
}