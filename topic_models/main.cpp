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

#include "lda.h"

using namespace std;

int main()
{
    map<int, vector<int>> docs = {
        { 1,{ 0, 0, 1, 2, 2, 3, 3, 4, 5 } },
    { 2,{ 1, 5, 6, 7, 8, 9, 10, 11, 12 } },
    { 3,{ 8, 13, 14, 15, 16, 17, 18, 19, 20, 21 } },
    { 4,{ 1, 5, 8, 19, 22, 23, 24, 25, 26, 27, 28, 29 } },
    { 5,{ 0, 3, 16, 16, 30, 31 } }
    };
    int word_cnt = 36;
    int topics = 2;
    std::map<int, std::vector<int>> nmk;
    std::vector<std::vector<int>> nkt;
    lda<>(topics, 20, 1.0 / topics, 0.01, word_cnt, docs, nmk, nkt);
    ofstream terms;
    terms.open("terms.csv");
    for (int i = 0; i != nkt.size(); i++) {
        terms << i;
        for (int j = 0; j != nkt[i].size(); j++) {
            terms << "," << nkt[i][j];
        }
        terms << endl;
    }
    terms.close();
    ofstream doctopic;
    doctopic.open("docs.csv");
    for (auto it = nmk.begin(); it != nmk.end(); it++) {
        doctopic << it->first;
        for (auto itm = it->second.begin(); itm != it->second.end(); itm++)
            doctopic << "," << *itm;
        doctopic << endl;
    }
    doctopic.close();
    system("pause");
}