#include <iostream>
#include <algorithm>

using namespace std;

// =================================================================
// 1. NODE STRUCTURE & BASIC CHECKERS
// =================================================================

// Represents a single node in the Link-Cut Tree forest
struct Node {

    // Child pointers: ch[0] is Left child, ch[1] is Right child
    Node *ch[2];

    // Parent pointer (can point to a splay parent or a path-parent)
    Node *par;

    // Lazy propagation flag for path/subtree reversal
    bool rev;

    // Constructor initializes pointers to null and flags to false
    Node() : ch{}, par(nullptr), rev(false) {}

    // Checks if this node is the root of its localized Splay Tree.
    // An LCT node is considered a Splay root if it has no parent,
    // OR if its parent's child pointers do not point back to it
    // (which means the edge connecting them is a dashed/path-parent edge).
    bool is_root() {
        return !par || (par->ch[0] != this && par->ch[1] != this);
    }
};

// Pushes down the lazy path-reversal tag to the node's immediate children
void push(Node* x) {
    if (!x || !x->rev) return;

    // 1. Swap the left and right subtrees physically
    swap(x->ch[0], x->ch[1]);

    // 2. Pass the reversal flag down to existing children
    if (x->ch[0]) x->ch[0]->rev ^= 1;
    if (x->ch[1]) x->ch[1]->rev ^= 1;

    // 3. Clear the lazy tag on the current node
    x->rev = false;
}

// Recursively climbs up and clears all lazy tags from the Splay root down to x
void push_all(Node* x) {
    if (!x->is_root()) {
        push_all(x->par);
    }
    push(x);
}

// =================================================================
// 2. SPLAY TREE ROTATIONS & COMPLEXITY BALANCING
// =================================================================

// Standard Binary Search Tree rotation subroutine
void rotate(Node* x) {
    Node* p = x->par;
    Node* g = p->par;

    // s determines child orientation: 0 for Left child, 1 for Right child
    int s = (p->ch[1] == x);

    // Step 1: Move x's inner child over to p's side
    p->ch[s] = x->ch[s^1];
    if (x->ch[s^1]) {
        x->ch[s^1]->par = p;
    }

    // Step 2: Make p a child of x
    x->ch[s^1] = p;
    p->par = x;

    // Step 3: Link x up to the grandparent g (retaining dashed/solid state)
    x->par = g;
    if (g) {
        if (g->ch[0] == p) g->ch[0] = x;
        if (g->ch[1] == p) g->ch[1] = x;
    }
}

// Brings node x up to the root of its individual Auxiliary Splay Tree
void splay(Node* x) {

    // Ensure all lazy tags above x are cleared before reshaping the tree
    push_all(x);

    // Loop until x becomes the absolute root of its local Splay Tree
    while (!x->is_root()) {
        Node* p = x->par;

        // ---------------------------------------------------------
        // CASE 1: ZIG STEP
        // ---------------------------------------------------------
        // If the parent p is the splay root, we execute a single rotation
        if (p->is_root()) {
            rotate(x);
        }
        else {
            // Determine structural alignment relative to grandparent
            bool p_is_left  = (p->par->ch[0] == p);
            bool x_is_left  = (p->ch[0] == x);
            bool is_collinear = (p_is_left == x_is_left);

            // -----------------------------------------------------
            // CASE 2: ZIG-ZIG STEP (Collinear Configuration)
            // -----------------------------------------------------
            // Parent is rotated first, followed by x to flatten deep chains
            if (is_collinear) {
                rotate(p);
                rotate(x);
            }
            // -----------------------------------------------------
            // CASE 3: ZIG-ZAG STEP (Asymmetric Configuration)
            // -----------------------------------------------------
            // Node x is rotated twice consecutively to climb straight up
            else {
                rotate(x);
                rotate(x);
            }
        }
    }
}

// =================================================================
// 3. CORE LINK-CUT TREE INTERNAL SUBROUTINES
// =================================================================

// Constructs a preferred path spanning from the absolute forest root down to x
void access(Node* x) {
    Node* last = nullptr;

    // Climb up through path-parent pointers to reach the tree's true root
    for (Node* y = x; y; y = y->par) {

        // Splay y to isolate it at the top of its current Auxiliary Tree
        splay(y);

        // Sever y's right child (converting that solid link to a dashed link)
        // Then, hook up 'last' as the new preferred right child
        y->ch[1] = last;

        // Keep track of the node we just modified as we ascend
        last = y;
    }

    // Splay x to ensure it sits cleanly at the root of this newly built path
    splay(x);
}

// Redirects the tree hierarchy so that x becomes the top root of the entire component
void make_root(Node* x) {

    // 1. Build a root-to-x path (x becomes the lowest/deepest node on it)
    access(x);

    // 2. Apply a lazy reverse tag to invert the deep-to-shallow node hierarchy
    x->rev ^= 1;
}

// Returns the true representative root node of the component containing x
Node* get_root(Node* x) {

    // Expose the solid path from the root down to x
    access(x);

    // The true root node is always the shallowest node in this Splay Tree (leftmost)
    while (x->ch[0]) {
        push(x);
        x = x->ch[0];
    }

    // Splay the root node to preserve the O(log n) amortized complexity bound
    splay(x);
    return x;
}

// =================================================================
// 4. TOP-LEVEL USER COMPONENT INTERFACES (APIs)
// =================================================================

// Evaluates whether nodes u and v reside within the same tree component
bool connected(Node* u, Node* v) {
    return get_root(u) == get_root(v);
}

// Establishes a brand new structural edge/link joining u to v
void link(Node* u, Node* v) {

    // Make u the absolute root of its own isolated tree component
    make_root(u);

    // Attach u to v using a dashed path-parent pointer link
    if (get_root(v) != u) {
        u->par = v;
    }
}

// Breaks the existing structural link between nodes u and v
void cut(Node* u, Node* v) {

    // 1. Orient u as the root of the component
    make_root(u);

    // 2. Expose the structural path up to v
    access(v);

    // 3. Confirm that a direct structural edge exists between u and v
    // If it exists, u must be the immediate left child of v in the Splay tree
    if (v->ch[0] == u && !u->ch[1]) {

        // Sever the physical pointer connections entirely
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

    cout << boolalpha; // Configures terminal output to show true/false strings

    // Test 1: Verify initial tree connectivity
    cout << "--- INITIAL FOREST CONNECTIONS ---\n";
    cout << "Is node 1 connected to node 6? " << connected(&nd[1], &nd[6]) << "\n"; // true
    cout << "Is node 1 connected to node 4? " << connected(&nd[1], &nd[4]) << "\n\n"; // true

    // Test 2: Cut an edge and verify component partitioning
    cut(&nd[3], &nd[5]);
    cout << "--- AFTER EXECUTING cut(3, 5) ---\n";
    cout << "Is node 1 connected to node 6? " << connected(&nd[1], &nd[6]) << "\n"; // false
    cout << "Is node 1 connected to node 4? " << connected(&nd[1], &nd[4]) << "\n"; // true

    return 0;
}
