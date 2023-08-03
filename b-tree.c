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

void insertion(int item){
    
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
