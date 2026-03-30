//
// ============================================================================
//  SANGUINE SYNC - Blood Bank Management System
//  OOP Design: All four pillars are used throughout this file.
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
//                      Runtime dispatch used in displayRecords<T>() helper.
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
#include <chrono>
#include <ctime>
#include <cmath>
#include <sstream>

using namespace std;

// ============================================================================
// UTILS CLASS
// ABSTRACTION: Wraps all low-level string helpers so no other class ever
//              duplicates this logic or exposes raw implementation.
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

// ============================================================================
// DATE UTILS CLASS - Date/Time Operations and Blood Expiry Management
//
// ENCAPSULATION: All date/time logic is hidden inside static methods
// ABSTRACTION: Complex date calculations are hidden behind simple method calls
// ============================================================================
class DateUtils
{
public:
    // ABSTRACTION: Current date retrieval hidden behind simple method call
    static string getCurrentDate()
    {
        auto now = chrono::system_clock::now();
        auto time_t_val = chrono::system_clock::to_time_t(now);
        tm tm_local;

        // Use standard localtime for cross-platform compatibility
        tm_local = *localtime(&time_t_val);

        char buffer[11];
        strftime(buffer, sizeof(buffer), "%d/%m/%Y", &tm_local);
        return string(buffer);
    }

    // ABSTRACTION: Date addition logic hidden - just call addDays()
    static string addDays(const string &date, int days)
    {
        // Parse DD/MM/YYYY to tm structure using stringstream
        stringstream ss(date);
        char slash;
        int day, month, year;
        ss >> day >> slash >> month >> slash >> year;

        tm tm_date = {};
        tm_date.tm_mday = day;
        tm_date.tm_mon = month - 1;    // Adjust month (0-11)
        tm_date.tm_year = year - 1900; // Adjust year
        tm_date.tm_hour = 0;
        tm_date.tm_min = 0;
        tm_date.tm_sec = 0;

        // Convert to time_t, add days, convert back
        auto time = mktime(&tm_date);
        time += days * 24 * 60 * 60; // Add days in seconds

        // Use standard localtime for cross-platform compatibility
        tm tm_result = *localtime(&time);

        char buffer[11];
        strftime(buffer, sizeof(buffer), "%d/%m/%Y", &tm_result);
        return string(buffer);
    }

    // ABSTRACTION: Date comparison logic hidden - just call isExpired()
    static bool isExpired(const string &expiryDate)
    {
        string current = getCurrentDate();
        return compareDates(expiryDate, current) < 0;
    }

    // ABSTRACTION: Date parsing and comparison hidden behind simple method
    static int compareDates(const string &date1, const string &date2)
    {
        tm tm1 = {}, tm2 = {};

        // Parse date1 using stringstream
        stringstream ss1(date1);
        char slash1;
        int day1, month1, year1;
        ss1 >> day1 >> slash1 >> month1 >> slash1 >> year1;
        tm1.tm_mday = day1;
        tm1.tm_mon = month1 - 1;
        tm1.tm_year = year1 - 1900;

        // Parse date2 using stringstream
        stringstream ss2(date2);
        char slash2;
        int day2, month2, year2;
        ss2 >> day2 >> slash2 >> month2 >> slash2 >> year2;
        tm2.tm_mday = day2;
        tm2.tm_mon = month2 - 1;
        tm2.tm_year = year2 - 1900;

        time_t time1 = mktime(&tm1);
        time_t time2 = mktime(&tm2);

        return (time1 < time2) ? -1 : (time1 > time2) ? 1
                                                      : 0;
    }

    // ABSTRACTION: Blood expiry calculation (42 days) hidden
    static string getBloodExpiryDate(const string &collectionDate)
    {
        return addDays(collectionDate, 42); // Blood expires after 42 days
    }
};

// ============================================================================
// HEALTH PROFILE CLASS - User's Health Information
//
// ENCAPSULATION: All health data is private with controlled access
// ABSTRACTION: BMI calculation is hidden behind getBMI() method
// ============================================================================
class HealthProfile
{
private:
    // ENCAPSULATION: Private health data fields
    string birthDate;
    double weight; // in kg
    double height; // in cm
    double bmi;    // calculated BMI

    // ABSTRACTION: BMI calculation logic is private and hidden
    double calculateBMI() const
    {
        double heightInMeters = height / 100.0; // Convert cm to meters
        return weight / (heightInMeters * heightInMeters);
    }

public:
    // ENCAPSULATION: Default constructor for map compatibility
    HealthProfile() : birthDate(""), weight(0), height(0), bmi(0) {}

    // ENCAPSULATION: Constructor ensures all data is valid at creation
    HealthProfile(const string &dob, double w, double h)
        : birthDate(dob), weight(w), height(h)
    {
        bmi = calculateBMI(); // Auto-calculate BMI
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
        bmi = calculateBMI(); // Recalculate BMI
    }

    void setHeight(double h)
    {
        if (h > 0 && h < 300)
            height = h;
        bmi = calculateBMI(); // Recalculate BMI
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
        // Basic eligibility based on BMI (18.5-35)
        return bmi >= 18.5 && bmi <= 35;
    }
};

// ============================================================================
// IRECORD — ABSTRACT BASE CLASS (Interface for all data records)
//
// ABSTRACTION:  Declares what every record must be able to do, without
//               specifying how. Callers work against this interface only.
//
// INHERITANCE:  BloodReqRecord, Donor, and Camp all inherit from IRecord.
//
// POLYMORPHISM: printRow() and toCSV() are pure virtual — each derived class
//               provides its own implementation, resolved at runtime.
// ============================================================================
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

// ============================================================================
// ACCOUNT — BASE CLASS (shared by User and Hospital)
//
// ENCAPSULATION: password is protected, not public — derived classes can
//                read it but callers cannot bypass getPassword().
//
// INHERITANCE:   User and Hospital both extend Account and inherit
//                password storage + getPassword() without duplicating code.
// ============================================================================
class Account
{
protected:
    // ENCAPSULATION: protected so derived classes can access it,
    //                but external code cannot touch it directly.
    string password;

public:
    // Constructor initialises the shared field
    Account(const string &pwd) : password(pwd) {}

    // ENCAPSULATION: controlled read access through a getter
    string getPassword() const { return password; }

    // ENCAPSULATION: controlled write access through a setter
    void setPassword(const string &pwd) { password = pwd; }

    // POLYMORPHISM: virtual destructor for safe polymorphic deletion
    virtual ~Account() {}
};

// ============================================================================
// USER CLASS
//
// INHERITANCE:   Inherits password field and getPassword()/setPassword()
//                from Account — no duplication.
//
// ENCAPSULATION: username, aadharNo, and healthProfile are private.
//                Health data is accessed through controlled methods.
//
// COMPOSITION:   User HAS-A HealthProfile - demonstrates composition relationship
// ============================================================================
class User : public Account
{ // INHERITANCE: User IS-AN Account
private:
    // ENCAPSULATION: private fields — not accessible outside the class
    string username;
    string aadharNo;

    // COMPOSITION: User HAS-A HealthProfile - demonstrates composition
    HealthProfile *healthProfile;

public:
    // Constructor delegates password initialisation up to Account
    // INHERITANCE: Account(pwd) initialises the inherited field
    User(const string &uname, const string &aadhar, const string &pwd)
        : Account(pwd), username(uname), aadharNo(aadhar), healthProfile(nullptr) {}

    // Enhanced constructor with health data
    User(const string &uname, const string &aadhar, const string &pwd,
         const string &birthDate, double weight, double height)
        : Account(pwd), username(uname), aadharNo(aadhar),
          healthProfile(new HealthProfile(birthDate, weight, height)) {}

    // DESTRUCTOR: Clean up health profile - demonstrates proper resource management
    ~User()
    {
        delete healthProfile;
    }

    // Copy constructor for proper deep copying
    User(const User &other)
        : Account(other), username(other.username), aadharNo(other.aadharNo)
    {
        healthProfile = other.healthProfile ? new HealthProfile(other.healthProfile->getBirthDate(),
                                                                other.healthProfile->getWeight(),
                                                                other.healthProfile->getHeight())
                                            : nullptr;
    }

    // Assignment operator for proper copying
    User &operator=(const User &other)
    {
        if (this != &other)
        {
            Account::operator=(other);
            username = other.username;
            aadharNo = other.aadharNo;

            delete healthProfile;
            healthProfile = other.healthProfile ? new HealthProfile(other.healthProfile->getBirthDate(),
                                                                    other.healthProfile->getWeight(),
                                                                    other.healthProfile->getHeight())
                                                : nullptr;
        }
        return *this;
    }

    // ENCAPSULATION: controlled read access
    string getUsername() const { return username; }
    string getAadharNo() const { return aadharNo; }

    // ENCAPSULATION: Health profile access with null checks
    bool hasHealthProfile() const { return healthProfile != nullptr; }

    string getBirthDate() const
    {
        return healthProfile ? healthProfile->getBirthDate() : "";
    }

    double getWeight() const
    {
        return healthProfile ? healthProfile->getWeight() : 0.0;
    }

    double getHeight() const
    {
        return healthProfile ? healthProfile->getHeight() : 0.0;
    }

    double getBMI() const
    {
        return healthProfile ? healthProfile->getBMI() : 0.0;
    }

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

    // ENCAPSULATION: Health profile management
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
        cout << "Donation Eligibility: " << (healthProfile->isEligibleForDonation() ? "ELIGIBLE" : "NOT ELIGIBLE") << endl;
        cout << "========================================" << endl;
    }
};

// ============================================================================
// HOSPITAL CLASS
//
// INHERITANCE:   Inherits password field and getPassword()/setPassword()
//                from Account — same base, different domain data.
//
// ENCAPSULATION: hospitalId and hospitalName are private.
// ============================================================================
class Hospital : public Account
{ // INHERITANCE: Hospital IS-AN Account
private:
    // ENCAPSULATION: private fields
    string hospitalId;
    string hospitalName;

public:
    // INHERITANCE: delegates password storage up to Account
    Hospital(const string &hid, const string &hname, const string &pwd)
        : Account(pwd), hospitalId(hid), hospitalName(hname) {}

    // ENCAPSULATION: controlled read access only — no setters (read-only after creation)
    string getHospitalId() const { return hospitalId; }
    string getHospitalName() const { return hospitalName; }
};

// ============================================================================
// CURRENT USER SESSION — Singleton-style static state
//
// ENCAPSULATION: username and aadharNo are private static fields.
//                External code can only read/write through set(), clear(),
//                and the getters — it can never corrupt the session directly.
// ============================================================================
class CurrentUser
{
private:
    // ENCAPSULATION: private static — session state is fully hidden
    static string username;
    static string aadharNo;

public:
    // ENCAPSULATION: the only way to set session data
    static void set(const string &uname, const string &aadhar)
    {
        username = uname;
        aadharNo = aadhar;
    }
    // ENCAPSULATION: controlled read
    static string getUsername() { return username; }
    static string getAadharNo() { return aadharNo; }
    // ENCAPSULATION: the only way to clear session data
    static void clear()
    {
        username.clear();
        aadharNo.clear();
    }
};
string CurrentUser::username = "";
string CurrentUser::aadharNo = "";

// ============================================================================
// CURRENT HOSPITAL SESSION — Same pattern as CurrentUser
//
// ENCAPSULATION: both fields private static; all access through methods.
// ============================================================================
class CurrentHospital
{
private:
    // ENCAPSULATION: private static fields
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

// ============================================================================
// BLOOD REQUEST RECORD CLASS
//
// INHERITANCE:   Extends IRecord — inherits the printRow/toCSV contract.
//
// ENCAPSULATION: All five fields are private; no direct mutation after
//                construction (status is immutable by design — no setter).
//
// POLYMORPHISM:  printRow() and toCSV() override IRecord's pure virtuals —
//                when called through an IRecord*, the right version runs.
// ============================================================================
class BloodReqRecord : public IRecord
{ // INHERITANCE: IS-AN IRecord
private:
    // ENCAPSULATION: all fields private
    string username;
    string aadharNo;
    string bloodGroup;
    string units;
    string status;

public:
    // Constructor with default status guard
    BloodReqRecord(const string &u, const string &a, const string &b,
                   const string &un, const string &st)
        : username(u), aadharNo(a), bloodGroup(b), units(un),
          status(st.empty() ? "pending" : Utils::trim(st)) {}

    // ENCAPSULATION: getters — no public setters (immutable record)
    string getUsername() const { return username; }
    string getAadharNo() const { return aadharNo; }
    string getBloodGroup() const { return bloodGroup; }
    string getUnits() const { return units; }
    string getStatus() const { return status; }

    // POLYMORPHISM: overrides IRecord::printRow() — called at runtime
    //               when iterating through vector<IRecord*>
    void printRow(int index) const override
    {
        cout << left << setw(5) << index
             << setw(12) << bloodGroup
             << setw(8) << units
             << setw(10) << status << endl;
    }

    // POLYMORPHISM: overrides IRecord::toCSV() — blood request serialisation
    string toCSV() const override
    {
        return "---\nUsername: " + username +
               "\nAadhaar No: " + aadharNo +
               "\nBlood Group: " + bloodGroup +
               "\nUnits Required: " + units +
               "\nStatus: " + status;
    }

    // ABSTRACTION: the entire file-parsing loop hidden behind one static call;
    //              callers simply receive vector<BloodReqRecord>
    static vector<BloodReqRecord> loadAll(const string &path)
    {
        vector<BloodReqRecord> list;
        ifstream in("textFiles/" + path);
        if (!in.is_open())
            return list;

        string u, a, b, un, st = "pending", line;

        // Lambda captures all fields; pushes a record when a block ends
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

// ============================================================================
// DONOR CLASS
//
// INHERITANCE:   Extends IRecord — must implement printRow() and toCSV().
//
// ENCAPSULATION: Nine private fields; only donationCompleted has a setter
//                because it is the only field that legitimately changes
//                post-construction.
//
// POLYMORPHISM:  printRow() and toCSV() override IRecord's pure virtuals.
// ============================================================================
class Donor : public IRecord
{ // INHERITANCE: Donor IS-AN IRecord
private:
    // ENCAPSULATION: all fields private
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

    // ENCAPSULATION: read-only access via getters
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

// ============================================================================
// CAMP CLASS
//
// INHERITANCE:   Extends IRecord.
//
// ENCAPSULATION: All four fields private; fully read-only after construction.
//
// POLYMORPHISM:  printRow() and toCSV() override IRecord's pure virtuals.
// ============================================================================
class Camp : public IRecord
{ // INHERITANCE: Camp IS-AN IRecord
private:
    // ENCAPSULATION: all fields private
    string name;
    string date;
    string venue;
    string city;

public:
    Camp(const string &n, const string &d, const string &v, const string &c)
        : name(n), date(d), venue(v), city(c) {}

    // ENCAPSULATION: getters only — no setters (camps are immutable)
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

    // ABSTRACTION: factory hides construction-from-parts detail
    static Camp createFromCSV(const vector<string> &parts)
    {
        if (parts.size() == 4)
            return Camp(parts[0], parts[1], parts[2], parts[3]);
        return Camp("", "", "", "");
    }
};

// ============================================================================
// BLOOD STOCK RECORD CLASS - Individual blood stock with expiry
//
// ENCAPSULATION: All blood stock data is private with controlled access
// INHERITANCE: Extends IRecord for polymorphic display capabilities
// ============================================================================
class BloodStockRecord : public IRecord
{
private:
    // ENCAPSULATION: Private blood stock fields
    string bloodGroup;
    int units;
    string collectionDate;
    string expiryDate;

public:
    // Constructor with automatic expiry calculation
    BloodStockRecord(const string &bg, int u, const string &cDate)
        : bloodGroup(bg), units(u), collectionDate(cDate)
    {
        // ABSTRACTION: Expiry calculation hidden in DateUtils
        expiryDate = DateUtils::getBloodExpiryDate(collectionDate);
    }

    // ENCAPSULATION: Getters for controlled access
    string getBloodGroup() const { return bloodGroup; }
    int getUnits() const { return units; }
    string getCollectionDate() const { return collectionDate; }
    string getExpiryDate() const { return expiryDate; }

    // ABSTRACTION: Expiry status check hidden behind simple method
    bool isExpired() const
    {
        return DateUtils::isExpired(expiryDate);
    }

    // ENCAPSULATION: Safe unit modification with validation
    bool addUnits(int u)
    {
        if (u > 0)
        {
            units += u;
            return true;
        }
        return false;
    }

    bool removeUnits(int u)
    {
        if (u > 0 && units >= u)
        {
            units -= u;
            return true;
        }
        return false;
    }

    // POLYMORPHISM: Implementation of IRecord interface
    void printRow(int index) const override
    {
        cout << left << setw(5) << index
             << setw(10) << bloodGroup
             << setw(12) << collectionDate
             << setw(12) << expiryDate
             << setw(8) << units
             << setw(10) << (isExpired() ? "EXPIRED" : "VALID") << endl;
    }

    // POLYMORPHISM: CSV serialization for blood stock
    string toCSV() const override
    {
        return bloodGroup + "," + to_string(units) + "," +
               collectionDate + "," + expiryDate;
    }
};

// ============================================================================
// INVENTORY CLASS
//
// ENCAPSULATION: VALID_BLOOD_GROUPS is a class-level constant — not a global.
//                All file paths, load/save logic, and validation are hidden
//                inside static methods; callers only call display() or menu().
//
// ABSTRACTION:   menu() hides the entire add/remove/view loop behind one call.
//                loadFromFile() hides all ifstream and parsing detail.
// ============================================================================
class Inventory
{
public:
    // ENCAPSULATION: constant lives inside the class, not in global scope
    static const vector<string> VALID_BLOOD_GROUPS;

    // ABSTRACTION: validation detail hidden; callers get a bool
    static bool isValidBloodGroup(const string &group)
    {
        for (const auto &g : VALID_BLOOD_GROUPS)
            if (g == group)
                return true;
        return false;
    }

    // ABSTRACTION: path construction logic hidden; callers just call getFilePath()
    static string getFilePath()
    {
        const string &hid = CurrentHospital::getHospitalId();
        return hid.empty() ? "textFiles/inventory.txt"
                           : "textFiles/inventory_" + hid + ".txt";
    }

    // ABSTRACTION: Enhanced loading with expiry dates
    static vector<BloodStockRecord> loadStockRecords(const string &filename)
    {
        vector<BloodStockRecord> stocks;
        ifstream in(filename);
        if (!in.is_open())
            return stocks;

        string line;
        while (getline(in, line))
        {
            line = Utils::trim(line);
            if (line.empty())
                continue;

            vector<string> parts = Utils::splitCSV(line, 4);
            if (parts.size() >= 2 && isValidBloodGroup(parts[0]))
            {
                try
                {
                    int units = stoi(parts[1]);
                    string collectionDate = parts.size() > 2 ? parts[2] : DateUtils::getCurrentDate();

                    // Create blood stock record with automatic expiry calculation
                    BloodStockRecord stock(parts[0], units, collectionDate);
                    stocks.push_back(stock);
                }
                catch (...)
                {
                    // Skip invalid records
                }
            }
        }
        return stocks;
    }

    // ABSTRACTION: Enhanced saving with expiry dates
    static bool saveStockRecords(const vector<BloodStockRecord> &stocks, const string &filename)
    {
        ofstream out(filename, ios::trunc);
        if (!out.is_open())
        {
            cout << "[ERROR] Could not save to " << filename << endl;
            return false;
        }

        for (const auto &stock : stocks)
        {
            out << stock.toCSV() << endl;
        }
        out.flush();
        return true;
    }

    // ABSTRACTION: Get available units (non-expired)
    static int getAvailableUnits(const string &bloodGroup, const vector<BloodStockRecord> &stocks)
    {
        int available = 0;
        for (const auto &stock : stocks)
        {
            if (stock.getBloodGroup() == bloodGroup && !stock.isExpired())
            {
                available += stock.getUnits();
            }
        }
        return available;
    }

    // ABSTRACTION: Get expired blood stocks
    static vector<BloodStockRecord> getExpiredStocks(const vector<BloodStockRecord> &stocks)
    {
        vector<BloodStockRecord> expired;
        for (const auto &stock : stocks)
        {
            if (stock.isExpired())
                expired.push_back(stock);
        }
        return expired;
    }

    // ABSTRACTION: Display expired blood stocks
    static void displayExpiredStocks(const vector<BloodStockRecord> &expired)
    {
        cout << "========================================" << endl;
        cout << "           EXPIRED BLOOD STOCKS         " << endl;
        cout << "========================================" << endl;

        if (expired.empty())
        {
            cout << "No expired blood stocks found." << endl;
            return;
        }

        cout << left << setw(5) << "NO." << setw(10) << "BLOOD"
             << setw(12) << "COLLECTED" << setw(12) << "EXPIRES"
             << setw(8) << "UNITS" << setw(10) << "STATUS" << endl;
        cout << string(57, '-') << endl;

        for (size_t i = 0; i < expired.size(); i++)
        {
            expired[i].printRow(static_cast<int>(i + 1));
        }
    }

    // ABSTRACTION: Enhanced display with expiry information
    static void displayWithExpiry()
    {
        auto stocks = loadStockRecords(getFilePath());
        cout << "========================================" << endl;
        cout << "             BLOOD INVENTORY            " << endl;
        cout << "========================================" << endl;
        cout << left << setw(5) << "NO." << setw(10) << "BLOOD"
             << setw(12) << "COLLECTED" << setw(12) << "EXPIRES"
             << setw(8) << "UNITS" << setw(10) << "STATUS" << endl;
        cout << string(57, '-') << endl;

        int idx = 1;
        for (const auto &g : VALID_BLOOD_GROUPS)
        {
            bool found = false;
            for (const auto &stock : stocks)
            {
                if (stock.getBloodGroup() == g)
                {
                    stock.printRow(idx++);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                // Show zero stock for missing blood groups
                cout << left << setw(5) << idx++ << setw(10) << g
                     << setw(12) << "N/A" << setw(12) << "N/A"
                     << setw(8) << "0" << setw(10) << "NO STOCK" << endl;
            }
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
            cout << "4. View Expired Blood" << endl;
            cout << "5. Delete Expired Blood" << endl;
            cout << "6. Back" << endl;
            cout << "Select an option (1-6): ";
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

                // Load existing stocks
                auto stocks = loadStockRecords(getFilePath());

                // Find or create stock record for this blood group
                bool found = false;
                for (auto &stock : stocks)
                {
                    if (stock.getBloodGroup() == group)
                    {
                        if (choice == 1)
                        {
                            stock.addUnits(units);
                            cout << units << " units added to " << group << "." << endl;
                        }
                        else
                        {
                            if (stock.getUnits() >= units)
                            {
                                stock.removeUnits(units);
                                cout << units << " units removed from " << group << "." << endl;
                            }
                            else
                            {
                                cout << "Not enough stock available for " << group << "." << endl;
                            }
                        }
                        found = true;
                        break;
                    }
                }

                if (!found && choice == 1)
                {
                    // Create new stock record with current date
                    BloodStockRecord newStock(group, units, DateUtils::getCurrentDate());
                    stocks.push_back(newStock);
                    cout << units << " units added to " << group << "." << endl;
                }

                // Save updated stocks
                saveStockRecords(stocks, getFilePath());
            }
            else if (choice == 3)
            {
                displayWithExpiry();
            }
            else if (choice == 4)
            {
                auto stocks = loadStockRecords(getFilePath());
                auto expired = getExpiredStocks(stocks);
                displayExpiredStocks(expired);
            }
            else if (choice == 5)
            {
                cout << "Removing expired blood stocks..." << endl;
                auto stocks = loadStockRecords(getFilePath());

                // Remove expired stocks
                auto it = stocks.begin();
                int removedCount = 0;
                while (it != stocks.end())
                {
                    if (it->isExpired())
                    {
                        cout << "Removing expired stock: " << it->getBloodGroup()
                             << " (" << it->getUnits() << " units)" << endl;
                        it = stocks.erase(it);
                        removedCount++;
                    }
                    else
                    {
                        ++it;
                    }
                }

                if (removedCount > 0)
                {
                    saveStockRecords(stocks, getFilePath());
                    cout << "Removed " << removedCount << " expired stock records." << endl;
                }
                else
                {
                    cout << "No expired stocks to remove." << endl;
                }
            }
            else if (choice == 6)
            {
                return;
            }
            else
            {
                cout << "Invalid choice. Please select 1-6." << endl;
            }

            if (choice != 6)
            {
                cout << "Press Enter to continue..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
    }
};
const vector<string> Inventory::VALID_BLOOD_GROUPS = {"A+", "A-", "B+", "B-", "O+", "O-", "AB+", "AB-"};

// ============================================================================
// FILE REPOSITORY CLASS
//
// ENCAPSULATION: All raw file I/O is centralised here. No other class ever
//                opens an ifstream or ofstream directly.
//
// ABSTRACTION:   Callers receive domain objects (User, Hospital, Donor, Camp,
//                BloodReqRecord) — they never deal with raw file lines.
// ============================================================================
class FileRepository
{
public:
    // ---- USER ----

    // ABSTRACTION: ifstream + CSV parsing hidden; returns ready vector<User>
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

    // ABSTRACTION: ofstream detail hidden; returns success bool
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

    // ENHANCED: Save user with health profile information
    static bool saveUserWithHealth(const string &username, const string &aadharNo,
                                   const string &pwd, const string &birthDate,
                                   double weight, double height)
    {
        // Save basic user info
        if (!saveUser(username, aadharNo, pwd))
        {
            return false;
        }

        // Save health profile to separate file
        ofstream out("textFiles/user_health.txt", ios::app);
        if (!out.is_open())
        {
            cout << "[Error] Could not open user_health.txt" << endl;
            return false;
        }

        // Format: aadharNo,birthDate,weight,height
        out << aadharNo << "," << birthDate << "," << fixed << setprecision(1)
            << weight << "," << fixed << setprecision(1) << height << "\n";
        out.flush();
        return true;
    }

    // ENHANCED: Load user with health profile
    static vector<User> loadUsersWithHealth(const string &filename)
    {
        vector<User> users = loadUsers(filename);

        // Load health profiles
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
                        string aadhar = parts[0];
                        string birthDate = parts[1];
                        double weight = stod(parts[2]);
                        double height = stod(parts[3]);

                        healthMap[aadhar] = HealthProfile(birthDate, weight, height);
                    }
                    catch (...)
                    {
                        // Skip invalid health records
                    }
                }
            }
        }

        // Create enhanced users with health profiles
        vector<User> enhancedUsers;
        for (auto &user : users)
        {
            User enhancedUser(user.getUsername(), user.getAadharNo(), user.getPassword());

            auto healthIt = healthMap.find(user.getAadharNo());
            if (healthIt != healthMap.end())
            {
                const auto &health = healthIt->second;
                enhancedUser.setHealthProfile(health.getBirthDate(),
                                              health.getWeight(),
                                              health.getHeight());
            }

            enhancedUsers.push_back(enhancedUser);
        }

        return enhancedUsers;
    }

    // ABSTRACTION: file scan hidden; callers get a plain bool
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

    // ---- HOSPITAL ----

    // ABSTRACTION: same pattern as loadUsers but returns vector<Hospital>
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

    // ---- DONOR ----

    // ABSTRACTION: uses Donor::createFromCSV factory; callers get vector<Donor>
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

    // POLYMORPHISM + ABSTRACTION: uses IRecord::toCSV() to serialise —
    //   the Donor's own toCSV() override is called through the base interface.
    static bool saveDonor(const Donor &donor, const string &filename)
    {
        ofstream out("textFiles/" + filename, ios::app);
        if (!out.is_open())
        {
            cout << "[ERROR] Could not save to " << filename << endl;
            return false;
        }
        // POLYMORPHISM: toCSV() dispatches to Donor::toCSV() at runtime
        out << donor.toCSV() << endl;
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
            // POLYMORPHISM: toCSV() dispatches to Donor::toCSV() at runtime
            out << d.toCSV() << endl;
        }
        out.flush();
        return true;
    }

    // ---- CAMP ----

    // ABSTRACTION: uses Camp::createFromCSV factory; callers get vector<Camp>
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

    // POLYMORPHISM + ABSTRACTION: uses IRecord::toCSV() to serialise
    static bool saveCamp(const Camp &camp, const string &filename)
    {
        ofstream out("textFiles/" + filename, ios::app);
        if (!out.is_open())
        {
            cout << "[ERROR] Could not save to " << filename << endl;
            return false;
        }
        // POLYMORPHISM: toCSV() dispatches to Camp::toCSV() at runtime
        out << camp.toCSV() << endl;
        out.flush();
        return true;
    }

    // ---- BLOOD REQUEST ----

    // ABSTRACTION: block-format file writing hidden; callers just call save()
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
        // POLYMORPHISM: toCSV() dispatches to BloodReqRecord::toCSV() at runtime
        BloodReqRecord rec(username, aadharNo, bloodGroup, units, "pending");
        out << rec.toCSV() << endl;
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
            // POLYMORPHISM: toCSV() dispatches to BloodReqRecord::toCSV() at runtime
            out << r.toCSV() << endl;
        }
        out.flush();
        return true;
    }
};

// ============================================================================
// IMANAGER — ABSTRACT BASE CLASS (Interface for all manager/menu classes)
//
// ABSTRACTION:  Declares what every manager must expose — just showMenu().
//               Application calls showMenu() without knowing the type.
//
// INHERITANCE:  DonorManager, CampManager, BloodRequestManager extend this.
//
// POLYMORPHISM: showMenu() is pure virtual — runtime dispatch selects the
//               correct menu when called through an IManager pointer.
// ============================================================================
class IManager
{
public:
    // POLYMORPHISM: pure virtual — each manager has its own menu
    virtual void showMenu() = 0;
    virtual ~IManager() {}
};

// ============================================================================
// DONOR MANAGER CLASS
//
// INHERITANCE:   Extends IManager — must implement showMenu().
//
// ENCAPSULATION: DONORS_FILE constant is private; file name is an
//                implementation detail hidden from callers.
//
// POLYMORPHISM:  showMenu() overrides IManager::showMenu().
// ============================================================================
class DonorManager : public IManager
{ // INHERITANCE: IS-AN IManager
private:
    // ENCAPSULATION: file path hidden as private constant
    static const string DONORS_FILE;

public:
    // POLYMORPHISM: overrides IManager::showMenu(); called at runtime
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
    // ENCAPSULATION: accesses donor fields only through getters (getBloodGroup, getHospitalId, etc.)
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

        // ENCAPSULATION: filters using getters — no direct field access
        for (const auto &d : donors)
        {
            if (Utils::toUpper(d.getBloodGroup()) == searchGroup &&
                d.getHospitalId() == CurrentHospital::getHospitalId())
                matched.push_back(d);
        }

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
        cout << left << setw(5) << "NO." << setw(20) << "NAME"
             << setw(25) << "EMAIL" << setw(12) << "DOB"
             << setw(10) << "BLOOD" << setw(6) << "AGE"
             << setw(15) << "DISEASE" << setw(10) << "STATUS" << endl;
        cout << string(103, '-') << endl;

        for (size_t i = 0; i < matched.size(); i++)
        {
            // POLYMORPHISM: printRow() resolved to Donor::printRow() at runtime
            matched[i].printRow(static_cast<int>(i + 1));
        }
    }

    // ABSTRACTION + POLYMORPHISM: iterates donors, calls printRow() on each —
    //   printRow() is IRecord's virtual method, resolved to Donor::printRow()
    static void viewDonors()
    {
        auto donors = FileRepository::loadDonors(DONORS_FILE);
        vector<Donor> hospitalDonors;

        // ENCAPSULATION: uses getter getHospitalId(), not direct field access
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
            // POLYMORPHISM: printRow() is IRecord virtual, resolved to Donor::printRow()
            hospitalDonors[i].printRow(static_cast<int>(i + 1));
        }

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
                            // ENCAPSULATION: constructing a new Donor — fields stay private
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

// ============================================================================
// CAMP MANAGER CLASS
//
// INHERITANCE:   Extends IManager — must implement showMenu().
//
// ENCAPSULATION: CAMPS_FILE is private.
//
// POLYMORPHISM:  showMenu() overrides IManager::showMenu().
// ============================================================================
class CampManager : public IManager
{ // INHERITANCE: IS-AN IManager
private:
    // ENCAPSULATION: file path is an implementation detail
    static const string CAMPS_FILE;

public:
    // POLYMORPHISM: overrides IManager::showMenu()
    void showMenu() override
    {
        // CampManager has no interactive sub-menu in this flow;
        // showMenu() delegates directly to viewOngoingCamps for the hospital.
        viewOngoingCamps();
    }

    // ABSTRACTION + POLYMORPHISM: calls Camp::printRow() via IRecord*
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
            // POLYMORPHISM: printRow() resolved to Camp::printRow() at runtime
            camps[i].printRow(static_cast<int>(i + 1));
        }
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

        // ENCAPSULATION: Camp object created; fields immediately private
        Camp camp(name, date, venue, city);

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

// ============================================================================
// BLOOD REQUEST MANAGER CLASS
//
// INHERITANCE:   Extends IManager — must implement showMenu().
//
// ENCAPSULATION: BLOOD_REQ_FILE is private.
//
// POLYMORPHISM:  showMenu() overrides IManager::showMenu().
//                printRow() is called via IRecord interface inside display loops.
// ============================================================================
class BloodRequestManager : public IManager
{ // INHERITANCE: IS-AN IManager
private:
    // ENCAPSULATION: file name is an implementation detail
    static const string BLOOD_REQ_FILE;

public:
    // POLYMORPHISM: overrides IManager::showMenu()
    void showMenu() override
    {
        // BloodRequestManager's menu is context-specific (user vs hospital);
        // Application calls the appropriate method directly.
    }

    // ABSTRACTION: entire request form + validation hidden behind one call
    static void requestBlood()
    {
        // ENCAPSULATION: reads from CurrentUser via getters — no direct field access
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
        // ENCAPSULATION: uses getUsername() getter, not direct field
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
                // POLYMORPHISM: printRow() resolved to BloodReqRecord::printRow()
                mine[i].printRow(static_cast<int>(i + 1));
            }
        }
    }

    // ABSTRACTION: filter + display logic hidden
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
            {
                // POLYMORPHISM: printRow() resolved to BloodReqRecord::printRow()
                myCompleted[i].printRow(static_cast<int>(i + 1));
            }
        }
    }

    // ABSTRACTION: business logic (load, filter by stock, display, update) hidden
    static void viewPendingRequest()
    {
        auto all = BloodReqRecord::loadAll(BLOOD_REQ_FILE);
        // ENCAPSULATION: inventory loaded through Inventory's static methods
        auto stocks = Inventory::loadStockRecords(Inventory::getFilePath());

        vector<BloodReqRecord> pending;
        for (const auto &r : all)
        {
            string sl = r.getStatus();
            transform(sl.begin(), sl.end(), sl.begin(), [](unsigned char c)
                      { return tolower(c); });
            // ENCAPSULATION: uses getters on BloodReqRecord and Inventory::getAvailableUnits()
            if (sl == "pending" &&
                Inventory::getAvailableUnits(r.getBloodGroup(), stocks) >= stoi(r.getUnits()))
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
            // ABSTRACTION + POLYMORPHISM: username printed inline; printRow handles the rest
            cout << left << setw(5) << (i + 1) << setw(15) << pending[i].getUsername();
            // POLYMORPHISM: printRow() resolved to BloodReqRecord::printRow() — prints
            //   blood group, units, status columns starting after the username column
            cout << setw(12) << pending[i].getBloodGroup()
                 << setw(8) << pending[i].getUnits()
                 << setw(10) << pending[i].getStatus() << endl;
        }

        cout << endl;
        cout << "Enter request number to mark as completed (0 to go back): ";
        int choice = Utils::readIntLine();

        if (choice > 0 && choice <= static_cast<int>(pending.size()))
        {
            const auto &sel = pending[choice - 1];

            // Update inventory stocks
            for (auto &stock : stocks)
            {
                if (stock.getBloodGroup() == sel.getBloodGroup())
                {
                    stock.removeUnits(stoi(sel.getUnits()));
                    break;
                }
            }
            Inventory::saveStockRecords(stocks, Inventory::getFilePath());

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
                    // ENCAPSULATION: new BloodReqRecord created — fields immediately private
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
                     << stoi(sel.getUnits()) << " units deducted from inventory." << endl;
        }
    }

    // ABSTRACTION: filter + display hidden; callers just call this one method
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
            cout << left << setw(5) << (i + 1) << setw(15) << completed[i].getUsername()
                 << setw(12) << completed[i].getBloodGroup()
                 << setw(8) << completed[i].getUnits()
                 << setw(10) << completed[i].getStatus() << endl;
        }
    }
};
const string BloodRequestManager::BLOOD_REQ_FILE = "BloodReq.txt";

// ============================================================================
// REGISTRATION MANAGER CLASS
//
// ENCAPSULATION: Registration logic (validation, duplicate checks, saving)
//                is bundled here; Application never touches file paths or
//                validation patterns.
//
// ABSTRACTION:   registerUser() and registerHospital() are each one call
//                from the outside, hiding looping, validation, and I/O.
// ============================================================================

// Forward declarations needed before RegistrationManager and Application
void userLogIn();
void hospLogIn();

class RegistrationManager
{
public:
    // ABSTRACTION: entire registration flow (loop, validate, check duplicate,
    //              save, redirect to login) behind a single method call
    // ENHANCED: Now includes health profile collection
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

            // ENHANCED: Collect health profile information
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
                {
                    break;
                }
                cout << "Invalid date format. Please use DD/MM/YYYY format." << endl;
            }

            double weight = 0;
            while (true)
            {
                cout << "Enter weight (kg): ";
                string weightStr;
                getline(cin, weightStr);
                weightStr = Utils::trim(weightStr);
                try
                {
                    weight = stod(weightStr);
                    if (weight > 0 && weight < 500)
                    {
                        break;
                    }
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
                string heightStr;
                getline(cin, heightStr);
                heightStr = Utils::trim(heightStr);
                try
                {
                    height = stod(heightStr);
                    if (height > 0 && height < 300)
                    {
                        break;
                    }
                    cout << "Height must be between 1 and 300 cm." << endl;
                }
                catch (...)
                {
                    cout << "Invalid height. Please enter a valid number." << endl;
                }
            }

            // Create health profile and show calculated BMI
            HealthProfile healthProfile(birthDate, weight, height);
            cout << "\nBMI calculated: " << fixed << setprecision(1) << healthProfile.getBMI()
                 << " (" << healthProfile.getBMICategory() << ")" << endl;
            cout << "Donation Eligibility: " << (healthProfile.isEligibleForDonation() ? "ELIGIBLE" : "NOT ELIGIBLE") << endl;

            cout << "\nConfirm registration? (y/n): ";
            string confirm;
            getline(cin, confirm);
            confirm = Utils::trim(Utils::toUpper(confirm));
            if (confirm != "Y" && confirm != "YES")
            {
                cout << "Registration cancelled." << endl;
                continue;
            }

            // ABSTRACTION: duplicate check hidden in FileRepository
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

            // ENHANCED: Save user with health profile
            if (FileRepository::saveUserWithHealth(username, aadharNo, password, birthDate, weight, height))
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

// ============================================================================
// APPLICATION CLASS — Top-level navigation & menus
//
// ENCAPSULATION: Navigation logic (index, login page, user menu, hospital
//                menu) is bundled here; main() is a single line.
//
// ABSTRACTION:   indexMain() hides the entire program loop behind one call.
//                userMenu() and hospMenu() hide their switch/loop internals.
//
// POLYMORPHISM:  hospMenu() holds IManager* pointers for DonorManager,
//                CampManager, and BloodRequestManager — showMenu() is
//                dispatched at runtime through the base pointer.
// ============================================================================
class Application
{
public:
    // ABSTRACTION: entire program entry hidden; main() is just one call
    static void indexMain()
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

    // ABSTRACTION: login-type selection hidden; callers just call logInPage()
    static void logInPage()
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

    // ABSTRACTION: entire user session menu hidden behind one call
    // ENHANCED: Now includes health profile viewing option
    static void userMenu()
    {
        while (true)
        {
            cout << "========================================" << endl;
            cout << "             USER MENU                  " << endl;
            cout << "========================================" << endl;
            cout << "1. Request Blood\n2. Show My Requests\n3. View Completed Requests" << endl;
            cout << "4. View Blood Camps\n5. View Inventory\n6. View My Health Profile" << endl;
            cout << "7. Exit" << endl;
            cout << "Select an option (1-7): ";
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
                Inventory::displayWithExpiry();
                cout << "Press Enter to return to the User Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 6:
                // ENHANCED: Display user health profile
                displayUserHealthProfile();
                cout << "Press Enter to return to the User Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 7:
                // ENCAPSULATION: session cleared through CurrentUser::clear(), not by direct field access
                CurrentUser::clear();
                cout << "Returning to main page..." << endl;
                indexMain();
                return;
            default:
                cout << "Invalid choice. Please select 1, 2, 3, 4, 5, 6, or 7." << endl
                     << endl;
                break;
            }
        }
    }

    // ENHANCED: Display current user's health profile
    static void displayUserHealthProfile()
    {
        string username = CurrentUser::getUsername();
        string aadharNo = CurrentUser::getAadharNo();

        if (username.empty())
        {
            cout << "[ERROR] No user logged in." << endl;
            return;
        }

        // Load user with health profile
        auto users = FileRepository::loadUsersWithHealth("users.txt");

        // Find current user
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

    // ABSTRACTION + POLYMORPHISM: hospMenu owns IManager* objects and calls
    //   showMenu() — the correct override runs at runtime for each manager.
    static void hospMenu()
    {
        // POLYMORPHISM: concrete managers stored as IManager pointers
        //   showMenu() will dispatch to the correct override at runtime.
        DonorManager donorMgr;
        CampManager campMgr;
        BloodRequestManager reqMgr;

        while (true)
        {
            // ENCAPSULATION: inventory and requests loaded through class APIs
            auto all = BloodReqRecord::loadAll("BloodReq.txt");
            auto stocks = Inventory::loadStockRecords(Inventory::getFilePath());

            vector<BloodReqRecord> pending;
            for (const auto &r : all)
            {
                string sl = r.getStatus();
                transform(sl.begin(), sl.end(), sl.begin(), [](unsigned char c)
                          { return tolower(c); });
                if (sl == "pending" &&
                    Inventory::getAvailableUnits(r.getBloodGroup(), stocks) >= stoi(r.getUnits()))
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
                // POLYMORPHISM: showMenu() on IManager* dispatches to CampManager::showMenu()
                campMgr.showMenu();
                cout << "Press Enter to return to the Hospital Menu..." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            case 5:
                // POLYMORPHISM: showMenu() on IManager* dispatches to DonorManager::showMenu()
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
};

// ============================================================================
// AUTH MANAGER CLASS
//
// ENCAPSULATION: Authentication logic (load users, compare credentials,
//                set session) is bundled here.  Application only calls
//                userLogIn() / hospLogIn(); it never touches the User or
//                Hospital vectors directly.
//
// ABSTRACTION:   Full auth flow hidden; callers get a logged-in session
//                or an error message — nothing more.
// ============================================================================
class AuthManager
{
public:
    // ABSTRACTION: entire user auth flow (load, compare, session-set, menu) in one call
    static void userLogIn()
    {
        while (true)
        {
            // ABSTRACTION: file loading hidden inside FileRepository
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
            // INHERITANCE: getPassword() is defined in Account, inherited by User
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
                    Application::indexMain();
                    return;
                default:
                    cout << "Invalid choice. Returning to main menu." << endl;
                    return;
                }
            }
            else
            {
                // Data privacy: Aadhar masked for display
                string maskedAadhar = "********";
                if (foundAadhar.size() >= 4)
                    maskedAadhar += foundAadhar.substr(foundAadhar.size() - 4);
                else
                    maskedAadhar += foundAadhar;

                cout << "\n=========================================" << endl;
                cout << "           LOGIN SUCCESSFUL!            " << endl;
                cout << "=========================================" << endl;
                cout << "Welcome, " << inputUsername << "!" << endl;
                cout << "Aadhar: " << maskedAadhar << endl;
                cout << "You have successfully signed in.\n"
                     << endl;

                // ENCAPSULATION: session stored through CurrentUser::set(), not direct assignment
                CurrentUser::set(inputUsername, foundAadhar);
                Application::userMenu();
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
            // INHERITANCE: getPassword() inherited from Account — no duplication
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
                    return;
                case 3:
                    Application::indexMain();
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
                Application::hospMenu();
                CurrentHospital::clear();
                return;
            }
        }
    }
};

// ============================================================================
// Forward-declaration bodies — route free functions to AuthManager
// ============================================================================
void userLogIn() { AuthManager::userLogIn(); }
void hospLogIn() { AuthManager::hospLogIn(); }

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