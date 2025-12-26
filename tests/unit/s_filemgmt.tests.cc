#include <gtest/gtest.h>
#include <fstream>
#include "../../src/sNotebook.hpp"

using namespace s::FileManager;
using namespace s::StackManager;

class FileManagerTest : public ::testing::Test {
protected:
    std::string filename = "test_file.json";
    Stack* stack = nullptr;

    void SetUp() override {
        stack = initStack();
        push(stack, "A");
        push(stack, "B");
    }

    void TearDown() override {
        clear(stack);
        std::remove(filename.c_str());
    }
};

// =============================
// wopen
// =============================
TEST_F(FileManagerTest, WopenCreatesFile) {
    std::ofstream file;
    EXPECT_NO_THROW(wopen(file, filename));
    EXPECT_TRUE(file.is_open());
    wclosef(file);
}

TEST_F(FileManagerTest, WopenThrowsOnInvalidPath) {
    std::ofstream file;
    EXPECT_THROW(wopen(file, "/invalid_path/test.json"), std::runtime_error);
}

// =============================
// ropen
// =============================
TEST_F(FileManagerTest, RopenOpensExistingFile) {
    // сначала создаём файл
    {
        std::ofstream f(filename);
        f << "test";
    }

    std::ifstream file;
    EXPECT_NO_THROW(ropen(file, filename));
    EXPECT_TRUE(file.is_open());
    rclosef(file);
}

TEST_F(FileManagerTest, RopenThrowsOnNonExistingFile) {
    std::ifstream file;
    EXPECT_THROW(ropen(file, "non_existing_file.json"), std::runtime_error);
}

// =============================
// writef
// =============================
TEST_F(FileManagerTest, WritefWritesJson) {
    std::ofstream file;
    wopen(file, filename);
    EXPECT_NO_THROW(writef(file, stack));
    wclosef(file);

    // проверяем, что файл не пустой
    std::ifstream f(filename);
    std::string content;
    std::getline(f, content);
    EXPECT_FALSE(content.empty());
}

// =============================
// fileExists
// =============================
TEST_F(FileManagerTest, FileExistsReturnsTrueIfFileExists) {
    std::ofstream f(filename);
    f << "test";
    f.close();
    EXPECT_TRUE(fileExists(filename));
}

TEST_F(FileManagerTest, FileExistsReturnsFalseIfFileDoesNotExist) {
    EXPECT_FALSE(fileExists("non_existing_file.json"));
}

// =============================
// wclosef / rclosef
// =============================
TEST_F(FileManagerTest, WclosefClosesOpenFile) {
    std::ofstream file;
    wopen(file, filename);
    EXPECT_TRUE(file.is_open());
    wclosef(file);
    EXPECT_FALSE(file.is_open());
}

TEST_F(FileManagerTest, RclosefClosesOpenFile) {
    std::ifstream file;
    {
        std::ofstream f(filename);
        f << "data";
    }
    ropen(file, filename);
    EXPECT_TRUE(file.is_open());
    rclosef(file);
    EXPECT_FALSE(file.is_open());
}

