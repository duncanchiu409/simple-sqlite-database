#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define MIN 2 // t = minimum degree of the b tree
enum { MAX = MIN * 2 - 1 };

typedef struct _BTreeNode BTreeNode;

// Both BTreeNode && keys are MAX + 1 because it require 
struct _BTreeNode{
    BTreeNode* children[MAX + 1];
    int keys[MAX + 1];
    int count; // Current number of node
};

BTreeNode* root;

BTreeNode* create_b_tree_node(int value, BTreeNode* child){
    BTreeNode* new_node = (BTreeNode*)malloc(sizeof(BTreeNode));
    if(new_node == NULL){
        printf("Create new B Tree node has failed.\n");
        exit(EXIT_FAILURE);
    }

    new_node->keys[1] = value;
    new_node->count = 1;
    new_node->children[0] = root;
    new_node->children[1] = child;

    return new_node;
}

void insertNode(int value, int pos, BTreeNode* node, BTreeNode* child){
    int i = node->count;
    while(i > pos){
        node->keys[i+1] = node->keys[i];
        node->children[i+1] = node->children[i];
        i--;
    }
    node->keys[i+1] = value;
    node->children[i+1] = child;
    node->count++;
}

void splitNode(int value, int* pos_value, int pos, BTreeNode* node, BTreeNode* child, BTreeNode** new_node){
    int median;
    int j;

    // Find median
    if(pos > MIN){
        median = MIN + 1;
    }
    else{
        median = MIN;
    }

    // Put children into new node
    *new_node = (BTreeNode*)malloc(sizeof(BTreeNode));
    j = median + 1;
    while(j <= MAX){
        (*new_node)->keys[j - median] = node->keys[j];
        (*new_node)->children[j - median] = node->children[j];
        j++;
    }
    node->count = median;
    (*new_node)->count = MAX - median;

    // insert value into Node
    if(pos < MIN){
        insertNode(value, pos, node, child);
    }
    else{
        insertNode(value, pos-median, *new_node, child);
    }
    
    // Moving median key and its children into some where else
    *pos_value = node->keys[node->count];
    (*new_node)->children[0] = node->children[node->count];
    node->count--;
}

int setNodeValue(int item, int* i, BTreeNode* node, BTreeNode** child){
    int pos;
    if(!node){
        *i = item;
        *child = NULL;
        return 1;
    }

    // Pick the right slot
    if(item < node->keys[1]){
        pos = 0;
    }
    else{
        for(pos = node->count; pos > 1 && item < node->keys[pos]; pos--){
            if(item == node->keys[pos]){
                printf("Duplicate are permitted\n");
                return 0;
            }
        }
    }

    // Node operation
    if(setNodeValue(item, i, node->children[pos], child)){
        if(node->count < MAX){
            insertNode(*i, pos, node, *child);
        }
        else{
            splitNode(*i, i, pos, node, *child, child);
            return 1;
        }
    }

    return 0;
}

void insertion(int item){
    int flag;
    int i;
    BTreeNode* child;
    if(setNodeValue(item, &i, root, &child)){
        root = create_b_tree_node(i, child);
    }
}

// Duplication from example
void traversal(BTreeNode* myNode) {
  int i;
  if (myNode) {
    for (i = 0; i < myNode->count; i++) {
      traversal(myNode->children[i]);
      printf("%d ", myNode->keys[i + 1]);
    }
    traversal(myNode->children[i]);
  }
}

int main(int argc, char* arg[]){
    insertion(8);
    insertion(9);
    insertion(10);
    insertion(11);
    insertion(15);
    insertion(16);
    insertion(17);
    insertion(18);
    insertion(20);
    insertion(23);

    traversal(root);
    return 0;
}
