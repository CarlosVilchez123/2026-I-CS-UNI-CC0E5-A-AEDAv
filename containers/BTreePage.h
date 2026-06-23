#ifndef __BTREE_PAGE_H__
#define __BTREE_PAGE_H__

#include <utility>
#include "Vector.h"
#include "../types.h"
#include "traits.h"
#include "BTreeNode.h"
#include "util.h"

template <typename Trait> class BTree;

template <typename Trait>
class BTreePage : public BTreeNode<Trait, BTreePage<Trait>> {
    friend class BTree<Trait>;

    using Self    = BTreePage<Trait>;
    using Base    = BTreeNode<Trait, Self>;
    using Entry   = typename Base::Entry;
    using Value   = typename Trait::value_type;
    using Comp    = typename Trait::Comp;
    static constexpr size Order = Trait::Order;

    Comp m_comp{};

    
    size locate(const Value& key) const {
        size pos = btree_utils::lowerBound(this->m_keys, this->m_keyCount, key);
        return pos;
    }

    
    Self* child(size i) const {
        return this->m_children[i];
    }

    
    size freeOnLeft(size pos)  const {
        return pos > 0 ? child(pos - 1)->freeCells() : 0;
    }
    size freeOnRight(size pos) const {
        return pos < this->m_keyCount ? child(pos + 1)->freeCells() : 0;
    }

    
    Entry& leftmostEntry() {
        return this->isLeaf() ? this->m_keys[0] : child(0)->leftmostEntry();
    }

    
    void shiftFromRight(size pos) {
        Self* src = child(pos);
        Self* dst = child(pos - 1);
        while (src->m_keyCount > src->minKeys() &&
               dst->m_keyCount < src->m_keyCount)
        {
            btree_utils::insertAt(dst->m_keys,    this->m_keys[pos - 1], dst->m_keyCount);
            btree_utils::insertAt(dst->m_children, src->m_children[0],   dst->m_keyCount + 1);
            ++dst->m_keyCount;
            this->m_keys[pos - 1] = src->m_keys[0];
            btree_utils::removeAt(src->m_keys,    0);
            btree_utils::removeAt(src->m_children, 0);
            --src->m_keyCount;
        }
    }

    
    void shiftFromLeft(size pos) {
        Self* src = child(pos);
        Self* dst = child(pos + 1);
        while (src->m_keyCount > src->minKeys() &&
               dst->m_keyCount < src->m_keyCount)
        {
            btree_utils::insertAt(dst->m_keys,    this->m_keys[pos], 0);
            btree_utils::insertAt(dst->m_children, src->m_children[src->m_keyCount], 0);
            ++dst->m_keyCount;
            this->m_keys[pos] = src->m_keys[src->m_keyCount - 1];
            --src->m_keyCount;
        }
    }

    
    void drain(Self* node, Vector<VectorTrait<Entry>>& keys, Vector<VectorTrait<Self*>>& children) {
        size n = node->m_keyCount;
        for (size i = 0; i < n; ++i) {
            keys.push_back(node->m_keys[i]);
            children.push_back(child_of(node, i), Ref{});
        }
        children.push_back(child_of(node, n), Ref{});
        node->clearKeys();
    }

    
    static Self* child_of(Self* node, size i) {
        return node->m_children[i];
    }

    
    void distributeIntoThree(
        Vector<VectorTrait<Entry>>&  tmpKeys,
        Vector<VectorTrait<Self*>>&  tmpChildren,
        Self*& c1, Self*& c2, Self*& c3,
        Entry& e1,  Entry& e2)
    {
        size childMaxK = this->m_childMaxKeys;

        auto fill = [&](Self*& node, size from, size to) -> size {
            if (!node) node = new Self(childMaxK, this->m_unique);
            node->clearKeys();
            size i = from;
            for (; i < to; ++i) {
                node->m_keys[i - from]     = tmpKeys[i];
                node->m_children[i - from] = tmpChildren[i];
                ++node->m_keyCount;
            }
            node->m_children[node->m_keyCount] = tmpChildren[i];
            return i;
        };

        size total = tmpKeys.numElems();
        size third = (total - 2) / 3;

        size mid1 = fill(c1, 0, third);
        e1 = tmpKeys[mid1];

        size mid2 = fill(c2, mid1 + 1, mid1 + 1 + third + ((total - 2) % 3 > 0 ? 1 : 0));
        e2 = tmpKeys[mid2];

        fill(c3, mid2 + 1, total);
    }

    
    void splitChild(size pos) {
        Self *c1 = nullptr, *c2 = nullptr;

        // Elige el par de hijos llenos adyacentes a pos
        if (pos > 0 && child(pos - 1)->isFull()) {
            c1 = child(pos - 1);
            c2 = child(pos);
            --pos;
        } else {
            c1 = child(pos);
            c2 = child(pos + 1);
        }

        Vector<VectorTrait<Entry>> tmpKeys;
        Vector<VectorTrait<Self*>> tmpChildren;
        drain(c1, tmpKeys, tmpChildren);
        tmpKeys.push_back(this->m_keys[pos]);
        drain(c2, tmpKeys, tmpChildren);

        Self* c3 = nullptr;
        Entry e1, e2;
        distributeIntoThree(tmpKeys, tmpChildren, c1, c2, c3, e1, e2);

        this->m_keys[pos]     = e1;
        this->m_children[pos] = c1;
        btree_utils::insertAt(this->m_keys,    e2, pos + 1);
        btree_utils::insertAt(this->m_children, c2, pos + 1);
        ++this->m_keyCount;
        this->m_children[pos + 2] = c3;
    }

    
    flag splitRoot() {
        Self *c1 = nullptr, *c2 = nullptr, *c3 = nullptr;
        Entry e1, e2;

        
        Vector<VectorTrait<Entry>> tmpKeys;
        Vector<VectorTrait<Self*>> tmpChildren;
        for (size i = 0; i < this->m_keyCount; ++i)
            tmpKeys.push_back(this->m_keys[i]);
        for (size i = 0; i <= this->m_keyCount; ++i)
            tmpChildren.push_back(child(i), Ref{});

        this->clearKeys();
        distributeIntoThree(tmpKeys, tmpChildren, c1, c2, c3, e1, e2);

        this->m_keys[0]     = e1;  this->m_children[0] = c1;  ++this->m_keyCount;
        this->m_keys[1]     = e2;  this->m_children[1] = c2;  ++this->m_keyCount;
        this->m_children[2] = c3;
        return true;
    }

    
    flag tryRedistribute(size& pos) {
        if (child(pos)->isUnderflow()) {
            size nLeft  = pos > 0                  ? child(pos - 1)->m_keyCount : 0;
            size nRight = pos < this->m_keyCount   ? child(pos + 1)->m_keyCount : 0;
            if (nLeft > nRight) {
                if (child(pos - 1)->m_keyCount > child(pos - 1)->minKeys()) {
                    shiftFromRight(pos); return true;
                }
                if (pos == this->m_keyCount) { --pos; return false; }
                return false;
            } else {
                if (child(pos + 1)->m_keyCount > child(pos + 1)->minKeys()) {
                    shiftFromLeft(pos); return true;
                }
                if (pos == 0) { ++pos; return false; }
                return false;
            }
        }
        
        size fl = freeOnLeft(pos), fr = freeOnRight(pos);
        if (!fl && !fr && child(pos)->isFull()) return false;
        if (fl > fr) shiftFromRight(pos); else shiftFromLeft(pos);
        return true;
    }

    flag tryRedistributeDouble(size pos) {
        if (child(pos - 1)->isUnderflow()) {
            shiftFromLeft(pos + 1); shiftFromLeft(pos);
            return !child(pos - 1)->isUnderflow();
        }
        if (child(pos + 1)->isUnderflow()) {
            shiftFromRight(pos - 1); shiftFromRight(pos);
            return !child(pos + 1)->isUnderflow();
        }
        shiftFromLeft(pos - 1); shiftFromRight(pos + 1);
        return !child(pos)->isUnderflow();
    }

    flag resolveUnderflow(size& pos) {
        return tryRedistribute(pos) || tryRedistributeDouble(pos);
    }

    
    bt_code mergeChildren(size pos) {
        Self *c1 = child(pos - 1), *c2 = child(pos), *c3 = child(pos + 1);

        Vector<VectorTrait<Entry>> tmpKeys;
        Vector<VectorTrait<Self*>> tmpChildren;
        drain(c1, tmpKeys, tmpChildren); tmpKeys.push_back(this->m_keys[pos - 1]);
        drain(c2, tmpKeys, tmpChildren); tmpKeys.push_back(this->m_keys[pos]);
        drain(c3, tmpKeys, tmpChildren);

        
        c3->m_keyCount = 0;
        delete c3;

        
        size cap = c1->freeCells(), i = 0;
        for (; i < cap; ++i) {
            c1->m_keys[i]     = tmpKeys[i];
            c1->m_children[i] = tmpChildren[i];
            ++c1->m_keyCount;
        }
        c1->m_children[c1->m_keyCount] = tmpChildren[i];

        this->m_keys[pos - 1]     = tmpKeys[i];
        this->m_children[pos - 1] = c1;
        btree_utils::removeAt(this->m_keys,    pos);
        btree_utils::removeAt(this->m_children, pos);
        --this->m_keyCount;

        
        size cap2 = c2->freeCells(), j = ++i;
        for (size k = 0; k < cap2; ++k, ++j) {
            c2->m_keys[k]     = tmpKeys[j];
            c2->m_children[k] = tmpChildren[j];
            ++c2->m_keyCount;
        }
        c2->m_children[c2->m_keyCount] = tmpChildren[j];
        this->m_children[pos] = c2;

        return this->isUnderflow() ? bt_code::underflow : bt_code::ok;
    }

    
    bt_code mergeRoot() {
        Self *c1 = child(0), *c2 = child(1), *c3 = child(2);
        size total = c1->m_keyCount + c2->m_keyCount + c3->m_keyCount + 2;

        Vector<VectorTrait<Entry>> tmpKeys;
        Vector<VectorTrait<Self*>> tmpChildren;
        drain(c1, tmpKeys, tmpChildren); tmpKeys.push_back(this->m_keys[0]);
        drain(c2, tmpKeys, tmpChildren); tmpKeys.push_back(this->m_keys[1]);
        drain(c3, tmpKeys, tmpChildren);

        this->clearKeys();
        for (size i = 0; i < total; ++i) {
            this->m_keys[i]     = tmpKeys[i];
            this->m_children[i] = tmpChildren[i];
            ++this->m_keyCount;
        }
        this->m_children[total] = tmpChildren[total];

        
        c1->m_keyCount = 0; delete c1;
        c2->m_keyCount = 0; delete c2;
        c3->m_keyCount = 0; delete c3;
        return bt_code::rootMerged;
    }

public:
    explicit BTreePage(size maxKeys, flag unique = true)
        : Base(maxKeys, unique) {}

    ~BTreePage() = default;

    
    bt_code insert(const Value& key, Ref ref) {
        size pos = locate(key);

        if (pos < this->m_keyCount && this->m_keys[pos] == key) {
            if (this->m_unique) return bt_code::duplicate;
            this->m_keys.node(pos).getDataRef().touch();
            return bt_code::ok;
        }

        if (this->isLeaf()) {
            btree_utils::insertAt(this->m_keys, Entry(key, ref), pos);
            ++this->m_keyCount;
            return this->isOverflow() ? bt_code::overflow : bt_code::ok;
        }

        bt_code result = child(pos)->insert(key, ref);
        if (result == bt_code::duplicate) return bt_code::duplicate;

        if (result == bt_code::overflow) {
            if (!tryRedistribute(pos)) splitChild(pos);
            return this->isOverflow() ? bt_code::overflow : bt_code::ok;
        }
        return this->isOverflow() ? bt_code::overflow : bt_code::ok;
    }

    
    bt_code remove(const Value& key, Value& outKey, Ref& outRef) {
        size pos = locate(key);
        bt_code result = bt_code::ok;

        if (pos < this->m_keyCount && this->m_keys[pos] == key) {
            outKey = this->m_keys[pos];
            outRef = this->m_keys.node(pos).getRef();

            if (this->isLeaf()) {
                btree_utils::removeAt(this->m_keys, pos);
                --this->m_keyCount;
                return this->isUnderflow() ? bt_code::underflow : bt_code::ok;
            }
            
            Entry& successor = child(pos + 1)->leftmostEntry();
            std::swap(this->m_keys[pos], successor);
            Value discard{}; Ref discardRef{};
            result = child(pos + 1)->remove(key, discard, discardRef);
            pos = pos + 1;
        } else if (pos <= this->m_keyCount && this->m_children[pos]) {
            result = child(pos)->remove(key, outKey, outRef);
        } else {
            return bt_code::notFound;
        }

        if (result == bt_code::underflow) {
            if (resolveUnderflow(pos)) return bt_code::ok;
            if (this->isRoot() && this->m_keyCount == 2) return mergeRoot();
            return mergeChildren(pos);
        }
        return result;
    }

    
    flag search(const Value& key, Value& outKey, Ref& outRef) {
        size pos = locate(key);

        if (pos < this->m_keyCount && this->m_keys[pos] == key) {
            outKey = this->m_keys[pos];
            outRef = this->m_keys.node(pos).getRef();
            this->m_keys.node(pos).getDataRef().touch();
            return true;
        }
        if (this->m_children[pos])
            return child(pos)->search(key, outKey, outRef);
        return false;
    }

    
    template <typename Func, typename... Args>
    void forEach(level lv, Func func, Args&&... args) {
        for (size i = 0; i < this->m_keyCount; ++i) {
            if (this->m_children[i])
                child(i)->forEach(lv + 1, func, std::forward<Args>(args)...);
            func(this->m_keys[i], lv, std::forward<Args>(args)...);
        }
        if (this->m_children[this->m_keyCount])
            child(this->m_keyCount)->forEach(lv + 1, func, std::forward<Args>(args)...);
    }

    // fisrthat
    template <typename Func, typename... Args>
    Entry* firstThat(level lv, Func func, Args&&... args) {
        for (size i = 0; i < this->m_keyCount; ++i) {
            if (this->m_children[i])
                if (Entry* found = child(i)->firstThat(lv + 1, func, std::forward<Args>(args)...))
                    return found;
            if (func(this->m_keys[i], lv, std::forward<Args>(args)...))
                return &this->m_keys[i];
        }
        if (this->m_children[this->m_keyCount])
            return child(this->m_keyCount)->firstThat(lv + 1, func, std::forward<Args>(args)...);
        return nullptr;
    }

    // foreach
    template <typename Func, typename... Args>
    void forEachPage(level lv, Func func, Args&&... args) {
        func(this->m_keyCount, lv, std::forward<Args>(args)...);
        for (size i = 0; i <= this->m_keyCount; ++i)
            if (this->m_children[i])
                child(i)->forEachPage(lv + 1, func, std::forward<Args>(args)...);
    }
};

#endif // __BTREE_PAGE_H__