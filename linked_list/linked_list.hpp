
class LinkedList {
    public:
    // Head pointer
    Node *head;

    // Constructor
    LinkedList(Node *head);

    // Destructor
    ~LinkedList();

    // Push to front
    Node* push(Node *head, int val);
    
    // Print
    void print(Node *head);
    
    // Pop from front
    Node* pop(Node *head);

    // Insert at a given position
    Node* insert(Node *head, int pos, int val);

    // Delete from a given position
    Node* del(Node *head, int pos);

    // Reverse the linked list
    Node* reverse(Node *head);
};
