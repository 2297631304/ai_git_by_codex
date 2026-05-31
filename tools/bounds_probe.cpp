#include "esl/checked_access.hpp"

#include <exception>
#include <iostream>
#include <vector>

int main() {
    std::vector<int> values{1, 2, 3};
    try {
        (void)esl::checked_at(values, 5, "probe_values");
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 0;
    }
    std::cerr << "bounds probe failed to detect out-of-range access\n";
    return 1;
}
