#include "common.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct User {
    string username;
    string aadharNo;
    string password;
};

static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static vector<User> loadUsersFromFile(const string& filename) {
    vector<User> userList;
    ifstream file(filename);
    if (!file) {
        cout << "[ERROR] File '" << filename << "' not found!" << endl;
        return userList;
    }
    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;
        size_t p1 = line.find(',');
        size_t p2 = line.find(',', p1 + 1);
        if (p1 == string::npos || p2 == string::npos) continue;
        User u;
        u.username = trim(line.substr(0, p1));
        u.aadharNo = trim(line.substr(p1 + 1, p2 - p1 - 1));
        u.password = trim(line.substr(p2 + 1));
        userList.push_back(u);
    }
    return userList;
}

void runUserLogIn() {
    vector<User> users = loadUsersFromFile("users.txt");

    if (users.empty()) {
        cout << "[ERROR] No users found in file or file error!" << endl;
        cout << "Please register first." << endl;
        runRegisterUser();
        return;
    }

    cout << "=========================================" << endl;
    cout << "            ENTER CREDENTIALS            " << endl;
    cout << "=========================================" << endl;
    cout << endl;

    cout << "Enter Username: ";
    string inputUsername;
    getline(cin, inputUsername);
    inputUsername = trim(inputUsername);

    cout << "Enter Aadhar No (12 digits): ";
    string inputAadhar;
    getline(cin, inputAadhar);
    inputAadhar = trim(inputAadhar);

    cout << "Enter Password: ";
    string inputPassword;
    getline(cin, inputPassword);
    inputPassword = trim(inputPassword);

    bool userFound = false;
    for (size_t i = 0; i < users.size(); i++) {
        if (users[i].username == inputUsername &&
            users[i].aadharNo == inputAadhar &&
            users[i].password == inputPassword) {
            userFound = true;
            break;
        }
    }

    if (!userFound) {
        cout << endl;
        cout << "[ERROR] Invalid credentials!" << endl;
        cout << "Username, Aadhar, AND Password must all match." << endl;
        cout << "Please check and try again." << endl;
        cout << "OR register first." << endl;
        runUserLogIn();
    } else {
        string maskedAadhar = "********" + (inputAadhar.length() >= 4
            ? inputAadhar.substr(inputAadhar.length() - 4) : inputAadhar);

        cout << endl;
        cout << "=========================================" << endl;
        cout << "           LOGIN SUCCESSFUL!            " << endl;
        cout << "=========================================" << endl;
        cout << "Welcome, " << inputUsername << "!" << endl;
        cout << "Aadhar: " << maskedAadhar << endl;
        cout << "You have successfully signed in." << endl;
        cout << "=========================================" << endl;
        cout << endl;
        runUserMenu();
    }
}
