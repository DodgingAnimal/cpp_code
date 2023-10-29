#include <iostream>
#include <tuple>
#include "mariadb/mysql.h"
#include "mariadb/mysqld_error.h"

/*
    Connection to MariaDb related funtions
*/
struct SQLConnection
{
    // a struct for retaining connection info
    std::string server, user, password, database;

    SQLConnection(std::string server, std::string user, std::string password, std::string database)
    {
        this->server = server;
        this->user = user;
        this->password = password;
        this->database = database;
    }
};
// using struct as a method for returning multiple values
struct SqlResponse
{
    bool success;
    MYSQL_RES *res;
};

std::tuple<bool, MYSQL *> SqlConnectionSetup(SQLConnection mysql_details)
{
    // pass in detail and receive connection back if success

    // call init firt (ref: https://dev.mysql.com/doc/c-api/8.0/en/mysql-real-connect.html)
    MYSQL *connection = mysql_init(NULL); // mysql instance
    bool success = true;

    // connect database
    // c_str -> converts std::string to char
    connection = mysql_real_connect(connection, mysql_details.server.c_str(), \
        mysql_details.user.c_str(), mysql_details.password.c_str(), \
        mysql_details.database.c_str(), 0, NULL, 0);
    if (!connection)
    {
        success = false;
        std::cout << "Connection Error: " << mysql_error(connection) << std::endl;
    }

    return std::make_tuple(success, connection);
}

auto ExecSQLQuery(std::string query)
{
    bool success = true;

    int status = mysql_query(connection, query.c_str());

    // send query to db
    if (status != 0)
    {
        std::cout << "MySQL Query Error: " << mysql_error(connection) << std::endl;
        success = false;
    }

    return SqlResponse{success, mysql_use_result(connection)};
}

/*
    Some structures for data manipulation
*/

struct Employee
{
    int id;
    std::string fullName;
    std::string address;
    std::string birthDate;
    std::string joinDate;
    std::string rank;
    std::string phoneNumber;
    std::string departmentId;

    Employee() {}

    Employee(int id, std::string fullName, std::string address,
        std::string birthDate, std::string joinDate, std::string rank,
        std::string phoneNumber, std::string departmentId) {
        this->id = id;
        this->fullName = fullName;
        this->address = address;
        this->birthDate = birthDate;
        this->joinDate = joinDate;
        this->rank = rank;
        this->phoneNumber = phoneNumber;
        this->departmentId = departmentId;
    }
};

struct EmployeeList
{
    unsigned long long num;
    Employee* list;
};

struct Department
{
    std::string id;
    std::string name;
};

struct DepartmentList
{
    unsigned int num;
    Department* list;
};

/*
    Function for loading in db/ creating new database from std input
*/
void CreateNewTables() {
    // maybe add some gate-checking for table existence here

    // create new department table
    std::string createDepartmentTable = "create table departments(" \
                                        "id varchar(10) primary key," \
                                        "name varchar(30) not null);"; 
    ExecSQLQuery(createDepartmentTable);
    // create new employee table
    std::string createEmployeeTable = "create table employees(" \
                                    "id not null primary key auto_increment," \
                                    "employee_id int not null unique key," \
                                    "full_name varchar(60) not null," \
                                    "address varchar(100) not null," \
                                    "birth_date date," \
                                    "join_date date," \
                                    "rank varchar(20) not null," \
                                    "phone varcha(15)," \
                                    "department_id int not null," \
                                    "constrain fk_department_id" \
                                    "foreign key (department_id) references deparments (id)" \
                                    "on delete restrict on update restrict);";
    ExecSQLQuery(createEmployeeTable);
}

int LoadTableFromFile(std::string table, std::string absPath) {
    if (table == "employees") {
        std::string queryStr = "load data local infile '" + absPath + "' " \
                                "into table employees " \
                                "fields terminated by '\r\n' " \
                                "(employee_id, full_name, address, birth_date, " \
                                "join_date, rank, phone, department_id);";
        auto result = ExecSQLQuery(queryStr);
        if (!result.success) return -1;
    }
    else if (table == "departments") {
        std::string queryStr = "load data local infile '" + absPath + "' " \
                                "into table departments " \
                                "fields terminated by '\r\n' " \
                                "(id, name);";
        auto result = ExecSQLQuery(queryStr);
        if (!result.success) return -1;
    }
    else {
        std::cout << "Invalid table name.";
        return -1;
    }

    return 0;
}

int CreateNewDepartment(Department departmentInfo) {
    // using insert to create new row in the table
    std::string queryStr = "insert into departments (id, name) " \
                            "values (" + departmentInfo.id + "," + \
                            departmentInfo.name + ");";
    auto result = ExecSQLQuery(queryStr);
    if (!result.success) return -1;
    return 0;
}

int CreateNewEmployee(Employee employeeInfo) {
    std::string queryStr = "insert into employees (employee_id, full_name, " \
                            "address, birth_date, join_date, rank, phone, department_id) "\
                            "values (" + std::to_string(employeeInfo.id) + "," + \
                            employeeInfo.fullName + "," + \
                            employeeInfo.address + "," + \
                            employeeInfo.birthDate + "," + \
                            employeeInfo.joinDate + "," + \
                            employeeInfo.rank + "," + \
                            employeeInfo.phoneNumber + "," + \
                            employeeInfo.departmentId + "," + \
                            ");";
    auto result = ExecSQLQuery(queryStr);
    if (!result.success) return -1;
    return 0;
}

Department CreatNewDepartmentFromInput(std::string mode="create") {
    int success = 0;
    Department newDepartment = Department();
    do {
        if (success == -1) {
            std::cout << "Please try again.\n";

        std::cout << "\tDepartment id: ";
        std::cin >> newDepartment.id;
        std::cout << "\tDepartment name: ";
        std::cin >> newDepartment.name;
        
        success = CreateNewDepartment(newDepartment);
        }
    } while (success != 0);

    return newDepartment;
}

Employee CreatNewEmployeeFromInput(std::string mode="create") {
    int success = 0;
    Employee newEmployee = Employee();

    do {
        if (success == -1) {
            std::cout << "Please try again.\n";

        std::cout << "\tEmployee id (number only): ";
        std::cin >> newEmployee.id;
        std::cout << "\tFull name: ";
        std::cin >> newEmployee.fullName;
        std::cout << "\tAddress: ";
        std::cin >> newEmployee.address;
        std::cout << "\tDate of birth: ";
        std::cin >> newEmployee.birthDate;
        std::cout << "\tJoin data: ";
        std::cin >> newEmployee.joinDate;
        std::cout << "\tRank: ";
        std::cin >> newEmployee.rank;
        std::cout << "\tPhone number: ";
        std::cin >> newEmployee.phoneNumber;
        std::cout << "\tDepartment id: ";
        std::cin >> newEmployee.departmentId;
        
        if (mode == "create") \
            success = CreateNewEmployee(newEmployee);
        }
    } while (success != 0);

    return newEmployee;
}

void InitiateTableMethodsMenu() {
    std::string createTableMenu = "--------Creat new database / Import new database menu--------\n"
        "Please choose an option to proceed (Enter the option number).\n"
        "\t1. Create new database from keyboard.\n"
        "\t2. Update information.\n"
        "\t3. Return to main menu.\n"
        "Your selection: ";
    
    CreateNewTables();
    int option = 0;
    bool again = false;
    do {
        if (!again) std::cout << createTableMenu;
        again = false;
        std::cin >> option;

        if (option == 1) {
            // for department table
            int departmentNum = 0;
            std::cout << "Please enter the number of department for new table: ";
            std::cin >> departmentNum;
            while (departmentNum < 1) {
                std::cout << "The provided number is invalid. Please try again: ";
                std::cin >> departmentNum;
            }
            for (int i = 0; i < departmentNum; i++) {
                std::cout << "Enter information for department " + std::to_string(i) + ":\n";
                CreatNewDepartmentFromInput();
            }
            std::cout << "Successfully created Departments table.";

            // for employee table
            int employeeNum = 0;
            std::cout << "Please enter the number of employee for new table: ";
            std::cin >> employeeNum;
            while (employeeNum < 1) {
                std::cout << "The provided number is invalid. Please try again: ";
                std::cin >> employeeNum;
            }
            for (int i = 0; i < employeeNum; i++) {
                std::cout << "Enter information for employee " + std::to_string(i) + ":\n";
                CreatNewEmployeeFromInput();
            }
            std::cout << "Successfully created Employees table.";
        }
        else if (option == 2) {
            std::string tables[2] = {"departments", "employees"};
            for (std::string table: tables) {
                int response = 0;
                do {
                    if (response < 0) std::cout << "Try again.\n";
                    std::cout << "Enter absolute path to " + table + " data (.csv file): ";
                    std::string dataLoc;
                    std::cin >> dataLoc;
                    LoadTableFromFile(table , dataLoc);
                } while (response != 0);
            }
        }
        else if (option != 3) {
            std::cout << "Your selection is not valid. Please choose again: ";
            again = true;
        }
    } while (option != 3);
}

/*
    Functions for updating employee/department information
*/
int UpdateDepartmentInfo(Department departmentInfo,
    std::string changedFieldName, std::string oldValue) {
    if (changedFieldName == "name") {
        std::string updateQueryStr = "update departments set " \
                                    "name = " + departmentInfo.name + \
                                    "where department_id = " + departmentInfo.id + ";";
        auto result = ExecSQLQuery(updateQueryStr);
        if (!result.success) return -1;
    }
    else {
        // insert new value 
        std::string insertQueryStr = "insert into departments (id, name) " \
                                "values (" + departmentInfo.id + "," + \
                                departmentInfo.name + ");";
        auto result = ExecSQLQuery(insertQueryStr);
        if (!result.success) return -1;

        // update all value in employees table
        std::string updateQueryStr = "update employees set " \
                                    "department_id = " + departmentInfo.id + \
                                    "where department_id = " + oldValue + ";";
        auto result = ExecSQLQuery(updateQueryStr);
        if (!result.success) return -1;

        // delete old value
        std::string deleteQueryStr = "delete from departments " \
                                    "where department_id = " + oldValue + ";";
        auto result = ExecSQLQuery(deleteQueryStr);
        if (!result.success) return -1;
    }

    return 0;
}

int UpdateEmployeeInfo(Employee employeeInfo, std::string changedFieldName, int oldValue) {
    if (changedFieldName == "id") {
        CreateNewEmployee(employeeInfo);
        // delete old value
        std::string deleteQueryStr = "delete from employees " \
                                    "where employee_id = " + std::to_string(oldValue) + ";";
        auto result = ExecSQLQuery(deleteQueryStr);
        if (!result.success) return -1;
    }
    else {
        std::string queryStr = "update employees set " \
                            "full_name = '" + employeeInfo.fullName + \
                            "', address = " + employeeInfo.address + \
                            "', birth_date" + employeeInfo.birthDate + \
                            "', join_date" + employeeInfo.joinDate + \
                            "', rank" + employeeInfo.rank + \
                            "', phone" + employeeInfo.phoneNumber + \
                            "', department_id" + employeeInfo.departmentId + \
                            "', where employee_id = " + std::to_string(employeeInfo.id) + ";";
        auto result = ExecSQLQuery(queryStr);
        if (!result.success) return -1;
    }
    
    return 0;
}

void UpdateInfoMenu() {
    std::string updateInfoMenu = "--------Update information menu--------\n"
        "Please choose an option to proceed (Enter the option number).\n"
        "\t1. Update department information.\n"
        "\t2. Update employee information.\n"
        "\t3. Return to main menu.\n"
        "Your selection: ";
    
    int option = 0;
    bool again = false;
    do {
        if (!again) std::cout << updateInfoMenu;
        again = false;
        std::cin >> option;

        if (option == 1) {
            std::cout << "Enter id of the department you want to change: ";
            std::string id;
            std::cin >> id;
            SqlResponse resp = SqlResponse();
            SqlResponse* ref = &resp;
            if (FetchDepartmentWithId(id, ref) == -1) {
                again = true;
                continue;
            }
            std::cout << "Current values:\n";
            DisplayDepartmentInfo(resp.res);
            std::cout << "Enter new values:\n";
            Department newDepartment = CreatNewDepartmentFromInput("update");
            std::string changedField = newDepartment.id == id ? "name": "id";
            if (UpdateDepartmentInfo(newDepartment, changedField, id) == -1) again = true;
        }
        else if (option == 2) {
            std::cout << "Enter id of the employee you want to change: ";
            int id;
            std::cin >> id;
            SqlResponse resp = SqlResponse();
            SqlResponse* ref = &resp;
            if (FetchEmployeeWithId(id, ref) == -1) {
                again = true;
                continue;
            }
            std::cout << "Current values:\n";
            DisplayEmployeeInfo(resp.res);
            std::cout << "Enter new values:\n";
            Employee newEmployee = CreatNewEmployeeFromInput("update");
            std::string changedField = newEmployee.id == id ? "name": "id";
            if (UpdateEmployeeInfo(newEmployee, changedField, id)) again = true;
        }
        else if (option != 3) {
            std::cout << "Your selection is not valid. Please choose again: ";
            again = true;
        }
    } while (option != 3);
}

/*
    Function for displaying employees of a department in different orders
*/
int FetchDepartmentWithId(std::string id, SqlResponse* result) {
    std::string selectQueryStr = "select * from departments " \
                                "where id = " + id + ";";
    *result = ExecSQLQuery(selectQueryStr);
    if (!result->success) return -1;
    return 0;
}

int FetchEmployeeWithId(int id, SqlResponse* result) {
    std::string selectQueryStr = "select * from employees " \
                                "where employee_id = " + std::to_string(id) + ";";
    *result = ExecSQLQuery(selectQueryStr);
    if (!result->success) return -1;
    return 0;
}

int FetchEmployeeWithName(std::string fullName, SqlResponse* result) {
    std::string selectQueryStr = "select * from employees " \
                                "where full_name = " + fullName + ";";
    *result = ExecSQLQuery(selectQueryStr);
    if (!result->success) return -1;
    return 0;
}

int FetchEmployeesOfDepartment(std::string department_id,
    std::string sortByColumn, std::string order, SqlResponse* result) {
    std::string queryStr = "select * from employees" \
                            " where department_id = " + department_id + \
                            " order by " + sortByColumn + " " + order + ";";
    *result = ExecSQLQuery(queryStr);
    if (!result->success) return -1;
    return 0;
}

void FetchEmployeeInfoMenu() {
    std::string fetchEmployeesMenu = "--------Fetch employee(s) menu--------\n"
        "Please choose an option to proceed (Enter the option number).\n"
        "\t1. Fetch a employee.\n"
        "\t2. Fetch all employee in a department.\n"
        "\t3. Return to main menu.\n"
        "Your selection: ";
    
    int option = 0;
    bool again = false;
    do {
        if (!again) std::cout << fetchEmployeesMenu;
        again = false;
        std::cin >> option;

        if (option == 1) {
            std::cout << "Using name or id to find employee? ";
            std::string method;
            std::cin >> method;
            if (method == "id") {
                std::cout << "Enter employee id: ";
                int id;
                std::cin >> id;
                SqlResponse resp = SqlResponse();
                SqlResponse* ref = &resp;
                if (FetchEmployeeWithId(id, ref) == -1) {
                    again = true;
                    continue;
                }
                DisplayEmployeeInfo(resp.res);
            }
            else if (method == "name") {
                std::cout << "Enter employee name: ";
                std::string name;
                std::cin >> name;
                SqlResponse resp = SqlResponse();
                SqlResponse* ref = &resp;
                if (FetchEmployeeWithName(name, ref) == -1) {
                    again = true;
                    continue;
                }
                DisplayEmployeeInfo(resp.res);
            }
            else {
                std::cout << "Invalid value. Try again.\n";
            }
        }
        else if (option == 2) {
            std::cout << "Enter department id: ";
            std::string departmentId;
            std::cin >> departmentId;
            std::cout << "Enter field to sort by (id, full_name, birth_date): ";
            std::string field;
            std::cin >> field;
            std::cout << "Enter order (asc, desc): ";
            std::string order;
            std::cin >> order;
            SqlResponse resp = SqlResponse();
            SqlResponse* ref = &resp;
            if (FetchEmployeesOfDepartment(departmentId, field, order, ref) == -1) {
                again = true;
                continue;
            }
            DisplayEmployeeInfo(resp.res);
        }
        else if (option != 3) {
            std::cout << "Your selection is not valid. Please choose again: ";
            again = true;
        }
    } while (option != 3);
}

/*
    Helper function for displaying information from list of struct
*/
void FromResponseToList(MYSQL_RES *respone, EmployeeList* list) {
    MYSQL_ROW row; // the results rows (array)
    list->num = mysql_num_rows(respone);

    if (list->num == 0) {
        std::cout << "No employee satisfies!";
    } 
    else {
        Employee* ref = new Employee[list->num];
        
        int i = 0;
        // get row to a array and extract each field
        while ((row = mysql_fetch_row(respone)) != NULL)
        {
            ref[i++] = Employee(
                std::atoi(row[0]), row[1], row[2], row[3], 
                row[4], row[5], row[6], row[7]
            );
        }

        list->list = ref;
    }

    // clean up the database result
    mysql_free_result(respone);
}

void FromResponseToList(MYSQL_RES *respone, DepartmentList* list) {
    MYSQL_ROW row; // the results rows (array)
    // std::cout << mysql_num_fields(respone);
    list->num = mysql_num_rows(respone);

    if (list->num == 0) {
        std::cout << "No department satisfies!";
    } 
    else {
        Department* ref = new Department[list->num];
        
        int i = 0;
        // get row to a array and extract each field
        while ((row = mysql_fetch_row(respone)) != NULL)
        {
            Department resp = Department();
            resp.id = row[0];
            resp.name = row[1];
            ref[i++] = resp;
        }

        list->list = ref;
    }

    // clean up the database result
    mysql_free_result(respone);
}

void DisplayEmployeeInfo(MYSQL_RES *respone) {
    EmployeeList contentList = EmployeeList();
    EmployeeList *ref = &contentList;
    FromResponseToList(respone, ref);
    DisplayList(contentList);
}

void DisplayDepartmentInfo(MYSQL_RES *respone) {
    DepartmentList contentList = DepartmentList();
    DepartmentList *ref = &contentList;
    FromResponseToList(respone, ref);
    DisplayList(contentList);
}

void DisplayList(EmployeeList contentList) {
    if (contentList.num != 0) {
        for (int i = 0; i < contentList.num; i++) {
            std::cout << i + 1 << ")\n";
            std::cout << contentList.list[i] << std::endl;
        }
        delete[] contentList.list;
    }
}

void DisplayList(DepartmentList contentList) {
    if (contentList.num != 0) {
        for (int i = 0; i < contentList.num; i++) {
            std::cout << i + 1 << ")\n";
            std::cout << contentList.list[i] << std::endl;
        }
        delete[] contentList.list;
    }
}

std::ostream& std::operator<<(std::ostream& os, const Employee& employee) {
    return os << "\tID:\t\t" << employee.id << endl
              << "\tFull name:\t\t" << employee.fullName << endl
              << "\tAddress:\t\t" << employee.address << endl
              << "\tBirth date:\t\t" << employee.birthDate << endl
              << "\tJoin date:\t\t" << employee.joinDate << endl
              << "\tRank:\t\t" << employee.rank << endl
              << "\tPhone number:\t\t" << employee.phoneNumber << endl
              << "\tDepartment ID:\t\t" << employee.departmentId;
}

std::ostream& std::operator<<(std::ostream& os, const Department& department) {
    return os << "\tID:\t\t" << department.id << endl
              << "\tDepartment:\t\t" << department.name;
}

/*
    Function for saving database to file
*/

void ExportToCSV(std::string dirPath) {
    std::string employeeFile = dirPath + "/employees.csv";
    std::string departmentFile = dirPath + "/departments.csv";

    std::string queryStr = "select * from employees into " + employeeFile;
    ExecSQLQuery(queryStr);

    queryStr = "select * from departments into " + departmentFile;
    ExecSQLQuery(queryStr);
}

/*
    Main navigation menu
*/

void MainMenu() {
    std::string mainMenuString = "********WELCOME TO MAIN MENU********\n"
        "Please choose an option to proceed (Enter the option number).\n"
        "\t1. Create new / Import database.\n"
        "\t2. Update information.\n"
        "\t3. Fetch employee(s).\n"
        "\t4. Quit program.\n"
        "Your selection: ";
    
    int option = 0;
    bool again = false;
    do {
        if (!again) std::cout << mainMenuString;
        again = false;
        std::cin >> option;

        if (option == 1) {
            InitiateTableMethodsMenu();
        }
        else if (option == 2) {
            UpdateInfoMenu();
        }
        else if (option == 3) {
            FetchEmployeeInfoMenu();
        } 
        else if (option != 4) {
            std::cout << "Your selection is not valid. Please choose again: ";
            again = true;
        }
    } while (option != 4);
}

/*
    Main function
*/
MYSQL* connection; // a global variable since almost all functions use it

int main() {
    SQLConnection connDetail = SQLConnection("localhost", "root", "111", "test");
    // connect to the mysql database
    bool success;
    std::tie(success, connection) = SqlConnectionSetup(connDetail);
    if (!success) return 1;

    MainMenu();

    std::string savePath;
    char answer;
    std::cout << "Do yout want to export the database to external files? (y/n) ";
    std::cin >> answer;
    if (answer == 'y') {
        std::cout << "Please specify a absolute path to a local directory: ";
        std::string path;
        std::cin >> path;
        ExportToCSV(path);
    }

    // close database connection
    mysql_close(connection);

    return 0;
}