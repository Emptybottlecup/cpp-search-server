
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
            "Stop words must be excluded from documents"s);
    }
}
void TestAddDocument() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server;
        ASSERT_EQUAL(server.GetDocumentCount(), 0);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_EQUAL(server.GetDocumentCount(), 1);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

}

void TestMinusWords() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    const int doc_id_2 = 43;
    const string content_2 = "in the city"s;
    const vector<int> ratings_2 = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        ASSERT_EQUAL(server.FindTopDocuments("in -cat").size(), 1);
    }
}

void TestMatching() {

    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    { SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    auto [words, status] = server.MatchDocument("in the", doc_id);
    ASSERT_EQUAL(words.size(), 2);
    ASSERT(DocumentStatus::ACTUAL == status);
    ASSERT(DocumentStatus::IRRELEVANT != status);
    ASSERT(DocumentStatus::REMOVED != status);
    ASSERT(DocumentStatus::BANNED != status);
    vector<string> raw_query = SplitIntoWords("in the");
    ASSERT(words == raw_query);
    }

    { SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    auto [words, status] = server.MatchDocument("scum", doc_id);
    ASSERT(words.empty());
    }

    { SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    auto [words, status] = server.MatchDocument("in the scum", doc_id);
    vector<string> raw_query = SplitIntoWords("in the scum");
    ASSERT(!(words.empty()));
    ASSERT(raw_query.size() > words.size());
    ASSERT(words.size() == 2);
    ASSERT(words[0] == raw_query[0]);
    ASSERT(words[1] == raw_query[1]);
    }

    { SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    auto [words, status] = server.MatchDocument("in -the ", doc_id);
    ASSERT(words.empty());
    }

    { SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    server.SetStopWords("in");
    auto [words, status] = server.MatchDocument("in the", doc_id);
    ASSERT(words.size() == 1);
    ASSERT(words[0] == "the");
    }

}

void TestSort() {
    const int doc_id = 41;
    const string content = "cat fly on "s;
    const vector<int> ratings = { 1, 2, 3 };

    const int doc_id_2 = 43;
    const string content_2 = " in the city"s;
    const vector<int> ratings_2 = { 1, 1, 1 };

    const int doc_id_3 = 42;
    const string content_3 = "Cj from LOS Santos"s;
    const vector<int> ratings_3 = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id_2, content_2, DocumentStatus::ACTUAL, ratings_2);
        server.AddDocument(doc_id_3, content_3, DocumentStatus::ACTUAL, ratings_3);
        vector<Document> documents = server.FindTopDocuments("Cj in", DocumentStatus::ACTUAL);
        ASSERT(documents.size() >= 2);
        ASSERT(documents[0].relevance > documents[1].relevance);
    }


}

void TestRating() {
    const int doc_id = 41;
    const string content = "cat fly on "s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        vector<Document> documents = server.FindTopDocuments("cat", DocumentStatus::ACTUAL);
        int all_score = 0;
        for (int score : ratings) {
            all_score += score;
        }
        ASSERT_EQUAL(documents.size(), 1);
        ASSERT_EQUAL(all_score / ratings.size(), documents[0].rating);

    }

}

void TestPredicate() {
    SearchServer server;
    server.AddDocument(1, "Amg mercedes 5 "s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(2, "Bmw m5 cs"s, DocumentStatus::ACTUAL, { 0, 0, 0 });
    server.AddDocument(3, "Kebab Turkey"s, DocumentStatus::ACTUAL, { 10, 20, 30 });
    const auto documents = server.FindTopDocuments("Kebab cs 5", [](int document_id, [[maybe_unused]] DocumentStatus status, [[maybe_unused]] int rating) { return document_id % 2 == 0; });
    ASSERT(documents.size() == 1);
    ASSERT(documents[0].id == 2);
}
void TestStatusFilter() {

    SearchServer server;
    server.AddDocument(1, "Amg mercedes 5 "s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(2, "Bmw m5 cs"s, DocumentStatus::BANNED, { 0, 0, 0 });
    server.AddDocument(3, "Kebab Turkey"s, DocumentStatus::REMOVED, { 10, 20, 30 });
    server.AddDocument(4, "Kebab swarma"s, DocumentStatus::REMOVED, { 10, 20, 30 });
    const auto documents = server.FindTopDocuments("Kebab cs 5", DocumentStatus::ACTUAL);
    ASSERT(documents.size() == 1);
    ASSERT(documents[0].id == 1);
    const auto documents_2 = server.FindTopDocuments("Kebab cs 5", DocumentStatus::BANNED);
    ASSERT(documents_2.size() == 1);
    ASSERT(documents_2[0].id == 2);
    const auto  documents_3 = server.FindTopDocuments("Kebab cs 5", DocumentStatus::REMOVED);
    ASSERT(documents_3.size() == 2);
    ASSERT(documents_3[0].id == 3);
    ASSERT(documents_3[1].id == 4);
    const auto  documents_4 = server.FindTopDocuments("Kebab cs 5", DocumentStatus::IRRELEVANT);
    ASSERT(documents_4.empty());
}
void TestRelevanceCalc() {
    SearchServer server;
    server.AddDocument(1, "Amg mercedes 5 "s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    server.AddDocument(2, "Bmw m5 cs 5"s, DocumentStatus::ACTUAL, { 0, 0, 0 });
    server.AddDocument(3, "Kebab Turkey cs 5"s, DocumentStatus::ACTUAL, { 10, 20, 30 });
    vector<string> raw_query = { "Kebab","cs","5" };
    vector<Document> documents = server.FindTopDocuments("Kebab cs 5");
    double IDF_Kebab = log(3.0 / 1.0);
    double IDF_cs = log(3.0 / 2.0);
    double IDF_5 = log(3.0 / 3.0);
    double tf_Kebab_1 = 0.0 / 3.0, tf_cs_1 = 0.0 / 3.0, tf_5_1 = 1.0 / 3.0;
    double tf_Kebab_2 = 0.0 / 4.0, tf_cs_2 = 1.0 / 4.0, tf_5_2 = 1.0 / 4.0;
    double tf_Kebab_3 = 1.0 / 4.0, tf_cs_3 = 1.0 / 4.0, tf_5_3 = 1.0 / 4.0;
    double relevance_1 = IDF_Kebab * tf_Kebab_1 + IDF_cs * tf_cs_1 + IDF_5 * tf_5_1;
    double relevance_2 = IDF_Kebab * tf_Kebab_2 + IDF_cs * tf_cs_2 + IDF_5 * tf_5_2;
    double relevance_3 = IDF_Kebab * tf_Kebab_3 + IDF_cs * tf_cs_3 + IDF_5 * tf_5_3;
    ASSERT(abs(documents[2].relevance - relevance_1) < 1e-6);
    ASSERT(abs(documents[1].relevance - relevance_2) < 1e-6);
    ASSERT(abs(documents[0].relevance - relevance_3) < 1e-6);

}

void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddDocument);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestSort);
    RUN_TEST(TestRating);
    RUN_TEST(TestMatching);
    RUN_TEST(TestPredicate);
    RUN_TEST(TestStatusFilter);
    RUN_TEST(TestRelevanceCalc);
}