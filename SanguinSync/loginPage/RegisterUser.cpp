#include "common.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <cctype>

using namespace std;

static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static bool isAadharValid(const string& aadharNo) {
    if (aadharNo.length() != 12) return false;
    for (char c : aadharNo)
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    return true;
}

static bool isAadharExists(const string& aadharNo) {
    ifstream file("users.txt");
    if (!file) return false;
    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;
        size_t p1 = line.find(',');
        if (p1 == string::npos) continue;
        size_t p2 = line.find(',', p1 + 1);
        if (p2 == string::npos) continue;
        string aadhar = trim(line.substr(p1 + 1, p2 - p1 - 1));
        if (aadhar == aadharNo) {
            file.close();
            return true;
        }
    }
    return false;
}

void runRegisterUser() {
    cout << "=========================================" << endl;
    cout << "        REGISTER NEW USER ACCOUNT        " << endl;
    cout << "=========================================" << endl;
    cout << endl;

    while (true) {
        cout << "Enter username: ";
        string username;
        getline(cin, username);
        username = trim(username);

        cout << "Enter Aadhar No (exactly 12 digits, no spaces): ";
        string aadharNo;
        getline(cin, aadharNo);
        aadharNo = trim(aadharNo);

        if (!isAadharValid(aadharNo)) {
            cout << endl;
            cout << "[ERROR] Invalid Aadhar number!" << endl;
            cout << "- Must be EXACTLY 12 digits" << endl;
            cout << "- Only numbers (0-9), no spaces or letters" << endl;
            cout << "- Examples: 123456789012 ✓ | 123 456 789 012 ✗" << endl;
            cout << endl;
            continue;
        }

        cout << "Enter password: ";
        string password;
        getline(cin, password);
        password = trim(password);

        if (isAadharExists(aadharNo)) {
            cout << endl;
            cout << "[ERROR] Aadhar number already exists!" << endl;
            cout << "This Aadhar is already registered." << endl;
            cout << endl;
            cout << "1. Try Again" << endl;
            cout << "2. Log In" << endl;
            int choice = 0;
            cin >> choice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            switch (choice) {
                case 1:
                    continue;
                case 2:
                    runUserLogIn();
                    return;
                default:
                    cout << "Invalid choice. Returning to registration." << endl;
                    continue;
            }
        }

        ofstream fw("users.txt", ios::app);
        if (fw) {
            fw << username << "," << aadharNo << "," << password << "\n";
            fw.close();
            cout << "User saved successfully." << endl;
            cout << "Please log in with your new credentials." << endl;
            runUserLogIn();
            break;
        } else {
            cout << "[Error] Data could not be saved." << endl;
        }
    }
}
