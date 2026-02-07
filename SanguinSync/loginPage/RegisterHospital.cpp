#include "common.h"
#include <fstream>
#include <iostream>
#include <string>
#include <limits>

using namespace std;

static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static bool isHospitalExists(const string& hospitalId) {
    ifstream file("hospitals.txt");
    if (!file) return false;
    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;
        size_t p = line.find(',');
        if (p == string::npos) continue;
        string id = trim(line.substr(0, p));
        if (id == hospitalId) {
            file.close();
            return true;
        }
    }
    return false;
}

void runRegisterHospital() {
    cout << "=========================================" << endl;
    cout << "      REGISTER NEW HOSPITAL ACCOUNT      " << endl;
    cout << "=========================================" << endl;
    cout << endl;

    while (true) {
        cout << "Enter hospital name: ";
        string hospitalName;
        getline(cin, hospitalName);
        hospitalName = trim(hospitalName);

        cout << "Enter hospital ID: ";
        string hospitalId;
        getline(cin, hospitalId);
        hospitalId = trim(hospitalId);

        cout << "Enter password: ";
        string password;
        getline(cin, password);
        password = trim(password);

        if (isHospitalExists(hospitalId)) {
            cout << endl;
            cout << "[ERROR] Hospital ID already exists!" << endl;
            cout << "Please choose a different Hospital ID or log in." << endl;
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
                    runHospLogIn();
                    return;
                default:
                    cout << "Invalid choice. Returning to registration." << endl;
                    continue;
            }
        }

        ofstream fw("hospitals.txt", ios::app);
        if (fw) {
            fw << hospitalId << "," << hospitalName << "," << password << "\n";
            fw.close();
            cout << "Hospital saved successfully." << endl;
            cout << "Please log in with your new credentials." << endl;
            runHospLogIn();
            break;
        } else {
            cout << "[Error] Data could not be saved." << endl;
        }
    }
}
