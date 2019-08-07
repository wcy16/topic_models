/*
* Biterm Topic Model for short text.
* The context will be the whole text.
*/

#pragma once

#include <vector>
#include <random>
#include <map>
#include <chrono>
#include <iostream>
#include <time.h>
#include <array>

/*
* parameters:
* nz[topic] topic's biterm count
* nwz[topic][word] word's topic count
* biterms[] biterms generated from documents
*/
template<bool debug = false>
void btm(int topics, int iterations,
    double alpha, double beta, int word_count,
    const std::map<int, std::vector<int>>& docs,
    std::vector<int>& nz,
    std::vector<std::vector<int>>& nwz,
    std::map<long long int, int>& biterms)
{
    // store biterms into map to reduce memory usage
    // the key is [wi, wj], and value is count
    // wi <= wj
    biterms = std::map<long long int, int>();

    // extract biterms for every document
    int biterms_count = 0;
    for (auto it = docs.begin(); it != docs.end(); it++) {
        int doc_len = it->second.size();
        if (doc_len < 2) continue;

        for (auto i = 0; i != doc_len - 1; i++) {
            for (int j = i + 1; j != doc_len; j++) {
                int wi = it->second.at(i);
                int wj = it->second.at(j);
                if (wi > wj) std::swap(wi, wj);

                long long int w = wi;
                w = w << 32 | wj;
                ++biterms[w];

                ++biterms_count;
            }
        }
    }

    if constexpr(debug) {
        std::cout << "data loaded" << std::endl;
        std::cout << "biterms count: " << biterms_count << std::endl;
    }

    // param init
    nz = std::vector<int>(topics, 0);
    nwz = std::vector<std::vector<int>>(topics, std::vector<int>(word_count, 0));
    auto nbz = std::vector<int>(biterms_count, 0);

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);

    // init
    std::uniform_int_distribution<int> distribution(0, topics - 1);
    {
        int tmp_cnt = 0;
        for (auto it = biterms.begin(); it != biterms.end(); it++) {
            long long int w = it->first;
            int wi = w >> 32;
            int wj = w & 0xffff;

            for (int i = 0; i != it->second; i++) {
                int k = distribution(generator);   // topic k

                ++nz[k];
                ++nwz[k][wi];
                ++nwz[k][wj];
                nbz[tmp_cnt++] = k;
            }
        }
    }

    // debug info
    clock_t st, ed;
    int counter, part;
    if constexpr(debug) {
        std::cout << "Initialized" << std::endl;
        counter = 0;
        part = biterms_count / 50;
    }

    // iterate
    auto p = std::vector<double>(topics, 0.0);
    for (int i = 0; i != iterations; i++) {
        if constexpr(debug) {
            std::cout << i << " iteration(s)" << std::endl;
            st = clock();
        }

        int tmp_cnt = 0;
        for (auto it = biterms.begin(); it != biterms.end(); it++) {
            if constexpr(debug) {
                if (tmp_cnt == part) {
                    std::cout << " *";
                    counter = 0;
                }
            }
            
            long long int w = it->first;
            int wi = w >> 32;
            int wj = w & 0xffff;

            for (int i = 0; i != it->second; i++) {
                int k = nbz[tmp_cnt];   // topic k

                --nz[k];
                --nwz[k][wi];
                --nwz[k][wj];
                // multinomial probability
                for (int j = 0; j != topics; j++) {
                    // multipling w to avoid numerical problem according to author's implementation
                    p[j] = (nz[j] + alpha)
                        * (nwz[j][wi] + beta) * (nwz[j][wj] + beta)
                        * word_count / (2 * nz[j] + word_count * beta);
                }

                // sampling
                std::discrete_distribution<int> discrete(p.begin(), p.end());
                k = discrete(generator);
                ++nz[k];
                ++nwz[k][wi];
                ++nwz[k][wj];
                nbz[tmp_cnt] = k;
                ++tmp_cnt;
            }
        }

        if constexpr(debug) {
            ed = clock();
            std::cout << std::endl << "Time costs: " << (double)(ed - st) / CLOCKS_PER_SEC << "s" << std::endl;
        }
    }

    if constexpr(debug) {
        std::cout << "Train complete" << std::endl;
    }

}