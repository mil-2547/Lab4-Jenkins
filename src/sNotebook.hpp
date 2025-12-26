#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <fmt/base.h>
#include <iostream>
#include <nlohmann/json.hpp>


namespace s {

    namespace StackManager {}
    namespace FileManager {}
    namespace RuntimeManager {}
}

namespace s::StackManager {

    struct Stack {
	uint32_t idx;
	std::string timestamp;
        std::string data;
        Stack* previous;
    };

    Stack* initStack();
    void push(Stack *&pStack, const std::string& value);
    void pop(Stack *&pStack);

    void clear(Stack *&pStack);

    bool isEmpty(const Stack *pStack);
    const Stack* getLastEl(const Stack *pStack);
    std::string getTimestamp();
    void toJson(nlohmann::json& j, Stack *&pStack);
    Stack* fromJson(const nlohmann::json& j);
    Stack* loadStackFromFile(const std::string& filename);
}

namespace s::FileManager {
    void wopen(std::ofstream &file, const std::string& filename);
    void ropen(std::ifstream &file, const std::string& filename);
    void writef(std::ofstream &file, StackManager::Stack *&pStack);
    bool fileExists(const std::string& filename);
    void wclosef(std::ofstream &file);
    void rclosef(std::ifstream &file);
}

namespace s::RuntimeManager {

    std::string input();
    void takeNote(StackManager::Stack *&pStack);
    void deleteNote(StackManager::Stack *&stack);
    void printMenu();
    void printLast(StackManager::Stack *pStack);
    void printNotes(const StackManager::Stack* pStack);

    void run(std::string filename);
    void clearScreen();
    void pause();
}
