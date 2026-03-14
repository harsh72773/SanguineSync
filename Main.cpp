#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <map>
#include <limits>
#include <iomanip>

using namespace std;

// Helper utilities
static string trim(const string &s) {
    size_t start = 0;
    while (start < s.size() && isspace((unsigned char)s[start]))
        start++;
    size_t end = s.size();
    while (end > start && isspace((unsigned char)s[end - 1]))
        end--;
    return s.substr(start, end - start);
}

static string toUpper(const string &s) {
    string out = s;
    transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return toupper(c); });
    return out;
}

static bool regexMatch(const string &value, const string &pattern) {
    try {
        return regex_match(value, regex(pattern));
    } catch (const exception &) {
        return false;
    }
}

static int readIntLine() {
    string line;
    if (!getline(cin, line))
        return -1;
    try {
        return stoi(trim(line));
    } catch (...) {
        return -1;
    }
}

// -----------------------------------------------------------------------------
// USER MANAGEMENT CLASS - Encapsulation & Abstraction
// -----------------------------------------------------------------------------

class User {
private:
    string username;
    string aadharNo;
    string password;
    
public:
    // Constructor - Object Creation
    User(const string &uname, const string &aadhar, const string &pwd) 
        : username(uname), aadharNo(aadhar), password(pwd) {}
    
    // Getters - Data Abstraction
    string getUsername() const { return username; }
    string getAadharNo() const { return aadharNo; }
    string getPassword() const { return password; }
    
    // Setters - Controlled Data Modification
    void setUsername(const string &uname) { username = uname; }
    void setAadharNo(const string &aadhar) { aadharNo = aadhar; }
    void setPassword(const string &pwd) { password = pwd; }
};

// Current User Session - Singleton Pattern
class CurrentUser {
private:
    static string username;  // Private static variable - Encapsulation
    static string aadharNo;  // Private static variable - Encapsulation
    
public:
    // Static methods for session management - Class-level operations
    static void set(const string &uname, const string &aadhar) {
        username = uname;
        aadharNo = aadhar;
    }
    
    static string getUsername() { return username; }
    static string getAadharNo() { return aadharNo; }
    static void clear() {
        username.clear();
        aadharNo.clear();
    }
};

// Initialize static members - Definition outside class
string CurrentUser::username = "";
string CurrentUser::aadharNo = "";

// -----------------------------------------------------------------------------
// HOSPITAL MANAGEMENT CLASS - Encapsulation & Data Hiding
// -----------------------------------------------------------------------------

class Hospital {
private:
    string hospitalId;
    string hospitalName;
    string password;
    
public:
    // Constructor - Object Initialization
    Hospital(const string &hid, const string &hname, const string &pwd)
        : hospitalId(hid), hospitalName(hname), password(pwd) {}
    
    // Getters - Abstraction
    string getHospitalId() const { return hospitalId; }
    string getHospitalName() const { return hospitalName; }
    string getPassword() const { return password; }
};

// Current Hospital Session - Singleton Pattern
class CurrentHospital {
private:
    static string hospitalId;    // Private static - Encapsulation
    static string hospitalName;  // Private static - Encapsulation
    
public:
    // Static session management methods
    static void set(const string &hid, const string &hname) {
        hospitalId = hid;
        hospitalName = hname;
    }
    
    static string getHospitalId() { return hospitalId; }
    static string getHospitalName() { return hospitalName; }
    static void clear() {
        hospitalId.clear();
        hospitalName.clear();
    }
};

// Initialize static members
string CurrentHospital::hospitalId = "";
string CurrentHospital::hospitalName = "";

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

void viewMyCompletedRequests();

void viewPendingRequest();

void viewCompletedRequest();

void viewCamps();

void organiseCamp();

void viewOngoingCamps();

void inventoryMenu();

void displayInventory();

// User login helpers - Authentication & Data Management
// Load users from file - File I/O Operations & Object Creation
static vector<User> loadUsersFromFile(const string &filename) {
    vector<User> users;
    ifstream in("textFiles/" + filename);
    if (!in.is_open()) {
        cout << "[ERROR] File '" << filename << "' not found!" << endl;
        return users;
    }

    string line;
    while (getline(in, line)) {
        line = trim(line);
        if (line.empty())
            continue;

        vector<string> parts;
        size_t start = 0;
        for (int i = 0; i < 2; i++) {
            size_t pos = line.find(',', start);
            if (pos == string::npos)
                break;
            parts.push_back(trim(line.substr(start, pos - start)));
            start = pos + 1;
        }
        if (start < line.size()) {
            parts.push_back(trim(line.substr(start)));
        }

        // Create User object - Constructor Usage & Encapsulation
        if (parts.size() == 3) {
            users.push_back(User(parts[0], parts[1], parts[2]));
        }
    }
    return users;
}

// Hospital login helpers - Authentication & Data Management
// Load hospitals from file - File I/O Operations & Object Creation
static vector<Hospital> loadHospitalsFromFile(const string &filename) {
    vector<Hospital> hospitals;
    ifstream in("textFiles/" + filename);
    if (!in.is_open()) {
        cout << "[ERROR] File '" << filename << "' not found!" << endl;
        return hospitals;
    }

    string line;
    while (getline(in, line)) {
        line = trim(line);
        if (line.empty())
            continue;

        vector<string> parts;
        size_t start = 0;
        for (int i = 0; i < 2; i++) {
            size_t pos = line.find(',', start);
            if (pos == string::npos)
                break;
            parts.push_back(trim(line.substr(start, pos - start)));
            start = pos + 1;
        }
        if (start < line.size()) {
            parts.push_back(trim(line.substr(start)));
        }

        // Create Hospital object - Constructor Usage & Encapsulation
        if (parts.size() == 3) {
            hospitals.push_back(Hospital(parts[0], parts[1], parts[2]));
        }
    }
    return hospitals;
}

// -----------------------------------------------------------------------------
// BLOOD REQUEST RECORD CLASS - Data Management & Abstraction
// -----------------------------------------------------------------------------

class BloodReqRecord {
private:
    string username;
    string aadharNo;
    string bloodGroup;
    string units;
    string status;
    
public:
    // Constructor - Object Initialization
    BloodReqRecord(const string &u, const string &a, const string &b, const string &un, const string &st)
        : username(u), aadharNo(a), bloodGroup(b), units(un), status(st.empty() ? "pending" : trim(st)) {}
    
    // Getters - Data Abstraction
    string getUsername() const { return username; }
    string getAadharNo() const { return aadharNo; }
    string getBloodGroup() const { return bloodGroup; }
    string getUnits() const { return units; }
    string getStatus() const { return status; }
    
    // Static method for loading records - Factory Pattern
    static vector<BloodReqRecord> loadAll(const string &path) {
        vector<BloodReqRecord> list;
        ifstream in("textFiles/" + path);
        if (!in.is_open()) {
            return list;
        }

        string username;
        string aadharNo;
        string bloodGroup;
        string units;
        string status = "pending";
        string line;

        auto pushIfValid = [&]() {
            if (!username.empty()) {
                list.emplace_back(username, aadharNo, bloodGroup, units, status);
            }
        };

        while (getline(in, line)) {
            line = trim(line);
            if (line == "---") {
                pushIfValid();
                username.clear();
                aadharNo.clear();
                bloodGroup.clear();
                units.clear();
                status = "pending";
                continue;
            }
            if (line.rfind("Username: ", 0) == 0) {
                username = trim(line.substr(strlen("Username: ")));
            } else if (line.rfind("Aadhaar No: ", 0) == 0) {
                aadharNo = trim(line.substr(strlen("Aadhaar No: ")));
            } else if (line.rfind("Blood Group: ", 0) == 0) {
                bloodGroup = trim(line.substr(strlen("Blood Group: ")));
            } else if (line.rfind("Units Required: ", 0) == 0) {
                units = trim(line.substr(strlen("Units Required: ")));
            } else if (line.rfind("Status: ", 0) == 0) {
                status = trim(line.substr(strlen("Status: ")));
            }
        }
        pushIfValid();
        return list;
    }
};

// -----------------------------------------------------------------------------
// DONOR MANAGEMENT CLASS - Encapsulation & Data Validation
// -----------------------------------------------------------------------------

class Donor {
private:
    string name;
    string email;
    string dob;
    string bloodGroup;
    int age;
    string disease;
    bool eligible;
    string hospitalId;
    bool donationCompleted;
    
public:
    // Constructor - Object Creation
    Donor(const string &n, const string &e, const string &d, const string &bg, int a, 
          const string &dis, bool elg, const string &hid, bool completed)
        : name(n), email(e), dob(d), bloodGroup(bg), age(a), disease(dis), 
          eligible(elg), hospitalId(hid), donationCompleted(completed) {}
    
    // Getters - Data Abstraction
    string getName() const { return name; }
    string getEmail() const { return email; }
    string getDob() const { return dob; }
    string getBloodGroup() const { return bloodGroup; }
    int getAge() const { return age; }
    string getDisease() const { return disease; }
    bool isEligible() const { return eligible; }
    string getHospitalId() const { return hospitalId; }
    bool isDonationCompleted() const { return donationCompleted; }
    
    // Setters - Controlled Data Modification
    void setDonationCompleted(bool completed) { donationCompleted = completed; }
    
    // Static method for creating donor from CSV data - Factory Pattern
    static Donor createFromCSV(const vector<string> &parts) {
        if (parts.size() == 8) {
            return Donor(parts[0], parts[1], parts[2], parts[3], stoi(parts[4]), 
                        parts[5], (parts[6] == "1"), parts[7], false);
        }
        return Donor("", "", "", "", 0, "", false, "", false);
    }
};

// Constants - Data Management
static const string DONORS_FILE = "donors.txt";

static bool isValidDate(const string &date) {
    if (!regexMatch(date, "^\\d{2}/\\d{2}/\\d{4}$")) {
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
// DONOR MANAGEMENT FUNCTIONS - Polymorphism & Data Processing
// -----------------------------------------------------------------------------

// Load donors from file - File I/O Operations
static vector<Donor> loadDonorsFromFile(const string &filename) {
    vector<Donor> donors;
    ifstream in("textFiles/" + filename);
    if (!in.is_open()) {
        cout << "[ERROR] Could not open " << filename << endl;
        return donors;
    }

    string line;
    while (getline(in, line)) {
        line = trim(line);
        if (line.empty())
            continue;

        vector<string> parts;
        size_t start = 0;
        for (int i = 0; i < 7; i++) {
            size_t pos = line.find(',', start);
            if (pos == string::npos)
                break;
            parts.push_back(trim(line.substr(start, pos - start)));
            start = pos + 1;
        }
        if (start < line.size()) {
            parts.push_back(trim(line.substr(start)));
        }

        // Use factory method to create donor - Factory Pattern
        Donor donor = Donor::createFromCSV(parts);
        if (!donor.getName().empty()) {
            donors.push_back(donor);
        }
    }
    return donors;
}

// Save donor to file - File I/O Operations
static bool saveDonorToFile(const Donor &donor, const string &filename) {
    ofstream out("textFiles/" + filename, ios::app);
    if (!out.is_open()) {
        cout << "[ERROR] Could not save to " << filename << ": could not open file" << endl;
        return false;
    }
    // Use getter methods - Encapsulation
    out << donor.getName() << "," << donor.getEmail() << "," << donor.getDob() << "," << donor.getBloodGroup() 
        << "," << donor.getAge() << "," << donor.getDisease() << "," << (donor.isEligible() ? "1" : "0") 
        << "," << donor.getHospitalId() << endl;
    out.flush();
    return true;
}

// Add new donor - User Interaction & Data Validation
void addDonor() {
    // Create donor object - Object Creation
    Donor donor("", "", "", "", 0, "", false, CurrentHospital::getHospitalId(), false);
    
    cout << "========================================" << endl;
    cout << "          ADD NEW DONOR               " << endl;
    cout << "========================================" << endl;
    
    // Get donor details - User Input
    string name, email, dob, bloodGroup, disease;
    int age = 0;
    
    cout << "Enter Donor Name: ";
    getline(cin, name);
    name = trim(name);
    
    cout << "Enter Email: ";
    getline(cin, email);
    email = trim(email);
    
    while (true) {
        cout << "Enter Date of Birth (DD/MM/YYYY): ";
        getline(cin, dob);
        dob = trim(dob);
        
        if (isValidDate(dob)) {
            break;
        } else {
            cout << "Invalid date format. Please use DD/MM/YYYY format." << endl;
        }
    }
    
    cout << "Enter Blood Group: ";
    getline(cin, bloodGroup);
    bloodGroup = trim(toUpper(bloodGroup));
    
    cout << "========================================" << endl;
    cout << "        HEALTH DECLARATION           " << endl;
    cout << "========================================" << endl;
    
    while (true) {
        cout << "Enter Age: ";
        string ageStr;
        getline(cin, ageStr);
        ageStr = trim(ageStr);
        
        try {
            age = stoi(ageStr);
            if (age < 18) {
                cout << "Not eligible (Age must be 18 or above)" << endl;
                return;
            }
            break;
        } catch (...) {
            cout << "Invalid age. Please enter a valid number." << endl;
        }
    }
    
    cout << "Any Serious Disease? (Yes/No): ";
    getline(cin, disease);
    disease = trim(disease);
    
    // Create donor object with all data - Constructor Usage
    Donor newDonor(name, email, dob, bloodGroup, age, disease, 
                   (disease != "Yes" && disease != "YES"), 
                   CurrentHospital::getHospitalId(), false);
    
    if (disease == "Yes" || disease == "YES") {
        cout << "Sorry, donor NOT eligible for donation" << endl;
        return;
    }
    
    // Save donor to file - Data Persistence
    if (saveDonorToFile(newDonor, DONORS_FILE)) {
        cout << "========================================" << endl;
        cout << "    Thank you " << name << endl;
        cout << "    Happy donation" << endl;
        cout << "========================================" << endl;
    }
}

// View donor list - Data Display & Filtering
void viewDonors() {
    // Load all donors - Data Retrieval
    auto donors = loadDonorsFromFile(DONORS_FILE);
    vector<Donor> hospitalDonors;
    
    // Filter donors for current hospital - Data Processing
    for (const auto &donor : donors) {
        if (donor.getHospitalId() == CurrentHospital::getHospitalId()) {
            hospitalDonors.push_back(donor);
        }
    }

    cout << "========================================" << endl;
    cout << "          HOSPITAL DONOR LIST            " << endl;
    cout << "========================================" << endl;
    
    if (hospitalDonors.empty()) {
        cout << "No donors found for your hospital." << endl;
        return;
    }
    
    // Display donor list with index numbers - User Interface
    cout << left << setw(5) << "NO." << setw(20) << "NAME" << setw(25) << "EMAIL" << setw(12) << "DOB" 
         << setw(10) << "BLOOD" << setw(6) << "AGE" << setw(15) << "DISEASE" 
         << setw(10) << "STATUS" << endl;
    cout << string(103, '-') << endl;
    
    for (size_t i = 0; i < hospitalDonors.size(); i++) {
        const auto &d = hospitalDonors[i];
        // Use getter methods - Encapsulation
        cout << left << setw(5) << (i + 1) << setw(20) << d.getName() << setw(25) << d.getEmail() << setw(12) << d.getDob()
             << setw(10) << d.getBloodGroup() << setw(6) << d.getAge() << setw(15) 
             << (d.getDisease() == "Yes" || d.getDisease() == "YES" ? "Yes" : "No")
             << setw(10) << (d.isDonationCompleted() ? "Completed" : "Pending") << endl;
    }
    
    // Show marking option for pending donations - User Interaction
    bool hasPending = false;
    for (const auto &d : hospitalDonors) {
        if (!d.isDonationCompleted()) {
            hasPending = true;
            break;
        }
    }
    
    if (hasPending) {
        cout << endl;
        cout << "Enter donor number to mark as completed (0 to go back): ";
        int choice = readIntLine();
        
        if (choice > 0 && choice <= static_cast<int>(hospitalDonors.size())) {
            const auto &selectedDonor = hospitalDonors[choice - 1];
            
            if (!selectedDonor.isDonationCompleted()) {
                // Mark donation as completed - Data Update
                vector<Donor> updatedDonors;
                for (const auto &donor : donors) {
                    if (donor.getEmail() == selectedDonor.getEmail() && 
                        donor.getHospitalId() == selectedDonor.getHospitalId()) {
                        // Create updated donor with completed status - Object Modification
                        Donor updatedDonor = Donor(
                            donor.getName(), donor.getEmail(), donor.getDob(), 
                            donor.getBloodGroup(), donor.getAge(), donor.getDisease(), 
                            donor.isEligible(), donor.getHospitalId(), true
                        );
                        updatedDonors.push_back(updatedDonor);
                    } else {
                        updatedDonors.push_back(donor);
                    }
                }
                
                // Rewrite the file with updated donation status - Data Persistence
                ofstream out("textFiles/" + DONORS_FILE, ios::trunc);
                if (out.is_open()) {
                    for (const auto &donor : updatedDonors) {
                        // Use getter methods - Encapsulation
                        out << donor.getName() << "," << donor.getEmail() << "," << donor.getDob() << "," << donor.getBloodGroup() 
                            << "," << donor.getAge() << "," << donor.getDisease() << "," << (donor.isEligible() ? "1" : "0") 
                            << "," << donor.getHospitalId() << endl;
                    }
                    out.flush();
                    cout << "Donation marked as completed!" << endl;
                }
            } else {
                cout << "This donation is already completed." << endl;
            }
        }
    }
}

void markDonationCompleted() {
    auto donors = loadDonorsFromFile(DONORS_FILE);
    
    cout << "========================================" << endl;
    cout << "      MARK DONATION COMPLETED          " << endl;
    cout << "========================================" << endl;
    
    cout << "Enter Donor Phone Number (as identifier): ";
    string donorPhone;
    getline(cin, donorPhone);
    donorPhone = trim(donorPhone);
    
    bool found = false;
    for (auto &donor : donors) {
        // Use getter methods - Encapsulation
        if (donor.getHospitalId() == CurrentHospital::getHospitalId() && 
            donor.getEmail().find(donorPhone) != string::npos && 
            !donor.isDonationCompleted()) {
            // Create updated donor with completed status - Object Modification
            donor = Donor(donor.getName(), donor.getEmail(), donor.getDob(), 
                         donor.getBloodGroup(), donor.getAge(), donor.getDisease(), 
                         donor.isEligible(), donor.getHospitalId(), true);
            found = true;
            break;
        }
    }
    
    if (found) {
        // Rewrite the file with updated donation status - Data Persistence
        ofstream out("textFiles/" + DONORS_FILE, ios::trunc);
        if (out.is_open()) {
            for (const auto &donor : donors) {
                // Use getter methods - Encapsulation
                out << donor.getName() << "," << donor.getEmail() << "," << donor.getDob() << "," << donor.getBloodGroup() 
                    << "," << donor.getAge() << "," << donor.getDisease() << "," << (donor.isEligible() ? "1" : "0") 
                    << "," << donor.getHospitalId() << endl;
            }
            out.flush();
            cout << "Donation marked as completed!" << endl;
        }
    } else {
        cout << "Donor not found or already completed." << endl;
    }
}

void donorMenu() {
    while (true) {
        cout << "========================================" << endl;
        cout << "          DONOR MANAGEMENT             " << endl;
        cout << "========================================" << endl;
        cout << "1. Add Donor" << endl;
        cout << "2. View Donor List" << endl;
        cout << "3. Back" << endl;
        cout << "Select an option (1-3): ";
        int choice = readIntLine();
        cout << endl;
        
        switch (choice) {
            case 1:
                addDonor();
                break;
            case 2:
                viewDonors();
                break;
            case 3:
                return;
            default:
                cout << "Invalid choice. Please select 1-3." << endl;
                break;
        }
        
        if (choice != 3) {
            cout << "Press Enter to continue..." << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

// -----------------------------------------------------------------------------
// BLOOD CAMP MANAGEMENT CLASS - Event Organization
// -----------------------------------------------------------------------------

class Camp {
private:
    string name;
    string date;
    string venue;
    string city;
    
public:
    // Constructor - Object Initialization
    Camp(const string &n, const string &d, const string &v, const string &c)
        : name(n), date(d), venue(v), city(c) {}
    
    // Getters - Data Abstraction
    string getName() const { return name; }
    string getDate() const { return date; }
    string getVenue() const { return venue; }
    string getCity() const { return city; }
    
    // Static method for creating camp from CSV data - Factory Pattern
    static Camp createFromCSV(const vector<string> &parts) {
        if (parts.size() == 4) {
            return Camp(parts[0], parts[1], parts[2], parts[3]);
        }
        return Camp("", "", "", "");
    }
};

// Constants - Data Management
static const string CAMPS_FILE = "camps.txt";

// Load camps from file - File I/O Operations
static vector<Camp> loadCampsFromFile(const string &filename) {
    vector<Camp> camps;
    ifstream in("textFiles/" + filename);
    if (!in.is_open()) {
        return camps;
    }

    string line;
    while (getline(in, line)) {
        line = trim(line);
        if (line.empty())
            continue;

        vector<string> parts;
        size_t start = 0;
        for (int i = 0; i < 3; i++) {
            size_t pos = line.find(',', start);
            if (pos == string::npos)
                break;
            parts.push_back(trim(line.substr(start, pos - start)));
            start = pos + 1;
        }
        if (start < line.size()) {
            parts.push_back(trim(line.substr(start)));
        }

        // Use factory method to create camp - Factory Pattern
        Camp camp = Camp::createFromCSV(parts);
        if (!camp.getName().empty()) {
            camps.push_back(camp);
        }
    }

    return camps;
}

// Save camp to file - File I/O Operations
static bool saveCampToFile(const Camp &camp, const string &filename) {
    ofstream out("textFiles/" + filename, ios::app);
    if (!out.is_open()) {
        cout << "[ERROR] Could not save to " << filename << ": could not open file" << endl;
        return false;
    }
    // Use getter methods - Encapsulation
    out << camp.getName() << "," << camp.getDate() << "," << camp.getVenue() << "," << camp.getCity() << endl;
    out.flush();
    return true;
}

// View blood camps - Data Display
void viewCamps() {
    // Load camps from file - Data Retrieval
    auto camps = loadCampsFromFile(CAMPS_FILE);

    cout << "========================================" << endl;
    cout << "            AVAILABLE CAMPS              " << endl;
    cout << "========================================" << endl;
    
    if (camps.empty()) {
        cout << "No camps are currently available." << endl;
        return;
    }
    
    // Display camps with index numbers - User Interface
    cout << left << setw(5) << "NO." << setw(25) << "CAMP NAME" << setw(12) << "DATE" << setw(20) << "VENUE" << setw(15) << "CITY" << endl;
    cout << string(77, '-') << endl;
    
    for (size_t i = 0; i < camps.size(); i++) {
        const auto &c = camps[i];
        // Use getter methods - Encapsulation
        cout << left << setw(5) << (i + 1) << setw(25) << c.getName() << setw(12) << c.getDate() 
             << setw(20) << c.getVenue() << setw(15) << c.getCity() << endl;
    }
}

// Organize blood camp - User Input & Data Creation
void organiseCamp() {
    // Get camp details from user - User Interaction
    string name, date, venue, city;
    
    cout << "========================================" << endl;
    cout << "          ORGANISE BLOOD CAMP           " << endl;
    cout << "========================================" << endl;

    cout << "Enter Camp Name: ";
    getline(cin, name);
    
    while (true) {
        cout << "Enter Date (DD/MM/YYYY): ";
        getline(cin, date);
        date = trim(date);
        
        if (isValidDate(date)) {
            break;
        } else {
            cout << "Invalid date format. Please use DD/MM/YYYY format." << endl;
        }
    }
    
    cout << "Enter Venue: ";
    getline(cin, venue);
    cout << "Enter City: ";
    getline(cin, city);

    // Create camp object - Object Creation
    Camp camp(name, date, venue, city);

    // Save camp to file - Data Persistence
    if (saveCampToFile(camp, CAMPS_FILE)) {
        cout << "Blood camp saved to " << CAMPS_FILE << "" << endl;
    }
}

void viewOngoingCamps() {
    cout << "========================================" << endl;
    cout << "           ONGOING BLOOD CAMPS          " << endl;
    cout << "========================================" << endl;
    viewCamps();
}

// -----------------------------------------------------------------------------
// Inventory Management
// -----------------------------------------------------------------------------

static const string INVENTORY_FILE = "inventory.txt";
static const vector<string> VALID_BLOOD_GROUPS = {"A+", "A-", "B+", "B-", "O+", "O-", "AB+", "AB-"};

static string getInventoryFilePath() {
    const string &hid = CurrentHospital::getHospitalId();
    if (hid.empty()) {
        return "textFiles/inventory.txt";
    }
    return "textFiles/inventory_" + hid + ".txt";
}

static bool isValidBloodGroup(const string &group) {
    for (const auto &g : VALID_BLOOD_GROUPS) {
        if (g == group)
            return true;
    }
    return false;
}

static map<string, int> loadInventoryFromFile(const string &filename) {
    map<string, int> inventory;
    for (const auto &g : VALID_BLOOD_GROUPS) {
        inventory[g] = 0;
    }

    ifstream in(filename);
    if (!in.is_open()) {
        return inventory;
    }

    string line;
    while (getline(in, line)) {
        line = trim(line);
        if (line.empty())
            continue;

        size_t sep = line.find(',');
        if (sep == string::npos)
            continue;

        string group = trim(line.substr(0, sep));
        string unitsStr = trim(line.substr(sep + 1));
        if (!isValidBloodGroup(group))
            continue;

        try {
            int units = stoi(unitsStr);
            if (units < 0)
                continue;
            inventory[group] = units;
        } catch (...) {
            // ignore invalid numbers
        }
    }

    return inventory;
}

static bool saveInventoryToFile(const map<string, int> &inventory, const string &filename) {
    ofstream out(filename, ios::trunc);
    if (!out.is_open()) {
        cout << "[ERROR] Could not save to " << filename << ": could not open file" << endl;
        return false;
    }

    for (const auto &g : VALID_BLOOD_GROUPS) {
        out << g << "," << inventory.at(g) << endl;
    }
    out.flush();
    return true;
}

void displayInventory() {
    auto inventory = loadInventoryFromFile(getInventoryFilePath());

    cout << "========================================" << endl;
    cout << "             BLOOD INVENTORY            " << endl;
    cout << "========================================" << endl;
    
    cout << left << setw(5) << "NO." << setw(10) << "BLOOD" << setw(10) << "UNITS" << endl;
    cout << string(25, '-') << endl;
    
    int index = 1;
    for (const auto &g : VALID_BLOOD_GROUPS) {
        cout << left << setw(5) << index << setw(10) << g << setw(10) << inventory[g] << " units" << endl;
        index++;
    }
}

void inventoryMenu() {
    while (true) {
        cout << "========================================" << endl;
        cout << "           INVENTORY MANAGEMENT        " << endl;
        cout << "========================================" << endl;
        cout << "1. Add Stock" << endl;
        cout << "2. Remove Stock" << endl;
        cout << "3. View Inventory" << endl;
        cout << "4. Back" << endl;
        cout << "Select an option (1-4): ";
        int choice = readIntLine();
        cout << endl;

        if (choice == 1) {
            string group;
            string unitsStr;
            cout << "Enter blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ";
            getline(cin, group);
            group = trim(toUpper(group));

            if (!isValidBloodGroup(group)) {
                cout << "Invalid blood group." << endl;
                continue;
            }

            cout << "Enter units to add: ";
            getline(cin, unitsStr);
            unitsStr = trim(unitsStr);
            if (!regexMatch(unitsStr, "^\\d+$")) {
                cout << "Units must be a positive whole number." << endl;
                continue;
            }

            int units = stoi(unitsStr);
            if (units <= 0) {
                cout << "Units must be greater than 0." << endl;
                continue;
            }

            auto inventory = loadInventoryFromFile(getInventoryFilePath());
            inventory[group] += units;
            if (saveInventoryToFile(inventory, getInventoryFilePath())) {
                cout << units << " units added to " << group << "." << endl;
            }
        } else if (choice == 2) {
            string group;
            string unitsStr;
            cout << "Enter blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ";
            getline(cin, group);
            group = trim(toUpper(group));

            if (!isValidBloodGroup(group)) {
                cout << "Invalid blood group." << endl;
                continue;
            }

            cout << "Enter units to remove: ";
            getline(cin, unitsStr);
            unitsStr = trim(unitsStr);
            if (!regexMatch(unitsStr, "^\\d+$")) {
                cout << "Units must be a positive whole number." << endl;
                continue;
            }

            int units = stoi(unitsStr);
            if (units <= 0) {
                cout << "Units must be greater than 0." << endl;
                continue;
            }

            auto inventory = loadInventoryFromFile(getInventoryFilePath());
            if (inventory[group] < units) {
                cout << "Not enough stock available for " << group << "." << endl;
                continue;
            }

            inventory[group] -= units;
            if (saveInventoryToFile(inventory, getInventoryFilePath())) {
                cout << units << " units removed from " << group << "." << endl;
            }
        } else if (choice == 3) {
            displayInventory();
        } else if (choice == 4) {
            return;
        } else {
            cout << "Invalid choice. Please select 1-4." << endl;
        }

        cout << "Press Enter to continue..." << endl;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// -----------------------------------------------------------------------------
// Index (main entry point) - Program Start
void indexMain() {
    while (true) {  // Prevent infinite recursion with loop
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

        switch (choice) {
            case 1:
                registerUser();
                break;
            case 2:
                registerHospital();
                break;
            case 3:
                cout << "========================================" << endl;
                logInPage();
                return;  // Exit loop after successful login
            case 4:
                cout << "Exiting system..." << endl;
                exit(0);
            default:
                cout << "Invalid choice. Please select 1, 2, 3, or 4." << endl;
                cout << "Try again." << endl;
                continue;  // Continue loop instead of recursion
        }
    }
}

// -----------------------------------------------------------------------------
// Login flow
// -----------------------------------------------------------------------------

void logInPage() {
    cout << endl;
    cout << "       Who is loging in?       " << endl;
    cout << "1. User" << endl;
    cout << "2. Hospital" << endl;
    cout << endl;
    cout << "Select an option (1-2): ";

    int choice = readIntLine();
    cout << endl;

    switch (choice) {
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

// User login - Authentication & Session Management
void userLogIn() {
    while (true) {  // Prevent infinite recursion with loop
        // Load users from file - Data Retrieval
        vector<User> users = loadUsersFromFile("users.txt");

        if (users.empty()) {
            cout << "[ERROR] No users found in file or file error!" << endl;
            cout << "Please register first." << endl;
            registerUser();
            return;
        }

        cout << "=========================================" << endl;
        cout << "            ENTER CREDENTIALS            " << endl;
        cout << "=========================================" << endl;
        cout << endl;

        // Get user credentials - User Input
        cout << "Enter Username: ";
        string inputUsername;
        getline(cin, inputUsername);
        inputUsername = trim(inputUsername);

        cout << "Enter Password: ";
        string inputPassword;
        getline(cin, inputPassword);
        inputPassword = trim(inputPassword);

        // Authenticate user - Authentication Logic
        bool userFound = false;
        string foundAadhar = "";
        for (const auto &u : users) {
            // Use getter methods - Encapsulation
            if (u.getUsername() == inputUsername && u.getPassword() == inputPassword) {
                userFound = true;
                foundAadhar = u.getAadharNo();
                break;
            }
        }

        if (!userFound) {
            cout << endl;
            cout << "[ERROR] Invalid credentials!" << endl;
            cout << "Username and Password must match." << endl;
            cout << "Please check and try again." << endl;
            cout << "OR register first." << endl;
            
            cout << "1. Try Again" << endl;
            cout << "2. Register New User" << endl;
            cout << "3. Back to Main Menu" << endl;
            cout << "Select an option (1-3): ";
            int choice = readIntLine();
            
            switch (choice) {
                case 1:
                    continue;  // Continue loop for retry
                case 2:
                    registerUser();
                    return;
                case 3:
                    return;
                default:
                    cout << "Invalid choice. Returning to main menu." << endl;
                    return;
            }
        } else {
            // Mask Aadhar for display - Data Privacy
            string maskedAadhar = "********";
            if (foundAadhar.size() >= 4) {
                maskedAadhar += foundAadhar.substr(foundAadhar.size() - 4);
            } else {
                maskedAadhar += foundAadhar;
            }

            cout << endl;
            cout << "=========================================" << endl;
            cout << "           LOGIN SUCCESSFUL!            " << endl;
            cout << "=========================================" << endl;
            cout << "Welcome, " << inputUsername << "!" << endl;
            cout << "Aadhar: " << maskedAadhar << endl;
            cout << "You have successfully signed in." << endl;
            cout << endl;
            CurrentUser::set(inputUsername, foundAadhar);
            userMenu();
            return;  // Exit loop after successful login
        }
    }
}

void hospLogIn() {
    while (true) {  // Prevent infinite recursion with loop
        // Load hospitals from file - Data Retrieval
        vector<Hospital> hospitals = loadHospitalsFromFile("hospitals.txt");

        if (hospitals.empty()) {
            cout << "[ERROR] No hospitals found in file or file error!" << endl;
            cout << "Please register first." << endl;
            registerHospital();
            return;
        }

        cout << "=========================================" << endl;
        cout << "            ENTER CREDENTIALS            " << endl;
        cout << "=========================================" << endl;
        cout << endl;

        // Get hospital credentials - User Input
        cout << "Enter Hospital ID: ";
        string inputHospitalId;
        getline(cin, inputHospitalId);
        inputHospitalId = trim(inputHospitalId);

        cout << "Enter Password: ";
        string inputPassword;
        getline(cin, inputPassword);
        inputPassword = trim(inputPassword);

        // Authenticate hospital - Authentication Logic
        int foundIndex = -1;
        for (size_t i = 0; i < hospitals.size(); i++) {
            // Use getter methods - Encapsulation
            if (hospitals[i].getHospitalId() == inputHospitalId && hospitals[i].getPassword() == inputPassword) {
                foundIndex = i;
                break;
            }
        }

        if (foundIndex == -1) {
            cout << endl;
            cout << "[ERROR] Invalid credentials!" << endl;
            cout << "Hospital ID and Password must match." << endl;
            cout << "Please check and try again." << endl;
            cout << "OR register first." << endl;
            
            cout << "1. Try Again" << endl;
            cout << "2. Register New Hospital" << endl;
            cout << "3. Back to Main Menu" << endl;
            cout << "Select an option (1-3): ";
            int choice = readIntLine();
            
            switch (choice) {
                case 1:
                    continue;  // Continue loop for retry
                case 2:
                    registerHospital();
                    return;
                case 3:
                    return;
                default:
                    cout << "Invalid choice. Returning to main menu." << endl;
                    return;
            }
        } else {
            cout << endl;
            cout << "=========================================" << endl;
            cout << "           LOGIN SUCCESSFUL!            " << endl;
            cout << "=========================================" << endl;
            // Use getter methods - Encapsulation
            cout << "Welcome, " << hospitals[foundIndex].getHospitalName() << "!" << endl;
            cout << "You have successfully signed in." << endl;
            cout << endl;
            // Set hospital session - Session Management
            CurrentHospital::set(hospitals[foundIndex].getHospitalId(), hospitals[foundIndex].getHospitalName());
            hospMenu();
            CurrentHospital::clear();
            return;  // Exit loop after successful login
        }
    }
}

// Registration
static bool isAadharExists(const string &aadharNo) {
    ifstream in("textFiles/users.txt");
    if (!in.is_open()) {
        return false;
    }

    string line;
    while (getline(in, line)) {
        line = trim(line);
        if (line.empty())
            continue;

        vector<string> parts;
        size_t start = 0;
        for (int i = 0; i < 2; i++) {
            size_t pos = line.find(',', start);
            if (pos == string::npos)
                break;
            parts.push_back(trim(line.substr(start, pos - start)));
            start = pos + 1;
        }
        if (start < line.size()) {
            parts.push_back(trim(line.substr(start)));
        }

        if (parts.size() > 1 && parts[1] == aadharNo) {
            return true;
        }
    }

    return false;
}

void registerUser() {
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

        if (!regexMatch(aadharNo, "^\\d{12}$")) {
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
            int choice = readIntLine();
            switch (choice) {
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

        ofstream out("textFiles/users.txt", ios::app);
        if (!out.is_open()) {
            cout << "[Error] Data could not be saved: could not open users.txt" << endl;
        } else {
            out << username << "," << aadharNo << "," << password << "\n";
            out.flush();
            cout << "User saved successfully." << endl;
            cout << "Please log in with your new credentials." << endl;
            userLogIn();
            break;
        }
    }
}

static bool isHospitalExists(const string &hospitalId) {
    ifstream in("textFiles/hospitals.txt");
    if (!in.is_open()) {
        return false;
    }

    string line;
    while (getline(in, line)) {
        line = trim(line);
        if (line.empty())
            continue;

        vector<string> parts;
        size_t start = 0;
        for (int i = 0; i < 2; i++) {
            size_t pos = line.find(',', start);
            if (pos == string::npos)
                break;
            parts.push_back(trim(line.substr(start, pos - start)));
            start = pos + 1;
        }
        if (start < line.size()) {
            parts.push_back(trim(line.substr(start)));
        }

        if (!parts.empty() && parts[0] == hospitalId) {
            return true;
        }
    }

    return false;
}

void registerHospital() {
    cout << "=========================================" << endl;
    cout << "      REGISTER NEW HOSPITAL ACCOUNT      " << endl;
    cout << "=========================================" << endl;
    cout << endl;

    while (true) {
        cout << "Enter hospital name: ";
        string hospitalName;
        getline(cin, hospitalName);
        hospitalName = trim(hospitalName);

        while (true) {
            cout << "Enter hospital ID (exactly 10 digits): ";
            string hospitalId;
            getline(cin, hospitalId);
            hospitalId = trim(hospitalId);

            if (!regexMatch(hospitalId, "^\\d{10}$")) {
                cout << endl;
                cout << "[ERROR] Invalid Hospital ID!" << endl;
                cout << "- Must be EXACTLY 10 digits" << endl;
                cout << "- Only numbers (0-9), no spaces or letters" << endl;
                cout << "- Examples: 1234567890 ✓ | 123 456 789 0 ✗" << endl;
                cout << endl;
                continue;
            }

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
                int choice = readIntLine();
                switch (choice) {
                    case 1:
                        break;
                    case 2:
                        hospLogIn();
                        return;
                    default:
                        cout << "Invalid choice. Returning to registration." << endl;
                        break;
                }
                continue;
            }

            ofstream out("textFiles/hospitals.txt", ios::app);
            if (!out.is_open()) {
                cout << "[Error] Data could not be saved: could not open hospitals.txt" << endl;
            } else {
                out << hospitalId << "," << hospitalName << "," << password << "\n";
                out.flush();
                cout << "Hospital saved successfully." << endl;
                cout << "Please log in with your new credentials." << endl;
                hospLogIn();
                return;
            }
        }
    }
}

// User menu
void userMenu() {
    while (true) {
        cout << "========================================" << endl;
        cout << "             USER MENU                  " << endl;
        cout << "========================================" << endl;
        cout << "1. Request Blood" << endl;
        cout << "2. Show My Requests" << endl;
        cout << "3. View Completed Requests" << endl;
        cout << "4. View Blood Camps" << endl;
        cout << "5. View Inventory" << endl;
        cout << "6. Exit" << endl;
        cout << "Select an option (1-6): ";
        int choice = readIntLine();
        cout << endl;

        switch (choice) {
            case 1:
                requestBlood();
                break;
            case 2:
                viewMyRequests();
                break;
            case 3:
                viewMyCompletedRequests();
                cout << "Press Enter to return to the User Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 4:
                viewCamps();
                cout << "Press Enter to return to the User Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 5:
                displayInventory();
                cout << "Press Enter to return to the User Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 6:
                CurrentUser::clear();
                cout << "Returning to main page..." << endl;
                indexMain();
                return;
            default:
                cout << "Invalid choice. Please select 1, 2, 3, 4, 5, or 6." << endl;
                cout << endl;
                break;
        }
    }
}

// Hospital menu
// Hospital menu - Main Hospital Interface
void hospMenu() {
    while (true) {
        // Count pending requests for display - Data Processing
        vector<BloodReqRecord> all = BloodReqRecord::loadAll("BloodReq.txt");
        vector<BloodReqRecord> pending;
        auto inventory = loadInventoryFromFile(getInventoryFilePath());
        
        // Filter requests that hospital can fulfill - Business Logic
        for (const auto &r : all) {
            // Use getter methods - Encapsulation
            string statusLower = r.getStatus();
            transform(statusLower.begin(), statusLower.end(), statusLower.begin(), [](unsigned char c) { return tolower(c); });
            if (statusLower == "pending" && inventory.count(r.getBloodGroup()) > 0 && 
                inventory[r.getBloodGroup()] >= stoi(r.getUnits())) {
                pending.push_back(r);
            }
        }
        
        cout << "========================================" << endl;
        cout << "            HOSPITAL MENU               " << endl;
        cout << "========================================" << endl;
        cout << "Pending Requests: " << pending.size() << endl;
        cout << endl;
        cout << "1. View Pending Request" << endl;
        cout << "2. View Completed Request" << endl;
        cout << "3. Organise Blood Camp" << endl;
        cout << "4. View Ongoing Camps" << endl;
        cout << "5. Donor Management" << endl;
        cout << "6. Inventory Management" << endl;
        cout << "7. Exit" << endl;
        cout << "Select an option (1-7): ";
        int choice = readIntLine();
        cout << endl;

        switch (choice) {
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
                organiseCamp();
                cout << "Press Enter to return to the Hospital Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 4:
                viewOngoingCamps();
                cout << "Press Enter to return to the Hospital Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 5:
                donorMenu();
                break;
            case 6:
                inventoryMenu();
                break;
            case 7:
                cout << "Returning to main page..." << endl;
                indexMain();
                return;
            default:
                cout << "Invalid choice. Please select 1-7." << endl;
                cout << endl;
                break;
        }
    }
}

// Blood request flow
static const string BLOOD_REQ_FILE = "BloodReq.txt";

// -----------------------------------------------------------------------------
// BLOOD REQUEST FUNCTIONS - Request Management
// -----------------------------------------------------------------------------

// Save blood request to file - Data Persistence
void saveBloodRequestToFile(const string &username, const string &aadharNo, const string &bloodGroup, const string &units) {
    ofstream out("textFiles/" + BLOOD_REQ_FILE, ios::app);
    if (!out.is_open()) {
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

void requestBlood() {
    string username = CurrentUser::getUsername();
    string aadharNo = CurrentUser::getAadharNo();

    if (username.empty() || aadharNo.empty()) {
        cout << "[ERROR] No user logged in. Please log in first." << endl;
        return;
    }

    cout << "========================================" << endl;
    cout << "           REQUEST BLOOD FORM           " << endl;
    cout << "========================================" << endl;
    cout << "Logged in as: " << username << " (Aadhaar: ****";
    if (aadharNo.size() >= 4) {
        cout << aadharNo.substr(aadharNo.size() - 4);
    } else {
        cout << aadharNo;
    }
    cout << ")" << endl;
    cout << endl;

    string group;
    while (true) {
        cout << "Enter required blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ";
        getline(cin, group);
        group = trim(toUpper(group));

        if (regexMatch(group, "^(A|B|AB|O)[+-]$")) {
            break;
        }

        cout << "[ERROR] Invalid blood group!" << endl;
        cout << "Valid types: A+, A-, B+, B-, AB+, AB-, O+, O-." << endl;
        cout << "Please try again.\n" << endl;
    }

    string units;
    while (true) {
        cout << "Enter units required (positive whole number): ";
        getline(cin, units);
        units = trim(units);

        if (!regexMatch(units, "^\\d+$")) {
            cout << "[ERROR] Units must be a whole number (digits only)." << endl;
            continue;
        }

        long long unitsInt = 0;
        try {
            unitsInt = stoll(units);
        } catch (...) {
            cout << "[ERROR] Units value is too large. Please enter a smaller number." << endl;
            continue;
        }

        if (unitsInt <= 0) {
            cout << "[ERROR] Units must be greater than 0." << endl;
            continue;
        }

        if (unitsInt > 20) {
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

// View user's blood requests - Data Display & Filtering
void viewMyRequests() {
    // Get current user session - Session Management
    string username = CurrentUser::getUsername();
    if (username.empty()) {
        cout << "[ERROR] No user logged in. Please log in first." << endl;
        return;
    }

    // Load all blood requests - Data Retrieval
    vector<BloodReqRecord> all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
    vector<BloodReqRecord> mine;
    
    // Filter requests for current user - Data Processing
    for (const auto &r : all) {
        if (username == r.getUsername()) {
            mine.push_back(r);
        }
    }

    cout << "========================================" << endl;
    cout << "         MY BLOOD REQUESTS               " << endl;
    cout << "========================================" << endl;
    if (mine.empty()) {
        cout << "You have no blood requests yet." << endl;
    } else {
        // Display requests with index numbers - User Interface
        cout << left << setw(5) << "NO." << setw(12) << "BLOOD" << setw(8) << "UNITS" << setw(10) << "STATUS" << endl;
        cout << string(35, '-') << endl;
        for (size_t i = 0; i < mine.size(); i++) {
            const auto &r = mine[i];
            // Use getter methods - Encapsulation
            cout << left << setw(5) << (i + 1) << setw(12) << r.getBloodGroup() << setw(8) << r.getUnits() 
                 << setw(10) << r.getStatus() << endl;
        }
    }
}

// View user's completed blood requests - Data Display & Filtering
void viewMyCompletedRequests() {
    // Get current user session - Session Management
    string username = CurrentUser::getUsername();
    if (username.empty()) {
        cout << "[ERROR] No user logged in. Please log in first." << endl;
        return;
    }

    // Load all blood requests - Data Retrieval
    vector<BloodReqRecord> all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
    vector<BloodReqRecord> myCompleted;
    
    // Filter completed requests for current user - Data Processing
    for (const auto &r : all) {
        if (username == r.getUsername() && r.getStatus() == "completed") {
            myCompleted.push_back(r);
        }
    }

    cout << "========================================" << endl;
    cout << "       MY COMPLETED REQUESTS            " << endl;
    cout << "========================================" << endl;
    if (myCompleted.empty()) {
        cout << "You have no completed requests yet." << endl;
    } else {
        // Display completed requests with index numbers - User Interface
        cout << left << setw(5) << "NO." << setw(12) << "BLOOD" << setw(8) << "UNITS" << setw(10) << "STATUS" << endl;
        cout << string(35, '-') << endl;
        for (size_t i = 0; i < myCompleted.size(); i++) {
            const auto &r = myCompleted[i];
            // Use getter methods - Encapsulation
            cout << left << setw(5) << (i + 1) << setw(12) << r.getBloodGroup() << setw(8) << r.getUnits() 
                 << setw(10) << r.getStatus() << endl;
        }
    }
}

// View pending blood requests for hospitals - Data Display & Filtering
void viewPendingRequest() {
    // Load all blood requests - Data Retrieval
    vector<BloodReqRecord> all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
    vector<BloodReqRecord> pending;
    
    // Get current hospital's inventory - Inventory Management
    auto inventory = loadInventoryFromFile(getInventoryFilePath());
    
    // Filter requests that hospital can fulfill - Business Logic
    for (const auto &r : all) {
        // Use getter methods - Encapsulation
        string statusLower = r.getStatus();
        transform(statusLower.begin(), statusLower.end(), statusLower.begin(), [](unsigned char c) { return tolower(c); });
        
        // Check if request is pending and hospital has sufficient stock
        if (statusLower == "pending" && inventory.count(r.getBloodGroup()) > 0 && 
            inventory[r.getBloodGroup()] >= stoi(r.getUnits())) {
            pending.push_back(r);
        }
    }

    cout << "========================================" << endl;
    cout << "         PENDING BLOOD REQUESTS         " << endl;
    cout << "========================================" << endl;
    cout << "Total Pending Requests: " << pending.size() << endl;
    cout << endl;
    
    if (pending.empty()) {
        cout << "No pending requests that you can fulfill with current stock." << endl;
    } else {
        // Display pending requests with index numbers - User Interface
        cout << left << setw(5) << "NO." << setw(15) << "USERNAME" << setw(12) << "BLOOD" << setw(8) << "UNITS" << setw(10) << "STATUS" << endl;
        cout << string(50, '-') << endl;
        for (size_t i = 0; i < pending.size(); i++) {
            const auto &r = pending[i];
            // Use getter methods - Encapsulation
            cout << left << setw(5) << (i + 1) << setw(15) << r.getUsername() << setw(12) << r.getBloodGroup() 
                 << setw(8) << r.getUnits() << setw(10) << r.getStatus() << endl;
        }
        
        cout << endl;
        cout << "Enter request number to mark as completed (0 to go back): ";
        int choice = readIntLine();
        
        if (choice > 0 && choice <= static_cast<int>(pending.size())) {
            // Mark the selected request as completed - Data Update
            const auto &selectedReq = pending[choice - 1];
            
            // Update inventory (deduct units) - Inventory Management
            inventory[selectedReq.getBloodGroup()] -= stoi(selectedReq.getUnits());
            saveInventoryToFile(inventory, getInventoryFilePath());
            
            // Update request status - Data Processing
            vector<BloodReqRecord> updatedAll;
            for (const auto &req : all) {
                if (req.getUsername() == selectedReq.getUsername() && 
                    req.getBloodGroup() == selectedReq.getBloodGroup() && 
                    req.getUnits() == selectedReq.getUnits() && 
                    req.getStatus() == "pending") {
                    // Create updated request with completed status - Object Modification
                    updatedAll.emplace_back(req.getUsername(), req.getAadharNo(), 
                                           req.getBloodGroup(), req.getUnits(), "completed");
                } else {
                    updatedAll.push_back(req);
                }
            }
            
            // Rewrite the file with updated status - Data Persistence
            ofstream out("textFiles/" + BLOOD_REQ_FILE, ios::trunc);
            if (out.is_open()) {
                for (const auto &req : updatedAll) {
                    // Use getter methods - Encapsulation
                    out << "---" << endl;
                    out << "Username: " << req.getUsername() << endl;
                    out << "Aadhaar No: " << req.getAadharNo() << endl;
                    out << "Blood Group: " << req.getBloodGroup() << endl;
                    out << "Units Required: " << req.getUnits() << endl;
                    out << "Status: " << req.getStatus() << endl;
                }
                out.flush();
                cout << "Request marked as completed! " << stoi(selectedReq.getUnits()) << " units deducted from inventory." << endl;
            }
        }
    }
}

// View completed blood requests for hospitals - Data Display & Filtering
void viewCompletedRequest() {
    // Load all blood requests - Data Retrieval
    vector<BloodReqRecord> all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
    vector<BloodReqRecord> completed;
    
    // Filter completed requests - Data Processing
    for (const auto &r : all) {
        string statusLower = r.getStatus();
        transform(statusLower.begin(), statusLower.end(), statusLower.begin(), [](unsigned char c) { return tolower(c); });
        if (statusLower == "completed") {
            completed.push_back(r);
        }
    }

    cout << "========================================" << endl;
    cout << "        COMPLETED BLOOD REQUESTS        " << endl;
    cout << "========================================" << endl;
    if (completed.empty()) {
        cout << "No completed requests." << endl;
    } else {
        // Display completed requests with index numbers - User Interface
        cout << left << setw(5) << "NO." << setw(15) << "USERNAME" << setw(12) << "BLOOD" << setw(8) << "UNITS" << setw(10) << "STATUS" << endl;
        cout << string(50, '-') << endl;
        for (size_t i = 0; i < completed.size(); i++) {
            const auto &r = completed[i];
            // Use getter methods - Encapsulation
            cout << left << setw(5) << (i + 1) << setw(15) << r.getUsername() << setw(12) << r.getBloodGroup() 
                 << setw(8) << r.getUnits() << setw(10) << r.getStatus() << endl;
        }
    }
}

int main() {
    indexMain();
    return 0;
}
