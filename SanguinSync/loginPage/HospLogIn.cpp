#include "common.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct Hospital {
    string hospitalId;
    string hospitalName;
    string password;
};

static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static vector<Hospital> loadHospitalsFromFile(const string& filename) {
    vector<Hospital> hospitalList;
    ifstream file(filename);
    if (!file) {
        cout << "[ERROR] File '" << filename << "' not found!" << endl;
        return hospitalList;
    }
    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;
        size_t p1 = line.find(',');
        size_t p2 = line.find(',', p1 + 1);
        if (p1 == string::npos || p2 == string::npos) continue;
        Hospital h;
        h.hospitalId = trim(line.substr(0, p1));
        h.hospitalName = trim(line.substr(p1 + 1, p2 - p1 - 1));
        h.password = trim(line.substr(p2 + 1));
        hospitalList.push_back(h);
    }
    return hospitalList;
}

void runHospLogIn() {
    vector<Hospital> hospitals = loadHospitalsFromFile("hospitals.txt");

    if (hospitals.empty()) {
        cout << "[ERROR] No hospitals found in file or file error!" << endl;
        return;
    }

    cout << "=========================================" << endl;
    cout << endl;

    cout << "Enter Hospital ID: ";
    string inputHospitalId;
    getline(cin, inputHospitalId);
    inputHospitalId = trim(inputHospitalId);

    cout << "Enter Password: ";
    string inputPassword;
    getline(cin, inputPassword);
    inputPassword = trim(inputPassword);

    bool hospitalFound = false;
    bool passwordCorrect = false;
    size_t foundIndex = 0;

    for (size_t i = 0; i < hospitals.size(); i++) {
        if (hospitals[i].hospitalId == inputHospitalId) {
            hospitalFound = true;
            foundIndex = i;
            if (hospitals[i].password == inputPassword) {
                passwordCorrect = true;
            }
            break;
        }
    }

    if (!hospitalFound) {
        cout << endl;
        cout << "[ERROR] Hospital ID not found!" << endl;
        cout << "Please check your Hospital ID and try again." << endl;
        runHospLogIn();
    } else if (!passwordCorrect) {
        cout << endl;
        cout << "[ERROR] Incorrect password!" << endl;
        cout << "Please try again." << endl;
        runHospLogIn();
    } else {
        cout << endl;
        cout << "=========================================" << endl;
        cout << "         HOSPITAL LOGIN SUCCESSFUL!     " << endl;
        cout << "=========================================" << endl;
        cout << "Welcome, " << hospitals[foundIndex].hospitalName << "!" << endl;
        cout << "You have successfully signed in." << endl;
        cout << "=========================================" << endl;
        cout << endl;
        runHospMenu();
    }
}
