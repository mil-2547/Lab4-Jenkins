#include <gtest/gtest.h>
#include "../../src/sNotebook.hpp"

using namespace s::StackManager;

// FORMAT: [TEST](<NameSpace>_Method, MethodCheck) {}

// --- helper: safe delete whole stack ---
static void freeStack(Stack* &p) {
    clear(p);
}

// =============================
// initStack
// =============================
TEST(StackManagerTest, InitStackReturnsNullptr) {
    Stack* stack = initStack();
    EXPECT_EQ(stack, nullptr);
}

// =============================
// push
// =============================
TEST(StackManagerTest, PushAddsFirstElement) {
    Stack* stack = initStack();
    push(stack, "Data");

    ASSERT_NE(stack, nullptr);
    EXPECT_EQ(stack->idx, 0);
    EXPECT_EQ(stack->data, "Data");
    EXPECT_EQ(stack->previous, nullptr);
    EXPECT_EQ(stack->timestamp.size(), 19); // ISO8601

    freeStack(stack);
}

TEST(StackManagerTest, PushAddsSecondElement) {
    Stack* stack = initStack();
    push(stack, "One");
    push(stack, "Two");

    ASSERT_NE(stack, nullptr);
    EXPECT_EQ(stack->idx, 1);
    EXPECT_EQ(stack->data, "Two");
    EXPECT_NE(stack->previous, nullptr);
    EXPECT_EQ(stack->previous->idx, 0);

    freeStack(stack);
}

// =============================
// pop
// =============================
TEST(StackManagerTest, PopFromEmptyDoesNothing) {
    Stack* stack = initStack();
    pop(stack);
    EXPECT_EQ(stack, nullptr);
}

TEST(StackManagerTest, PopRemovesTopElement) {
    Stack* stack = initStack();
    push(stack, "A");
    push(stack, "B");

    pop(stack);

    ASSERT_NE(stack, nullptr);
    EXPECT_EQ(stack->data, "A");
    EXPECT_EQ(stack->idx, 0);
    EXPECT_EQ(stack->previous, nullptr);

    freeStack(stack);
}

// =============================
// clear
// =============================
TEST(StackManagerTest, ClearRemovesAllElements) {
    Stack* stack = initStack();
    push(stack, "A");
    push(stack, "B");
    push(stack, "C");

    clear(stack);

    EXPECT_EQ(stack, nullptr);
}

// =============================
// isEmpty
// =============================
TEST(StackManagerTest, isEmptyCheck) {
    Stack* stack = initStack();
    EXPECT_TRUE(isEmpty(stack));

    push(stack, "X");
    EXPECT_FALSE(isEmpty(stack));

    freeStack(stack);
}

// =============================
// getLastEl
// =============================
TEST(StackManagerTest, GetLastElReturnsTop) {
    Stack* stack = initStack();
    push(stack, "A");

    EXPECT_EQ(getLastEl(stack), stack);

    freeStack(stack);
}

// =============================
// JSON conversion
// =============================
TEST(StackManagerTest, ToJsonConvertsToJsonCorrectly) {
    Stack* stack = initStack();
    push(stack, "A");
    push(stack, "B");

    nlohmann::json j;
    toJson(j, stack);

    ASSERT_TRUE(j.contains("stack"));
    ASSERT_EQ(j["stack"].size(), 2);

    EXPECT_EQ(j["stack"][0]["data"], "A");
    EXPECT_EQ(j["stack"][1]["data"], "B");

    freeStack(stack);
}

TEST(StackManagerTest, FromJsonLoadsFromJsonCorrectly) {
    nlohmann::json j;
    j["stack"] = {
        { {"idx",0},{"timestamp","T1"},{"data","A"} },
        { {"idx",1},{"timestamp","T2"},{"data","B"} }
    };

    Stack* stack = fromJson(j);

    ASSERT_NE(stack, nullptr);
    EXPECT_EQ(stack->data, "B");
    EXPECT_EQ(stack->idx, 1);
    ASSERT_NE(stack->previous, nullptr);
    EXPECT_EQ(stack->previous->data, "A");

    freeStack(stack);
}

TEST(StackManagerTest, FromJsonReturnsNullptrOnInvalidJson) {
    nlohmann::json j;
    j["wrong"] = {};

    Stack* stack = fromJson(j);
    EXPECT_EQ(stack, nullptr);
}

// =============================
// loadStackFromFile
// =============================

// Для теста используем временный файл
TEST(StackManagerTest, LoadStackFromFileLoadsCorrectly) {
    std::string filename = "test_stack.json";

    // создаём json
    nlohmann::json j;
    j["stack"] = {
        { {"idx",0},{"timestamp","T1"},{"data","A"} },
        { {"idx",1},{"timestamp","T2"},{"data","B"} }
    };

    // пишем в файл
    {
        std::ofstream f(filename);
        f << j.dump(4);
    }

    // читаем через функцию
    Stack* stack = loadStackFromFile(filename);

    ASSERT_NE(stack, nullptr);
    EXPECT_EQ(stack->data, "B");
    EXPECT_EQ(stack->previous->data, "A");

    freeStack(stack);

    // удаляем файл
    std::remove(filename.c_str());
}
