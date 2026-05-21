#include <iostream>
#include <algorithm>

using namespace std;

// =================================================================
// 1. NODE STRUCTURE & UTILITIES
// =================================================================

struct Node {
    Node *ch[2], *par;
    bool rev; // Lazy tag for path/subtree reversal

    Node() : ch{}, par(nullptr), rev(false) {}

    // True if x is a Splay Tree root (connected via a dashed parent link)
    bool is_root() {
        return !par || (par->ch[0] != this && par->ch[1] != this);
    }
};

// Push down lazy path-reversal tags to children
void push(Node* x) {
    if (!x || !x->rev) return;
    swap(x->ch[0], x->ch[1]);
    if (x->ch[0]) x->ch[0]->rev ^= 1;
    if (x->ch[1]) x->ch[1]->rev ^= 1;
    x->rev = false;
}

// Recursively clear tags from the Splay root down to node x
void push_all(Node* x) {
    if (!x->is_root()) push_all(x->par);
    push(x);
}

// =================================================================
// 2. SPLAY TREE OPERATIONS (ROTATIONS & SPLAY)
// =================================================================

// Standard BST tree rotation subroutine
void rotate(Node* x) {
    Node* p = x->par;
    Node* g = p->par;
    int s = (p->ch[1] == x); // 0: Left child, 1: Right child

    p->ch[s] = x->ch[s^1];
    if (x->ch[s^1]) x->ch[s^1]->par = p;

    x->ch[s^1] = p;
    p->par = x;
    x->par = g;

    if (g) {
        if (g->ch[0] == p) g->ch[0] = x;
        if (g->ch[1] == p) g->ch[1] = x;
    }
}

// Moves x to the root of its localized Splay Tree
void splay(Node* x) {
    push_all(x);
    while (!x->is_root()) {
        Node* p = x->par;
        if (!p->is_root()) {
            // Evaluates Zig-Zig (collinear) vs Zig-Zag (asymmetric)
            bool zigzig = (p->par->ch[0] == p) == (p->ch[0] == x);
            rotate(zigzig ? p : x);
        }
        rotate(x); // Handles the final Zig step
    }
}

// =================================================================
// 3. CORE LINK-CUT TREE INTERNAL OPERATIONS
// =================================================================

// Restructures forest so the path from absolute root to x is preferred
void access(Node* x) {
    Node* last = nullptr;
    for (Node* y = x; y; y = y->par) {
        splay(y);
        y->ch[1] = last; // Hook up new preferred right child
        last = y;
    }
    splay(x);
}

// Re-orients tree hierarchy so x becomes the absolute forest root
void make_root(Node* x) {
    access(x);
    x->rev ^= 1; // Inverts deep-to-shallow path order via lazy tag
}

// Finds the representative root of the component containing x
Node* get_root(Node* x) {
    access(x);
    while (x->ch[0]) {
        push(x);
        x = x->ch[0]; // True root is the shallowest/leftmost node
    }
    splay(x); // Preserves O(log n) amortized complexity bound
    return x;
}

// =================================================================
// 4. TOP-LEVEL USER COMPONENT APIs
// =================================================================

// Evaluates whether u and v are part of the same tree component
bool connected(Node* u, Node* v) {
    return get_root(u) == get_root(v);
}

// Links node u to node v via a dashed path-parent pointer
void link(Node* u, Node* v) {
    make_root(u);
    if (get_root(v) != u) {
        u->par = v;
    }
}

// Severs the existing dynamic edge between u and v
void cut(Node* u, Node* v) {
    make_root(u);
    access(v);
    // Verifies structural adjacency before clearing pointers
    if (v->ch[0] == u && !u->ch[1]) {
        v->ch[0] = nullptr;
        u->par   = nullptr;
    }
}

// =================================================================
// 5. DEMO & VERIFICATION TESTING
// =================================================================

int main() {
    // Instantiate 7 nodes (Indices 1 to 6 used)
    Node nd[7];

    // Construct a linear chain and branch layout:
    // 1 — 2 — 3 — 4
    //         |
    //         5 — 6
    link(&nd[1], &nd[2]);
    link(&nd[2], &nd[3]);
    link(&nd[3], &nd[4]);
    link(&nd[3], &nd[5]);
    link(&nd[5], &nd[6]);

    cout << boolalpha; // Prints true/false instead of 1/0

    // Test 1: Verify connectivity through components
    cout << "Initial connection state:\n";
    cout << "Is 1 connected to 6? " << connected(&nd[1], &nd[6]) << "\n"; // true
    cout << "Is 1 connected to 4? " << connected(&nd[1], &nd[4]) << "\n\n"; // true

    // Test 2: Cut an edge and verify graph partitioning
    cut(&nd[3], &nd[5]);
    cout << "After executing cut(3, 5):\n";
    cout << "Is 1 connected to 6? " << connected(&nd[1], &nd[6]) << "\n"; // false
    cout << "Is 1 connected to 4? " << connected(&nd[1], &nd[4]) << "\n"; // true

    return 0;
}
