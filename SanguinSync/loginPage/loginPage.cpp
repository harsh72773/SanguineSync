#include <iostream>
#include <string>
using namespace std;

// User
class User {
public:
    string username;
    string password;

    User(string u, string p) {
        username = u;
        password = p;
    }
};

// UserSignIn
class UserSignIn {
public:
    static void userLogin(User users[], int size) {
        cout << "=======================================" << endl;
        cout << "     WELCOME TO USER SIGN-IN SYSTEM    " << endl;
        cout << "=======================================" << endl;
        cout << endl;

        string inputUsername, inputPassword;

        cout << "Enter Username: ";
        getline(cin, inputUsername);

        cout << "Enter Password: ";
        getline(cin, inputPassword);

        bool usernameFound = false;
        bool passwordCorrect = false;

        for (int i = 0; i < size; i++) {
            if (users[i].username == inputUsername) {
                usernameFound = true;
                if (users[i].password == inputPassword) {
                    passwordCorrect = true;
                }
                break;
            }
        }

        if (!usernameFound) {
            cout << endl;
            cout << "[ERROR] Username not found!" << endl;
            cout << "Please check your username and try again." << endl;
        }
        else if (!passwordCorrect) {
            cout << endl;
            cout << "[ERROR] Incorrect password!" << endl;
            cout << "Please try again." << endl;
        }
        else {
            cout << endl;
            cout << "=======================================" << endl;
            cout << "           LOGIN SUCCESSFUL!           " << endl;
            cout << "=======================================" << endl;
            cout << "Welcome, " << inputUsername << "!" << endl;
            cout << "You have successfully signed in." << endl;
            cout << "=======================================" << endl;
            cout << endl;
        }
    }
};

int main() {
    int choice;

    User users[4] = {
        User("User1", "User1@123"),
        User("User2", "User2@123"),
        User("User3", "User3@123"),
        User("User4", "User4@123")
    };

    cout << "=======================================" << endl;
    cout << "             SANGUINE SYNC             " << endl;
    cout << "=======================================" << endl;
    cout << endl;
    cout << "       Who is signing in?       " << endl;
    cout << "1. User" << endl;
    cout << "2. Hospital" << endl;
    cout << endl;
    cin >> choice;
    cin.ignore(); // Buffer
    
    switch (choice) {
        case 1:
            UserSignIn::userLogin(users, 4);
            break;

        case 2:
            cout << "Hospital login selected." << endl;
            // hospitalLogin();
            break;

        default:
            cout << "Invalid choice. Please select 1 or 2." << endl;
            break;
    }

    return 0;
}