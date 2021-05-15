//
// Created by Ildar Zinatulin on 2/6/21.
//

#ifndef PERSISTENTPARALLELTREE_BASICPERSISTENTTREE_H
#define PERSISTENTPARALLELTREE_BASICPERSISTENTTREE_H

#include <__bit_reference>
#include <utility>
#include <datapar.hpp>

template<typename T>
class BasicPersistentTree {
public:

    explicit BasicPersistentTree(T &value) {
        this->value = &value;
    }

    BasicPersistentTree() = default;

    BasicPersistentTree(BasicPersistentTree *leftChild, BasicPersistentTree *rightChild, T &value) {
        this->leftChild = leftChild;
        this->rightChild = rightChild;
        this->value = &value;
    }

    BasicPersistentTree<T> *insert(T &newValue) {
        if (isEmpty()) {
            return new BasicPersistentTree<T>(newValue);
        }
        ResultOfSplit *resultOfSplit = split(newValue);
        auto *result = join(resultOfSplit->left, resultOfSplit->right, newValue);
        delete resultOfSplit;
        return result;
    }

    /**
     * Возращает вектор персистентных деревьев, где result[i] - дерево, где вставлены [0, i] элементы items
     */
    std::vector<BasicPersistentTree<T>*> *insertAll(std::vector<T> items) {
        if (items.empty()) {
            return new std::vector<BasicPersistentTree<T>*>();
        }
        if (items.size() == 1) {
            return new std::vector<BasicPersistentTree<T>*>({insert(items[0])});
        }

        std::sort(items.begin(), items.end());
        auto *result = new std::vector<BasicPersistentTree<T>*>(items.size());
        auto *treeNodes = new std::vector<InsertNode*>(items.size());
        int currentIndex = items.size() / 2;
        ResultOfSplit *resultOfSplit = split(items[currentIndex]);

        BasicPersistentTree<T> *leftChildWithNewItems;
        if (!parallelize) {
            leftChildWithNewItems = insertAllHelperStage1(resultOfSplit->left, items, 0, currentIndex - 1,
                                                          treeNodes);

            insertAllHelperStage1(resultOfSplit->right, items, currentIndex + 1, items.size() - 1, treeNodes);

            insertAllHelperStage2(0, currentIndex - 1, treeNodes, nullptr, resultOfSplit->right, result);
            insertAllHelperStage2(currentIndex + 1, items.size() - 1, treeNodes,
                                  leftChildWithNewItems->insert(items[currentIndex]), nullptr, result);
        } else {
            pasl::pctl::granularity::fork2([&] (){
                leftChildWithNewItems = insertAllHelperStage1(resultOfSplit->left, items, 0, currentIndex - 1,
                                                              treeNodes);
            }, [&] {
                insertAllHelperStage1(resultOfSplit->right, items, currentIndex + 1, items.size() - 1, treeNodes);
            });

            pasl::pctl::granularity::fork2([&] {
                insertAllHelperStage2(0, currentIndex - 1, treeNodes, nullptr, resultOfSplit->right, result);
            }, [&] {
                insertAllHelperStage2(currentIndex + 1, items.size() - 1, treeNodes,
                                      leftChildWithNewItems->insert(items[currentIndex]), nullptr, result);
            });
        }

        (*result)[currentIndex] = join(leftChildWithNewItems, resultOfSplit->right, items[currentIndex]);
        delete resultOfSplit;
        return result;
    }

    bool isLeaf() {
        return rightChild == nullptr && leftChild == nullptr;
    }

    bool contains(T &v) {
        BasicPersistentTree<T> *currentTree = this;
        while (!currentTree->isEmpty()) {
            if (*currentTree->value == v) {
                return true;
            }
            if (*currentTree->value < v) {
                if (currentTree->rightChild == nullptr or currentTree->rightChild->isEmpty()) return false;
                currentTree = currentTree->rightChild;
            } else {
                if (currentTree->leftChild == nullptr or currentTree->leftChild->isEmpty()) return false;
                currentTree = currentTree->leftChild;
            }
        }
        return false;
    }

    bool isEmpty() {
        if (this == nullptr) return true;
        return leftChild == nullptr and rightChild == nullptr and value == nullptr;
    }

    void setParallelize(bool t) {
        parallelize = t;
    }

    //BasicPersistentTree<T> find(T *value);

    //void remove(T *value);

    //std::vector<BasicPersistentTree<T>> removeAll(std::vector<T> *items);

protected:
    BasicPersistentTree<T> *leftChild = nullptr;
    BasicPersistentTree<T> *rightChild = nullptr;
    T *value = nullptr;
    bool parallelize = true;

    virtual BasicPersistentTree<T> *join(BasicPersistentTree<T> *l, BasicPersistentTree<T> *r, T &newValue) {
        return new BasicPersistentTree<T>(l, r, newValue);
    }

    BasicPersistentTree<T> *join(BasicPersistentTree<T> *l, BasicPersistentTree<T> *r) {
        if ((l == nullptr or l->isEmpty()) and (r == nullptr or r->isEmpty())) {
            return new BasicPersistentTree<T>();
        }
        if (l == nullptr or l->isEmpty()) {
            return r;
        }
        if ((r == nullptr or r->isEmpty())) {
            return l;
        }

        auto *newLTree = new BasicPersistentTree<T>(l->leftChild, l->rightChild, *(l->value));
        BasicPersistentTree<T> *prevTree = newLTree;
        BasicPersistentTree<T> *currentTree = prevTree->rightChild;
        if (currentTree == nullptr) {
            prevTree->rightChild = nullptr;
            return join(newLTree->leftChild, r, *(newLTree->value));
        }
        while (currentTree->rightChild != nullptr and !currentTree->rightChild->isEmpty()) {
            auto *newRightChild = new BasicPersistentTree<T>(currentTree->leftChild,
                                                             currentTree->rightChild,
                                                             *(currentTree->value));
            prevTree->rightChild = newRightChild;
            prevTree = currentTree;
            currentTree = currentTree->rightChild;
        }
        prevTree->rightChild = currentTree->leftChild;
        return join(newLTree, r, *(currentTree->value));
    }

private:
    struct ResultOfSplit {
        BasicPersistentTree<T> *left;
        BasicPersistentTree<T> *right;
    };
    struct InsertNode {
        BasicPersistentTree<T> *leftChildWithNewItem;
        BasicPersistentTree<T> *rightResultOfSplit;
        T *item;
    };

    void insertAllHelperStage2(int l, int r, std::vector<InsertNode*> *treeNodes,
                               BasicPersistentTree<T> *leftPartWithNewItems,
                               BasicPersistentTree<T> *rightPartWithOutNewItems,
                               std::vector<BasicPersistentTree<T>*> *result) {
        if (l == r) {
            (*result)[l] = join(join(leftPartWithNewItems, (*treeNodes)[l]->leftChildWithNewItem),
                                join((*treeNodes)[l]->rightResultOfSplit, rightPartWithOutNewItems),
                                *((*treeNodes)[l]->item));
            return;
        }
        int currentIndex = l + ((r - l) / 2);

        if (!parallelize) {
            if (l != currentIndex) {
                insertAllHelperStage2(l, currentIndex - 1, treeNodes, leftPartWithNewItems,
                                      join((*treeNodes)[currentIndex]->rightResultOfSplit, rightPartWithOutNewItems), result);
            }
            if (r != currentIndex) {
                insertAllHelperStage2(currentIndex + 1, r, treeNodes,
                                      join(leftPartWithNewItems,
                                           (*treeNodes)[currentIndex]->leftChildWithNewItem->insert(
                                                   *((*treeNodes)[currentIndex]->item))),
                                      rightPartWithOutNewItems, result);
            }

        } else {
            pasl::pctl::granularity::fork2([&] (){
                if (l != currentIndex) {
                    insertAllHelperStage2(l, currentIndex - 1, treeNodes, leftPartWithNewItems,
                                          join((*treeNodes)[currentIndex]->rightResultOfSplit, rightPartWithOutNewItems), result);
                }
            }, [&] {
                if (r != currentIndex) {
                    insertAllHelperStage2(currentIndex + 1, r, treeNodes,
                                          join(leftPartWithNewItems,
                                               (*treeNodes)[currentIndex]->leftChildWithNewItem->insert(
                                                       *((*treeNodes)[currentIndex]->item))),
                                          rightPartWithOutNewItems, result);
                }
            });
        }

        (*result)[currentIndex] = join(join(leftPartWithNewItems, (*treeNodes)[currentIndex]->leftChildWithNewItem),
                                       join((*treeNodes)[currentIndex]->rightResultOfSplit, rightPartWithOutNewItems),
                                       *(*treeNodes)[currentIndex]->item);
    }

    BasicPersistentTree<T> *insertAllHelperStage1(BasicPersistentTree<T> *subtree, std::vector<T> &items, int l, int r,
                                                  std::vector<InsertNode*> *treeNodes) {
        if (l == r) {
            auto *subtreeWithNewItem = subtree->insert(items[l]);
            ResultOfSplit *resultOfSplit = subtree->split(items[l]);
            (*treeNodes)[l] = new InsertNode{resultOfSplit->left, resultOfSplit->right, &items[l]};
            return subtreeWithNewItem;
        }

        int currentIndex = l + ((r - l) / 2);
        ResultOfSplit *resultOfSplit = subtree->split(items[currentIndex]);
        BasicPersistentTree<T> *leftChildWithNewItems = nullptr;
        BasicPersistentTree<T> *rightChildWithNewItems = nullptr;

        if (!parallelize) {
            if (l == currentIndex) {
                leftChildWithNewItems = resultOfSplit->left;
            } else {
                leftChildWithNewItems = insertAllHelperStage1(resultOfSplit->left, items, l, currentIndex - 1, treeNodes);
            }

            if (r == currentIndex) {
                rightChildWithNewItems = resultOfSplit->right;
            } else {
                rightChildWithNewItems = insertAllHelperStage1(resultOfSplit->right, items, currentIndex + 1, r, treeNodes);
            }
        } else {
            pasl::pctl::granularity::fork2([&] (){
                if (l == currentIndex) {
                    leftChildWithNewItems = resultOfSplit->left;
                } else {
                    leftChildWithNewItems = insertAllHelperStage1(resultOfSplit->left, items, l, currentIndex - 1, treeNodes);
                }
            }, [&] {
                if (r == currentIndex) {
                    rightChildWithNewItems = resultOfSplit->right;
                } else {
                    rightChildWithNewItems = insertAllHelperStage1(resultOfSplit->right, items, currentIndex + 1, r, treeNodes);
                }
            });
        }

        (*treeNodes)[currentIndex] = new InsertNode{leftChildWithNewItems, resultOfSplit->right, &items[currentIndex]};
        delete resultOfSplit;
        return join(leftChildWithNewItems, rightChildWithNewItems, items[currentIndex]);
    }

    ResultOfSplit *split(T &valueForSplit) {
        auto *result = new ResultOfSplit();
        if (isEmpty()) {
            result->left = nullptr;
            result->right = nullptr;
            return result;
        }
        if (isLeaf()) {
            if (valueForSplit < *value) {
                result->left = nullptr;
                result->right = this;
            } else {
                result->left = this;
                result->right = nullptr;
            }
            return result;
        }
        if (*value == valueForSplit) {
            result->left = leftChild;
            result->right = rightChild;
        } else {
            if (valueForSplit < *value) {
                ResultOfSplit *splitedLeft;
                if (leftChild == nullptr) {
                    splitedLeft = new ResultOfSplit{nullptr, nullptr};
                } else {
                    splitedLeft = leftChild->split(valueForSplit);
                }
                result->left = splitedLeft->left;
                result->right = join(splitedLeft->right, rightChild, *value);
                delete splitedLeft;
            } else {
                ResultOfSplit *splitedRight;
                if (rightChild == nullptr) {
                    splitedRight = new ResultOfSplit{nullptr, nullptr};
                } else {
                    splitedRight = rightChild->split(valueForSplit);
                }
                result->left = join(leftChild, splitedRight->left, *value);
                result->right = splitedRight->right;
                delete splitedRight;
            }
        }
        return result;
    }

};

#endif //PERSISTENTPARALLELTREE_BASICPERSISTENTTREE_H
