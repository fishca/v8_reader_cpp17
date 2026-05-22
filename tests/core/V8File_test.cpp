#include <gtest/gtest.h>
#include "v8reader/core/IV8Repository.h"

TEST(V8CoreTest, RepositoryCreation) {
    auto repo = v8::core::createV8Repository();
    EXPECT_NE(repo, nullptr);
    EXPECT_EQ(repo->getRoot(), nullptr);
}