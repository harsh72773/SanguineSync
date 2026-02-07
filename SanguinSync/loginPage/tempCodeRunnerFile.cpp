#include "common.h"
#include <iostream>
#include <limits>

void runIndex();

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    runIndex();
    return 0;
}

void runIndex() {
    std::cout << "========================================" << std::endl;
    std::cout << "        WELCOME TO SANGUINE SYNC        " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Please Log In or Create an Account to Continue" << std::endl;
    std::cout << "1. New User Account" << std::endl;
    std::cout << "2. New Hospital Account" << std::endl;
    std::cout << "3. Log In" << std::endl;
    std::cout << std::endl;
    std::cout << "Select an option (1-3): ";

    int choice = 0;
    std::cin >> choice;
    std::cout << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    switch (choice) {
        case 1:
            runRegisterUser();
            break;
        case 2:
            runRegisterHospital();
            break;
        case 3:
            std::cout << "========================================" << std::endl;
            runLogInPage();
            break;
        default:
            std::cout << "Invalid choice. Please select 1 or 2." << std::endl;
            std::cout << "Try again." << std::endl;
            runIndex();
            break;
    }
}
