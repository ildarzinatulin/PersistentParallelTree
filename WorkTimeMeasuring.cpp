#include <iostream>
#include "bench.hpp"

#include "Trees/PersistentParallelTreap.h"

std::vector<TreapNode<int, int>> *generateTreapNodes(int num) {
    auto res = new std::vector<TreapNode<int, int>>();
    for (int i = 0; i < num; i++) {
        res->emplace_back(rand() % 100000, rand() % 100000);
    }
    return res;
}

int main(int argc, char** argv) {
    printf("Start treap measuring\n");

    pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
        printf ("In launch\n");

        std::vector<TreapNode<int, int>> *initialValues = generateTreapNodes(1000000);
        std::vector<TreapNode<int, int>> *valuesForAdd = generateTreapNodes(100000);
        auto *tree = new PersistentParallelTreap<int, int>();
        printf ("Start adding init data\n");
        auto trees = (*tree->insertAll(*initialValues));
        printf ("Trees size: %lu\n", trees.size());
        tree = trees[initialValues->size() - 1];
        tree->setMaxDepth(100);
        printf ("Starting...\n");

        auto start = std::chrono::system_clock::now();
        tree->insertAll(*valuesForAdd);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<float> diff = end - start;
        printf ("exectime: %.3lf\n", diff.count());
    });

    return 0;
}
