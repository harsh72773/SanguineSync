#include "common.h"
#include <iostream>
#include <limits>

using namespace std;

void runIndex();

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    runIndex();
    return 0;
}

void runIndex() {
    cout << "========================================" << endl;
    cout << "        WELCOME TO SANGUINE SYNC        " << endl;
    cout << "========================================" << endl;
    cout << endl;
    cout << "Please Log In or Create an Account to Continue" << endl;
    cout << "1. New User Account" << endl;
    cout << "2. New Hospital Account" << endl;
    cout << "3. Log In" << endl;
    cout << endl;
    cout << "Select an option (1-3): ";

    int choice = 0;
    cin >> choice;
    cout << endl;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    switch (choice) {
        case 1:
            runRegisterUser();
            break;
        case 2:
            runRegisterHospital();
            break;
        case 3:
            cout << "========================================" << endl;
            runLogInPage();
            break;
        default:
            cout << "Invalid choice. Please select 1, 2, or 3." << endl;
            cout << "Try again." << endl;
            runIndex();
            break;
    }
}
