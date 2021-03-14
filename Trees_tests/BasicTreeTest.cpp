//
// Created by Ildar Zinatulin on 3/13/21.
//

#include <gtest/gtest.h>
#include <BasicPersistentTree.h>

TEST(BasicTree, insert) {
    int a = 5;
    BasicPersistentTree<int> tree(a);
    tree.setParallelize(false);
    EXPECT_TRUE(tree.isLeaf());
    EXPECT_TRUE(tree.contains(a));
    int b = 12;
    int c = 9;
    int d = 6;
    BasicPersistentTree<int> *newTree1 = tree.insert(b);
    BasicPersistentTree<int> *newTree2 = newTree1->insert(c)->insert(d);
    EXPECT_TRUE(newTree1->contains(a) and newTree1->contains(b));
    EXPECT_FALSE(tree.contains(c) and tree.contains(d));
    EXPECT_TRUE(newTree2->contains(a) and newTree2->contains(b) and newTree2->contains(c) and newTree2->contains(d));
    EXPECT_TRUE(tree.contains(a));
    EXPECT_FALSE(tree.contains(b));
}

TEST(BasicTree, insertAll1) {
    int a = 22;
    BasicPersistentTree<int> tree(a);
    tree.setParallelize(false);
    int b = 12;
    int c = 9;
    int d = 6;
    std::vector<int> items({12, 9, 6});
    std::vector<BasicPersistentTree<int> *> *newTrees = tree.insertAll(items);
    for (int i = 0; i < items.size(); i++) {
        EXPECT_TRUE((*newTrees)[i]->contains(a) and (*newTrees)[i]->contains(d));
    }
    for (int i = 1; i < items.size(); i++) {
        EXPECT_TRUE((*newTrees)[i]->contains(c));
    }
    EXPECT_TRUE((*newTrees)[2]->contains(b));
}

TEST(BasicTree, insertAll2) {
    BasicPersistentTree<int> tree;
    tree.setParallelize(false);
    std::vector<int> items({12, 9, 6, 100 , 5, 345, 72, 1, 43, 3535, 13554, 13, 974, 15453, 43, 539, 53215, 65, 13});
    std::vector<BasicPersistentTree<int> *> *newTrees = tree.insertAll(items);
    std::sort(items.begin(), items.end());
    for (int i = 0; i < items.size(); i++) {
        for (int j = i; j < items.size(); j++) {
            EXPECT_TRUE((*newTrees)[j]->contains(items[i]));
        }
    }
}

TEST(BasicTree, insertAll1Parallelize) {
    int a = 22;
    BasicPersistentTree<int> tree(a);
    std::vector<int> items({12, 9, 6});
    std::vector<BasicPersistentTree<int> *> *newTrees = tree.insertAll(items);
    std::sort(items.begin(), items.end());
    for (int i = 0; i < items.size(); i++) {
        for (int j = i; j < items.size(); j++) {
            EXPECT_TRUE((*newTrees)[j]->contains(items[i]));
        }
    }
}

TEST(BasicTree, insertAll2Parallelize) {
    BasicPersistentTree<int> tree;
    std::vector<int> items({12, 9, 6, 100 , 5, 345, 72, 1, 43, 3535, 13554, 13, 974, 15453, 43, 539, 53215, 65, 13});
    std::vector<BasicPersistentTree<int> *> *newTrees = tree.insertAll(items);
    std::sort(items.begin(), items.end());
    for (int i = 0; i < items.size(); i++) {
        for (int j = i; j < items.size(); j++) {
            EXPECT_TRUE((*newTrees)[j]->contains(items[i]));
        }
    }
}
