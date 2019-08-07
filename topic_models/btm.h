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
    std::vector<std::pair<int, int>>& biterms)
{
    // todo check efficiency for vector/deque
    biterms = std::vector<std::pair<int, int>>();

    // extract biterms for every document
    for (auto it = docs.begin(); it != docs.end(); it++) {
        int doc_len = it->second.size();
        if (doc_len < 2) continue;

        for (auto i = 0; i != doc_len - 1; i++) {
            for (int j = i + 1; j != doc_len; j++) {
                biterms.push_back(std::pair<int, int>(it->second.at(i), it->second.at(j)));
            }
        }
    }

    if constexpr(debug) {
        cout << "data loaded" << std::endl;
    }

    // param init
    nz = std::vector<int>(topics, 0);
    nwz = std::vector<std::vector<int>>(topics, std::vector<int>(word_count, 0));
    auto nbz = std::vector<int>(biterms.size(), 0);

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);

    // init
    std::uniform_int_distribution<int> distribution(0, topics - 1);
    for (int b = 0; b != biterms.size(); b++) {
        int k = distribution(generator);   // topic k
        ++nz[k];
        ++nwz[k][biterms[b].first];
        ++nwz[k][biterms[b].second];
        nbz[b] = k;
    }

    // debug info
    clock_t st, ed;
    int counter, part;
    if constexpr(debug) {
        std::cout << "Initialized" << endl;
        counter = 0;
        part = biterms.size() / 20;
    }

    // iterate
    auto p = std::vector<double>(topics, 0.0);
    for (int i = 0; i != iterations; i++) {
        if constexpr(debug) {
            std::cout << i << " iteration(s)" << endl;
            st = clock();
        }

        for (int b = 0; b != biterms.size(); b++) {
            if constexpr(debug) {
                if (counter++ == part) {
                    cout << " *";
                    counter = 0;
                }
            }
            
            int k = nbz[b];

            int w1 = biterms[b].first;
            int w2 = biterms[b].second;
            
            --nz[k];
            --nwz[k][w1];
            --nwz[k][w2];

            // multinomial probability
            for (int j = 0; j != topics; j++) {
                // multipling w to avoid numerical problem according to author's implementation
                p[j] = (nz[j] + alpha)
                    * (nwz[j][w1] + beta) * (nwz[j][w2] + beta)
                    * word_count / (2 * nz[j] + word_count * beta);
            }

            // sampling
            std::discrete_distribution<int> discrete(p.begin(), p.end());
            k = discrete(generator);
            ++nz[k];
            ++nwz[k][biterms[b].first];
            ++nwz[k][biterms[b].second];
            nbz[b] = k;
        }

        if constexpr(debug) {
            ed = clock();
            std::cout << std::endl << "Time costs: " << (double)(ed - st) / CLOCKS_PER_SEC << "s" << endl;
        }
    }

    if constexpr(debug) {
        std::cout << "Train complete" << endl;
    }

}