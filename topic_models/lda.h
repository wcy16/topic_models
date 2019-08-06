#pragma once

#include <vector>
#include <random>
#include <map>
#include <chrono>
#include <iostream>
#include <time.h>

/*
* parameters:
* nk[topic]  topic word count
* nkt[topic][word] word's topic count
* nm[doc]  doc word count
* nmk[doc][topic]  doc's topic word count
*/

template<bool lda_debug = false>
void lda(int topics, int iterations,
    double alpha, double beta, int word_count,
    std::map<int, std::vector<int>> docs,
    std::map<int, std::vector<int>>& nmk,
    std::vector<std::vector<int>>& nkt
)
{
    // lda param init
    auto nm = std::map<int, int>();
    nmk = std::map<int, std::vector<int>>();
    auto nk = std::vector<int>(topics, 0);
    nkt = std::vector<std::vector<int>>(topics, std::vector<int>(word_count, 0));

    auto zmn = std::map<int, std::vector<int>>();  // topic for every word
    for (auto it = docs.begin(); it != docs.end(); it++) {
        nm[it->first] = 0;
        nmk[it->first] = std::vector<int>(topics, 0);
        zmn[it->first] = std::vector<int>((it->second).size(), 0);
    }


    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    //std::default_random_engine generator;

    // init
    std::uniform_int_distribution<int> distribution(0, topics - 1);
    for (auto it = docs.begin(); it != docs.end(); it++) {
        int cter = 0;
        for (auto itw = it->second.begin(); itw != it->second.end(); itw++) {
            int z = distribution(generator);
            zmn[it->first][cter++] = z;
            nm[it->first] += 1;
            nmk[it->first][z] += 1;
            nk[z] += 1;
            nkt[z][*itw] += 1;
        }
    }

    clock_t st, ed;
    int counter, part;
    if constexpr(lda_debug) {
        std::cout << "Initialized" << endl;
        counter = 0;
        part = docs.size() / 20;
    }

    // iterate
    for (int i = 0; i != iterations; i++) {
        if constexpr(lda_debug) {
            std::cout << i << " iteration(s)" << endl;
            st = clock();
        }

        for (auto it = docs.begin(); it != docs.end(); it++) {
            if constexpr(lda_debug) {
                if (counter++ == part) {
                    cout << " *";
                    counter = 0;
                }
            }


            auto p = std::vector<double>(topics, 0.0);
            int cter = 0;
            for (auto itw = it->second.begin(); itw != it->second.end(); itw++) {
                int z = zmn[it->first][cter];
                //zmn[it->first][*itw] = z;
                nm[it->first] -= 1;
                nmk[it->first][z] -= 1;
                nk[z] -= 1;
                nkt[z][*itw] -= 1;

                // p = (nkt + beta) * (nmk + alpha) / (nk + beta)
                //double tmp = (nk[*itw] + beta);
                //double tmp1 = (beta + nkt[*itw][z]) / (nk[*itw] + beta);
                //double tmp2 = tmp1 * nmk[it->first][z];

                for (int j = 0; j != topics; j++) {
                    p[j] = (beta + nkt[j][*itw]) * (nmk[it->first][j] + alpha) / (nk[j] + word_count * beta);
                }

                std::discrete_distribution<int> discrete(p.begin(), p.end());
                z = discrete(generator);

                zmn[it->first][cter++] = z;
                nm[it->first] += 1;
                nmk[it->first][z] += 1;
                nk[z] += 1;
                nkt[z][*itw] += 1;
            }
        }

        if constexpr(lda_debug) {
            ed = clock();
            cout << endl << "Time costs: " << (double)(ed - st) / CLOCKS_PER_SEC << "s" << endl;
        }
    }

    if constexpr(lda_debug) {
        std::cout << "Train complete" << endl;
    }
}

