//
// Created by Ildar Zinatulin on 3/7/21.
//

#ifndef PERSISTENTPARALLELTREE_PERSISTENTPARALLELTREAP_H
#define PERSISTENTPARALLELTREE_PERSISTENTPARALLELTREAP_H

#include <datapar.hpp>

template<typename V, typename P>
class TreapNode {
protected:
    V *value;
    P *priority;

public:

    TreapNode(V *value, P *priority) : value(value), priority(priority) {}

    TreapNode(V value, P prority) : value(new V(value)), priority(new P(prority)) {}

    V *getValue() const {
        return value;
    }

    P *getPriority() const {
        return priority;
    }

    bool prior(TreapNode<V,P> &node) {
        return *priority > *(node.priority);
    }

    friend bool operator == (TreapNode<V,P> node1, TreapNode<V,P> node2) {
        return *(node1.value) == *(node2.value);
    }
    friend bool operator != (TreapNode<V,P> node1, TreapNode<V,P> node2) {
        return *(node1.value) != *(node2.value);
    }
    friend bool operator > (TreapNode<V,P> node1, TreapNode<V,P> node2) {
        return *(node1.value) > *(node2.value);
    }
    friend bool operator < (TreapNode<V,P> node1, TreapNode<V,P> node2) {
        return *(node1.value) < *(node2.value);
    }
};

template<typename V, typename P>
class PersistentParallelTreap {
public:
    PersistentParallelTreap(PersistentParallelTreap<V, P> *leftChild,
                            PersistentParallelTreap<V, P> *rightChild,
                            TreapNode<V, P> &value) {
        this->leftChild = leftChild;
        this->rightChild = rightChild;
        this->value = &value;
    }

    PersistentParallelTreap() = default;

    explicit PersistentParallelTreap(TreapNode<V, P> &value) {
        this->value = &value;
    }

    PersistentParallelTreap<V, P> *insert(TreapNode<V, P> &newValue) {
        if (isEmpty()) {
            return new PersistentParallelTreap<V, P>(newValue);
        }
        ResultOfSplit *resultOfSplit = split(newValue);
        auto *result = join(resultOfSplit->left, resultOfSplit->right, newValue);
        delete resultOfSplit;
        return result;
    }

    /**
     * Возращает вектор персистентных деревьев, где result[i] - дерево, где вставлены [0, i] элементы items
     */
    std::vector<PersistentParallelTreap<V, P>*> *insertAll(std::vector<TreapNode<V, P>> items) {
        if (items.empty()) {
            return new std::vector<PersistentParallelTreap<V, P>*>();
        }
        if (items.size() == 1) {
            return new std::vector<PersistentParallelTreap<V, P>*>({insert(items[0])});
        }

        std::sort(items.begin(), items.end());
        auto *result = new std::vector<PersistentParallelTreap<V, P>*>(items.size());
        auto *treeNodes = new std::vector<InsertNode*>(items.size());
        int currentIndex = items.size() / 2;
        ResultOfSplit *resultOfSplit = split(items[currentIndex]);

        PersistentParallelTreap<V, P> *leftChildWithNewItems;
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

    bool contains(TreapNode<V, P> &v) {
        PersistentParallelTreap<V, P> *currentTree = this;
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
        return leftChild == nullptr and rightChild == nullptr and value == nullptr;
    }

    void setParallelize(bool t) {
        parallelize = t;
    }

    //PersistentParallelTreap<V, P> find(T *value);

    //void remove(T *value);

    //std::vector<PersistentParallelTreap<V, P>> removeAll(std::vector<T> *items);

protected:
    PersistentParallelTreap<V, P> *leftChild = nullptr;
    PersistentParallelTreap<V, P> *rightChild = nullptr;
    TreapNode<V, P> *value = nullptr;
    bool parallelize = true;

    PersistentParallelTreap<V, P> *join(PersistentParallelTreap<V, P> *l, PersistentParallelTreap<V, P> *r,
                                        TreapNode<V, P> &newValue) {
        if ((l == nullptr or l->isEmpty()) and (r == nullptr or r->isEmpty())) {
            return new PersistentParallelTreap<V, P>(newValue);
        }

        if (l == nullptr or l->isEmpty()) {
            if (newValue.prior(*(r->value))) {
                return new PersistentParallelTreap<V, P>(nullptr, r, newValue);
            }
            PersistentParallelTreap<V, P> *result = new PersistentParallelTreap<V, P>(r->leftChild, r->rightChild,
                                                                                      *(r->value));
            PersistentParallelTreap<V, P> *prevTree = result;
            PersistentParallelTreap<V, P> *currentTree = result->leftChild;
            while (currentTree != nullptr and !(currentTree->isEmpty()) and currentTree->value->prior(newValue)) {
                prevTree->leftChild = new PersistentParallelTreap<V, P>(currentTree->leftChild,
                        currentTree->rightChild, *(currentTree->value));
                currentTree = currentTree->leftChild;
            }
            prevTree->leftChild = new PersistentParallelTreap<V, P>(nullptr, currentTree, newValue);

            return result;
        }
        if (r == nullptr or r->isEmpty()) {
            return join(l, new PersistentParallelTreap<V, P>(newValue));
        }

        if (newValue.prior(*(l->value)) and newValue.prior(*(r->value))) {
            return new PersistentParallelTreap<V, P>(l, r, newValue);
        }
        if (l->value->prior(*(r->value))) {
            return new PersistentParallelTreap<V, P>(l->leftChild, join(l->rightChild, r, newValue), *(l->value));
        }

        return new PersistentParallelTreap<V, P>(join(l, r->leftChild, newValue), r->rightChild, *(r->value));
    };

    PersistentParallelTreap<V, P> *join(PersistentParallelTreap<V, P> *l, PersistentParallelTreap<V, P> *r) {
        if ((l == nullptr or l->isEmpty()) and (r == nullptr or r->isEmpty())) {
            return new PersistentParallelTreap<V, P>();
        }
        if (l == nullptr or l->isEmpty()) {
            return r;
        }
        if ((r == nullptr or r->isEmpty())) {
            return l;
        }

        auto *newLTree = new PersistentParallelTreap<V, P>(l->leftChild, l->rightChild, *(l->value));
        PersistentParallelTreap<V, P> *prevTree = newLTree;
        PersistentParallelTreap<V, P> *currentTree = prevTree->rightChild;
        if (currentTree == nullptr) {
            prevTree->rightChild = nullptr;
            return join(newLTree->leftChild, r, *(newLTree->value));
        }
        while (currentTree->rightChild != nullptr and !currentTree->rightChild->isEmpty()) {
            auto *newRightChild = new PersistentParallelTreap<V, P>(currentTree->leftChild, currentTree->rightChild,
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
        PersistentParallelTreap<V, P> *left;
        PersistentParallelTreap<V, P> *right;
    };

    struct InsertNode {
        PersistentParallelTreap<V, P> *leftChildWithNewItem;
        PersistentParallelTreap<V, P> *rightResultOfSplit;
        TreapNode<V, P> *item;
    };

    void insertAllHelperStage2(int l, int r, std::vector<InsertNode*> *treeNodes,
                               PersistentParallelTreap<V, P> *leftPartWithNewItems,
                               PersistentParallelTreap<V, P> *rightPartWithOutNewItems,
                               std::vector<PersistentParallelTreap<V, P>*> *result) {
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
                                          join((*treeNodes)[currentIndex]->rightResultOfSplit,
                                               rightPartWithOutNewItems),
                                          result);
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

    PersistentParallelTreap<V, P> *insertAllHelperStage1(PersistentParallelTreap<V, P> *subtree,
                                                         std::vector<TreapNode<V, P>> &items, int l, int r,
                                                         std::vector<InsertNode*> *treeNodes) {
        if (l == r) {
            auto *subtreeWithNewItem = subtree->insert(items[l]);
            ResultOfSplit *resultOfSplit = subtree->split(items[l]);
            (*treeNodes)[l] = new InsertNode{resultOfSplit->left, resultOfSplit->right, &items[l]};
            return subtreeWithNewItem;
        }

        int currentIndex = l + ((r - l) / 2);
        ResultOfSplit *resultOfSplit = subtree->split(items[currentIndex]);
        PersistentParallelTreap<V, P> *leftChildWithNewItems = nullptr;
        PersistentParallelTreap<V, P> *rightChildWithNewItems = nullptr;

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

        delete resultOfSplit;
        (*treeNodes)[currentIndex] = new InsertNode{leftChildWithNewItems, resultOfSplit->right, &items[currentIndex]};
        return join(leftChildWithNewItems, rightChildWithNewItems, items[currentIndex]);
    }

    ResultOfSplit *split(TreapNode<V, P> &valueForSplit) {
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
            if (valueForSplit< *value) {
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

#endif //PERSISTENTPARALLELTREE_PERSISTENTPARALLELTREAP_H
