//
// Created by Ildar Zinatulin on 3/13/21.
//

#include <gtest/gtest.h>
#include "../Trees/PersistentParallelTreap.h"

TEST(Treap, insert) {
    int a1 = 5;
    int a2 = 10;
    TreapNode<int, int> a(a1, a2);
    PersistentParallelTreap<int, int> tree(a);
    tree.setParallelize(false);
    EXPECT_TRUE(tree.isLeaf());
    EXPECT_TRUE(tree.contains(a));
    int b1 = 12;
    int b2 = 5;
    TreapNode<int, int> b(b1, b2);
    int c1 = 9;
    int c2 = 1;
    TreapNode<int, int> c(c1, c2);
    int d1 = 6;
    int d2 = 100;
    TreapNode<int, int> d(d1, d2);
    auto *newTree1 = tree.insert(b);
    auto *newTree2 = newTree1->insert(c)->insert(d);
    EXPECT_TRUE(newTree1->contains(a) and newTree1->contains(b));
    EXPECT_FALSE(tree.contains(c) and tree.contains(d));
    EXPECT_TRUE(newTree2->contains(a) and newTree2->contains(b) and newTree2->contains(c) and newTree2->contains(d));
    EXPECT_TRUE(tree.contains(a));
    EXPECT_FALSE(tree.contains(b));
    EXPECT_TRUE(tree.isCorrectHeap());
}

TEST(BasicTree, insertAll1) {
    int a1 = 5;
    int a2 = 10;
    TreapNode<int, int> a(a1, a2);
    PersistentParallelTreap<int, int> tree(a);
    tree.setParallelize(false);
    int b1 = 12;
    int b2 = 5;
    TreapNode<int, int> b(b1, b2);
    int c1 = 9;
    int c2 = 1;
    TreapNode<int, int> c(c1, c2);
    int d1 = 6;
    int d2 = 100;
    TreapNode<int, int> d(d1, d2);
    std::vector<TreapNode<int, int>> items({b, c, d});
    std::vector<PersistentParallelTreap<int, int> *> *newTrees = tree.insertAll(items);
    std::sort(items.begin(), items.end());
    for (int i = 0; i < items.size(); i++) {
        for (int j = i; j < items.size(); j++) {
            EXPECT_TRUE((*newTrees)[j]->contains(items[i]));
            EXPECT_TRUE((*newTrees)[j]->isCorrectHeap());
        }
    }
}

TEST(BasicTree, insertAll2) {
    PersistentParallelTreap<int, int> tree;
    tree.setParallelize(false);
    std::vector<TreapNode<int, int>> items({
        TreapNode<int, int>(12, 5), TreapNode<int, int>(9, 1), TreapNode<int, int>(6, 100),
        TreapNode<int, int>(100 , 5), TreapNode<int, int>(5, 9), TreapNode<int, int>(345, 7),
        TreapNode<int, int>(72, 2), TreapNode<int, int>(1, -5), TreapNode<int, int>(43, 10),
        TreapNode<int, int>(3535, 11), TreapNode<int, int>(13554, 12), TreapNode<int, int>(13, 13),
        TreapNode<int, int>(974, 14), TreapNode<int, int>(15453, 11), TreapNode<int, int>(43, 15),
        TreapNode<int, int>(539, 10), TreapNode<int, int>(53215, 0), TreapNode<int, int>(65, -9),
        TreapNode<int, int>(13, 12)
    });
    std::vector<PersistentParallelTreap<int, int> *> *newTrees = tree.insertAll(items);
    std::sort(items.begin(), items.end());
    for (int i = 0; i < items.size(); i++) {
        for (int j = i; j < items.size(); j++) {
            EXPECT_TRUE((*newTrees)[j]->contains(items[i]));
            EXPECT_TRUE((*newTrees)[j]->isCorrectHeap());
        }
    }
}

TEST(BasicTree, insertAll1Parallelize) {
    int a1 = 5;
    int a2 = 10;
    TreapNode<int, int> a(a1, a2);
    PersistentParallelTreap<int, int> tree(a);
    int b1 = 12;
    int b2 = 5;
    TreapNode<int, int> b(b1, b2);
    int c1 = 9;
    int c2 = 1;
    TreapNode<int, int> c(c1, c2);
    int d1 = 6;
    int d2 = 100;
    TreapNode<int, int> d(d1, d2);
    std::vector<TreapNode<int, int>> items({b, c, d});
    std::vector<PersistentParallelTreap<int, int> *> *newTrees = tree.insertAll(items);
    std::sort(items.begin(), items.end());
    for (int i = 0; i < items.size(); i++) {
        for (int j = i; j < items.size(); j++) {
            EXPECT_TRUE((*newTrees)[j]->contains(items[i]));
            EXPECT_TRUE((*newTrees)[j]->isCorrectHeap());
        }
    }
}

TEST(BasicTree, insertAll2Parallelize) {
    PersistentParallelTreap<int, int> tree;
    std::vector<TreapNode<int, int>> items({
        TreapNode<int, int>(12, 5), TreapNode<int, int>(9, 1), TreapNode<int, int>(6, 100),
        TreapNode<int, int>(100 , 5), TreapNode<int, int>(5, 9), TreapNode<int, int>(345, 7),
        TreapNode<int, int>(72, 2), TreapNode<int, int>(1, -5), TreapNode<int, int>(43, 10),
        TreapNode<int, int>(3535, 11), TreapNode<int, int>(13554, 12), TreapNode<int, int>(13, 13),
        TreapNode<int, int>(974, 14), TreapNode<int, int>(15453, 11), TreapNode<int, int>(43, 15),
        TreapNode<int, int>(539, 10), TreapNode<int, int>(53215, 0), TreapNode<int, int>(65, -9),
        TreapNode<int, int>(13, 12)
    });
    std::vector<PersistentParallelTreap<int, int> *> *newTrees = tree.insertAll(items);
    std::sort(items.begin(), items.end());
    for (int i = 0; i < items.size(); i++) {
        for (int j = i; j < items.size(); j++) {
            EXPECT_TRUE((*newTrees)[j]->contains(items[i]));
            EXPECT_TRUE((*newTrees)[j]->isCorrectHeap());
        }
    }
}
