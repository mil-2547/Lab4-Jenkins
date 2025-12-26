#include "sNotebook.hpp"
#include <ctime>
#include <chrono>
#include <fstream>
#include <iomanip>

namespace s::FileManager {

    void wopen(std::ofstream& file, const std::string& filename) {
        file.open(filename, std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("Error: cannot open file for writing: " + filename);
        }
    }

    void ropen(std::ifstream& file, const std::string& filename) {
        file.open(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Error: cannot open file for reading: " + filename);
        }
    }

    void writef(std::ofstream& file, StackManager::Stack*& pStack) {
        if (!file.is_open()) {
            throw std::runtime_error("File is not open for writing");
        }

        nlohmann::json j;
        toJson(j, pStack);
        file << j.dump(4);
    }

    bool fileExists(const std::string& filename) {
        std::ifstream f(filename);
        return f.is_open();
    }

    void wclosef(std::ofstream &file) {if (!file.is_open()) return; file.close();}
    void rclosef(std::ifstream &file) {if (!file.is_open()) return; file.close();}
}


namespace s::StackManager {

    Stack* initStack() {
        return nullptr;
    }

    void push(Stack *&pStack, const std::string &value) {
      Stack *newNode = new Stack();
      newNode->idx = pStack ? pStack->idx + 1 : 0;
      newNode->timestamp = getTimestamp();
      newNode->data = value;
      newNode->previous = pStack;
      pStack = newNode;
    }

    void pop(Stack *&pStack) {
        if (isEmpty(pStack)) return;
        Stack* temp = pStack;
        pStack = pStack->previous;
        delete temp;
    }

    void clear(Stack *&pStack) {
        while (pStack) {
            Stack* temp = pStack;
            pStack = pStack->previous;
            delete temp;
        }
    }

    bool isEmpty(const Stack *pStack) {
        return pStack == nullptr;
    }

    const Stack* getLastEl(const Stack *pStack) {
        return pStack;
    }

    std::string getTimestamp() {
	// Get the current time
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);

	// Convert to local time
	std::tm tm;
    #ifdef _WIN32
	localtime_s(&tm, &t);
    #else
	localtime_r(&t, &tm);
    #endif

	// Format as an ISO 8601 string: “YYYY-MM-DDTHH:MM:SS”
	std::ostringstream oss;
	oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
	return oss.str();
    }

    void toJson(nlohmann::json& j, Stack* &pStack) {
	std::vector<nlohmann::json> vec;
	Stack* current = pStack;

	// go from the top to the bottom of the stack
	while (current) {
	    nlohmann::json item;
	    item["idx"] = current->idx;
	    item["timestamp"] = current->timestamp;
	    item["data"] = current->data;
	    vec.push_back(item);
	    current = current->previous;
	}

	// unfold so that the first element is at the bottom of the stack
	std::reverse(vec.begin(), vec.end());

	j["stack"] = vec;
    }

    Stack* fromJson(const nlohmann::json& j) {
	Stack* pStack = initStack();
	if (!j.contains("stack") || !j["stack"].is_array()) return nullptr;

	Stack* prev = nullptr;
	for (const auto& item : j["stack"]) {
	    Stack* node = new Stack;
	    node->idx = item.at("idx").get<uint32_t>();
	    node->timestamp = item.at("timestamp").get<std::string>();
	    node->data = item.at("data").get<std::string>();
	    node->previous = prev;  // link to the previous one
	    prev = node;
	}

	// stack top — the last added element
	pStack = prev;
	return pStack;
    }
    Stack* loadStackFromFile(const std::string& filename) {
	std::ifstream f;
	FileManager::ropen(f, filename);

	nlohmann::json j;
	f >> j;
	FileManager::rclosef(f);
	return fromJson(j);
    }
}

namespace s::RuntimeManager {

    std::string input() {
        std::string data;
        std::getline(std::cin, data);
        return data;
    }

    void takeNote(StackManager::Stack *&pStack) {

        fmt::print("Enter your note: ");
        std::string data = input();
        if (data.empty()) {
            fmt::print("Note cannot be empty.\n");
            return;
        }
        StackManager::push(pStack, data);
        fmt::print("Note added.\n");
    }

    void deleteNote(StackManager::Stack *&stack) {

        if (StackManager::isEmpty(stack)) {
            fmt::println("Stack is empty");
            return;
        }
        char choice;
        fmt::print("Are you sure [Y/n]: ");
        std::cin >> choice;
        std::cin.ignore(10000, '\n'); // Clear buffer

        if (choice == 'y' || choice == 'Y' || choice == '\n' || choice == '\0') {
            StackManager::pop(stack);
            fmt::print("Note deleted.\n");
        } else {
            fmt::print("Cancelled.\n");
        }
    }

    void printMenu() {
        fmt::print( "\
╔═════════════════════════════════════════════╗\n\
║                 📋 MENU                     ║\n\
╠═════════════════════════════════════════════╣\n\
║  1. push   → Add new note                   ║\n\
║  2. pop    → Delete last note               ║\n\
║  3. last   → Show last note                 ║\n\
║  4. list   → Show all notes                 ║\n\
║  0. exit   → Save & quit                    ║\n\
╚═════════════════════════════════════════════╝\n");
    }

    void printLast(StackManager::Stack *pStack) {
        const StackManager::Stack* last = getLastEl(pStack);
        if (StackManager::isEmpty(last)) {
            fmt::println("Stack is empty");
            return;
        }
        fmt::println("[{}] {}: {}", last->timestamp, last->idx, last->data);
    }

    void printNotes(const StackManager::Stack *pStack) {
        if (StackManager::isEmpty(pStack)) {
            fmt::println("Stack is empty");
            return;
        }
        while (pStack) {
	    fmt::println("[{}] {}: {}", pStack->timestamp, pStack->idx, pStack->data);
            pStack = pStack->previous;
        }
    }

    void run(std::string filename) {
        
        
	std::ofstream file;
	StackManager::Stack* stack;
	if (FileManager::fileExists(filename)) {
	    stack = StackManager::loadStackFromFile(filename);
	} else {
	    fmt::print("File not found, creating a new stack.\n");
	    stack = StackManager::initStack();
	}
        char choice;

        while (true) {
            RuntimeManager::clearScreen();
            RuntimeManager::printMenu();
            fmt::print("> ");

            if (!(std::cin >> choice)) {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                continue;
            }

            std::cin.ignore(10000, '\n');

            switch (choice) {
                case '1':
                    takeNote(stack);
                    pause();
                    break;
                case '2':
                    deleteNote(stack);
                    pause();
                    break;
                case '3':
                    printLast(stack);
                    pause();
                    break;
                case '4':
                    printNotes(stack);
                    pause();
                    break;
                case '0':
		    FileManager::wopen(file, filename);
		    FileManager::writef(file, stack);
		    FileManager::wclosef(file);
                    StackManager::clear(stack);
                    fmt::print("Goodbye!\n");
		    return;
                default:
                    fmt::print("Invalid choice.\n");
                    pause();
                    break;
            }
        }
    }

    void clearScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void pause() {
        fmt::print("Press Enter to continue...\n");
#ifdef _WIN32
        system("pause");
#else
        system("read -s");
#endif
    }
}
