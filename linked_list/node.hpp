class Node {
    public:
    // Constructor
    Node (int val, Node*next){
        this->val = val;
        this->next = next;
    }
    
    int val;
    Node *next;
};