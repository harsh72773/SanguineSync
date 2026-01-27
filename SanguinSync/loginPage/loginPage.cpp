#include <iostream>
using namespace std;

int main() {
    char choice;

    cout << "================================" << endl;
    cout << "   Welcome to Sanguine Sync!!   " << endl;
    cout << "================================" << endl;
    cout << endl;
    cout << "       Who is signing in?       " << endl;
    cout << "1. User" << endl;
    cout << "2. Hospital" << endl;
    cout << endl;
    cin >> choice;

    switch (choice) {
        case '1':
            cout << "User login selected." << endl;
            // userLogin();
            break;

        case '2':
            cout << "Hospital login selected." << endl;
            // hospitalLogin();
            break;

        default:
            cout << "Invalid choice. Please select 1 or 2." << endl;
            break;
    }

    return 0;
}