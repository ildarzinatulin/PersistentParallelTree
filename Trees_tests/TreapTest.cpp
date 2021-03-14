//
// Created by Ildar Zinatulin on 3/13/21.
//

#include <gtest/gtest.h>
#include "../Trees/PersistentParallelTreap.h"

//TODO: написать метод, который бы определял, что это корректная куча.
//TODO: Скопировать и поправить тесты из BasicTreeTest.

TEST(Treap, insert) {
    TreapNode<int, int> a(5, 10);
    PersistentParallelTreap<int, int> tree(a);
    tree.setParallelize(false);
    EXPECT_TRUE(tree.isLeaf());
    EXPECT_TRUE(tree.contains(a));
    TreapNode<int, int> b(12, 5);
    TreapNode<int, int> c(9, 1);
    TreapNode<int, int> d(6, 100);
    auto *newTree1 = tree.insert(b);
    auto *newTree2 = newTree1->insert(c)->insert(d);
    EXPECT_TRUE(newTree1->contains(a) and newTree1->contains(b));
    EXPECT_FALSE(tree.contains(c) and tree.contains(d));
    EXPECT_TRUE(newTree2->contains(a) and newTree2->contains(b) and newTree2->contains(c) and newTree2->contains(d));
    EXPECT_TRUE(tree.contains(a));
    EXPECT_FALSE(tree.contains(b));
}