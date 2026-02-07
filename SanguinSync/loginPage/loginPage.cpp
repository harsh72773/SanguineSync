#include "common.h"
#include <iostream>
#include <limits>

using namespace std;

void runLogInPage() {
    cout << endl;
    cout << "       Who is loging in?       " << endl;
    cout << "1. User" << endl;
    cout << "2. Hospital" << endl;
    cout << "3. Go Back" << endl;
    cout << endl;
    cout << "Select an option (1-3): ";

    int choice = 0;
    cin >> choice;
    cout << endl;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    switch (choice) {
        case 1:
            runUserLogIn();
            break;
        case 2:
            runHospLogIn();
            break;
        case 3:
            runIndex();
            break;
        default:
            cout << "Invalid choice. Please select 1, 2, or 3." << endl;
            cout << "Try again." << endl;
            runLogInPage();
            break;
    }
}
