#ifndef __C_FILESYSTEM_RBTREE_H__
#define __C_FILESYSTEM_RBTREE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

#include "ccore/c_debug.h"

namespace ncore
{
    // https://sedgewick.io/wp-content/themes/sedgewick/papers/2008LLRB.pdf
    // https://github.com/petar/GoLLRB

    struct rbnode_t
    {
        rbnode_t() : m_left(nullptr), m_right(nullptr), m_item(nullptr), m_black(false) {}
        rbnode_t* m_left;
        rbnode_t* m_right;
        void*     m_item;
        bool      m_black;
    };

    struct rbtree_t
    {
        rbnode_t* m_root;
        s32       m_count;
    };

    struct rbstack_t
    {
        rbnode_t* item_ptr[32];
        u32       item_isleft;
        s32       sp;

        rbstack_t() : flag(0), sp(0) {}

        bool empty() const { return sp == 0; }

        rbnode_t* pop()
        {
            if (sp == 0)
                return nullptr;
            return item_ptr[--sp];
        }

        rbnode_t* pop(bool& left)
        {
            left = false;
            if (sp == 0)
                return nullptr;
            rbnode_t* n = item_ptr[--sp];
            left        = (item_isleft & (1 << sp)) != 0;
            return n;
        }

        void push(rbnode_t* node)
        {
            ASSERT(sp < 32);
            item_ptr[sp++] = node;
        }

        void push_left(rbnode_t* node)
        {
            ASSERT(sp < 32);
            item_isleft    = item_isleft | (1 << sp);
            item_ptr[sp++] = node;
        }

        void push_right(rbnode_t* node)
        {
            ASSERT(sp < 32);
            item_ptr[sp++] = node;
        }
    };

    inline bool isRed(rbnode_t* h) { return (h == nullptr) ? false : !h->m_black; }
    inline void flip(rbnode_t* h)
    {
        h->m_black          = !h->m_black;
        h->m_left->m_black  = !h->m_left->m_black;
        h->m_right->m_black = !h->m_right->m_black;
    }

    inline rbnode_t* rotateLeft(rbnode_t* h)
    {
        rbnode_t* x = h->m_right;
        if (x->m_black)
        {
            // error!, rotating a black link?
            return nullptr;
        }
        h->m_right = x->m_left;
        x->m_left  = h;
        x->m_black = h->m_black;
        h->m_black = false;
        return x;
    }

    inline rbnode_t* rotateRight(rbnode_t* h)
    {
        rbnode_t* x = h->m_left;
        if (x->m_black)
        {
            // error!, rotating a black link?
            return nullptr;
        }
        h->m_left  = x->m_right;
        x->m_right = h;
        x->m_black = h->m_black;
        h->m_black = false;
        return x;
    }

    // REQUIRE: Left and Right children must be present
    inline rbnode_t* moveRedLeft(rbnode_t* h)
    {
        flip(h);
        if (isRed(h->m_right->m_left))
        {
            h->m_right = rotateRight(h->m_right);
            h          = rotateLeft(h);
            flip(h);
        }
        return h;
    }

    // REQUIRE: Left and Right children must be present
    inline rbnode_t* moveRedRight(rbnode_t* h)
    {
        flip(h);
        if (isRed(h->m_left->m_left))
        {
            h = rotateRight(h);
            flip(h);
        }
        return h;
    }

    inline rbnode_t* fixUp(rbnode_t* h)
    {
        if (isRed(h->m_right))
        {
            h = rotateLeft(h);
        }
        if (isRed(h->m_left) && isRed(h->m_left->m_left))
        {
            h = rotateRight(h);
        }
        if (isRed(h->m_left) && isRed(h->m_right))
        {
            flip(h);
        }
        return h;
    }
    inline rbnode_t* walkDownRot23(rbnode_t* h) { return h; }
    inline rbnode_t* walkUpRot23(rbnode_t* h)
    {
        if (isRed(h->m_right) && !isRed(h->m_left))
        {
            h = rotateLeft(h)
        }
        if (isRed(h->m_left) && isRed(h->m_left->m_left))
        {
            h = rotateRight(h)
        }
        if (isRed(h->m_left) && isRed(h->m_right))
        {
            flip(h)
        }
        return h
    }

    rbnode_t* Find(rbnode_t* root, void* item, s32 (*compare)(void* item1, void* item2))
    {
        rbnode_t* node = root;
        while (node != nullptr)
        {
            const s32 result = compare(item, node->m_item);
            if (result == 0)
                return node;
            node = (result < 0) ? node->m_left : node->m_right;
        }
        return nullptr;
    }

    rbnode_t* Min(rbnode_t* node)
    {
        if (node == nullptr)
            return nullptr;
        while (node->m_left != nullptr)
            node = node->m_left;
        return node;
    }

    rbnode_t* Max(rbnode_t* node)
    {
        if (node == nullptr)
            return nullptr;
        while (node->m_right != nullptr)
            node = node->m_right;
        return node;
    }

    rbnode_t* ReplaceOrInsert(rbtree_t& t, void* item, s32 (*compare)(void* item1, void* item2), void*& out_replaced)
    {
        if (t.m_root == nullptr)
        {
            // create new node and set it as the root
            rbnode_t* new_node = nullptr;
            out_replaced       = nullptr;
            return new_node;
        }

        rbstack_t stack(t.m_root);
        while (!stack.empty())
        {
            rbnode_t* h = stack.peek();
            h           = walkDownRot23(h);

            const s32 c = compare(item, h->m_item);
            if (c < 0)
            {
                if (h->m_left == nullptr)
                {
                    // create new node and set it as the left child
                    out_replaced = nullptr;
                    break;
                }
                stack.push(h->m_left);
            }
            else if (c > 0)
            {
                if (h->m_right == nullptr)
                {
                    // create new node and set it as the right child
                    out_replaced = nullptr;
                    break;
                }
                stack.push(h->m_right);
            }
            else
            {
                out_replaced = h->m_item;
                h->m_item    = item;
                break;
            }
        }

        // rewind the stack
        while (!stack.empty())
        {
            rbnode_t* h = stack.pop();
            h           = walkUpRot23(h);
        }
    }

    rbnode_t* rbDeleteMin(rbnode_t* node, void*& deleted)
    {
        deleted = nullptr;
        if (node == nullptr)
            return nullptr;

        rbstack_t stack;
        rbnode_t* h = node;
        while (true)
        {
            if (h == nullptr)
                break;
            if (h->m_left == nullptr)
            {
                h       = nullptr;
                deleted = h->m_item;
                break;
            }

            if (!isRed(h->m_left) && !isRed(h->m_left->m_left))
            {
                h = moveRedLeft(h);
            }

            stack.push(h);
            h = h->m_left;
        }

        while (!stack.empty())
        {
            rbnode_t* n = stack.pop();
            n->m_left   = h;
            h           = fixUp(n);
        }

        return h;
    }

    void* DeleteMin(rbtree_t& t)
    {
        void* deleted = nullptr;
        t.m_root      = rbDeleteMin(t.m_root, deleted);
        if (t.m_root != nullptr)
            t.m_root->m_black = true;
        if (deleted != nullptr)
            t.m_count--;
        return deleted;
    }

    rbnode_t* rbDeleteMax(rbnode_t* node, void*& deleted)
    {
        deleted = nullptr;
        if (node == nullptr)
            return nullptr;

        rbstack_t stack;
        rbnode_t* h = node;
        while (true)
        {
            if (h == nullptr)
                break;

            if (isRed(h->m_left))
            {
                h = rotateRight(h);
            }
            if (h->m_right == nullptr)
            {
                h       = nullptr;
                deleted = h->m_item;
                break;
            }
            if (!isRed(h->m_right) && !isRed(h->m_right->m_left))
            {
                h = moveRedRight(h);
            }

            stack.push(h);
            h = h->m_right;
        }

        while (!stack.empty())
        {
            rbnode_t* n = stack.pop();
            n->m_right  = h;
            h           = fixUp(n);
        }

        return h;
    }

    void* DeleteMax(rbtree_t& t)
    {
        void* deleted = nullptr;
        t.m_root      = rbDeleteMax(t.m_root, deleted);
        if (t.m_root != nullptr)
            t.m_root->m_black = true;
        if (deleted != nullptr)
            t.m_count--;
        return deleted;
    }

    rbnode_t* rbRemove(rbnode_t* node, bool (*less)(void* item1, void* item2), void* item, void*& deleted)
    {
        deleted = nullptr;

        rbstack_t stack;
        rbnode_t* h = node;
        while (true)
        {
            if (h == nullptr)
            {
                deleted = nullptr;
                break;
            }
            if (less(item, h->m_item))
            {
                if (h->m_left == nullptr)
                {
                    // item not present. Nothing to delete
                    deleted = nullptr;
                    break;
                }
                if (!isRed(h->m_left) && !isRed(h->m_left->m_left))
                {
                    h = moveRedLeft(h);
                }
                stack.push_left(h);
                h = h->m_left;
            }
            else
            {
                if (isRed(h->m_left))
                {
                    h = rotateRight(h);
                }
                // If @item equals @h->m_item and no right children at @h
                if (!less(h->m_item, item) && h->m_right == nullptr)
                {
                    h       = nullptr;
                    deleted = nullptr;
                    break;
                }
                if (h->m_right != nullptr && !isRed(h->m_right) && !isRed(h->m_right->m_left))
                {
                    h = moveRedRight(h);
                }
                // If @item equals @h->m_item, and (from above) 'h->m_right != nullptr'
                if (!less(h->m_item, item))
                {
                    void* subDeleted = nullptr;
                    h->m_right       = rbDeleteMin(h->m_right, subDeleted);
                    if (subDeleted == nullptr)
                    {
                        break;
                    }
                    deleted   = h->m_item;
                    h->m_item = subDeleted;
                    break;
                }
                else
                {
                    // @item is bigger than @h->m_item
                    stack.push_right(h);
                    h = h->m_right;
                }
            }
        }

        while (!stack.empty())
        {
            bool      left;
            rbnode_t* n = stack.pop(left);
            if (left)
                n->m_left = h;
            else
                n->m_right = h;
            n->h = fixUp(h);
        }

        return h;
    }

    void* Remove(rbtree_t& t, void* item, bool (*less)(void* item1, void* item2))
    {
        void* deleted = nullptr;
        t.m_root      = rbRemove(t.m_root, less, item, deleted);
        if (t.m_root != nullptr)
            t.m_root->m_black = true;
        if (deleted != nullptr)
            t.m_count--;
        return deleted;
    }

} // namespace ncore

#endif