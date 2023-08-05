#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define MIN 2 // t = minimum degree of the b tree
const int MAX = MIN * 2 - 1;

// Both BTreeNode && keys are MAX + 1 because it require 
typedef struct{
    struct BTreeNode* children[MAX + 1];
    int keys[MAX + 1];
    int count; // Current number of node
} BTreeNode;

struct BTreeNode* root;

BTreeNode* create_b_tree_node(int item, BTreeNode* child){
    BTreeNode* new_node = (BTreeNode*)malloc(sizeof(BTreeNode));
    if(new_node == -1){
        printf("Create new B Tree node has failed.\n");
        exit(EXIT_FAILURE);
    }

    new_node->keys[1] = item;
    new_node->children[0] = root;
    new_node->count = 1;
    new_node->children[1] = child;

    return new_node;
}

void insertNode(int item, int pos, BTreeNode* node, BTreeNode* child){
    int i = node->count;
    while(i > pos){
        node->keys[i+1] = node->keys[i];
        node->children[i+1] = node->children[i];
    }
    node->keys[i+1] = item;
    node->children[i+1] = child;
    node->count++;
}

void splitNode(int value, int* i, int pos, BTreeNode* node, BTreeNode** child, BTreeNode** new_node){
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
    *new_node->count = MAX - median;

    
}

int setNodeValue(int item, int* i, BTreeNode* node, BTreeNode** child){
    int pos;
    if(!node){
        *i = item;
        *child = NULL;
        return 1;
    }

    // Pick the right slot
    if(item < node->children[1]){
        pos = 0;
    }
    else{
        for(pos = node->count; pos > 1 && item < node->children[pos]; pos--){
            if(item == node->children[pos]){
                printf("Duplicate are permitted\n");
                return 0;
            }
        }
    }

    // Node operation
    if(setNodeValue(item, &i, node->children[pos], child)){
        if(node->count < MAX){
            insertNode(*i, pos, node, *child);
        }
        else{
            spliteNode();
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

void print_b_tree_node(BTreeNode* tree_node, int level){
    if(tree_node->leaf == true){
        printf("Level : %d, Keys: ", level);
        for(int i = 0; i < tree_node->n; i++){
            printf("%d", tree_node->keys[i]);
        }
    }
}

void print_b_tree(BTree* tree){
    if(tree->root != NULL){
        print_tree_node(tree->root, 0);
    }
}

int main(int argc, char* arg[]){
    BTree* tree = new_b_tree(MIN);
    BTreeNode* tree_node = new_b_tree_node(MIN, false);
    return 0;
}
