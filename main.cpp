#include <iostream>
#include <datapar.hpp>
#include <bench.hpp>

std::vector<int> *generateNumbers(int num) {
    auto res = new std::vector<int>();
    for (int i = 0; i < num; i++) {
        res->emplace_back(rand() % 10000);
    }
    return res;
}

int main(int argc, char** argv) {
    printf("Start sum measuring\n");

    std::vector<int> *aa = generateNumbers(1000000000);

    pbbs::launch(argc, argv, [&] (pbbs::measured_type measured) {
        auto start = std::chrono::system_clock::now();
        pasl::pctl::reduce(aa->begin(), aa->end(), 0, [&] (int x, int y) {
            return x + y;
        });
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<float> diff = end - start;
        printf ("exectime: %.3lf\n", diff.count());
    });

    return 0;
}
