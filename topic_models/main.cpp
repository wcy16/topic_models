#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <random>
#include <array>
#include <chrono>

#include <time.h>
#include <tuple>

#include "btm.h"

using namespace std;

int main()
{
    const int term_cnt = 698938;

    ifstream in_docs;

    // load journal subd
    in_docs.open("E:\\Projects\\Python\\findexperts\\search\\lda\\btm_doc.csv");

    std::map<int, std::vector<int>> docs;

    // load publications
    int docid;
    int wid;  // word id

    int sum = 0;    // 2142780007

    while (!in_docs.eof()) {
        in_docs >> docid;

        auto v = vector<int>();
        while (in_docs.get() != '\n' && !in_docs.eof()) {
            in_docs >> wid;
            v.push_back(wid);
        }
        docs[docid] = v;
        {
            int tmp = v.size();
            sum += tmp * (tmp - 1) / 2;
        }
    }
    in_docs.close();
    cout << "docs loaded" << endl;
    cout << "biterms count: " << sum << endl;
    system("pause");
    return 0;

    int topics = 10;
    std::vector<int> nz;
    std::vector<std::vector<int>> nwz;
    std::vector<std::pair<int, int>> biterms;
    double alpha = 50.0 / topics;
    double beta = 0.01;
    int iterations = 1000;
    btm<true>(topics, iterations, alpha, beta, term_cnt, docs, nz, nwz, biterms, sum);

    //
    cout << "biterm counts: " << biterms.size() << endl;

    // output to a file
    ofstream terms;
    terms.open("nwz.csv");
    for (int i = 0; i != topics; i++) {
        for (int j = 0; j < term_cnt - 1; j++) {
            terms << nwz[i][j] << ",";
        }
        terms << nwz[i][term_cnt - 1] << endl;
    }
    terms.close();
    ofstream ofnz;
    ofnz.open("nz.csv");

    for (auto it = nz.begin(); it != nz.end(); it++) {
        ofnz << (*it) << ",";
    }
    ofnz.close();
    cout << "dumped into file" << endl;

    system("pause");
}