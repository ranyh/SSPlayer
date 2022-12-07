#include <gtest/gtest.h>

#include "frame_pool.h"


struct FakeFrame: public playos::player::FrameLink {
    int val;
};

TEST(FakeFrameTest, BasicAssertions) {
    auto pool = playos::player::FramePool<FakeFrame>::create(5);
    std::vector<FakeFrame *> frames;
    FakeFrame *frame;

    for (int i = 0; i < 5; i++) {
        frame = pool->getFrame();
        frame->val = i;
        frames.push_back(frame);
    }

    EXPECT_EQ(5, frames.size());
    EXPECT_EQ(true, pool->isEmpty());

    pool->putFrame(frames.back());
    EXPECT_EQ(false, pool->isEmpty());

    for (int i = 3; i >= 0; i--) {
        pool->putFrame(frames[i]);
    }

    frame = pool->getFrame();
    EXPECT_EQ(frames[0], frame);
    EXPECT_EQ(0, frame->val);

    frame = pool->getFrame();
    EXPECT_EQ(frames[1], frame);
    EXPECT_EQ(1, frame->val);

    frame = pool->getFrame();
    EXPECT_EQ(frames[2], frame);
    EXPECT_EQ(2, frame->val);

    frame = pool->getFrame();
    EXPECT_EQ(frames[3], frame);
    EXPECT_EQ(3, frame->val);

    frame = pool->getFrame();
    EXPECT_EQ(frames[4], frame);
    EXPECT_EQ(4, frame->val);

    EXPECT_EQ(true, pool->isEmpty());

    for (int i = 5; i < 9; ++i) {
        frame = pool->getFrame();
        frame->val = i;
        frames.push_back(frame);
        EXPECT_EQ(frames[i], frame);
    }

    for (int i = 5; i < 9; ++i) {
        pool->putFrame(frames[i]);
    }

    frame = pool->getFrame();
    EXPECT_EQ(frames[8], frame);
    EXPECT_EQ(8, frame->val);

    frame = pool->getFrame();
    EXPECT_EQ(frames[7], frame);
    EXPECT_EQ(7, frame->val);

    frame = pool->getFrame();
    EXPECT_EQ(frames[6], frame);
    EXPECT_EQ(6, frame->val);

    frame = pool->getFrame();
    EXPECT_EQ(frames[5], frame);
    EXPECT_EQ(5, frame->val);

    EXPECT_EQ(9, pool->size());
}
