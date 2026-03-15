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
#include <cstring>

using namespace std;

// =============================================================================
// UTILITY CLASS - Static helper methods (Abstraction & Reusability)
// =============================================================================
class Utils
{
public:
    // Trim whitespace from both ends of a string
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

    // Convert string to uppercase
    static string toUpper(const string &s)
    {
        string out = s;
        transform(out.begin(), out.end(), out.begin(),
                  [](unsigned char c)
                  { return toupper(c); });
        return out;
    }

    // Regex match helper
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

    // Read an integer from a line of stdin
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

    // Validate DD/MM/YYYY date format
    static bool isValidDate(const string &date)
    {
        return regexMatch(date, "^\\d{2}/\\d{2}/\\d{4}$");
    }

    // Split a CSV line into at most (maxFields) parts
    static vector<string> splitCSV(const string &line, int maxFields)
    {
        vector<string> parts;
        size_t start = 0;
        for (int i = 0; i < maxFields - 1; i++)
        {
            size_t pos = line.find(',', start);
            if (pos == string::npos)
                break;
            parts.push_back(trim(line.substr(start, pos - start)));
            start = pos + 1;
        }
        if (start < line.size())
            parts.push_back(trim(line.substr(start)));
        return parts;
    }
};

// =============================================================================
// USER CLASS - Encapsulation & Abstraction
// =============================================================================
class User
{
private:
    string username;
    string aadharNo;
    string password;

public:
    // Constructor - Object Initialization
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

// =============================================================================
// CURRENT USER SESSION CLASS - Singleton Pattern & Encapsulation
// =============================================================================
class CurrentUser
{
private:
    static string username;
    static string aadharNo;

public:
    static void set(const string &uname, const string &aadhar)
    {
        username = uname;
        aadharNo = aadhar;
    }
    static string getUsername() { return username; }
    static string getAadharNo() { return aadharNo; }
    static void clear()
    {
        username.clear();
        aadharNo.clear();
    }
};
string CurrentUser::username = "";
string CurrentUser::aadharNo = "";

// =============================================================================
// HOSPITAL CLASS - Encapsulation & Data Hiding
// =============================================================================
class Hospital
{
private:
    string hospitalId;
    string hospitalName;
    string password;

public:
    // Constructor - Object Initialization
    Hospital(const string &hid, const string &hname, const string &pwd)
        : hospitalId(hid), hospitalName(hname), password(pwd) {}

    // Getters - Data Abstraction
    string getHospitalId() const { return hospitalId; }
    string getHospitalName() const { return hospitalName; }
    string getPassword() const { return password; }
};

// =============================================================================
// CURRENT HOSPITAL SESSION CLASS - Singleton Pattern & Encapsulation
// =============================================================================
class CurrentHospital
{
private:
    static string hospitalId;
    static string hospitalName;

public:
    static void set(const string &hid, const string &hname)
    {
        hospitalId = hid;
        hospitalName = hname;
    }
    static string getHospitalId() { return hospitalId; }
    static string getHospitalName() { return hospitalName; }
    static void clear()
    {
        hospitalId.clear();
        hospitalName.clear();
    }
};
string CurrentHospital::hospitalId = "";
string CurrentHospital::hospitalName = "";

// =============================================================================
// BLOOD REQUEST RECORD CLASS - Data Management & Abstraction
// =============================================================================
class BloodReqRecord
{
private:
    string username;
    string aadharNo;
    string bloodGroup;
    string units;
    string status;

public:
    // Constructor - Object Initialization
    BloodReqRecord(const string &u, const string &a, const string &b,
                   const string &un, const string &st)
        : username(u), aadharNo(a), bloodGroup(b), units(un),
          status(st.empty() ? "pending" : Utils::trim(st)) {}

    // Getters - Data Abstraction
    string getUsername() const { return username; }
    string getAadharNo() const { return aadharNo; }
    string getBloodGroup() const { return bloodGroup; }
    string getUnits() const { return units; }
    string getStatus() const { return status; }

    // Static factory - load all records from file
    static vector<BloodReqRecord> loadAll(const string &path)
    {
        vector<BloodReqRecord> list;
        ifstream in("textFiles/" + path);
        if (!in.is_open())
            return list;

        string username, aadharNo, bloodGroup, units, status = "pending", line;

        auto pushIfValid = [&]()
        {
            if (!username.empty())
                list.emplace_back(username, aadharNo, bloodGroup, units, status);
        };

        while (getline(in, line))
        {
            line = Utils::trim(line);
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
                username = Utils::trim(line.substr(strlen("Username: ")));
            else if (line.rfind("Aadhaar No: ", 0) == 0)
                aadharNo = Utils::trim(line.substr(strlen("Aadhaar No: ")));
            else if (line.rfind("Blood Group: ", 0) == 0)
                bloodGroup = Utils::trim(line.substr(strlen("Blood Group: ")));
            else if (line.rfind("Units Required: ", 0) == 0)
                units = Utils::trim(line.substr(strlen("Units Required: ")));
            else if (line.rfind("Status: ", 0) == 0)
                status = Utils::trim(line.substr(strlen("Status: ")));
        }
        pushIfValid();
        return list;
    }
};

// =============================================================================
// DONOR CLASS - Encapsulation & Data Validation
// =============================================================================
class Donor
{
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
    Donor(const string &n, const string &e, const string &d, const string &bg,
          int a, const string &dis, bool elg, const string &hid, bool completed)
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

    // Setter - Controlled Modification
    void setDonationCompleted(bool completed) { donationCompleted = completed; }

    // Static factory method - Factory Pattern
    static Donor createFromCSV(const vector<string> &parts)
    {
        if (parts.size() == 8)
            return Donor(parts[0], parts[1], parts[2], parts[3],
                         stoi(parts[4]), parts[5], (parts[6] == "1"), parts[7], false);
        return Donor("", "", "", "", 0, "", false, "", false);
    }
};

// =============================================================================
// CAMP CLASS - Event Organization & Encapsulation
// =============================================================================
class Camp
{
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

    // Static factory method - Factory Pattern
    static Camp createFromCSV(const vector<string> &parts)
    {
        if (parts.size() == 4)
            return Camp(parts[0], parts[1], parts[2], parts[3]);
        return Camp("", "", "", "");
    }
};

// =============================================================================
// INVENTORY CLASS - Blood Stock Management (Encapsulation & Data Hiding)
// =============================================================================
class Inventory
{
public:
    // All valid blood groups as a class-level constant
    static const vector<string> VALID_BLOOD_GROUPS;

    // Validate a blood group string
    static bool isValidBloodGroup(const string &group)
    {
        for (const auto &g : VALID_BLOOD_GROUPS)
            if (g == group)
                return true;
        return false;
    }

    // Derive the per-hospital inventory file path
    static string getFilePath()
    {
        const string &hid = CurrentHospital::getHospitalId();
        if (hid.empty())
            return "textFiles/inventory.txt";
        return "textFiles/inventory_" + hid + ".txt";
    }

    // Load inventory map from file - File I/O
    static map<string, int> loadFromFile(const string &filename)
    {
        map<string, int> inv;
        for (const auto &g : VALID_BLOOD_GROUPS)
            inv[g] = 0;

        ifstream in(filename);
        if (!in.is_open())
            return inv;

        string line;
        while (getline(in, line))
        {
            line = Utils::trim(line);
            if (line.empty())
                continue;
            size_t sep = line.find(',');
            if (sep == string::npos)
                continue;
            string group = Utils::trim(line.substr(0, sep));
            string unitsStr = Utils::trim(line.substr(sep + 1));
            if (!isValidBloodGroup(group))
                continue;
            try
            {
                int u = stoi(unitsStr);
                if (u >= 0)
                    inv[group] = u;
            }
            catch (...)
            {
            }
        }
        return inv;
    }

    // Save inventory map to file - Data Persistence
    static bool saveToFile(const map<string, int> &inv, const string &filename)
    {
        ofstream out(filename, ios::trunc);
        if (!out.is_open())
        {
            cout << "[ERROR] Could not save to " << filename << ": could not open file" << endl;
            return false;
        }
        for (const auto &g : VALID_BLOOD_GROUPS)
            out << g << "," << inv.at(g) << endl;
        out.flush();
        return true;
    }

    // Display inventory table
    static void display()
    {
        auto inv = loadFromFile(getFilePath());
        cout << "========================================" << endl;
        cout << "             BLOOD INVENTORY            " << endl;
        cout << "========================================" << endl;
        cout << left << setw(5) << "NO." << setw(10) << "BLOOD" << setw(10) << "UNITS" << endl;
        cout << string(25, '-') << endl;
        int index = 1;
        for (const auto &g : VALID_BLOOD_GROUPS)
        {
            cout << left << setw(5) << index << setw(10) << g
                 << setw(10) << inv[g] << " units" << endl;
            index++;
        }
    }

    // Interactive inventory management menu
    static void menu()
    {
        while (true)
        {
            cout << "========================================" << endl;
            cout << "           INVENTORY MANAGEMENT        " << endl;
            cout << "========================================" << endl;
            cout << "1. Add Stock" << endl;
            cout << "2. Remove Stock" << endl;
            cout << "3. View Inventory" << endl;
            cout << "4. Back" << endl;
            cout << "Select an option (1-4): ";
            int choice = Utils::readIntLine();
            cout << endl;

            if (choice == 1)
            {
                string group, unitsStr;
                cout << "Enter blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ";
                getline(cin, group);
                group = Utils::trim(Utils::toUpper(group));
                if (!isValidBloodGroup(group))
                {
                    cout << "Invalid blood group." << endl;
                    continue;
                }
                cout << "Enter units to add: ";
                getline(cin, unitsStr);
                unitsStr = Utils::trim(unitsStr);
                if (!Utils::regexMatch(unitsStr, "^\\d+$"))
                {
                    cout << "Units must be a positive whole number." << endl;
                    continue;
                }
                int units = stoi(unitsStr);
                if (units <= 0)
                {
                    cout << "Units must be greater than 0." << endl;
                    continue;
                }
                auto inv = loadFromFile(getFilePath());
                inv[group] += units;
                if (saveToFile(inv, getFilePath()))
                    cout << units << " units added to " << group << "." << endl;
            }
            else if (choice == 2)
            {
                string group, unitsStr;
                cout << "Enter blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ";
                getline(cin, group);
                group = Utils::trim(Utils::toUpper(group));
                if (!isValidBloodGroup(group))
                {
                    cout << "Invalid blood group." << endl;
                    continue;
                }
                cout << "Enter units to remove: ";
                getline(cin, unitsStr);
                unitsStr = Utils::trim(unitsStr);
                if (!Utils::regexMatch(unitsStr, "^\\d+$"))
                {
                    cout << "Units must be a positive whole number." << endl;
                    continue;
                }
                int units = stoi(unitsStr);
                if (units <= 0)
                {
                    cout << "Units must be greater than 0." << endl;
                    continue;
                }
                auto inv = loadFromFile(getFilePath());
                if (inv[group] < units)
                {
                    cout << "Not enough stock available for " << group << "." << endl;
                    continue;
                }
                inv[group] -= units;
                if (saveToFile(inv, getFilePath()))
                    cout << units << " units removed from " << group << "." << endl;
            }
            else if (choice == 3)
            {
                display();
            }
            else if (choice == 4)
            {
                return;
            }
            else
            {
                cout << "Invalid choice. Please select 1-4." << endl;
            }
            cout << "Press Enter to continue..." << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
};

// Static member definition
const vector<string> Inventory::VALID_BLOOD_GROUPS = {"A+", "A-", "B+", "B-", "O+", "O-", "AB+", "AB-"};

// =============================================================================
// FILE REPOSITORY CLASS - File I/O for Users, Hospitals, Donors, Camps
//                         (Single Responsibility & Abstraction)
// =============================================================================
class FileRepository
{
public:
    // ---------- USER FILE OPERATIONS ----------

    static vector<User> loadUsers(const string &filename)
    {
        vector<User> users;
        ifstream in("textFiles/" + filename);
        if (!in.is_open())
        {
            cout << "[ERROR] File '" << filename << "' not found!" << endl;
            return users;
        }
        string line;
        while (getline(in, line))
        {
            line = Utils::trim(line);
            if (line.empty())
                continue;
            auto parts = Utils::splitCSV(line, 3);
            if (parts.size() == 3)
                users.push_back(User(parts[0], parts[1], parts[2]));
        }
        return users;
    }

    static bool saveUser(const string &username, const string &aadharNo, const string &password)
    {
        ofstream out("textFiles/users.txt", ios::app);
        if (!out.is_open())
        {
            cout << "[Error] Data could not be saved: could not open users.txt" << endl;
            return false;
        }
        out << username << "," << aadharNo << "," << password << "\n";
        out.flush();
        return true;
    }

    static bool aadharExists(const string &aadharNo)
    {
        ifstream in("textFiles/users.txt");
        if (!in.is_open())
            return false;
        string line;
        while (getline(in, line))
        {
            line = Utils::trim(line);
            if (line.empty())
                continue;
            auto parts = Utils::splitCSV(line, 3);
            if (parts.size() > 1 && parts[1] == aadharNo)
                return true;
        }
        return false;
    }

    // ---------- HOSPITAL FILE OPERATIONS ----------

    static vector<Hospital> loadHospitals(const string &filename)
    {
        vector<Hospital> hospitals;
        ifstream in("textFiles/" + filename);
        if (!in.is_open())
        {
            cout << "[ERROR] File '" << filename << "' not found!" << endl;
            return hospitals;
        }
        string line;
        while (getline(in, line))
        {
            line = Utils::trim(line);
            if (line.empty())
                continue;
            auto parts = Utils::splitCSV(line, 3);
            if (parts.size() == 3)
                hospitals.push_back(Hospital(parts[0], parts[1], parts[2]));
        }
        return hospitals;
    }

    static bool saveHospital(const string &hospitalId, const string &hospitalName, const string &password)
    {
        ofstream out("textFiles/hospitals.txt", ios::app);
        if (!out.is_open())
        {
            cout << "[Error] Data could not be saved: could not open hospitals.txt" << endl;
            return false;
        }
        out << hospitalId << "," << hospitalName << "," << password << "\n";
        out.flush();
        return true;
    }

    static bool hospitalExists(const string &hospitalId)
    {
        ifstream in("textFiles/hospitals.txt");
        if (!in.is_open())
            return false;
        string line;
        while (getline(in, line))
        {
            line = Utils::trim(line);
            if (line.empty())
                continue;
            auto parts = Utils::splitCSV(line, 3);
            if (!parts.empty() && parts[0] == hospitalId)
                return true;
        }
        return false;
    }

    // ---------- DONOR FILE OPERATIONS ----------

    static vector<Donor> loadDonors(const string &filename)
    {
        vector<Donor> donors;
        ifstream in("textFiles/" + filename);
        if (!in.is_open())
        {
            cout << "[ERROR] Could not open " << filename << endl;
            return donors;
        }
        string line;
        while (getline(in, line))
        {
            line = Utils::trim(line);
            if (line.empty())
                continue;
            auto parts = Utils::splitCSV(line, 8);
            Donor d = Donor::createFromCSV(parts);
            if (!d.getName().empty())
                donors.push_back(d);
        }
        return donors;
    }

    static bool saveDonor(const Donor &donor, const string &filename)
    {
        ofstream out("textFiles/" + filename, ios::app);
        if (!out.is_open())
        {
            cout << "[ERROR] Could not save to " << filename << ": could not open file" << endl;
            return false;
        }
        out << donor.getName() << "," << donor.getEmail() << ","
            << donor.getDob() << "," << donor.getBloodGroup() << ","
            << donor.getAge() << "," << donor.getDisease() << ","
            << (donor.isEligible() ? "1" : "0") << ","
            << donor.getHospitalId() << endl;
        out.flush();
        return true;
    }

    static bool rewriteDonors(const vector<Donor> &donors, const string &filename)
    {
        ofstream out("textFiles/" + filename, ios::trunc);
        if (!out.is_open())
            return false;
        for (const auto &d : donors)
        {
            out << d.getName() << "," << d.getEmail() << ","
                << d.getDob() << "," << d.getBloodGroup() << ","
                << d.getAge() << "," << d.getDisease() << ","
                << (d.isEligible() ? "1" : "0") << ","
                << d.getHospitalId() << endl;
        }
        out.flush();
        return true;
    }

    // ---------- CAMP FILE OPERATIONS ----------

    static vector<Camp> loadCamps(const string &filename)
    {
        vector<Camp> camps;
        ifstream in("textFiles/" + filename);
        if (!in.is_open())
            return camps;
        string line;
        while (getline(in, line))
        {
            line = Utils::trim(line);
            if (line.empty())
                continue;
            auto parts = Utils::splitCSV(line, 4);
            Camp c = Camp::createFromCSV(parts);
            if (!c.getName().empty())
                camps.push_back(c);
        }
        return camps;
    }

    static bool saveCamp(const Camp &camp, const string &filename)
    {
        ofstream out("textFiles/" + filename, ios::app);
        if (!out.is_open())
        {
            cout << "[ERROR] Could not save to " << filename << ": could not open file" << endl;
            return false;
        }
        out << camp.getName() << "," << camp.getDate() << ","
            << camp.getVenue() << "," << camp.getCity() << endl;
        out.flush();
        return true;
    }

    // ---------- BLOOD REQUEST FILE OPERATIONS ----------

    static bool saveBloodRequest(const string &username, const string &aadharNo,
                                 const string &bloodGroup, const string &units,
                                 const string &reqFile)
    {
        ofstream out("textFiles/" + reqFile, ios::app);
        if (!out.is_open())
        {
            cout << "[ERROR] Could not save to " << reqFile << ": could not open file" << endl;
            return false;
        }
        out << "---" << endl;
        out << "Username: " << username << endl;
        out << "Aadhaar No: " << aadharNo << endl;
        out << "Blood Group: " << bloodGroup << endl;
        out << "Units Required: " << units << endl;
        out << "Status: pending" << endl;
        out.flush();
        return true;
    }

    static bool rewriteBloodRequests(const vector<BloodReqRecord> &records, const string &reqFile)
    {
        ofstream out("textFiles/" + reqFile, ios::trunc);
        if (!out.is_open())
            return false;
        for (const auto &r : records)
        {
            out << "---" << endl;
            out << "Username: " << r.getUsername() << endl;
            out << "Aadhaar No: " << r.getAadharNo() << endl;
            out << "Blood Group: " << r.getBloodGroup() << endl;
            out << "Units Required: " << r.getUnits() << endl;
            out << "Status: " << r.getStatus() << endl;
        }
        out.flush();
        return true;
    }
};

// =============================================================================
// DONOR MANAGER CLASS - Donor CRUD & Display (Encapsulation & SRP)
// =============================================================================
class DonorManager
{
private:
    static const string DONORS_FILE;

public:
    // Add a new donor interactively
    static void addDonor()
    {
        string name, email, dob, bloodGroup, disease;
        int age = 0;

        cout << "========================================" << endl;
        cout << "          ADD NEW DONOR               " << endl;
        cout << "========================================" << endl;

        cout << "Enter Donor Name: ";
        getline(cin, name);
        name = Utils::trim(name);

        cout << "Enter Email: ";
        getline(cin, email);
        email = Utils::trim(email);

        while (true)
        {
            cout << "Enter Date of Birth (DD/MM/YYYY): ";
            getline(cin, dob);
            dob = Utils::trim(dob);
            if (Utils::isValidDate(dob))
                break;
            cout << "Invalid date format. Please use DD/MM/YYYY format." << endl;
        }

        cout << "Enter Blood Group: ";
        getline(cin, bloodGroup);
        bloodGroup = Utils::trim(Utils::toUpper(bloodGroup));

        cout << "========================================" << endl;
        cout << "        HEALTH DECLARATION           " << endl;
        cout << "========================================" << endl;

        while (true)
        {
            cout << "Enter Age: ";
            string ageStr;
            getline(cin, ageStr);
            ageStr = Utils::trim(ageStr);
            try
            {
                age = stoi(ageStr);
                if (age < 18)
                {
                    cout << "Not eligible (Age must be 18 or above)" << endl;
                    return;
                }
                break;
            }
            catch (...)
            {
                cout << "Invalid age. Please enter a valid number." << endl;
            }
        }

        cout << "Any Serious Disease? (Yes/No): ";
        getline(cin, disease);
        disease = Utils::trim(disease);

        // Create donor object - Constructor Usage
        Donor newDonor(name, email, dob, bloodGroup, age, disease,
                       (disease != "Yes" && disease != "YES"),
                       CurrentHospital::getHospitalId(), false);

        if (disease == "Yes" || disease == "YES")
        {
            cout << "Sorry, donor NOT eligible for donation" << endl;
            return;
        }

        if (FileRepository::saveDonor(newDonor, DONORS_FILE))
        {
            cout << "========================================" << endl;
            cout << "    Thank you " << name << endl;
            cout << "    Happy donation" << endl;
            cout << "========================================" << endl;
        }
    }

    // View donors for current hospital with mark-as-completed option
    static void viewDonors()
    {
        auto donors = FileRepository::loadDonors(DONORS_FILE);
        vector<Donor> hospitalDonors;
        for (const auto &d : donors)
            if (d.getHospitalId() == CurrentHospital::getHospitalId())
                hospitalDonors.push_back(d);

        cout << "========================================" << endl;
        cout << "          HOSPITAL DONOR LIST            " << endl;
        cout << "========================================" << endl;

        if (hospitalDonors.empty())
        {
            cout << "No donors found for your hospital." << endl;
            return;
        }

        cout << left << setw(5) << "NO." << setw(20) << "NAME"
             << setw(25) << "EMAIL" << setw(12) << "DOB"
             << setw(10) << "BLOOD" << setw(6) << "AGE"
             << setw(15) << "DISEASE" << setw(10) << "STATUS" << endl;
        cout << string(103, '-') << endl;

        for (size_t i = 0; i < hospitalDonors.size(); i++)
        {
            const auto &d = hospitalDonors[i];
            cout << left << setw(5) << (i + 1)
                 << setw(20) << d.getName() << setw(25) << d.getEmail()
                 << setw(12) << d.getDob() << setw(10) << d.getBloodGroup()
                 << setw(6) << d.getAge()
                 << setw(15) << (d.getDisease() == "Yes" || d.getDisease() == "YES" ? "Yes" : "No")
                 << setw(10) << (d.isDonationCompleted() ? "Completed" : "Pending") << endl;
        }

        // Show marking option if pending donations exist
        bool hasPending = false;
        for (const auto &d : hospitalDonors)
            if (!d.isDonationCompleted())
            {
                hasPending = true;
                break;
            }

        if (hasPending)
        {
            cout << endl;
            cout << "Enter donor number to mark as completed (0 to go back): ";
            int choice = Utils::readIntLine();
            if (choice > 0 && choice <= static_cast<int>(hospitalDonors.size()))
            {
                const auto &sel = hospitalDonors[choice - 1];
                if (!sel.isDonationCompleted())
                {
                    vector<Donor> updated;
                    for (const auto &d : donors)
                    {
                        if (d.getEmail() == sel.getEmail() && d.getHospitalId() == sel.getHospitalId())
                            updated.push_back(Donor(d.getName(), d.getEmail(), d.getDob(),
                                                    d.getBloodGroup(), d.getAge(), d.getDisease(),
                                                    d.isEligible(), d.getHospitalId(), true));
                        else
                            updated.push_back(d);
                    }
                    if (FileRepository::rewriteDonors(updated, DONORS_FILE))
                        cout << "Donation marked as completed!" << endl;
                }
                else
                {
                    cout << "This donation is already completed." << endl;
                }
            }
        }
    }

    // Donor management sub-menu
    static void menu()
    {
        while (true)
        {
            cout << "========================================" << endl;
            cout << "          DONOR MANAGEMENT             " << endl;
            cout << "========================================" << endl;
            cout << "1. Add Donor" << endl;
            cout << "2. View Donor List" << endl;
            cout << "3. Back" << endl;
            cout << "Select an option (1-3): ";
            int choice = Utils::readIntLine();
            cout << endl;

            switch (choice)
            {
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
            if (choice != 3)
            {
                cout << "Press Enter to continue..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
    }
};
const string DonorManager::DONORS_FILE = "donors.txt";

// =============================================================================
// CAMP MANAGER CLASS - Blood Camp CRUD & Display (Encapsulation & SRP)
// =============================================================================
class CampManager
{
private:
    static const string CAMPS_FILE;

public:
    // Display all available camps
    static void viewCamps()
    {
        auto camps = FileRepository::loadCamps(CAMPS_FILE);

        cout << "========================================" << endl;
        cout << "            AVAILABLE CAMPS              " << endl;
        cout << "========================================" << endl;

        if (camps.empty())
        {
            cout << "No camps are currently available." << endl;
            return;
        }

        cout << left << setw(5) << "NO." << setw(25) << "CAMP NAME"
             << setw(12) << "DATE" << setw(20) << "VENUE"
             << setw(15) << "CITY" << endl;
        cout << string(77, '-') << endl;

        for (size_t i = 0; i < camps.size(); i++)
        {
            const auto &c = camps[i];
            cout << left << setw(5) << (i + 1) << setw(25) << c.getName()
                 << setw(12) << c.getDate() << setw(20) << c.getVenue()
                 << setw(15) << c.getCity() << endl;
        }
    }

    // Organise a new blood camp
    static void organiseCamp()
    {
        string name, date, venue, city;

        cout << "========================================" << endl;
        cout << "          ORGANISE BLOOD CAMP           " << endl;
        cout << "========================================" << endl;

        cout << "Enter Camp Name: ";
        getline(cin, name);

        while (true)
        {
            cout << "Enter Date (DD/MM/YYYY): ";
            getline(cin, date);
            date = Utils::trim(date);
            if (Utils::isValidDate(date))
                break;
            cout << "Invalid date format. Please use DD/MM/YYYY format." << endl;
        }

        cout << "Enter Venue: ";
        getline(cin, venue);
        cout << "Enter City: ";
        getline(cin, city);

        // Create camp object - Constructor Usage
        Camp camp(name, date, venue, city);

        if (FileRepository::saveCamp(camp, CAMPS_FILE))
            cout << "Blood camp saved to " << CAMPS_FILE << endl;
    }

    // Show ongoing camps (alias for viewCamps)
    static void viewOngoingCamps()
    {
        cout << "========================================" << endl;
        cout << "           ONGOING BLOOD CAMPS          " << endl;
        cout << "========================================" << endl;
        viewCamps();
    }
};
const string CampManager::CAMPS_FILE = "camps.txt";

// =============================================================================
// BLOOD REQUEST MANAGER CLASS - Request Handling (Encapsulation & SRP)
// =============================================================================
class BloodRequestManager
{
private:
    static const string BLOOD_REQ_FILE;

public:
    // Submit a new blood request
    static void requestBlood()
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
            cout << aadharNo.substr(aadharNo.size() - 4);
        else
            cout << aadharNo;
        cout << ")" << endl
             << endl;

        string group;
        while (true)
        {
            cout << "Enter required blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ";
            getline(cin, group);
            group = Utils::trim(Utils::toUpper(group));
            if (Utils::regexMatch(group, "^(A|B|AB|O)[+-]$"))
                break;
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
            units = Utils::trim(units);
            if (!Utils::regexMatch(units, "^\\d+$"))
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
        FileRepository::saveBloodRequest(username, aadharNo, group, units, BLOOD_REQ_FILE);
        cout << "Request saved to BloodReq.txt" << endl;
        cout << "Press Enter to return to the User Menu..." << endl;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    // View all requests for the current user
    static void viewMyRequests()
    {
        string username = CurrentUser::getUsername();
        if (username.empty())
        {
            cout << "[ERROR] No user logged in. Please log in first." << endl;
            return;
        }
        auto all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
        vector<BloodReqRecord> mine;
        for (const auto &r : all)
            if (username == r.getUsername())
                mine.push_back(r);

        cout << "========================================" << endl;
        cout << "         MY BLOOD REQUESTS               " << endl;
        cout << "========================================" << endl;
        if (mine.empty())
        {
            cout << "You have no blood requests yet." << endl;
        }
        else
        {
            cout << left << setw(5) << "NO." << setw(12) << "BLOOD"
                 << setw(8) << "UNITS" << setw(10) << "STATUS" << endl;
            cout << string(35, '-') << endl;
            for (size_t i = 0; i < mine.size(); i++)
            {
                const auto &r = mine[i];
                cout << left << setw(5) << (i + 1) << setw(12) << r.getBloodGroup()
                     << setw(8) << r.getUnits() << setw(10) << r.getStatus() << endl;
            }
        }
    }

    // View completed requests for current user
    static void viewMyCompletedRequests()
    {
        string username = CurrentUser::getUsername();
        if (username.empty())
        {
            cout << "[ERROR] No user logged in. Please log in first." << endl;
            return;
        }
        auto all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
        vector<BloodReqRecord> myCompleted;
        for (const auto &r : all)
            if (username == r.getUsername() && r.getStatus() == "completed")
                myCompleted.push_back(r);

        cout << "========================================" << endl;
        cout << "       MY COMPLETED REQUESTS            " << endl;
        cout << "========================================" << endl;
        if (myCompleted.empty())
        {
            cout << "You have no completed requests yet." << endl;
        }
        else
        {
            cout << left << setw(5) << "NO." << setw(12) << "BLOOD"
                 << setw(8) << "UNITS" << setw(10) << "STATUS" << endl;
            cout << string(35, '-') << endl;
            for (size_t i = 0; i < myCompleted.size(); i++)
            {
                const auto &r = myCompleted[i];
                cout << left << setw(5) << (i + 1) << setw(12) << r.getBloodGroup()
                     << setw(8) << r.getUnits() << setw(10) << r.getStatus() << endl;
            }
        }
    }

    // View + fulfill pending requests (hospital side)
    static void viewPendingRequest()
    {
        auto all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
        auto inventory = Inventory::loadFromFile(Inventory::getFilePath());

        vector<BloodReqRecord> pending;
        for (const auto &r : all)
        {
            string statusLower = r.getStatus();
            transform(statusLower.begin(), statusLower.end(), statusLower.begin(),
                      [](unsigned char c)
                      { return tolower(c); });
            if (statusLower == "pending" &&
                inventory.count(r.getBloodGroup()) > 0 &&
                inventory[r.getBloodGroup()] >= stoi(r.getUnits()))
                pending.push_back(r);
        }

        cout << "========================================" << endl;
        cout << "         PENDING BLOOD REQUESTS         " << endl;
        cout << "========================================" << endl;
        cout << "Total Pending Requests: " << pending.size() << endl
             << endl;

        if (pending.empty())
        {
            cout << "No pending requests that you can fulfill with current stock." << endl;
            return;
        }

        cout << left << setw(5) << "NO." << setw(15) << "USERNAME"
             << setw(12) << "BLOOD" << setw(8) << "UNITS"
             << setw(10) << "STATUS" << endl;
        cout << string(50, '-') << endl;
        for (size_t i = 0; i < pending.size(); i++)
        {
            const auto &r = pending[i];
            cout << left << setw(5) << (i + 1) << setw(15) << r.getUsername()
                 << setw(12) << r.getBloodGroup() << setw(8) << r.getUnits()
                 << setw(10) << r.getStatus() << endl;
        }

        cout << endl;
        cout << "Enter request number to mark as completed (0 to go back): ";
        int choice = Utils::readIntLine();

        if (choice > 0 && choice <= static_cast<int>(pending.size()))
        {
            const auto &sel = pending[choice - 1];
            inventory[sel.getBloodGroup()] -= stoi(sel.getUnits());
            Inventory::saveToFile(inventory, Inventory::getFilePath());

            // Update request status in full list
            vector<BloodReqRecord> updatedAll;
            bool alreadyMarked = false;
            for (const auto &req : all)
            {
                if (!alreadyMarked &&
                    req.getUsername() == sel.getUsername() &&
                    req.getBloodGroup() == sel.getBloodGroup() &&
                    req.getUnits() == sel.getUnits() &&
                    req.getStatus() == "pending")
                {
                    updatedAll.emplace_back(req.getUsername(), req.getAadharNo(),
                                            req.getBloodGroup(), req.getUnits(), "completed");
                    alreadyMarked = true;
                }
                else
                {
                    updatedAll.push_back(req);
                }
            }
            if (FileRepository::rewriteBloodRequests(updatedAll, BLOOD_REQ_FILE))
                cout << "Request marked as completed! "
                     << stoi(sel.getUnits()) << " units deducted from inventory." << endl;
        }
    }

    // View all completed requests (hospital side)
    static void viewCompletedRequest()
    {
        auto all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
        vector<BloodReqRecord> completed;
        for (const auto &r : all)
        {
            string sl = r.getStatus();
            transform(sl.begin(), sl.end(), sl.begin(), [](unsigned char c)
                      { return tolower(c); });
            if (sl == "completed")
                completed.push_back(r);
        }

        cout << "========================================" << endl;
        cout << "        COMPLETED BLOOD REQUESTS        " << endl;
        cout << "========================================" << endl;
        if (completed.empty())
        {
            cout << "No completed requests." << endl;
            return;
        }
        cout << left << setw(5) << "NO." << setw(15) << "USERNAME"
             << setw(12) << "BLOOD" << setw(8) << "UNITS"
             << setw(10) << "STATUS" << endl;
        cout << string(50, '-') << endl;
        for (size_t i = 0; i < completed.size(); i++)
        {
            const auto &r = completed[i];
            cout << left << setw(5) << (i + 1) << setw(15) << r.getUsername()
                 << setw(12) << r.getBloodGroup() << setw(8) << r.getUnits()
                 << setw(10) << r.getStatus() << endl;
        }
    }
};
const string BloodRequestManager::BLOOD_REQ_FILE = "BloodReq.txt";

// =============================================================================
// REGISTRATION MANAGER CLASS - User & Hospital Registration (Encapsulation & SRP)
// =============================================================================

// Forward declarations needed for cross-calls between menus
void userLogIn();
void hospLogIn();

class RegistrationManager
{
public:
    // Register a new user account
    static void registerUser()
    {
        cout << "=========================================" << endl;
        cout << "        REGISTER NEW USER ACCOUNT        " << endl;
        cout << "=========================================" << endl
             << endl;

        while (true)
        {
            cout << "Enter username: ";
            string username;
            getline(cin, username);
            username = Utils::trim(username);

            cout << "Enter Aadhar No (exactly 12 digits, no spaces): ";
            string aadharNo;
            getline(cin, aadharNo);
            aadharNo = Utils::trim(aadharNo);

            if (!Utils::regexMatch(aadharNo, "^\\d{12}$"))
            {
                cout << endl;
                cout << "[ERROR] Invalid Aadhar number!" << endl;
                cout << "- Must be EXACTLY 12 digits" << endl;
                cout << "- Only numbers (0-9), no spaces or letters" << endl;
                cout << "- Examples: 123456789012 ✓ | 123 456 789 012 ✗" << endl
                     << endl;
                continue;
            }

            cout << "Enter password: ";
            string password;
            getline(cin, password);
            password = Utils::trim(password);

            if (FileRepository::aadharExists(aadharNo))
            {
                cout << endl;
                cout << "[ERROR] Aadhar number already exists!" << endl;
                cout << "This Aadhar is already registered." << endl
                     << endl;
                cout << "1. Try Again" << endl;
                cout << "2. Log In" << endl;
                int choice = Utils::readIntLine();
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

            if (FileRepository::saveUser(username, aadharNo, password))
            {
                cout << "User saved successfully." << endl;
                cout << "Please log in with your new credentials." << endl;
                userLogIn();
                break;
            }
        }
    }

    // Register a new hospital account
    static void registerHospital()
    {
        cout << "=========================================" << endl;
        cout << "      REGISTER NEW HOSPITAL ACCOUNT      " << endl;
        cout << "=========================================" << endl
             << endl;

        while (true)
        {
            cout << "Enter hospital name: ";
            string hospitalName;
            getline(cin, hospitalName);
            hospitalName = Utils::trim(hospitalName);

            while (true)
            {
                cout << "Enter hospital ID (exactly 10 digits): ";
                string hospitalId;
                getline(cin, hospitalId);
                hospitalId = Utils::trim(hospitalId);

                if (!Utils::regexMatch(hospitalId, "^\\d{10}$"))
                {
                    cout << endl;
                    cout << "[ERROR] Invalid Hospital ID!" << endl;
                    cout << "- Must be EXACTLY 10 digits" << endl;
                    cout << "- Only numbers (0-9), no spaces or letters" << endl;
                    cout << "- Examples: 1234567890 ✓ | 123 456 789 0 ✗" << endl
                         << endl;
                    continue;
                }

                cout << "Enter password: ";
                string password;
                getline(cin, password);
                password = Utils::trim(password);

                if (FileRepository::hospitalExists(hospitalId))
                {
                    cout << endl;
                    cout << "[ERROR] Hospital ID already exists!" << endl;
                    cout << "Please choose a different Hospital ID or log in." << endl
                         << endl;
                    cout << "1. Try Again" << endl;
                    cout << "2. Log In" << endl;
                    int choice = Utils::readIntLine();
                    switch (choice)
                    {
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

                if (FileRepository::saveHospital(hospitalId, hospitalName, password))
                {
                    cout << "Hospital saved successfully." << endl;
                    cout << "Please log in with your new credentials." << endl;
                    hospLogIn();
                    return;
                }
            }
        }
    }
};

// =============================================================================
// APPLICATION CLASS - Top-level navigation & menus (Encapsulation & SRP)
// =============================================================================
class Application
{
public:
    // Main entry point
    static void indexMain()
    {
        while (true)
        {
            cout << "========================================" << endl;
            cout << "        WELCOME TO SANGUINE SYNC        " << endl;
            cout << "========================================" << endl
                 << endl;
            cout << "Please Log In or Create an Account to Continue" << endl;
            cout << "1. New User Account" << endl;
            cout << "2. New Hospital Account" << endl;
            cout << "3. Log In" << endl;
            cout << "4. Exit System" << endl
                 << endl;
            cout << "Select an option (1-4): ";
            int choice = Utils::readIntLine();
            cout << endl;

            switch (choice)
            {
            case 1:
                RegistrationManager::registerUser();
                break;
            case 2:
                RegistrationManager::registerHospital();
                break;
            case 3:
                cout << "========================================" << endl;
                logInPage();
                return;
            case 4:
                cout << "Exiting system..." << endl;
                exit(0);
            default:
                cout << "Invalid choice. Please select 1, 2, 3, or 4." << endl;
                cout << "Try again." << endl;
                continue;
            }
        }
    }

    // Login type selection
    static void logInPage()
    {
        cout << endl;
        cout << "       Who is loging in?       " << endl;
        cout << "1. User" << endl;
        cout << "2. Hospital" << endl
             << endl;
        cout << "Select an option (1-2): ";
        int choice = Utils::readIntLine();
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

    // User menu
    static void userMenu()
    {
        while (true)
        {
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
            int choice = Utils::readIntLine();
            cout << endl;

            switch (choice)
            {
            case 1:
                BloodRequestManager::requestBlood();
                break;
            case 2:
                BloodRequestManager::viewMyRequests();
                break;
            case 3:
                BloodRequestManager::viewMyCompletedRequests();
                cout << "Press Enter to return to the User Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 4:
                CampManager::viewCamps();
                cout << "Press Enter to return to the User Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 5:
                Inventory::display();
                cout << "Press Enter to return to the User Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 6:
                CurrentUser::clear();
                cout << "Returning to main page..." << endl;
                indexMain();
                return;
            default:
                cout << "Invalid choice. Please select 1, 2, 3, 4, 5, or 6." << endl
                     << endl;
                break;
            }
        }
    }

    // Hospital menu
    static void hospMenu()
    {
        while (true)
        {
            // Count fulfillable pending requests - Business Logic
            auto all = BloodReqRecord::loadAll("BloodReq.txt");
            auto inventory = Inventory::loadFromFile(Inventory::getFilePath());
            vector<BloodReqRecord> pending;
            for (const auto &r : all)
            {
                string sl = r.getStatus();
                transform(sl.begin(), sl.end(), sl.begin(), [](unsigned char c)
                          { return tolower(c); });
                if (sl == "pending" &&
                    inventory.count(r.getBloodGroup()) > 0 &&
                    inventory[r.getBloodGroup()] >= stoi(r.getUnits()))
                    pending.push_back(r);
            }

            cout << "========================================" << endl;
            cout << "            HOSPITAL MENU               " << endl;
            cout << "========================================" << endl;
            cout << "Pending Requests: " << pending.size() << endl
                 << endl;
            cout << "1. View Pending Request" << endl;
            cout << "2. View Completed Request" << endl;
            cout << "3. Organise Blood Camp" << endl;
            cout << "4. View Ongoing Camps" << endl;
            cout << "5. Donor Management" << endl;
            cout << "6. Inventory Management" << endl;
            cout << "7. Exit" << endl;
            cout << "Select an option (1-7): ";
            int choice = Utils::readIntLine();
            cout << endl;

            switch (choice)
            {
            case 1:
                BloodRequestManager::viewPendingRequest();
                cout << "Press Enter to return to the Hospital Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 2:
                BloodRequestManager::viewCompletedRequest();
                cout << "Press Enter to return to the Hospital Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 3:
                CampManager::organiseCamp();
                cout << "Press Enter to return to the Hospital Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 4:
                CampManager::viewOngoingCamps();
                cout << "Press Enter to return to the Hospital Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 5:
                DonorManager::menu();
                break;
            case 6:
                Inventory::menu();
                break;
            case 7:
                cout << "Returning to main page..." << endl;
                indexMain();
                return;
            default:
                cout << "Invalid choice. Please select 1-7." << endl
                     << endl;
                break;
            }
        }
    }
};

// =============================================================================
// AUTH MANAGER CLASS - Login Authentication (Encapsulation & SRP)
// =============================================================================
class AuthManager
{
public:
    // User login flow
    static void userLogIn()
    {
        while (true)
        {
            auto users = FileRepository::loadUsers("users.txt");

            if (users.empty())
            {
                cout << "[ERROR] No users found in file or file error!" << endl;
                cout << "Please register first." << endl;
                RegistrationManager::registerUser();
                return;
            }

            cout << "=========================================" << endl;
            cout << "            ENTER CREDENTIALS            " << endl;
            cout << "=========================================" << endl
                 << endl;

            cout << "Enter Username: ";
            string inputUsername;
            getline(cin, inputUsername);
            inputUsername = Utils::trim(inputUsername);

            cout << "Enter Password: ";
            string inputPassword;
            getline(cin, inputPassword);
            inputPassword = Utils::trim(inputPassword);

            // Authenticate - compare against all loaded User objects
            bool userFound = false;
            string foundAadhar = "";
            for (const auto &u : users)
            {
                if (u.getUsername() == inputUsername && u.getPassword() == inputPassword)
                {
                    userFound = true;
                    foundAadhar = u.getAadharNo();
                    break;
                }
            }

            if (!userFound)
            {
                cout << endl;
                cout << "[ERROR] Invalid credentials!" << endl;
                cout << "Username and Password must match." << endl;
                cout << "Please check and try again." << endl;
                cout << "OR register first." << endl;
                cout << "1. Try Again" << endl;
                cout << "2. Register New User" << endl;
                cout << "3. Back to Main Menu" << endl;
                cout << "Select an option (1-3): ";
                int choice = Utils::readIntLine();
                switch (choice)
                {
                case 1:
                    continue;
                case 2:
                    RegistrationManager::registerUser();
                    return;
                case 3:
                    return;
                default:
                    cout << "Invalid choice. Returning to main menu." << endl;
                    return;
                }
            }
            else
            {
                // Mask Aadhar for display - Data Privacy
                string maskedAadhar = "********";
                if (foundAadhar.size() >= 4)
                    maskedAadhar += foundAadhar.substr(foundAadhar.size() - 4);
                else
                    maskedAadhar += foundAadhar;

                cout << endl;
                cout << "=========================================" << endl;
                cout << "           LOGIN SUCCESSFUL!            " << endl;
                cout << "=========================================" << endl;
                cout << "Welcome, " << inputUsername << "!" << endl;
                cout << "Aadhar: " << maskedAadhar << endl;
                cout << "You have successfully signed in." << endl
                     << endl;

                CurrentUser::set(inputUsername, foundAadhar);
                Application::userMenu();
                return;
            }
        }
    }

    // Hospital login flow
    static void hospLogIn()
    {
        while (true)
        {
            auto hospitals = FileRepository::loadHospitals("hospitals.txt");

            if (hospitals.empty())
            {
                cout << "[ERROR] No hospitals found in file or file error!" << endl;
                cout << "Please register first." << endl;
                RegistrationManager::registerHospital();
                return;
            }

            cout << "=========================================" << endl;
            cout << "            ENTER CREDENTIALS            " << endl;
            cout << "=========================================" << endl
                 << endl;

            cout << "Enter Hospital ID: ";
            string inputHospitalId;
            getline(cin, inputHospitalId);
            inputHospitalId = Utils::trim(inputHospitalId);

            cout << "Enter Password: ";
            string inputPassword;
            getline(cin, inputPassword);
            inputPassword = Utils::trim(inputPassword);

            // Authenticate - compare against loaded Hospital objects
            int foundIndex = -1;
            for (size_t i = 0; i < hospitals.size(); i++)
            {
                if (hospitals[i].getHospitalId() == inputHospitalId &&
                    hospitals[i].getPassword() == inputPassword)
                {
                    foundIndex = static_cast<int>(i);
                    break;
                }
            }

            if (foundIndex == -1)
            {
                cout << endl;
                cout << "[ERROR] Invalid credentials!" << endl;
                cout << "Hospital ID and Password must match." << endl;
                cout << "Please check and try again." << endl;
                cout << "OR register first." << endl;
                cout << "1. Try Again" << endl;
                cout << "2. Register New Hospital" << endl;
                cout << "3. Back to Main Menu" << endl;
                cout << "Select an option (1-3): ";
                int choice = Utils::readIntLine();
                switch (choice)
                {
                case 1:
                    continue;
                case 2:
                    RegistrationManager::registerHospital();
                    return;
                case 3:
                    return;
                default:
                    cout << "Invalid choice. Returning to main menu." << endl;
                    return;
                }
            }
            else
            {
                cout << endl;
                cout << "=========================================" << endl;
                cout << "           LOGIN SUCCESSFUL!            " << endl;
                cout << "=========================================" << endl;
                cout << "Welcome, " << hospitals[foundIndex].getHospitalName() << "!" << endl;
                cout << "You have successfully signed in." << endl
                     << endl;

                CurrentHospital::set(hospitals[foundIndex].getHospitalId(),
                                     hospitals[foundIndex].getHospitalName());
                Application::hospMenu();
                CurrentHospital::clear();
                return;
            }
        }
    }
};

// =============================================================================
// Forward-declaration bodies (resolve cross-calls between Application & Auth)
// =============================================================================
void userLogIn() { AuthManager::userLogIn(); }
void hospLogIn() { AuthManager::hospLogIn(); }

// =============================================================================
// ENTRY POINT
// =============================================================================
int main()
{
    Application::indexMain();
    return 0;
}