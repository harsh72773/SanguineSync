// ============================================================================
//  SANGUINE SYNC - Blood Bank Management System
//
//  PILLAR SUMMARY
//  --------------
//  1. ENCAPSULATION  - Every class hides its data as private/protected and
//                      exposes only what callers need through public methods.
//
//  2. ABSTRACTION    - Complex logic (file I/O, validation, parsing) is hidden
//                      behind clean, single-purpose method names. Callers never
//                      deal with raw implementation details.
//
//  3. INHERITANCE    - Account  <-- User
//                      Account  <-- Hospital
//                      IRecord  <-- BloodReqRecord
//                      IRecord  <-- Donor
//                      IRecord  <-- Camp
//                      IManager <-- DonorManager
//                      IManager <-- CampManager
//                      IManager <-- BloodRequestManager
//
//  4. POLYMORPHISM   - IRecord::printRow()  overridden by each record class
//                      IRecord::toCSV()     overridden by each record class
//                      IManager::showMenu() overridden by each manager class
//                      Runtime dispatch via base pointers in manager loops.
//
//  CLASS LAYOUT
//  ------------
//  [SECTION 1 — HELPER CLASSES]
//    Utils, HealthProfile, IRecord, Account, CurrentUser, CurrentHospital,
//    Inventory, FileRepository, IManager
//
//  [SECTION 2 — OUTPUT CLASSES  (strictly in flow of output)]
//    IndexMain      -> Application::indexMain()  — entry banner & top menu
//    Registration   -> RegistrationManager       — user & hospital signup
//    Authentication -> AuthManager + free fns    — credential verification
//    Menus          -> Application::userMenu / hospMenu
//    BloodRequest   -> BloodRequestManager       — request blood, view, approve
//    DonationSystem -> DonorManager              — add/view/search donors
//    PatientInfo    -> Application::displayUserHealthProfile
//    Camp           -> CampManager               — organise & view camps
// ============================================================================

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
#include <sstream>

using namespace std;

// ============================================================================
//  SECTION 1 — HELPER CLASSES
//  These provide utilities, data models, session state, and file I/O that
//  every output class depends on.
// ============================================================================
class Utils
{
public:
    // ABSTRACTION: caller never sees the loop; just calls trim()
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

    // ABSTRACTION: transform complexity hidden behind one word
    static string toUpper(const string &s)
    {
        string out = s;
        transform(out.begin(), out.end(), out.begin(),
                  [](unsigned char c)
                  { return toupper(c); });
        return out;
    }

    // ABSTRACTION: try/catch around regex hidden; callers get a plain bool
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

    // ABSTRACTION: getline + stoi + error handling hidden; callers get an int
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

    // ABSTRACTION: regex detail hidden; callers just check bool
    static bool isValidDate(const string &date)
    {
        return regexMatch(date, "^\\d{2}/\\d{2}/\\d{4}$");
    }

    // ABSTRACTION: manual find-and-slice loop hidden; callers get vector<string>
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

class HealthProfile
{
private:
    // ENCAPSULATION: Private health data fields
    string birthDate;
    double weight;
    double height;
    double bmi;

    // ABSTRACTION: BMI calculation logic is private and hidden
    double calculateBMI() const
    {
        double heightInMeters = height / 100.0;
        return weight / (heightInMeters * heightInMeters);
    }

public:
    // ENCAPSULATION: Default constructor for map compatibility
    HealthProfile() : birthDate(""), weight(0), height(0), bmi(0) {}

    // ENCAPSULATION: Constructor ensures all data is valid at creation
    HealthProfile(const string &dob, double w, double h)
        : birthDate(dob), weight(w), height(h)
    {
        bmi = calculateBMI();
    }

    // ENCAPSULATION: Getters provide controlled read access
    string getBirthDate() const { return birthDate; }
    double getWeight() const { return weight; }
    double getHeight() const { return height; }
    double getBMI() const { return bmi; }

    // ENCAPSULATION: Setters with validation
    void setWeight(double w)
    {
        if (w > 0 && w < 500)
            weight = w;
        bmi = calculateBMI();
    }

    void setHeight(double h)
    {
        if (h > 0 && h < 300)
            height = h;
        bmi = calculateBMI();
    }

    // ABSTRACTION: BMI category logic hidden behind simple method
    string getBMICategory() const
    {
        if (bmi < 18.5)
            return "Underweight";
        if (bmi < 25)
            return "Normal";
        if (bmi < 30)
            return "Overweight";
        return "Obese";
    }

    // ABSTRACTION: Health validation logic hidden
    bool isEligibleForDonation() const
    {
        return bmi >= 18.5 && bmi <= 35;
    }
};

class IRecord
{
public:
    // POLYMORPHISM: pure virtual — forces every subclass to define its own
    //               row-printing logic; called via base pointer at runtime.
    virtual void printRow(int index) const = 0;

    // POLYMORPHISM: pure virtual — each record type serialises itself
    //               differently; resolved at runtime through base pointer.
    virtual string toCSV() const = 0;

    // POLYMORPHISM: virtual destructor ensures correct cleanup through
    //               base pointers (required when using polymorphism).
    virtual ~IRecord() {}
};

class Account
{
protected:
    // ENCAPSULATION: protected so derived classes can access it,
    //                but external code cannot touch it directly.
    string password;

public:
    Account(const string &pwd) : password(pwd) {}

    // ENCAPSULATION: controlled read access through a getter
    string getPassword() const { return password; }

    // ENCAPSULATION: controlled write access through a setter
    void setPassword(const string &pwd) { password = pwd; }

    // POLYMORPHISM: virtual destructor for safe polymorphic deletion
    virtual ~Account() {}
};

class User : public Account
{
private:
    // ENCAPSULATION: private fields
    string username;
    string aadharNo;

    // COMPOSITION: User HAS-A HealthProfile
    HealthProfile *healthProfile;

public:
    // Constructor delegates password initialisation up to Account
    User(const string &uname, const string &aadhar, const string &pwd)
        : Account(pwd), username(uname), aadharNo(aadhar), healthProfile(nullptr) {}

    // Enhanced constructor with health data
    User(const string &uname, const string &aadhar, const string &pwd,
         const string &birthDate, double weight, double height)
        : Account(pwd), username(uname), aadharNo(aadhar),
          healthProfile(new HealthProfile(birthDate, weight, height)) {}

    // DESTRUCTOR: Clean up health profile
    ~User()
    {
        delete healthProfile;
    }

    // Copy constructor for proper deep copying
    User(const User &other)
        : Account(other), username(other.username), aadharNo(other.aadharNo)
    {
        healthProfile = other.healthProfile
                            ? new HealthProfile(other.healthProfile->getBirthDate(),
                                                other.healthProfile->getWeight(),
                                                other.healthProfile->getHeight())
                            : nullptr;
    }

    // Assignment operator
    User &operator=(const User &other)
    {
        if (this != &other)
        {
            Account::operator=(other);
            username = other.username;
            aadharNo = other.aadharNo;
            delete healthProfile;
            healthProfile = other.healthProfile
                                ? new HealthProfile(other.healthProfile->getBirthDate(),
                                                    other.healthProfile->getWeight(),
                                                    other.healthProfile->getHeight())
                                : nullptr;
        }
        return *this;
    }

    // ENCAPSULATION: controlled read access
    string getUsername() const { return username; }
    string getAadharNo() const { return aadharNo; }

    bool hasHealthProfile() const { return healthProfile != nullptr; }

    string getBirthDate() const { return healthProfile ? healthProfile->getBirthDate() : ""; }
    double getWeight() const { return healthProfile ? healthProfile->getWeight() : 0.0; }
    double getHeight() const { return healthProfile ? healthProfile->getHeight() : 0.0; }
    double getBMI() const { return healthProfile ? healthProfile->getBMI() : 0.0; }

    string getBMICategory() const
    {
        return healthProfile ? healthProfile->getBMICategory() : "Unknown";
    }

    bool isEligibleForDonation() const
    {
        return healthProfile ? healthProfile->isEligibleForDonation() : false;
    }

    // ENCAPSULATION: controlled write access
    void setUsername(const string &uname) { username = uname; }
    void setAadharNo(const string &aadhar) { aadharNo = aadhar; }

    void setHealthProfile(const string &birthDate, double weight, double height)
    {
        delete healthProfile;
        healthProfile = new HealthProfile(birthDate, weight, height);
    }

    void updateWeight(double weight)
    {
        if (healthProfile)
            healthProfile->setWeight(weight);
    }

    void updateHeight(double height)
    {
        if (healthProfile)
            healthProfile->setHeight(height);
    }

    // ABSTRACTION: Display health summary with proper formatting
    void displayHealthSummary() const
    {
        if (!healthProfile)
        {
            cout << "No health profile data available." << endl;
            return;
        }
        cout << "========================================" << endl;
        cout << "           HEALTH SUMMARY              " << endl;
        cout << "========================================" << endl;
        cout << "Birth Date: " << healthProfile->getBirthDate() << endl;
        cout << "Weight: " << healthProfile->getWeight() << " kg" << endl;
        cout << "Height: " << healthProfile->getHeight() << " cm" << endl;
        cout << "BMI: " << fixed << setprecision(1) << healthProfile->getBMI() << endl;
        cout << "BMI Category: " << healthProfile->getBMICategory() << endl;
        cout << "Donation Eligibility: "
             << (healthProfile->isEligibleForDonation() ? "ELIGIBLE" : "NOT ELIGIBLE") << endl;
        cout << "========================================" << endl;
    }
};

class Hospital : public Account
{
private:
    // ENCAPSULATION: private fields
    string hospitalId;
    string hospitalName;

public:
    Hospital(const string &hid, const string &hname, const string &pwd)
        : Account(pwd), hospitalId(hid), hospitalName(hname) {}

    // ENCAPSULATION: read-only after construction
    string getHospitalId() const { return hospitalId; }
    string getHospitalName() const { return hospitalName; }
};

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

class Inventory
{
public:
    // ENCAPSULATION: constant lives inside the class, not in global scope
    static const vector<string> VALID_BLOOD_GROUPS;

    static bool isValidBloodGroup(const string &group)
    {
        for (const auto &g : VALID_BLOOD_GROUPS)
            if (g == group)
                return true;
        return false;
    }

    static string getFilePath()
    {
        const string &hid = CurrentHospital::getHospitalId();
        return hid.empty() ? "textFiles/inventory.txt"
                           : "textFiles/inventory_" + hid + ".txt";
    }

    // ABSTRACTION: file parsing hidden; returns map of blood group -> units
    static map<string, int> loadStock(const string &filename)
    {
        map<string, int> stock;
        ifstream in(filename);
        if (!in.is_open())
            return stock;

        string line;
        while (getline(in, line))
        {
            line = Utils::trim(line);
            if (line.empty())
                continue;
            vector<string> parts = Utils::splitCSV(line, 2);
            if (parts.size() >= 2 && isValidBloodGroup(parts[0]))
            {
                try
                {
                    stock[parts[0]] = stoi(parts[1]);
                }
                catch (...)
                {
                }
            }
        }
        return stock;
    }

    static bool saveStock(const map<string, int> &stock, const string &filename)
    {
        ofstream out(filename, ios::trunc);
        if (!out.is_open())
        {
            cout << "[ERROR] Could not save to " << filename << endl;
            return false;
        }
        for (const auto &entry : stock)
            out << entry.first << "," << entry.second << "\n";
        out.flush();
        return true;
    }

    static int getAvailableUnits(const string &bloodGroup, const map<string, int> &stock)
    {
        auto it = stock.find(bloodGroup);
        return it != stock.end() ? it->second : 0;
    }

    static void display()
    {
        auto stock = loadStock(getFilePath());
        cout << "========================================" << endl;
        cout << "             BLOOD INVENTORY            " << endl;
        cout << "========================================" << endl;
        cout << left << setw(5) << "NO." << setw(10) << "BLOOD GROUP"
             << setw(10) << "UNITS" << endl;
        cout << string(25, '-') << endl;

        int idx = 1;
        for (const auto &g : VALID_BLOOD_GROUPS)
        {
            auto it = stock.find(g);
            int units = (it != stock.end()) ? it->second : 0;
            cout << left << setw(5) << idx++ << setw(10) << g
                 << setw(10) << units << endl;
        }
    }

    // ABSTRACTION: entire menu loop hidden; Application just calls menu()
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

            if (choice == 1 || choice == 2)
            {
                string group, ustr;
                cout << "Enter blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ";
                getline(cin, group);
                group = Utils::trim(Utils::toUpper(group));
                if (!isValidBloodGroup(group))
                {
                    cout << "Invalid blood group." << endl;
                    continue;
                }
                cout << (choice == 1 ? "Enter units to add: " : "Enter units to remove: ");
                getline(cin, ustr);
                ustr = Utils::trim(ustr);
                if (!Utils::regexMatch(ustr, "^\\d+$"))
                {
                    cout << "Units must be a positive whole number." << endl;
                    continue;
                }
                int units = stoi(ustr);
                if (units <= 0)
                {
                    cout << "Units must be greater than 0." << endl;
                    continue;
                }

                auto stock = loadStock(getFilePath());

                if (choice == 1)
                {
                    stock[group] += units;
                    cout << units << " units added to " << group << "." << endl;
                }
                else
                {
                    int current = getAvailableUnits(group, stock);
                    if (current >= units)
                    {
                        stock[group] = current - units;
                        cout << units << " units removed from " << group << "." << endl;
                    }
                    else
                    {
                        cout << "Not enough stock available for " << group << "." << endl;
                    }
                }

                saveStock(stock, getFilePath());
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

            if (choice != 4)
            {
                cout << "Press Enter to continue..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
    }
};
const vector<string> Inventory::VALID_BLOOD_GROUPS = {"A+", "A-", "B+", "B-", "O+", "O-", "AB+", "AB-"};

class BloodReqRecord : public IRecord
{
private:
    string username;
    string aadharNo;
    string bloodGroup;
    string units;
    string status;

public:
    BloodReqRecord(const string &u, const string &a, const string &b,
                   const string &un, const string &st)
        : username(u), aadharNo(a), bloodGroup(b), units(un),
          status(st.empty() ? "pending" : Utils::trim(st)) {}

    // ENCAPSULATION: getters only
    string getUsername() const { return username; }
    string getAadharNo() const { return aadharNo; }
    string getBloodGroup() const { return bloodGroup; }
    string getUnits() const { return units; }
    string getStatus() const { return status; }

    // POLYMORPHISM: overrides IRecord::printRow()
    void printRow(int index) const override
    {
        cout << left << setw(5) << index
             << setw(12) << bloodGroup
             << setw(8) << units
             << setw(10) << status << endl;
    }

    // POLYMORPHISM: overrides IRecord::toCSV() — block format for file storage
    string toCSV() const override
    {
        return "---\nUsername: " + username +
               "\nAadhaar No: " + aadharNo +
               "\nBlood Group: " + bloodGroup +
               "\nUnits Required: " + units +
               "\nStatus: " + status;
    }

    // ABSTRACTION: entire file-parsing loop hidden; callers receive vector<BloodReqRecord>
    static vector<BloodReqRecord> loadAll(const string &path)
    {
        vector<BloodReqRecord> list;
        ifstream in("textFiles/" + path);
        if (!in.is_open())
            return list;

        string u, a, b, un, st = "pending", line;

        auto pushIfValid = [&]()
        {
            if (!u.empty())
                list.emplace_back(u, a, b, un, st);
        };

        while (getline(in, line))
        {
            line = Utils::trim(line);
            if (line == "---")
            {
                pushIfValid();
                u.clear();
                a.clear();
                b.clear();
                un.clear();
                st = "pending";
                continue;
            }
            if (line.rfind("Username: ", 0) == 0)
                u = Utils::trim(line.substr(strlen("Username: ")));
            else if (line.rfind("Aadhaar No: ", 0) == 0)
                a = Utils::trim(line.substr(strlen("Aadhaar No: ")));
            else if (line.rfind("Blood Group: ", 0) == 0)
                b = Utils::trim(line.substr(strlen("Blood Group: ")));
            else if (line.rfind("Units Required: ", 0) == 0)
                un = Utils::trim(line.substr(strlen("Units Required: ")));
            else if (line.rfind("Status: ", 0) == 0)
                st = Utils::trim(line.substr(strlen("Status: ")));
        }
        pushIfValid();
        return list;
    }
};

class Donor : public IRecord
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
    Donor(const string &n, const string &e, const string &d, const string &bg,
          int a, const string &dis, bool elg, const string &hid, bool completed)
        : name(n), email(e), dob(d), bloodGroup(bg), age(a), disease(dis),
          eligible(elg), hospitalId(hid), donationCompleted(completed) {}

    // ENCAPSULATION: read-only getters
    string getName() const { return name; }
    string getEmail() const { return email; }
    string getDob() const { return dob; }
    string getBloodGroup() const { return bloodGroup; }
    int getAge() const { return age; }
    string getDisease() const { return disease; }
    bool isEligible() const { return eligible; }
    string getHospitalId() const { return hospitalId; }
    bool isDonationCompleted() const { return donationCompleted; }

    // ENCAPSULATION: only the completion flag may be changed externally
    void setDonationCompleted(bool completed) { donationCompleted = completed; }

    // POLYMORPHISM: runtime override of IRecord::printRow()
    void printRow(int index) const override
    {
        cout << left << setw(5) << index
             << setw(20) << name
             << setw(25) << email
             << setw(12) << dob
             << setw(10) << bloodGroup
             << setw(6) << age
             << setw(15) << (disease == "Yes" || disease == "YES" ? "Yes" : "No")
             << setw(10) << (donationCompleted ? "Completed" : "Pending") << endl;
    }

    // POLYMORPHISM: runtime override of IRecord::toCSV()
    string toCSV() const override
    {
        return name + "," + email + "," + dob + "," + bloodGroup + "," +
               to_string(age) + "," + disease + "," +
               (eligible ? "1" : "0") + "," + hospitalId;
    }

    // ABSTRACTION: CSV-to-object construction hidden; callers get a Donor
    static Donor createFromCSV(const vector<string> &parts)
    {
        if (parts.size() == 8)
            return Donor(parts[0], parts[1], parts[2], parts[3],
                         stoi(parts[4]), parts[5], (parts[6] == "1"), parts[7], false);
        return Donor("", "", "", "", 0, "", false, "", false);
    }
};

class Camp : public IRecord
{
private:
    string name;
    string date;
    string venue;
    string city;

public:
    Camp(const string &n, const string &d, const string &v, const string &c)
        : name(n), date(d), venue(v), city(c) {}

    string getName() const { return name; }
    string getDate() const { return date; }
    string getVenue() const { return venue; }
    string getCity() const { return city; }

    // POLYMORPHISM: runtime override of IRecord::printRow()
    void printRow(int index) const override
    {
        cout << left << setw(5) << index
             << setw(25) << name
             << setw(12) << date
             << setw(20) << venue
             << setw(15) << city << endl;
    }

    // POLYMORPHISM: runtime override of IRecord::toCSV()
    string toCSV() const override
    {
        return name + "," + date + "," + venue + "," + city;
    }

    static Camp createFromCSV(const vector<string> &parts)
    {
        if (parts.size() == 4)
            return Camp(parts[0], parts[1], parts[2], parts[3]);
        return Camp("", "", "", "");
    }
};

class FileRepository
{
public:
    // ---- USER ----------------------------------------------------------------

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
            auto p = Utils::splitCSV(line, 3);
            if (p.size() == 3)
                users.push_back(User(p[0], p[1], p[2]));
        }
        return users;
    }

    static bool saveUser(const string &username, const string &aadharNo, const string &pwd)
    {
        ofstream out("textFiles/users.txt", ios::app);
        if (!out.is_open())
        {
            cout << "[Error] Could not open users.txt" << endl;
            return false;
        }
        out << username << "," << aadharNo << "," << pwd << "\n";
        out.flush();
        return true;
    }

    // Save user together with health profile
    static bool saveUserWithHealth(const string &username, const string &aadharNo,
                                   const string &pwd, const string &birthDate,
                                   double weight, double height)
    {
        if (!saveUser(username, aadharNo, pwd))
            return false;

        ofstream out("textFiles/user_health.txt", ios::app);
        if (!out.is_open())
        {
            cout << "[Error] Could not open user_health.txt" << endl;
            return false;
        }
        out << aadharNo << "," << birthDate << ","
            << fixed << setprecision(1) << weight << ","
            << fixed << setprecision(1) << height << "\n";
        out.flush();
        return true;
    }

    // Load users enriched with health profiles from user_health.txt
    static vector<User> loadUsersWithHealth(const string &filename)
    {
        vector<User> users = loadUsers(filename);

        map<string, HealthProfile> healthMap;
        ifstream in("textFiles/user_health.txt");
        if (in.is_open())
        {
            string line;
            while (getline(in, line))
            {
                line = Utils::trim(line);
                if (line.empty())
                    continue;
                auto parts = Utils::splitCSV(line, 4);
                if (parts.size() >= 4)
                {
                    try
                    {
                        healthMap[parts[0]] =
                            HealthProfile(parts[1], stod(parts[2]), stod(parts[3]));
                    }
                    catch (...)
                    {
                    }
                }
            }
        }

        vector<User> enriched;
        for (auto &u : users)
        {
            User eu(u.getUsername(), u.getAadharNo(), u.getPassword());
            auto it = healthMap.find(u.getAadharNo());
            if (it != healthMap.end())
                eu.setHealthProfile(it->second.getBirthDate(),
                                    it->second.getWeight(),
                                    it->second.getHeight());
            enriched.push_back(eu);
        }
        return enriched;
    }

    static bool aadharExists(const string &aadharNo)
    {
        ifstream in("textFiles/users.txt");
        if (!in.is_open())
            return false;
        string line;
        while (getline(in, line))
        {
            auto p = Utils::splitCSV(Utils::trim(line), 3);
            if (p.size() > 1 && p[1] == aadharNo)
                return true;
        }
        return false;
    }

    // ---- HOSPITAL ------------------------------------------------------------

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
            auto p = Utils::splitCSV(line, 3);
            if (p.size() == 3)
                hospitals.push_back(Hospital(p[0], p[1], p[2]));
        }
        return hospitals;
    }

    static bool saveHospital(const string &hid, const string &hname, const string &pwd)
    {
        ofstream out("textFiles/hospitals.txt", ios::app);
        if (!out.is_open())
        {
            cout << "[Error] Could not open hospitals.txt" << endl;
            return false;
        }
        out << hid << "," << hname << "," << pwd << "\n";
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
            auto p = Utils::splitCSV(Utils::trim(line), 3);
            if (!p.empty() && p[0] == hospitalId)
                return true;
        }
        return false;
    }

    // ---- DONOR ---------------------------------------------------------------

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
            Donor d = Donor::createFromCSV(Utils::splitCSV(line, 8));
            if (!d.getName().empty())
                donors.push_back(d);
        }
        return donors;
    }

    // POLYMORPHISM + ABSTRACTION: uses IRecord::toCSV() to serialise
    static bool saveDonor(const Donor &donor, const string &filename)
    {
        ofstream out("textFiles/" + filename, ios::app);
        if (!out.is_open())
        {
            cout << "[ERROR] Could not save to " << filename << endl;
            return false;
        }
        out << donor.toCSV() << endl; // POLYMORPHISM: Donor::toCSV()
        out.flush();
        return true;
    }

    static bool rewriteDonors(const vector<Donor> &donors, const string &filename)
    {
        ofstream out("textFiles/" + filename, ios::trunc);
        if (!out.is_open())
            return false;
        for (const auto &d : donors)
            out << d.toCSV() << endl; // POLYMORPHISM: Donor::toCSV()
        out.flush();
        return true;
    }

    // ---- CAMP ----------------------------------------------------------------

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
            Camp c = Camp::createFromCSV(Utils::splitCSV(line, 4));
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
            cout << "[ERROR] Could not save to " << filename << endl;
            return false;
        }
        out << camp.toCSV() << endl; // POLYMORPHISM: Camp::toCSV()
        out.flush();
        return true;
    }

    // ---- BLOOD REQUEST -------------------------------------------------------

    static bool saveBloodRequest(const string &username, const string &aadharNo,
                                 const string &bloodGroup, const string &units,
                                 const string &reqFile)
    {
        ofstream out("textFiles/" + reqFile, ios::app);
        if (!out.is_open())
        {
            cout << "[ERROR] Could not save to " << reqFile << endl;
            return false;
        }
        BloodReqRecord rec(username, aadharNo, bloodGroup, units, "pending");
        out << rec.toCSV() << endl; // POLYMORPHISM: BloodReqRecord::toCSV()
        out.flush();
        return true;
    }

    static bool rewriteBloodRequests(const vector<BloodReqRecord> &records,
                                     const string &reqFile)
    {
        ofstream out("textFiles/" + reqFile, ios::trunc);
        if (!out.is_open())
            return false;
        for (const auto &r : records)
            out << r.toCSV() << endl; // POLYMORPHISM: BloodReqRecord::toCSV()
        out.flush();
        return true;
    }
};

class IManager
{
public:
    virtual void showMenu() = 0;
    virtual ~IManager() {}
};

// ============================================================================
//  SECTION 2 — OUTPUT CLASSES  (strictly in flow of output)
//
//  Flow:  IndexMain → Registration → Authentication → Menus →
//         BloodRequest → DonationSystem → PatientInfo → Camp
// ============================================================================
class RegistrationManager;
class AuthManager;

class Application
{
public:
    static void indexMain();
    static void logInPage();
    static void userMenu();
    static void hospMenu();
    static void displayUserHealthProfile();
};

void userLogIn();
void hospLogIn();

class RegistrationManager
{
public:
    // ABSTRACTION: entire registration flow (loop, validate, check duplicate,
    //              save, redirect to login) behind a single method call.
    // Includes health profile collection.
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
                cout << "\n[ERROR] Invalid Aadhar number!" << endl;
                cout << "- Must be EXACTLY 12 digits" << endl;
                cout << "- Only numbers (0-9), no spaces or letters" << endl;
                cout << "- Examples: 123456789012 \u2713 | 123 456 789 012 \u2717\n"
                     << endl;
                continue;
            }

            cout << "Enter password: ";
            string password;
            getline(cin, password);
            password = Utils::trim(password);

            // Collect health profile
            cout << "\n=========================================" << endl;
            cout << "           HEALTH PROFILE              " << endl;
            cout << "=========================================" << endl;

            string birthDate;
            while (true)
            {
                cout << "Enter Date of Birth (DD/MM/YYYY): ";
                getline(cin, birthDate);
                birthDate = Utils::trim(birthDate);
                if (Utils::isValidDate(birthDate))
                    break;
                cout << "Invalid date format. Please use DD/MM/YYYY format." << endl;
            }

            double weight = 0;
            while (true)
            {
                cout << "Enter weight (kg): ";
                string ws;
                getline(cin, ws);
                ws = Utils::trim(ws);
                try
                {
                    weight = stod(ws);
                    if (weight > 0 && weight < 500)
                        break;
                    cout << "Weight must be between 1 and 500 kg." << endl;
                }
                catch (...)
                {
                    cout << "Invalid weight. Please enter a valid number." << endl;
                }
            }

            double height = 0;
            while (true)
            {
                cout << "Enter height (cm): ";
                string hs;
                getline(cin, hs);
                hs = Utils::trim(hs);
                try
                {
                    height = stod(hs);
                    if (height > 0 && height < 300)
                        break;
                    cout << "Height must be between 1 and 300 cm." << endl;
                }
                catch (...)
                {
                    cout << "Invalid height. Please enter a valid number." << endl;
                }
            }

            // Show calculated BMI before confirming
            HealthProfile hp(birthDate, weight, height);
            cout << "\nBMI calculated: " << fixed << setprecision(1) << hp.getBMI()
                 << " (" << hp.getBMICategory() << ")" << endl;
            cout << "Donation Eligibility: "
                 << (hp.isEligibleForDonation() ? "ELIGIBLE" : "NOT ELIGIBLE") << endl;

            cout << "\nConfirm registration? (y/n): ";
            string confirm;
            getline(cin, confirm);
            confirm = Utils::trim(Utils::toUpper(confirm));
            if (confirm != "Y" && confirm != "YES")
            {
                cout << "Registration cancelled." << endl;
                continue;
            }

            if (FileRepository::aadharExists(aadharNo))
            {
                cout << "\n[ERROR] Aadhar number already exists!" << endl;
                cout << "This Aadhar is already registered.\n"
                     << endl;
                cout << "1. Try Again\n2. Log In" << endl;
                int choice = Utils::readIntLine();
                if (choice == 2)
                {
                    userLogIn();
                    return;
                }
                continue;
            }

            if (FileRepository::saveUserWithHealth(username, aadharNo, password,
                                                   birthDate, weight, height))
            {
                cout << "User saved successfully with health profile." << endl;
                cout << "Please log in with your new credentials." << endl;
                userLogIn();
                break;
            }
        }
    }

    // ABSTRACTION: entire hospital registration flow behind one call
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
                    cout << "\n[ERROR] Invalid Hospital ID!" << endl;
                    cout << "- Must be EXACTLY 10 digits" << endl;
                    cout << "- Only numbers (0-9), no spaces or letters" << endl;
                    cout << "- Examples: 1234567890 \u2713 | 123 456 789 0 \u2717\n"
                         << endl;
                    continue;
                }

                cout << "Enter password: ";
                string password;
                getline(cin, password);
                password = Utils::trim(password);

                if (FileRepository::hospitalExists(hospitalId))
                {
                    cout << "\n[ERROR] Hospital ID already exists!" << endl;
                    cout << "Please choose a different Hospital ID or log in.\n"
                         << endl;
                    cout << "1. Try Again\n2. Log In" << endl;
                    int choice = Utils::readIntLine();
                    if (choice == 2)
                    {
                        hospLogIn();
                        return;
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

class AuthManager
{
public:
    // ABSTRACTION: entire user auth flow (load, compare, session-set, menu)
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

            // ENCAPSULATION: accesses User fields via getUsername()/getPassword()
            // INHERITANCE:   getPassword() is defined in Account, inherited by User
            bool found = false;
            string foundAadhar = "";
            for (const auto &u : users)
            {
                if (u.getUsername() == inputUsername && u.getPassword() == inputPassword)
                {
                    found = true;
                    foundAadhar = u.getAadharNo();
                    break;
                }
            }

            if (!found)
            {
                cout << "\n[ERROR] Invalid credentials!" << endl;
                cout << "Username and Password must match." << endl;
                cout << "Please check and try again." << endl;
                cout << "OR register first." << endl;
                cout << "1. Try Again\n2. Register New User\n3. Back to Main Menu" << endl;
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
                // Mask Aadhar for display — data privacy
                string masked = "********";
                if (foundAadhar.size() >= 4)
                    masked += foundAadhar.substr(foundAadhar.size() - 4);
                else
                    masked += foundAadhar;

                cout << "\n=========================================" << endl;
                cout << "           LOGIN SUCCESSFUL!            " << endl;
                cout << "=========================================" << endl;
                cout << "Welcome, " << inputUsername << "!" << endl;
                cout << "Aadhar: " << masked << endl;
                cout << "You have successfully signed in.\n"
                     << endl;

                // ENCAPSULATION: session stored through CurrentUser::set()
                CurrentUser::set(inputUsername, foundAadhar);
                Application::userMenu(); // forward-declared — defined below
                return;
            }
        }
    }

    // ABSTRACTION: entire hospital auth flow in one call
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
            string inputId;
            getline(cin, inputId);
            inputId = Utils::trim(inputId);

            cout << "Enter Password: ";
            string inputPwd;
            getline(cin, inputPwd);
            inputPwd = Utils::trim(inputPwd);

            // ENCAPSULATION: accesses Hospital fields via getters
            // INHERITANCE:   getPassword() inherited from Account
            int foundIndex = -1;
            for (size_t i = 0; i < hospitals.size(); i++)
            {
                if (hospitals[i].getHospitalId() == inputId &&
                    hospitals[i].getPassword() == inputPwd)
                {
                    foundIndex = static_cast<int>(i);
                    break;
                }
            }

            if (foundIndex == -1)
            {
                cout << "\n[ERROR] Invalid credentials!" << endl;
                cout << "Hospital ID and Password must match." << endl;
                cout << "Please check and try again." << endl;
                cout << "OR register first." << endl;
                cout << "1. Try Again\n2. Register New Hospital\n3. Back to Main Menu" << endl;
                cout << "Select an option (1-3): ";
                int choice = Utils::readIntLine();
                switch (choice)
                {
                case 1:
                    continue;
                case 2:
                    RegistrationManager::registerHospital();
                    break;
                case 3:
                    return;
                default:
                    cout << "Invalid choice. Returning to main menu." << endl;
                    return;
                }
            }
            else
            {
                cout << "\n=========================================" << endl;
                cout << "           LOGIN SUCCESSFUL!            " << endl;
                cout << "=========================================" << endl;
                cout << "Welcome, " << hospitals[foundIndex].getHospitalName() << "!" << endl;
                cout << "You have successfully signed in.\n"
                     << endl;

                // ENCAPSULATION: session stored through CurrentHospital::set()
                CurrentHospital::set(hospitals[foundIndex].getHospitalId(),
                                     hospitals[foundIndex].getHospitalName());
                Application::hospMenu(); // forward-declared — defined below
                CurrentHospital::clear();
                return;
            }
        }
    }
};

void userLogIn() { AuthManager::userLogIn(); }
void hospLogIn() { AuthManager::hospLogIn(); }

class BloodRequestManager : public IManager
{
private:
    static const string BLOOD_REQ_FILE;

public:
    // POLYMORPHISM: overrides IManager::showMenu()
    // Context-specific (user vs hospital); Application calls methods directly.
    void showMenu() override {}

    // ABSTRACTION: entire request form + validation hidden behind one call
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
                cout << "[ERROR] Units value is too large." << endl;
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

    // ABSTRACTION + POLYMORPHISM: loads records, filters, calls printRow()
    static void viewMyRequests()
    {
        string username = CurrentUser::getUsername();
        if (username.empty())
        {
            cout << "[ERROR] No user logged in." << endl;
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
                mine[i].printRow(static_cast<int>(i + 1)); // POLYMORPHISM
        }
    }

    static void viewMyCompletedRequests()
    {
        string username = CurrentUser::getUsername();
        if (username.empty())
        {
            cout << "[ERROR] No user logged in." << endl;
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
                myCompleted[i].printRow(static_cast<int>(i + 1)); // POLYMORPHISM
        }
    }

    // ABSTRACTION: business logic (load, filter by stock, display, update) hidden
    static void viewPendingRequest()
    {
        auto all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
        auto stock = Inventory::loadStock(Inventory::getFilePath());

        vector<BloodReqRecord> pending;
        for (const auto &r : all)
        {
            string sl = r.getStatus();
            transform(sl.begin(), sl.end(), sl.begin(),
                      [](unsigned char c)
                      { return tolower(c); });
            if (sl == "pending" &&
                Inventory::getAvailableUnits(r.getBloodGroup(), stock) >= stoi(r.getUnits()))
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

        cout << left << setw(5) << "NO."
             << setw(15) << "USERNAME"
             << setw(12) << "BLOOD"
             << setw(8) << "UNITS"
             << setw(10) << "STATUS" << endl;
        cout << string(50, '-') << endl;
        for (size_t i = 0; i < pending.size(); i++)
        {
            cout << left << setw(5) << (i + 1)
                 << setw(15) << pending[i].getUsername()
                 << setw(12) << pending[i].getBloodGroup()
                 << setw(8) << pending[i].getUnits()
                 << setw(10) << pending[i].getStatus() << endl;
        }

        cout << endl;
        cout << "Enter request number to mark as completed (0 to go back): ";
        int choice = Utils::readIntLine();

        if (choice > 0 && choice <= static_cast<int>(pending.size()))
        {
            const auto &sel = pending[choice - 1];
            int requested = stoi(sel.getUnits());
            int current = Inventory::getAvailableUnits(sel.getBloodGroup(), stock);
            stock[sel.getBloodGroup()] = current - requested;
            Inventory::saveStock(stock, Inventory::getFilePath());

            vector<BloodReqRecord> updatedAll;
            bool marked = false;
            for (const auto &req : all)
            {
                if (!marked &&
                    req.getUsername() == sel.getUsername() &&
                    req.getBloodGroup() == sel.getBloodGroup() &&
                    req.getUnits() == sel.getUnits() &&
                    req.getStatus() == "pending")
                {
                    updatedAll.emplace_back(req.getUsername(), req.getAadharNo(),
                                            req.getBloodGroup(), req.getUnits(), "completed");
                    marked = true;
                }
                else
                {
                    updatedAll.push_back(req);
                }
            }
            if (FileRepository::rewriteBloodRequests(updatedAll, BLOOD_REQ_FILE))
                cout << "Request marked as completed! "
                     << requested << " units deducted from inventory." << endl;
        }
    }

    static void viewCompletedRequest()
    {
        auto all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
        vector<BloodReqRecord> completed;
        for (const auto &r : all)
        {
            string sl = r.getStatus();
            transform(sl.begin(), sl.end(), sl.begin(),
                      [](unsigned char c)
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

        cout << left << setw(5) << "NO."
             << setw(15) << "USERNAME"
             << setw(12) << "BLOOD"
             << setw(8) << "UNITS"
             << setw(10) << "STATUS" << endl;
        cout << string(50, '-') << endl;
        for (size_t i = 0; i < completed.size(); i++)
        {
            cout << left << setw(5) << (i + 1)
                 << setw(15) << completed[i].getUsername()
                 << setw(12) << completed[i].getBloodGroup()
                 << setw(8) << completed[i].getUnits()
                 << setw(10) << completed[i].getStatus() << endl;
        }
    }
};
const string BloodRequestManager::BLOOD_REQ_FILE = "BloodReq.txt";

class DonorManager : public IManager
{
private:
    static const string DONORS_FILE;

public:
    // POLYMORPHISM: overrides IManager::showMenu()
    void showMenu() override
    {
        while (true)
        {
            cout << "========================================" << endl;
            cout << "          DONOR MANAGEMENT             " << endl;
            cout << "========================================" << endl;
            cout << "1. Add Donor\n2. View Donor List\n3. Search Donors by Blood Group\n4. Back" << endl;
            cout << "Select an option (1-4): ";
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
                searchDonorsByBloodGroup();
                break;
            case 4:
                return;
            default:
                cout << "Invalid choice. Please select 1-4." << endl;
                break;
            }
            if (choice != 4)
            {
                cout << "Press Enter to continue..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
    }

    // ABSTRACTION: all input + validation + object creation hidden
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

        // ENCAPSULATION: Donor object created; its fields are private from here on
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

    // ABSTRACTION: all search input, filtering, and display hidden behind one call
    static void searchDonorsByBloodGroup()
    {
        cout << "========================================" << endl;
        cout << "      SEARCH DONORS BY BLOOD GROUP      " << endl;
        cout << "========================================" << endl;

        string searchGroup;
        while (true)
        {
            cout << "Enter Blood Group to search (A+, A-, B+, B-, AB+, AB-, O+, O-): ";
            getline(cin, searchGroup);
            searchGroup = Utils::trim(Utils::toUpper(searchGroup));
            if (Utils::regexMatch(searchGroup, "^(A|B|AB|O)[+-]$"))
                break;
            cout << "[ERROR] Invalid blood group!" << endl;
            cout << "Valid types: A+, A-, B+, B-, AB+, AB-, O+, O-." << endl;
            cout << "Please try again." << endl;
        }

        auto donors = FileRepository::loadDonors(DONORS_FILE);
        vector<Donor> matched;

        // ENCAPSULATION: filters using getters
        for (const auto &d : donors)
            if (Utils::toUpper(d.getBloodGroup()) == searchGroup &&
                d.getHospitalId() == CurrentHospital::getHospitalId())
                matched.push_back(d);

        cout << endl;
        cout << "========================================" << endl;
        cout << "  Results for Blood Group: " << searchGroup << endl;
        cout << "========================================" << endl;

        if (matched.empty())
        {
            cout << "No donors found with blood group " << searchGroup
                 << " in your hospital." << endl;
            return;
        }

        cout << "Total matching donors: " << matched.size() << endl
             << endl;
        cout << left << setw(5) << "NO."
             << setw(20) << "NAME"
             << setw(25) << "EMAIL"
             << setw(12) << "DOB"
             << setw(10) << "BLOOD"
             << setw(6) << "AGE"
             << setw(15) << "DISEASE"
             << setw(10) << "STATUS" << endl;
        cout << string(103, '-') << endl;

        for (size_t i = 0; i < matched.size(); i++)
            matched[i].printRow(static_cast<int>(i + 1)); // POLYMORPHISM
    }

    // ABSTRACTION + POLYMORPHISM: iterates donors, calls printRow() on each
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

        cout << left << setw(5) << "NO."
             << setw(20) << "NAME"
             << setw(25) << "EMAIL"
             << setw(12) << "DOB"
             << setw(10) << "BLOOD"
             << setw(6) << "AGE"
             << setw(15) << "DISEASE"
             << setw(10) << "STATUS" << endl;
        cout << string(103, '-') << endl;

        for (size_t i = 0; i < hospitalDonors.size(); i++)
            hospitalDonors[i].printRow(static_cast<int>(i + 1)); // POLYMORPHISM

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
                        if (d.getEmail() == sel.getEmail() &&
                            d.getHospitalId() == sel.getHospitalId())
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
};
const string DonorManager::DONORS_FILE = "donors.txt";

class CampManager : public IManager
{
private:
    static const string CAMPS_FILE;

public:
    // POLYMORPHISM: overrides IManager::showMenu()
    // Delegates directly to viewOngoingCamps() for the hospital flow.
    void showMenu() override
    {
        viewOngoingCamps();
    }

    // ABSTRACTION + POLYMORPHISM: calls Camp::printRow() via IRecord
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

        cout << left << setw(5) << "NO."
             << setw(25) << "CAMP NAME"
             << setw(12) << "DATE"
             << setw(20) << "VENUE"
             << setw(15) << "CITY" << endl;
        cout << string(77, '-') << endl;

        for (size_t i = 0; i < camps.size(); i++)
            camps[i].printRow(static_cast<int>(i + 1)); // POLYMORPHISM
    }

    // ABSTRACTION: input + object construction + save all hidden
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

        Camp camp(name, date, venue, city); // ENCAPSULATION: fields private immediately

        if (FileRepository::saveCamp(camp, CAMPS_FILE))
            cout << "Blood camp saved to " << CAMPS_FILE << endl;
    }

    static void viewOngoingCamps()
    {
        cout << "========================================" << endl;
        cout << "           ONGOING BLOOD CAMPS          " << endl;
        cout << "========================================" << endl;
        viewCamps();
    }
};
const string CampManager::CAMPS_FILE = "camps.txt";

void Application::indexMain()
{
    while (true)
    {
        cout << "========================================" << endl;
        cout << "        WELCOME TO SANGUINE SYNC        " << endl;
        cout << "========================================" << endl
             << endl;
        cout << "Please Log In or Create an Account to Continue" << endl;
        cout << "1. New User Account\n2. New Hospital Account\n3. Log In\n4. Exit System\n"
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
            break;
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

void Application::logInPage()
{
    cout << endl;
    cout << "       Who is loging in?       " << endl;
    cout << "1. User\n2. Hospital\n"
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

void Application::userMenu()
{
    while (true)
    {
        cout << "========================================" << endl;
        cout << "             USER MENU                  " << endl;
        cout << "========================================" << endl;
        cout << "1. Request Blood\n2. Show My Requests\n3. View Completed Requests" << endl;
        cout << "4. View Blood Camps\n5. View My Health Profile" << endl;
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
            // [8] Camp — viewCamps() from CampManager
            CampManager::viewCamps();
            cout << "Press Enter to return to the User Menu..." << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        case 5:
            // [7] Patient Info — health profile
            displayUserHealthProfile();
            cout << "Press Enter to return to the User Menu..." << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        case 6:
            // ENCAPSULATION: session cleared through CurrentUser::clear()
            CurrentUser::clear();
            cout << "Returning to main page..." << endl;
            indexMain();
            return;
        default:
            cout << "Invalid choice. Please select 1, 2, 3, 4, 5 or 6." << endl
                 << endl;
            break;
        }
    }
}

void Application::displayUserHealthProfile()
{
    string username = CurrentUser::getUsername();
    string aadharNo = CurrentUser::getAadharNo();

    if (username.empty())
    {
        cout << "[ERROR] No user logged in." << endl;
        return;
    }

    auto users = FileRepository::loadUsersWithHealth("users.txt");
    for (const auto &user : users)
    {
        if (user.getUsername() == username && user.getAadharNo() == aadharNo)
        {
            user.displayHealthSummary();
            return;
        }
    }
    cout << "Health profile not found for current user." << endl;
}

void Application::hospMenu()
{
    // POLYMORPHISM: concrete managers — showMenu() dispatches at runtime
    DonorManager donorMgr;
    CampManager campMgr;
    BloodRequestManager reqMgr;

    while (true)
    {
        auto all = BloodReqRecord::loadAll("BloodReq.txt");
        auto stock = Inventory::loadStock(Inventory::getFilePath());

        vector<BloodReqRecord> pending;
        for (const auto &r : all)
        {
            string sl = r.getStatus();
            transform(sl.begin(), sl.end(), sl.begin(),
                      [](unsigned char c)
                      { return tolower(c); });
            if (sl == "pending" &&
                Inventory::getAvailableUnits(r.getBloodGroup(), stock) >= stoi(r.getUnits()))
                pending.push_back(r);
        }

        cout << "========================================" << endl;
        cout << "            HOSPITAL MENU               " << endl;
        cout << "========================================" << endl;
        cout << "Pending Requests: " << pending.size() << endl
             << endl;
        cout << "1. View Pending Request\n2. View Completed Request" << endl;
        cout << "3. Organise Blood Camp\n4. View Ongoing Camps" << endl;
        cout << "5. Donor Management\n6. Inventory Management\n7. Exit" << endl;
        cout << "Select an option (1-7): ";
        int choice = Utils::readIntLine();
        cout << endl;

        switch (choice)
        {
        case 1:
            // [5] Blood Request Manager — pending
            BloodRequestManager::viewPendingRequest();
            cout << "Press Enter to return to the Hospital Menu..." << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        case 2:
            // [5] Blood Request Manager — completed
            BloodRequestManager::viewCompletedRequest();
            cout << "Press Enter to return to the Hospital Menu..." << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        case 3:
            // [8] Camp Manager — organise
            CampManager::organiseCamp();
            cout << "Press Enter to return to the Hospital Menu..." << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        case 4:
            // POLYMORPHISM: showMenu() dispatches to CampManager::showMenu()
            campMgr.showMenu();
            cout << "Press Enter to return to the Hospital Menu..." << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        case 5:
            // POLYMORPHISM: showMenu() dispatches to DonorManager::showMenu()
            donorMgr.showMenu();
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

// ============================================================================
// ENTRY POINT
// ABSTRACTION: main() is a single line — the entire program is behind
//              Application::indexMain().
// ============================================================================
int main()
{
    Application::indexMain();
    return 0;
}