// CS 300 Project Two - ABCU Advising Assistance Program
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
using namespace std;

struct Course {
    string number;
    string title;
    vector<string> prerequisites;
};

struct Node {
    Course course;
    Node* left;
    Node* right;
    explicit Node(const Course& value) : course(value), left(nullptr), right(nullptr) {}
};

string trim(const string& text) {
    size_t first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

string upper(string text) {
    transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) { return static_cast<char>(toupper(ch)); });
    return text;
}

class CourseTree {
private:
    Node* root = nullptr;

    static void clear(Node* node) {
        if (!node) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

    static Node* insert(Node* node, const Course& course) {
        if (!node) return new Node(course);
        if (course.number < node->course.number) node->left = insert(node->left, course);
        else if (course.number > node->course.number) node->right = insert(node->right, course);
        else node->course = course;
        return node;
    }

    static void printInOrder(const Node* node) {
        if (!node) return;
        printInOrder(node->left);
        cout << node->course.number << ", " << node->course.title << '\n';
        printInOrder(node->right);
    }

public:
    CourseTree() = default;
    CourseTree(const CourseTree&) = delete;
    CourseTree& operator=(const CourseTree&) = delete;
    ~CourseTree() { clear(root); }

    void reset() { clear(root); root = nullptr; }
    void add(const Course& course) { root = insert(root, course); }
    bool empty() const { return root == nullptr; }

    const Course* find(const string& number) const {
        const Node* current = root;
        while (current) {
            if (number == current->course.number) return &current->course;
            current = (number < current->course.number) ? current->left : current->right;
        }
        return nullptr;
    }

    void printCourses() const { printInOrder(root); }
};

bool loadCourses(const string& fileName, CourseTree& courses) {
    ifstream file(fileName);
    if (!file.is_open()) {
        cout << "Error: Could not open file: " << fileName << '\n';
        return false;
    }

    // Read all records before changing the current tree, so invalid files don't erase valid data.
    vector<Course> records;
    string line;
    int lineNumber = 0;
    while (getline(file, line)) {
        ++lineNumber;
        if (trim(line).empty()) continue;
        stringstream stream(line);
        string field;
        vector<string> fields;
        while (getline(stream, field, ',')) fields.push_back(trim(field));
        if (fields.size() < 2 || fields[0].empty() || fields[1].empty()) {
            cout << "Error: Invalid course data on line " << lineNumber << ".\n";
            return false;
        }
        Course course;
        course.number = upper(fields[0]);
        course.title = fields[1];
        for (size_t i = 2; i < fields.size(); ++i) {
            // Provided CSV uses empty trailing columns for courses with fewer prerequisites.
            if (!fields[i].empty()) course.prerequisites.push_back(upper(fields[i]));
        }
        for (const Course& existing : records) {
            if (existing.number == course.number) {
                cout << "Error: Duplicate course number " << course.number << ".\n";
                return false;
            }
        }
        records.push_back(course);
    }
    if (records.empty()) {
        cout << "Error: The file contains no courses.\n";
        return false;
    }

    // Check that each prerequisite refers to a course in the input file.
    for (const Course& course : records) {
        for (const string& prerequisite : course.prerequisites) {
            bool found = false;
            for (const Course& candidate : records) {
                if (candidate.number == prerequisite) { found = true; break; }
            }
            if (!found) {
                cout << "Error: Prerequisite " << prerequisite << " for " << course.number << " was not found.\n";
                return false;
            }
        }
    }

    courses.reset();
    for (const Course& course : records) courses.add(course);
    cout << "Course data loaded successfully (" << records.size() << " courses).\n";
    return true;
}

void printCourseDetails(const CourseTree& courses, string number) {
    number = upper(trim(number));
    const Course* course = courses.find(number);
    if (!course) {
        cout << "Course " << number << " was not found.\n";
        return;
    }
    cout << course->number << ", " << course->title << '\n';
    if (course->prerequisites.empty()) {
        cout << "Prerequisites: None\n";
        return;
    }
    cout << "Prerequisites:\n";
    for (const string& prerequisite : course->prerequisites) {
        const Course* required = courses.find(prerequisite);
        cout << "  " << prerequisite;
        if (required) cout << ", " << required->title;
        cout << '\n';
    }
}

int main() {
    CourseTree courses;
    string choice;
    cout << "Welcome to the ABCU Advising Assistance Program.\n";
    while (true) {
        cout << "\n1. Load Data Structure\n"
            << "2. Print Course List\n"
            << "3. Print Course\n"
            << "9. Exit\n"
            << "What would you like to do? ";
        if (!getline(cin, choice)) break;
        choice = trim(choice);
        if (choice == "1") {
            string fileName;
            cout << "Enter the course data file name (including .csv): ";
            if (!getline(cin, fileName)) break;
            loadCourses(trim(fileName), courses);
        }
        else if (choice == "2") {
            if (courses.empty()) cout << "Please load the course data first (option 1).\n";
            else { cout << "\nHere is a sample schedule:\n"; courses.printCourses(); }
        }
        else if (choice == "3") {
            if (courses.empty()) cout << "Please load the course data first (option 1).\n";
            else {
                string number;
                cout << "What course do you want to know about? ";
                if (!getline(cin, number)) break;
                printCourseDetails(courses, number);
            }
        }
        else if (choice == "9") {
            cout << "Thank you for using the ABCU Advising Assistance Program!\n";
            break;
        }
        else {
            cout << "Invalid option. Please enter 1, 2, 3, or 9.\n";
        }
    }
    return 0;
}