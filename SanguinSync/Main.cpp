#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

using namespace std;

// Helper utilities
static string trim(const string &s)
{
    size_t start = 0;
    while (start < s.size() && isspace((unsigned char)s[start]))
        start++;
    size_t end = s.size();
    while (end > start && isspace((unsigned char)s[end - 1]))
        end--;
    return s.substr(start, end - start);
}

static string toUpper(const string &s)
{
    string out = s;
    transform(out.begin(), out.end(), out.begin(), [](unsigned char c)
              { return toupper(c); });
    return out;
}

static bool regexMatch(const string &value, const string &pattern)
{
    try
    {
        return regex_match(value, regex(pattern));
    }
    catch (const exception &)
    {
        return false;
    }
}

static int readIntLine()
{
    string line;
    if (!getline(cin, line))
        return -1;
    try
    {
        return stoi(trim(line));
    }
    catch (...)
    {
        return -1;
    }
}

// Data class: Represents a registered user (username, Aadhar, password).
struct User
{
    string username;
    string aadharNo;
    string password;
};

// Data class: Represents a registered hospital (ID, name, password).
struct Hospital
{
    string hospitalId;
    string hospitalName;
    string password;
};

// Session holder: Stores the currently logged-in user's username and Aadhar.
struct CurrentUser
{
    static string username;
    static string aadharNo;

    static void set(const string &user, const string &aadhar)
    {
        username = user;
        aadharNo = aadhar;
    }

    static string getUsername()
    {
        return username;
    }

    static string getAadharNo()
    {
        return aadharNo;
    }

    static void clear()
    {
        username.clear();
        aadharNo.clear();
    }
};

string CurrentUser::username = "";
string CurrentUser::aadharNo = "";

// Forward declarations
void indexMain();

void logInPage();

void userLogIn();

void hospLogIn();

void registerUser();

void registerHospital();

void userMenu();

void hospMenu();

void requestBlood();

void viewMyRequests();

void viewPendingRequest();

void viewCompletedRequest();

// User login helpers
static vector<User> loadUsersFromFile(const string &filename)
{
    vector<User> users;
    ifstream in(filename);
    if (!in.is_open())
    {
        cout << "[ERROR] File '" << filename << "' not found!" << endl;
        return users;
    }

    string line;
    while (getline(in, line))
    {
        line = trim(line);
        if (line.empty())
            continue;

        vector<string> parts;
        size_t start = 0;
        for (int i = 0; i < 2; i++)
        {
            size_t pos = line.find(',', start);
            if (pos == string::npos)
                break;
            parts.push_back(trim(line.substr(start, pos - start)));
            start = pos + 1;
        }
        if (start < line.size())
        {
            parts.push_back(trim(line.substr(start)));
        }

        if (parts.size() == 3)
        {
            users.push_back({parts[0], parts[1], parts[2]});
        }
    }
    return users;
}

// Hospital login helpers
static vector<Hospital> loadHospitalsFromFile(const string &filename)
{
    vector<Hospital> hospitals;
    ifstream in(filename);
    if (!in.is_open())
    {
        cout << "[ERROR] File '" << filename << "' not found!" << endl;
        return hospitals;
    }

    string line;
    while (getline(in, line))
    {
        line = trim(line);
        if (line.empty())
            continue;

        vector<string> parts;
        size_t start = 0;
        for (int i = 0; i < 2; i++)
        {
            size_t pos = line.find(',', start);
            if (pos == string::npos)
                break;
            parts.push_back(trim(line.substr(start, pos - start)));
            start = pos + 1;
        }
        if (start < line.size())
        {
            parts.push_back(trim(line.substr(start)));
        }

        if (parts.size() == 3)
        {
            hospitals.push_back({parts[0], parts[1], parts[2]});
        }
    }
    return hospitals;
}

// Blood request record
struct BloodReqRecord
{
    string username;
    string aadharNo;
    string bloodGroup;
    string units;
    string status;

    BloodReqRecord(const string &u, const string &a, const string &b, const string &un, const string &st)
        : username(u), aadharNo(a), bloodGroup(b), units(un), status(st.empty() ? "pending" : trim(st)) {}

    static vector<BloodReqRecord> loadAll(const string &path)
    {
        vector<BloodReqRecord> list;
        ifstream in(path);
        if (!in.is_open())
        {
            return list;
        }

        string username;
        string aadharNo;
        string bloodGroup;
        string units;
        string status = "pending";
        string line;

        auto pushIfValid = [&]()
        {
            if (!username.empty())
            {
                list.emplace_back(username, aadharNo, bloodGroup, units, status);
            }
        };

        while (getline(in, line))
        {
            line = trim(line);
            if (line == "---")
            {
                pushIfValid();
                username.clear();
                aadharNo.clear();
                bloodGroup.clear();
                units.clear();
                status = "pending";
                continue;
            }
            if (line.rfind("Username: ", 0) == 0)
            {
                username = trim(line.substr(strlen("Username: ")));
            }
            else if (line.rfind("Aadhaar No: ", 0) == 0)
            {
                aadharNo = trim(line.substr(strlen("Aadhaar No: ")));
            }
            else if (line.rfind("Blood Group: ", 0) == 0)
            {
                bloodGroup = trim(line.substr(strlen("Blood Group: ")));
            }
            else if (line.rfind("Units Required: ", 0) == 0)
            {
                units = trim(line.substr(strlen("Units Required: ")));
            }
            else if (line.rfind("Status: ", 0) == 0)
            {
                status = trim(line.substr(strlen("Status: ")));
            }
        }
        pushIfValid();
        return list;
    }
};

// -----------------------------------------------------------------------------
// Index (main entry point)
// -----------------------------------------------------------------------------

void indexMain()
{
    cout << "========================================" << endl;
    cout << "        WELCOME TO SANGUINE SYNC        " << endl;
    cout << "========================================" << endl;
    cout << endl;
    cout << "Please Log In or Create an Account to Continue" << endl;
    cout << "1. New User Account" << endl;
    cout << "2. New Hospital Account" << endl;
    cout << "3. Log In" << endl;
    cout << "4. Exit System" << endl;
    cout << endl;
    cout << "Select an option (1-4): ";

    int choice = readIntLine();
    cout << endl;

    switch (choice)
    {
    case 1:
        registerUser();
        break;
    case 2:
        registerHospital();
        break;
    case 3:
        cout << "========================================" << endl;
        logInPage();
        break;
    case 4:
        cout << "Exiting system..." << endl;
        exit(0);
        break;
    default:
        cout << "Invalid choice. Please select 1, 2, or 3." << endl;
        cout << "Try again." << endl;
        indexMain();
        break;
    }
}

// -----------------------------------------------------------------------------
// Login flow
// -----------------------------------------------------------------------------

void logInPage()
{
    cout << endl;
    cout << "       Who is loging in?       " << endl;
    cout << "1. User" << endl;
    cout << "2. Hospital" << endl;
    cout << endl;
    cout << "Select an option (1-2): ";

    int choice = readIntLine();
    cout << endl;

    switch (choice)
    {
    case 1:
        userLogIn();
        break;
    case 2:
        hospLogIn();
        break;
    default:
        cout << "Invalid choice. Please select 1 or 2." << endl;
        cout << "Try again." << endl;
        logInPage();
        break;
    }
}

void userLogIn()
{
    vector<User> users = loadUsersFromFile("users.txt");

    if (users.empty())
    {
        cout << "[ERROR] No users found in file or file error!" << endl;
        cout << "Please register first." << endl;
        registerUser();
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
    for (const auto &u : users)
    {
        if (u.username == inputUsername && u.aadharNo == inputAadhar && u.password == inputPassword)
        {
            userFound = true;
            break;
        }
    }

    if (!userFound)
    {
        cout << endl;
        cout << "[ERROR] Invalid credentials!" << endl;
        cout << "Username, Aadhar, AND Password must all match." << endl;
        cout << "Please check and try again." << endl;
        cout << "OR register first." << endl;
        userLogIn();
    }
    else
    {
        string maskedAadhar = "********";
        if (inputAadhar.size() >= 4)
        {
            maskedAadhar += inputAadhar.substr(inputAadhar.size() - 4);
        }
        else
        {
            maskedAadhar += inputAadhar;
        }

        cout << endl;
        cout << "=========================================" << endl;
        cout << "           LOGIN SUCCESSFUL!            " << endl;
        cout << "=========================================" << endl;
        cout << "Welcome, " << inputUsername << "!" << endl;
        cout << "Aadhar: " << maskedAadhar << endl;
        cout << "You have successfully signed in." << endl;
        cout << endl;
        CurrentUser::set(inputUsername, inputAadhar);
        userMenu();
    }
}

void hospLogIn()
{
    vector<Hospital> hospitals = loadHospitalsFromFile("hospitals.txt");

    if (hospitals.empty())
    {
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
    int foundIndex = -1;

    for (size_t i = 0; i < hospitals.size(); i++)
    {
        if (hospitals[i].hospitalId == inputHospitalId)
        {
            hospitalFound = true;
            foundIndex = static_cast<int>(i);
            if (hospitals[i].password == inputPassword)
            {
                passwordCorrect = true;
            }
            break;
        }
    }

    if (!hospitalFound)
    {
        cout << endl;
        cout << "[ERROR] Hospital ID not found!" << endl;
        cout << "Please check your Hospital ID and try again." << endl;
        hospLogIn();
    }
    else if (!passwordCorrect)
    {
        cout << endl;
        cout << "[ERROR] Incorrect password!" << endl;
        cout << "Please try again." << endl;
        hospLogIn();
    }
    else
    {
        cout << endl;
        cout << "=========================================" << endl;
        cout << "         HOSPITAL LOGIN SUCCESSFUL!     " << endl;
        cout << "=========================================" << endl;
        cout << "Welcome, " << hospitals[foundIndex].hospitalName << "!" << endl;
        cout << "You have successfully signed in." << endl;
        cout << endl;
        hospMenu();
    }
}

// Registration
static bool isAadharExists(const string &aadharNo)
{
    ifstream in("users.txt");
    if (!in.is_open())
    {
        return false;
    }

    string line;
    while (getline(in, line))
    {
        line = trim(line);
        if (line.empty())
            continue;

        vector<string> parts;
        size_t start = 0;
        for (int i = 0; i < 2; i++)
        {
            size_t pos = line.find(',', start);
            if (pos == string::npos)
                break;
            parts.push_back(trim(line.substr(start, pos - start)));
            start = pos + 1;
        }
        if (start < line.size())
        {
            parts.push_back(trim(line.substr(start)));
        }

        if (parts.size() > 1 && parts[1] == aadharNo)
        {
            return true;
        }
    }

    return false;
}

void registerUser()
{
    cout << "=========================================" << endl;
    cout << "        REGISTER NEW USER ACCOUNT        " << endl;
    cout << "=========================================" << endl;
    cout << endl;

    while (true)
    {
        cout << "Enter username: ";
        string username;
        getline(cin, username);
        username = trim(username);

        cout << "Enter Aadhar No (exactly 12 digits, no spaces): ";
        string aadharNo;
        getline(cin, aadharNo);
        aadharNo = trim(aadharNo);

        if (!regexMatch(aadharNo, "^\\d{12}$"))
        {
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

        if (isAadharExists(aadharNo))
        {
            cout << endl;
            cout << "[ERROR] Aadhar number already exists!" << endl;
            cout << "This Aadhar is already registered." << endl;
            cout << endl;
            cout << "1. Try Again" << endl;
            cout << "2. Log In" << endl;
            int choice = readIntLine();
            switch (choice)
            {
            case 1:
                continue;
            case 2:
                userLogIn();
                return;
            default:
                cout << "Invalid choice. Returning to registration." << endl;
                continue;
            }
        }

        ofstream out("users.txt", ios::app);
        if (!out.is_open())
        {
            cout << "[Error] Data could not be saved: could not open users.txt" << endl;
        }
        else
        {
            out << username << "," << aadharNo << "," << password << "\n";
            out.flush();
            cout << "User saved successfully." << endl;
            cout << "Please log in with your new credentials." << endl;
            userLogIn();
            break;
        }
    }
}

static bool isHospitalExists(const string &hospitalId)
{
    ifstream in("hospitals.txt");
    if (!in.is_open())
    {
        return false;
    }

    string line;
    while (getline(in, line))
    {
        line = trim(line);
        if (line.empty())
            continue;

        vector<string> parts;
        size_t start = 0;
        for (int i = 0; i < 2; i++)
        {
            size_t pos = line.find(',', start);
            if (pos == string::npos)
                break;
            parts.push_back(trim(line.substr(start, pos - start)));
            start = pos + 1;
        }
        if (start < line.size())
        {
            parts.push_back(trim(line.substr(start)));
        }

        if (!parts.empty() && parts[0] == hospitalId)
        {
            return true;
        }
    }

    return false;
}

void registerHospital()
{
    cout << "=========================================" << endl;
    cout << "      REGISTER NEW HOSPITAL ACCOUNT      " << endl;
    cout << "=========================================" << endl;
    cout << endl;

    while (true)
    {
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

        if (isHospitalExists(hospitalId))
        {
            cout << endl;
            cout << "[ERROR] Hospital ID already exists!" << endl;
            cout << "Please choose a different Hospital ID or log in." << endl;
            cout << endl;
            cout << "1. Try Again" << endl;
            cout << "2. Log In" << endl;
            int choice = readIntLine();
            switch (choice)
            {
            case 1:
                continue;
            case 2:
                hospLogIn();
                return;
            default:
                cout << "Invalid choice. Returning to registration." << endl;
                continue;
            }
        }

        ofstream out("hospitals.txt", ios::app);
        if (!out.is_open())
        {
            cout << "[Error] Data could not be saved: could not open hospitals.txt" << endl;
        }
        else
        {
            out << hospitalId << "," << hospitalName << "," << password << "\n";
            out.flush();
            cout << "Hospital saved successfully." << endl;
            cout << "Please log in with your new credentials." << endl;
            hospLogIn();
            break;
        }
    }
}

// User menu
void userMenu()
{
    while (true)
    {
        cout << "========================================" << endl;
        cout << "             USER MENU                  " << endl;
        cout << "========================================" << endl;
        cout << "1. Request Blood" << endl;
        cout << "2. Show My Requests" << endl;
        cout << "3. Exit" << endl;
        cout << "Select an option (1-3): ";
        int choice = readIntLine();
        cout << endl;

        switch (choice)
        {
        case 1:
            requestBlood();
            break;
        case 2:
            viewMyRequests();
            break;
        case 3:
            CurrentUser::clear();
            cout << "Returning to main page..." << endl;
            indexMain();
            return;
        default:
            cout << "Invalid choice. Please select 1, 2, or 3." << endl;
            cout << endl;
            break;
        }
    }
}

// Hospital menu
void hospMenu()
{
    while (true)
    {
        cout << "========================================" << endl;
        cout << "           HOSPITAL MENU                " << endl;
        cout << "========================================" << endl;
        cout << "1. View Pending Request" << endl;
        cout << "2. View Completed Request" << endl;
        cout << "3. Exit" << endl;
        cout << "Select an option (1-3): ";
        int choice = readIntLine();
        cout << endl;

        switch (choice)
        {
        case 1:
            viewPendingRequest();
            cout << "Press Enter to return to the Hospital Menu..." << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        case 2:
            viewCompletedRequest();
            cout << "Press Enter to return to the Hospital Menu..." << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        case 3:
            cout << "Returning to main page..." << endl;
            indexMain();
            return;
        default:
            cout << "Invalid choice. Please select 1, 2, or 3." << endl;
            cout << endl;
            break;
        }
    }
}

// Blood request flow
static const string BLOOD_REQ_FILE = "BloodReq.txt";

void saveBloodRequestToFile(const string &username, const string &aadharNo, const string &bloodGroup, const string &units)
{
    ofstream out(BLOOD_REQ_FILE, ios::app);
    if (!out.is_open())
    {
        cout << "[ERROR] Could not save to BloodReq.txt: could not open file" << endl;
        return;
    }
    out << "---" << endl;
    out << "Username: " << username << endl;
    out << "Aadhaar No: " << aadharNo << endl;
    out << "Blood Group: " << bloodGroup << endl;
    out << "Units Required: " << units << endl;
    out << "Status: pending" << endl;
    out.flush();
}

void requestBlood()
{
    string username = CurrentUser::getUsername();
    string aadharNo = CurrentUser::getAadharNo();

    if (username.empty() || aadharNo.empty())
    {
        cout << "[ERROR] No user logged in. Please log in first." << endl;
        return;
    }

    cout << "========================================" << endl;
    cout << "           REQUEST BLOOD FORM           " << endl;
    cout << "========================================" << endl;
    cout << "Logged in as: " << username << " (Aadhaar: ****";
    if (aadharNo.size() >= 4)
    {
        cout << aadharNo.substr(aadharNo.size() - 4);
    }
    else
    {
        cout << aadharNo;
    }
    cout << ")" << endl;
    cout << endl;

    string group;
    while (true)
    {
        cout << "Enter required blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ";
        getline(cin, group);
        group = trim(toUpper(group));

        if (regexMatch(group, "^(A|B|AB|O)[+-]$"))
        {
            break;
        }

        cout << "[ERROR] Invalid blood group!" << endl;
        cout << "Valid types: A+, A-, B+, B-, AB+, AB-, O+, O-." << endl;
        cout << "Please try again.\n"
             << endl;
    }

    string units;
    while (true)
    {
        cout << "Enter units required (positive whole number): ";
        getline(cin, units);
        units = trim(units);

        if (!regexMatch(units, "^\\d+$"))
        {
            cout << "[ERROR] Units must be a whole number (digits only)." << endl;
            continue;
        }

        long long unitsInt = 0;
        try
        {
            unitsInt = stoll(units);
        }
        catch (...)
        {
            cout << "[ERROR] Units value is too large. Please enter a smaller number." << endl;
            continue;
        }

        if (unitsInt <= 0)
        {
            cout << "[ERROR] Units must be greater than 0." << endl;
            continue;
        }

        if (unitsInt > 20)
        {
            cout << "[ERROR] Units must not exceed 20." << endl;
            continue;
        }

        break;
    }

    cout << endl;
    cout << "Request submitted: Blood Group " << group << ", Units: " << units << endl;
    saveBloodRequestToFile(username, aadharNo, group, units);
    cout << "Request saved to BloodReq.txt" << endl;
    cout << "Press Enter to return to the User Menu..." << endl;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void viewMyRequests()
{
    string username = CurrentUser::getUsername();
    if (username.empty())
    {
        cout << "[ERROR] No user logged in. Please log in first." << endl;
        return;
    }

    vector<BloodReqRecord> all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
    vector<BloodReqRecord> mine;
    for (const auto &r : all)
    {
        if (username == r.username)
        {
            mine.push_back(r);
        }
    }

    cout << "========================================" << endl;
    cout << "         MY BLOOD REQUESTS               " << endl;
    cout << "========================================" << endl;
    if (mine.empty())
    {
        cout << "You have no blood requests yet." << endl;
    }
    else
    {
        for (size_t i = 0; i < mine.size(); i++)
        {
            const auto &r = mine[i];
            cout << (i + 1) << ". Blood: " << r.bloodGroup << " | Units: " << r.units << " | Status: " << r.status << endl;
        }
    }
}

void viewPendingRequest()
{
    vector<BloodReqRecord> all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
    vector<BloodReqRecord> pending;
    for (const auto &r : all)
    {
        string statusLower = r.status;
        transform(statusLower.begin(), statusLower.end(), statusLower.begin(), [](unsigned char c)
                  { return tolower(c); });
        if (statusLower == "pending")
        {
            pending.push_back(r);
        }
    }

    cout << "========================================" << endl;
    cout << "         PENDING BLOOD REQUESTS         " << endl;
    cout << "========================================" << endl;
    if (pending.empty())
    {
        cout << "No pending requests." << endl;
    }
    else
    {
        for (size_t i = 0; i < pending.size(); i++)
        {
            const auto &r = pending[i];
            cout << (i + 1) << ". " << r.username << " | Aadhaar: " << r.aadharNo
                 << " | Blood: " << r.bloodGroup << " | Units: " << r.units << endl;
        }
    }
}

void viewCompletedRequest()
{
    vector<BloodReqRecord> all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
    vector<BloodReqRecord> completed;
    for (const auto &r : all)
    {
        string statusLower = r.status;
        transform(statusLower.begin(), statusLower.end(), statusLower.begin(), [](unsigned char c)
                  { return tolower(c); });
        if (statusLower == "completed")
        {
            completed.push_back(r);
        }
    }

    cout << "========================================" << endl;
    cout << "        COMPLETED BLOOD REQUESTS        " << endl;
    cout << "========================================" << endl;
    if (completed.empty())
    {
        cout << "No completed requests." << endl;
    }
    else
    {
        for (size_t i = 0; i < completed.size(); i++)
        {
            const auto &r = completed[i];
            cout << (i + 1) << ". " << r.username << " | Aadhaar: " << r.aadharNo
                 << " | Blood: " << r.bloodGroup << " | Units: " << r.units << endl;
        }
    }
}

int main()
{
    indexMain();
    return 0;
}
