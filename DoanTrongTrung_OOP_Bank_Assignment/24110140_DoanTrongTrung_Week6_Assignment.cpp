#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>

using namespace std;

// Forward declarations
class Transaction;
class Account;

// Transaction class to represent banking transactions
class Transaction {
protected:
    double amount;
    string type;
    string date;

public:
    // Constructor
    Transaction(double amt, const string& transType)
        : amount(amt), type(transType) {
        time_t now = time(0);
        char* dt = ctime(&now);
        date = string(dt);
        date.pop_back(); // Remove newline character
    }

    // Virtual destructor for safe polymorphic deletion
    virtual ~Transaction() {}

    // Getters
    double getAmount() const { return amount; }
    string getType() const { return type; }
    string getDate() const { return date; }

    // Virtual display method for polymorphism
    virtual void displayTransaction() const {
        cout << "Transaction: " << type
             << " - $" << fixed << setprecision(2) << amount
             << " on " << date << endl;
    }
};

// Base Account class
class Account {
protected:
    static int nextAccountNumber; // Static member for auto-generating account numbers
    int accountNumber;
    double balance;
    string ownerName;
    vector<Transaction> transactionHistory;

public:
    // Constructor
    Account(const string& owner, double initialBalance = 0.0) 
        : ownerName(owner), balance(initialBalance) {
        accountNumber = nextAccountNumber++;
    }

    // Virtual destructor for proper inheritance
    virtual ~Account() {}

    // Virtual methods for polymorphism
    virtual void deposit(double amount) {
        if (amount > 0) {
            balance += amount;
            transactionHistory.push_back(Transaction(amount, "Deposit"));
            cout << "Deposited $" << fixed << setprecision(2) << amount 
                 << " to account " << accountNumber << endl;
        } else {
            cout << "Error: Deposit amount must be positive!" << endl;
        }
    }

    virtual bool withdraw(double amount) {
        if (amount <= 0) {
            cout << "Error: Withdrawal amount must be positive!" << endl;
            return false;
        }
        if (amount > balance) {
            cout << "Error: Insufficient funds! Balance: $" << fixed << setprecision(2) 
                 << balance << endl;
            return false;
        }
        balance -= amount;
        transactionHistory.push_back(Transaction(amount, "Withdrawal"));
        cout << "Withdrew $" << fixed << setprecision(2) << amount 
             << " from account " << accountNumber << endl;
        return true;
    }

    // Transfer money to another account
    bool transfer(Account& toAccount, double amount) {
        if (this->withdraw(amount)) {
            toAccount.deposit(amount);
            cout << "Transfer successful: $" << fixed << setprecision(2) 
                 << amount << " from account " << accountNumber 
                 << " to account " << toAccount.getAccountNumber() << endl;
            return true;
        }
        return false;
    }

    // Display account information
    virtual void displayInfo() const {
        cout << "\n--- Account Information ---" << endl;
        cout << "Account Number: " << accountNumber << endl;
        cout << "Owner: " << ownerName << endl;
        cout << "Balance: $" << fixed << setprecision(2) << balance << endl;
        cout << "Transaction History:" << endl;
        if (transactionHistory.empty()) {
            cout << "  No transactions" << endl;
        } else {
            for (const auto& trans : transactionHistory) {
                cout << "  ";
                trans.displayTransaction();
            }
        }
    }

    // Getters
    int getAccountNumber() const { return accountNumber; }
    double getBalance() const { return balance; }
    string getOwnerName() const { return ownerName; }

    // Operator overloading
    // += operator for adding transactions
    Account& operator+=(const Transaction& trans) {
        if (trans.getType() == "Deposit") {
            deposit(trans.getAmount());
        }
        return *this;
    }

    // == operator for comparing accounts (by balance)
    bool operator==(const Account& other) const {
        return balance == other.balance;
    }

    // > operator for comparing balances
    bool operator>(const Account& other) const {
        return balance > other.balance;
    }

    // Friend function for << operator
    friend ostream& operator<<(ostream& os, const Account& account) {
        os << "Account " << account.accountNumber << " (" << account.ownerName 
           << "): $" << fixed << setprecision(2) << account.balance;
        return os;
    }
};

// Derived SavingsAccount class
class SavingsAccount : public Account {
private:
    double interestRate;
    double withdrawalLimit;
    int withdrawalsThisMonth;
    static const int MAX_WITHDRAWALS = 6; // Federal savings account limit

public:
    // Constructor
    SavingsAccount(const string& owner, double initialBalance = 0.0, 
                   double rate = 0.02, double limit = 1000.0) 
        : Account(owner, initialBalance), interestRate(rate), 
          withdrawalLimit(limit), withdrawalsThisMonth(0) {}

    // Override withdraw method with savings account restrictions
    bool withdraw(double amount) override {
        if (withdrawalsThisMonth >= MAX_WITHDRAWALS) {
            cout << "Error: Exceeded monthly withdrawal limit for savings account!" << endl;
            return false;
        }
        if (amount > withdrawalLimit) {
            cout << "Error: Withdrawal amount exceeds limit of $" 
                 << fixed << setprecision(2) << withdrawalLimit << endl;
            return false;
        }
        
        if (Account::withdraw(amount)) { // Call base class method
            withdrawalsThisMonth++;
            return true;
        }
        return false;
    }

    // Apply monthly interest
    void applyInterest() {
        double interest = balance * (interestRate / 12); // Monthly interest
        balance += interest;
        transactionHistory.push_back(Transaction(interest, "Interest"));
        cout << "Applied monthly interest: $" << fixed << setprecision(2) 
             << interest << " to savings account " << accountNumber << endl;
    }

    // Reset monthly withdrawal counter (would be called monthly)
    void resetMonthlyWithdrawals() {
        withdrawalsThisMonth = 0;
    }

    // Override displayInfo to show savings-specific information
    void displayInfo() const override {
        Account::displayInfo(); // Call base class method
        cout << "Account Type: Savings Account" << endl;
        cout << "Interest Rate: " << fixed << setprecision(2) 
             << (interestRate * 100) << "% annually" << endl;
        cout << "Withdrawal Limit: $" << fixed << setprecision(2) 
             << withdrawalLimit << endl;
        cout << "Withdrawals This Month: " << withdrawalsThisMonth 
             << "/" << MAX_WITHDRAWALS << endl;
    }

    // Getters
    double getInterestRate() const { return interestRate; }
    double getWithdrawalLimit() const { return withdrawalLimit; }
};

// Derived BusinessAccount class
class BusinessAccount : public Account {
private:
    double transactionFee;
    double creditLimit;
public:
    BusinessAccount(const string& owner, double initialBalance, double fee, double limit)
        : Account(owner, initialBalance), transactionFee(fee), creditLimit(limit) {}

    bool withdraw(double amount) override {
        if (amount + transactionFee > balance + creditLimit) {
            cout << "Error: Exceeds credit limit!" << endl;
            return false;
        }
        balance -= (amount + transactionFee);
        transactionHistory.push_back(Transaction(amount, "Business Withdrawal"));
        transactionHistory.push_back(Transaction(transactionFee, "Transaction Fee"));
        return true;
    }

    void displayInfo() const override {
        Account::displayInfo();
        cout << "Account Type: Business Account" << endl;
        cout << "Transaction Fee: $" << fixed << setprecision(2) << transactionFee << endl;
        cout << "Credit Limit: $" << fixed << setprecision(2) << creditLimit << endl;
    }
};

// Derived RecurringTransaction class
class RecurringTransaction : public Transaction {
private:
    int recurrenceDays;
public:
    RecurringTransaction(double amt, const string& type, int days)
        : Transaction(amt, type), recurrenceDays(days) {}

    void displayTransaction() const override {
        Transaction::displayTransaction();
        cout << "Recurring every " << recurrenceDays << " days." << endl;
    }
};

// Customer class to manage multiple accounts
class Customer {

private:
    string name;
    int customerID;
    vector<Account*> accounts; // Using pointers to support polymorphism
    static int nextCustomerID;

public:
    // Constructor
    Customer(const string& customerName) : name(customerName) {
        customerID = nextCustomerID++;
    }

    // Destructor
    ~Customer() {
        // Clean up dynamically allocated accounts if any
        // Note: In this implementation, we're not dynamically allocating
    }

    // Add account to customer
    void addAccount(Account* account) {
        accounts.push_back(account);
        cout << "Account " << account->getAccountNumber() 
             << " added to customer " << name << endl;
    }

    // Calculate total balance across all accounts
    double getTotalBalance() const {
        double total = 0.0;
        for (const auto& account : accounts) {
            total += account->getBalance();
        }
        return total;
    }

    // Display customer information
    void displayCustomerInfo() const {
        cout << "\n=== Customer Information ===" << endl;
        cout << "Customer ID: " << customerID << endl;
        cout << "Name: " << name << endl;
        cout << "Total Accounts: " << accounts.size() << endl;
        cout << "Total Balance: $" << fixed << setprecision(2) 
             << getTotalBalance() << endl;
        
        cout << "\nAccount Details:" << endl;
        for (const auto& account : accounts) {
            cout << *account << endl; // Using overloaded << operator
        }
    }

    // Find account by account number
    Account* findAccount(int accountNum) {
        for (auto& account : accounts) {
            if (account->getAccountNumber() == accountNum) {
                return account;
            }
        }
        return nullptr;
    }

    // Getters
    string getName() const { return name; }
    int getCustomerID() const { return customerID; }
    const vector<Account*>& getAccounts() const { return accounts; }
};

// Initialize static member
int Customer::nextCustomerID = 1;
int Account::nextAccountNumber = 1;

// Operations class to manage all banking operations
class Operations {
private:
    vector<Customer*> customers;
    vector<Account*> allAccounts;

public:
    // Constructor
    Operations() {}

    // Destructor
    ~Operations() {
        // Clean up if needed
    }

    // Create a new customer
    Customer* createCustomer(const string& name) {
        Customer* newCustomer = new Customer(name);
        customers.push_back(newCustomer);
        cout << "Customer created: " << name << " (ID: " 
             << newCustomer->getCustomerID() << ")" << endl;
        return newCustomer;
    }

    // Create a regular account
    Account* createAccount(const string& ownerName, double initialBalance = 0.0) {
        Account* newAccount = new Account(ownerName, initialBalance);
        allAccounts.push_back(newAccount);
        cout << "Regular account created for " << ownerName 
             << " (Account #" << newAccount->getAccountNumber() << ")" << endl;
        return newAccount;
    }

    // Create a savings account
    SavingsAccount* createSavingsAccount(const string& ownerName, 
                                       double initialBalance = 0.0, 
                                       double rate = 0.02, 
                                       double limit = 1000.0) {
        SavingsAccount* newAccount = new SavingsAccount(ownerName, initialBalance, rate, limit);
        allAccounts.push_back(newAccount);
        cout << "Savings account created for " << ownerName 
             << " (Account #" << newAccount->getAccountNumber() << ")" << endl;
        return newAccount;
    }

    // Create a business account
    BusinessAccount* createBusinessAccount(const string& ownerName, double initialBalance, double fee, double limit) {
        BusinessAccount* newAccount = new BusinessAccount(ownerName, initialBalance, fee, limit);
        allAccounts.push_back(newAccount);
        cout << "Business account created for " << ownerName 
             << " (Account #" << newAccount->getAccountNumber() << ")" << endl;
        return newAccount;
    }

    // Link account to customer
    void assignAccountToCustomer(Customer* customer, Account* account) {
        if (customer && account) {
            customer->addAccount(account);
            cout << "Account " << account->getAccountNumber() 
                 << " assigned to customer " << customer->getName() << endl;
        } else {
            cout << "Error: Invalid customer or account!" << endl;
        }
    }

    // Perform deposit operation
    bool performDeposit(int accountNumber, double amount) {
        Account* account = findAccountByNumber(accountNumber);
        if (account) {
            account->deposit(amount);
            return true;
        } else {
            cout << "Error: Account " << accountNumber << " not found!" << endl;
            return false;
        }
    }

    // Perform withdrawal operation
    bool performWithdrawal(int accountNumber, double amount) {
        Account* account = findAccountByNumber(accountNumber);
        if (account) {
            return account->withdraw(amount);
        } else {
            cout << "Error: Account " << accountNumber << " not found!" << endl;
            return false;
        }
    }

    // Perform transfer operation
    bool performTransfer(int fromAccountNumber, int toAccountNumber, double amount) {
        Account* fromAccount = findAccountByNumber(fromAccountNumber);
        Account* toAccount = findAccountByNumber(toAccountNumber);
        
        if (fromAccount && toAccount) {
            return fromAccount->transfer(*toAccount, amount);
        } else {
            cout << "Error: One or both accounts not found!" << endl;
            return false;
        }
    }

    // Apply interest to all savings accounts
    void applyInterestToAllSavings() {
        cout << "\n--- Applying Interest to All Savings Accounts ---" << endl;
        int count = 0;
        for (Account* account : allAccounts) {
            // Try to cast to SavingsAccount
            SavingsAccount* savingsAcc = dynamic_cast<SavingsAccount*>(account);
            if (savingsAcc) {
                savingsAcc->applyInterest();
                count++;
            }
        }
        cout << "Interest applied to " << count << " savings accounts." << endl;
    }

    // Display all customers
    void displayAllCustomers() const {
        cout << "\n=== ALL CUSTOMERS ===" << endl;
        if (customers.empty()) {
            cout << "No customers in the system." << endl;
            return;
        }
        
        for (const Customer* customer : customers) {
            customer->displayCustomerInfo();
            cout << string(50, '-') << endl;
        }
    }

    // Display all accounts
    void displayAllAccounts() const {
        cout << "\n=== ALL ACCOUNTS ===" << endl;
        if (allAccounts.empty()) {
            cout << "No accounts in the system." << endl;
            return;
        }
        
        for (const Account* account : allAccounts) {
            account->displayInfo();
            cout << string(40, '-') << endl;
        }
    }

    // Display system summary
    void displaySystemSummary() const {
        cout << "\n=== BANKING SYSTEM SUMMARY ===" << endl;
        cout << "Total Customers: " << customers.size() << endl;
        cout << "Total Accounts: " << allAccounts.size() << endl;
        
        double totalSystemBalance = 0.0;
        int regularAccounts = 0, savingsAccounts = 0;
        
        for (const Account* account : allAccounts) {
            totalSystemBalance += account->getBalance();
            if (dynamic_cast<const SavingsAccount*>(account)) {
                savingsAccounts++;
            } else {
                regularAccounts++;
            }
        }
        
        cout << "Regular Accounts: " << regularAccounts << endl;
        cout << "Savings Accounts: " << savingsAccounts << endl;
        cout << "Total System Balance: $" << fixed << setprecision(2) 
             << totalSystemBalance << endl;
    }

    // Find customer by ID
    Customer* findCustomerById(int customerID) {
        for (Customer* customer : customers) {
            if (customer->getCustomerID() == customerID) {
                return customer;
            }
        }
        return nullptr;
    }

    // Find account by account number
    Account* findAccountByNumber(int accountNumber) {
        for (Account* account : allAccounts) {
            if (account->getAccountNumber() == accountNumber) {
                return account;
            }
        }
        return nullptr;
    }

    // Monthly operations (reset withdrawal counters, apply interest)
    void performMonthlyOperations() {
        cout << "\n--- Performing Monthly Operations ---" << endl;
        
        // Reset withdrawal counters for savings accounts
        for (Account* account : allAccounts) {
            SavingsAccount* savingsAcc = dynamic_cast<SavingsAccount*>(account);
            if (savingsAcc) {
                savingsAcc->resetMonthlyWithdrawals();
            }
        }
        
        // Apply interest to all savings accounts
        applyInterestToAllSavings();
        
        cout << "Monthly operations completed." << endl;
    }

    // Get system statistics
    void getSystemStatistics() const {
        cout << "\n=== SYSTEM STATISTICS ===" << endl;
        
        if (allAccounts.empty()) {
            cout << "No accounts in the system for statistics." << endl;
            return;
        }
        
        double totalBalance = 0.0;
        double maxBalance = allAccounts[0]->getBalance();
        double minBalance = allAccounts[0]->getBalance();
        Account* richestAccount = allAccounts[0];
        
        for (const Account* account : allAccounts) {
            double balance = account->getBalance();
            totalBalance += balance;
            
            if (balance > maxBalance) {
                maxBalance = balance;
                richestAccount = const_cast<Account*>(account);
            }
            if (balance < minBalance) {
                minBalance = balance;
            }
        }
        
        double averageBalance = totalBalance / allAccounts.size();
        
        cout << "Average Account Balance: $" << fixed << setprecision(2) 
             << averageBalance << endl;
        cout << "Highest Balance: $" << fixed << setprecision(2) 
             << maxBalance << " (Account #" << richestAccount->getAccountNumber() << ")" << endl;
        cout << "Lowest Balance: $" << fixed << setprecision(2) << minBalance << endl;
    }
};

// Main function with comprehensive testing
int main() {
    cout << "=== Bank Account Management System ===" << endl;
    cout << "Testing Object-Oriented Programming Concepts with Operations Class\n" << endl;

    // Create the Operations manager
    Operations bankSystem;

    // Test 1: Create customers using Operations class
    cout << "1. Creating Customers using Operations class..." << endl;
    Customer* alice = bankSystem.createCustomer("Alice Johnson");
    Customer* bob = bankSystem.createCustomer("Bob Smith");
    Customer* charlie = bankSystem.createCustomer("Charlie Brown");

    // Test 2: Create different types of accounts using Operations class
    cout << "\n2. Creating Accounts using Operations class..." << endl;
    Account* aliceChecking = bankSystem.createAccount("Alice Johnson", 1000.0);
    SavingsAccount* aliceSavings = bankSystem.createSavingsAccount("Alice Johnson", 5000.0, 0.025, 500.0);
    Account* bobChecking = bankSystem.createAccount("Bob Smith", 750.0);
    BusinessAccount* bobBusiness = bankSystem.createBusinessAccount("Bob Smith", 5000.0, 10.0, 1000.0);
    SavingsAccount* charlieSavings = bankSystem.createSavingsAccount("Charlie Brown", 3000.0, 0.03, 300.0);

    // Test 3: Assign accounts to customers using Operations class
    cout << "\n3. Assigning Accounts to Customers..." << endl;
    bankSystem.assignAccountToCustomer(alice, aliceChecking);
    bankSystem.assignAccountToCustomer(alice, aliceSavings);
    bankSystem.assignAccountToCustomer(bob, bobChecking);
    bankSystem.assignAccountToCustomer(bob, bobBusiness);
    bankSystem.assignAccountToCustomer(charlie, charlieSavings);

    // Test 4: Perform operations using Operations class
    cout << "\n4. Performing Banking Operations..." << endl;
    bankSystem.performDeposit(aliceChecking->getAccountNumber(), 200.0);
    bankSystem.performWithdrawal(aliceChecking->getAccountNumber(), 150.0);
    bankSystem.performDeposit(aliceSavings->getAccountNumber(), 1000.0);

    // Test 5: Transfer operations using Operations class
    cout << "\n5. Testing Transfer Operations..." << endl;
    bankSystem.performTransfer(aliceChecking->getAccountNumber(), 
                              bobChecking->getAccountNumber(), 100.0);

    // Test 6: Apply interest to all savings accounts
    cout << "\n6. Applying Interest to All Savings Accounts..." << endl;
    bankSystem.applyInterestToAllSavings();
    // Additional tests for BusinessAccount
    cout << "\nTesting Business Account Withdrawals..." << endl;

    cout << "\nTesting withdrawal within balance:" << endl;
    if (bobBusiness->withdraw(1000.0)) {
        cout << "Withdrawal successful. New balance: $" << fixed << setprecision(2) << bobBusiness->getBalance() << endl;
    } else {
        cout << "Withdrawal failed. Balance remains: $" << fixed << setprecision(2) << bobBusiness->getBalance() << endl;
    }

    cout << "\nTesting withdrawal exceeding balance but within credit limit:" << endl;
    if (bobBusiness->withdraw(4500.0)) {
        cout << "Withdrawal successful. New balance: $" << fixed << setprecision(2) << bobBusiness->getBalance() << endl;
    } else {
        cout << "Withdrawal failed. Balance remains: $" << fixed << setprecision(2) << bobBusiness->getBalance() << endl;
    }

    cout << "\nTesting withdrawal exceeding credit limit:" << endl;
    if (bobBusiness->withdraw(2000.0)) {
        cout << "Withdrawal successful. New balance: $" << fixed << setprecision(2) << bobBusiness->getBalance() << endl;
    } else {
        cout << "Withdrawal failed. Balance remains: $" << fixed << setprecision(2) << bobBusiness->getBalance() << endl;
    }

    // Hiển thị thông tin tài khoản sau các giao dịch
    cout << "\nBusiness Account Info after withdrawals:" << endl;
    bobBusiness->displayInfo();

    // Test 7: Test operator overloading (original functionality)
    cout << "\n7. Testing Operator Overloading..." << endl;
    
    // Test += operator
    cout << "Adding transaction using += operator:" << endl;
    Transaction bonusDeposit(100.0, "Deposit");
    *aliceChecking += bonusDeposit;

    // Test == operator
    cout << "\nComparing account balances using == operator:" << endl;
    if (*aliceChecking == *bobChecking) {
        cout << "Alice's checking and Bob's checking have equal balances" << endl;
    } else {
        cout << "Alice's checking and Bob's checking have different balances" << endl;
    }

    // Test > operator
    cout << "Comparing account balances using > operator:" << endl;
    if (*aliceSavings > *aliceChecking) {
        cout << "Alice's savings account has more money than her checking account" << endl;
    }
    // Test RecurringTransaction
    cout << "\nTesting Recurring Transactions..." << endl;
    RecurringTransaction netflixSub(15.99, "Subscription", 30);
    RecurringTransaction gymFee(45.00, "Gym Membership", 30);

    // Displaying recurring transactions
    cout << "Displaying recurring transactions:" << endl;
    netflixSub.displayTransaction();
    gymFee.displayTransaction();

    // Adding recurring transactions to Alice's checking account
    cout << "\nAdding recurring transactions to Alice's checking account:" << endl;
    // Simulate monthly deduction
    aliceChecking->withdraw(netflixSub.getAmount());
    aliceChecking->withdraw(gymFee.getAmount());
    // Also demonstrate += operator
    aliceChecking->operator+=(netflixSub);
    aliceChecking->operator+=(gymFee);

    // Display updated account info
    cout << "Updated info for Alice's checking account:" << endl;
    aliceChecking->displayInfo();

    // Test 8: Error handling with Operations class
    cout << "\n8. Testing Error Handling..." << endl;
    cout << "Attempting invalid operations:" << endl;
    bankSystem.performDeposit(9999, 100.0); // Non-existent account
    bankSystem.performWithdrawal(bobChecking->getAccountNumber(), 10000.0); // Overdraft attempt
    bankSystem.performTransfer(9999, 1001, 100.0); // Invalid account numbers

    // Test 9: Display system information using Operations class
    cout << "\n9. Displaying System Information..." << endl;
    bankSystem.displaySystemSummary();
    bankSystem.getSystemStatistics();

    // Test 10: Monthly operations
    cout << "\n10. Performing Monthly Operations..." << endl;
    bankSystem.performMonthlyOperations();

    // Test 11: Display all customers and accounts
    cout << "\n11. Displaying All Customers..." << endl;
    bankSystem.displayAllCustomers();

    cout << "\n12. Displaying All Accounts..." << endl;
    bankSystem.displayAllAccounts();

    // Test 12: Search functionality
    cout << "\n13. Testing Search Functionality..." << endl;
    Customer* foundCustomer = bankSystem.findCustomerById(alice->getCustomerID());
    if (foundCustomer) {
        cout << "Found customer: " << foundCustomer->getName() << endl;
    }

    Account* foundAccount = bankSystem.findAccountByNumber(aliceChecking->getAccountNumber());
    if (foundAccount) {
        cout << "Found account: " << *foundAccount << endl;
    }

    // Test 13: Polymorphism demonstration
    cout << "\n14. Demonstrating Polymorphism..." << endl;
    cout << "Adding small bonus to all accounts (polymorphic behavior):" << endl;
    vector<Account*> allSystemAccounts = {aliceChecking, aliceSavings, bobChecking, charlieSavings};
    
    for (Account* acc : allSystemAccounts) {
        cout << "Before bonus: " << *acc << endl;
        acc->deposit(25.0); // Virtual function call - polymorphic behavior
        cout << "After bonus: " << *acc << endl;
        cout << "---" << endl;
    }

    // Final system summary
    cout << "\n15. Final System Summary..." << endl;
    bankSystem.displaySystemSummary();
    bankSystem.getSystemStatistics();

    cout << "\n=== Complete System Testing with Operations Class Complete ===" << endl;
    return 0;
}