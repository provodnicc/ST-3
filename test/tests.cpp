// Copyright 2024 Pavel Smirnov

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <chrono> // NOLINT [build/c++11]
#include <thread> // NOLINT [build/c++11]
#include "../include/TimedDoor.h"

class MockTimerClient : public TimerClient {
 public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class TimedDoorTest : public testing::Test {
 protected:
    TimedDoor door;
    MockTimerClient mockClient;
    Timer timer;

    TimedDoorTest() : door(4), timer() {}

    void SetUp() override {
        DoorTimerAdapter adapter(door);
    }

    void TearDown() override {
        testing::Mock::VerifyAndClear(&mockClient);
    }
};

TEST_F(TimedDoorTest, TestDoorInit) {
    EXPECT_FALSE(door.isDoorOpened());
}

TEST_F(TimedDoorTest, TestDoorUnlock) {
    door.unlock();
    EXPECT_TRUE(door.isDoorOpened());
}

TEST_F(TimedDoorTest, TestDoorLock) {
    door.unlock();
    door.lock();
    EXPECT_FALSE(door.isDoorOpened());
}

TEST_F(TimedDoorTest, TestDoorThrowStateOpened) {
    door.unlock();
    EXPECT_THROW(door.throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, TestDoorLockTwice) {
    EXPECT_THROW(door.lock(), std::logic_error);
}


TEST_F(TimedDoorTest, TestDoorUnlockTwice) {
    door.unlock();
    EXPECT_THROW(door.unlock(), std::logic_error);
}

TEST_F(TimedDoorTest, TestTimerRegisterCallsSleep) {
    EXPECT_CALL(mockClient, Timeout()).Times(1);
    timer.tregister(1, &mockClient);
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

TEST_F(TimedDoorTest, TestDoorTimeoutException) {
    door.unlock();
    std::this_thread::sleep_for(
            std::chrono::seconds(door.getTimeOut() * 1 + 1));
    EXPECT_THROW(door.throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, TestIsDoorOpenedInsideTimeout) {
    door.unlock();
    std::this_thread::sleep_for(
            std::chrono::seconds(door.getTimeOut() * 1 / 2));
    EXPECT_TRUE(door.isDoorOpened());
}

TEST_F(TimedDoorTest, TestIsDoorClosedInsideTimeout) {
    door.unlock();
    door.lock();
    std::this_thread::sleep_for(
            std::chrono::seconds(door.getTimeOut() * 1 / 2));
    EXPECT_FALSE(door.isDoorOpened());
}
