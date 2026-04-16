#include "staff_control.hpp"


int main() {
    try {
        StaffControl().run();
    } catch (std::exception &ex) {
        std::cerr << "Ошибка во время выполнения: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
