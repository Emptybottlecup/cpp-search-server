#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>
using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double tf = 1.0 / words.size();
        for (string s : words) {
            word_to_document_freqs_[s][document_id] += tf;
        }
        ++document_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const  MinusPlus query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);
        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
private:
    double IDF(const string& word) const {
        return log((document_count_) / (word_to_document_freqs_.at(word).size() * 1.0));
    }

    int document_count_ = 0;
    struct MinusPlus {
        set<string> plus;
        set<string> minus;
    };
    MinusPlus ParseQuery(const string& text) const {
        MinusPlus query_words;
        for (string s : SplitIntoWords(text)) {
            if (s[0] == '-') {
                query_words.minus.insert(s.substr(1));
            }
        };
        for (string s : SplitIntoWordsNoStop(text)) {
            if (query_words.minus.count(s) == 0) {
                query_words.plus.insert(s);
            }
        }
        return query_words;
    }
    map<string, map<int, double>> word_to_document_freqs_;
    set<string> stop_words_;
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }
    vector<Document> FindAllDocuments(const MinusPlus& query_words) const {
        vector<Document> matched_documents;
        map<int, double>  document_to_relevance;
        for (string s : query_words.plus) {
            if (word_to_document_freqs_.count(s)) {
                double m = IDF(s);
                for (const auto [id, tf] : word_to_document_freqs_.at(s))
                    document_to_relevance[id] += tf * m;
            };

        };
        for (string s : query_words.minus) {
            if (word_to_document_freqs_.count(s)) {
                for (const auto i : word_to_document_freqs_.at(s)) {
                    document_to_relevance.erase(i.first);
                }
            }
        };
        for (pair<int, double> s : document_to_relevance) {
            matched_documents.push_back({ s.first,s.second });
        };
        return matched_documents;
    }
};
SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const Document i : search_server.FindTopDocuments(query)) {
        cout << "{ document_id ="s << ' ' << i.id << ", "s
            << "relevance = "s << i.relevance << " }"s << endl;
    }
}