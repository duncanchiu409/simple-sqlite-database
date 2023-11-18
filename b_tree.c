#include <stdio.h>
#include <stdlib.h>

#define MAX 3 // Maximum of child
#define MIN 2 // Minimum of child

struct BTreeNode{
    int item[MAX + 1];
    int count;
    struct BTreeNode *link[MAX + 1];
};

struct BTreeNode * root;

struct BTreeNode* createNode(int item, struct BTreeNode *child){
    struct BTreeNode* newNode;
    newNode = (struct BTreeNode*) malloc(sizeof(struct BTreeNode));
    newNode->item[1] = item;
    newNode->count = 1;
    newNode->link[0] = root;
    newNode->link[1] = child; // insert NULL
    return newNode;
}

void addValToNode(int item, int pos, struct BTreeNode* node, struct BTreeNode* child){
    // pos is the xth slot of the child
    int j = node->count;
    while(j > pos){
        node->item[j + 1] = node->item[j];
        node->link[j + 1] = node->link[j];
        j--;
    }
    node->item[j + 1] = item;
    node->link[j + 1] = child;
    node->count++;
}

void splitNode(int item, int *pval, int pos, struct BTreeNode* node, struct BTreeNode* child, struct BTreeNode** newNode){
    int median;
    int j;

    if(pos > MIN){
        median = MIN + 1;
    }
    else{
        median = MIN;
    }

    // This is the new right child node
    *newNode = (struct BTreeNode*)malloc(sizeof(struct BTreeNode));

    j = median + 1;
    while(j <= MAX){
        (*newNode)->item[j - median] = node->item[j];
        (*newNode)->link[j - median] = node->link[j];
        j++;
    }

    node->count = median;
    (*newNode)->count = MAX - median;

    if(pos <= MIN){
        addValToNode(item, pos, node, child);
    }
    else{
        addValToNode(item, pos - median, *newNode, child);
    }

    *pval = node->item[node->count];
    (*newNode)->link[0] = node->link[node->count];
    node->count--;
}

int setValueInNode(int item, int *pval, struct BTreeNode* node, struct BTreeNode** child){
    int pos;
    if(!node){
        *pval = item;
        *child = NULL;
        return 1;
    }

    if(item < node->item[1]){
        pos = 0;
    }
    else{
        for(pos = node->count; item < node->item[pos] && pos > 1; pos--){
            if(item == node->item[pos]){
                printf("Duplicates not allowed\n");
                return 0;
            }
        }
    }

    if(setValueInNode(item, pval, node->link[pos], child)){
        if(node->count < MAX){
            addValToNode(*pval, pos, node, *child);
        }
        else{
            splitNode(*pval, pval, pos, node, *child, child);
            return 1;
        }
    }
    return 0;
}

void insertion(int item){
    int flag, i;
    struct BTreeNode *child;
    
    flag = setValueInNode(item, &i, root, &child);
    if(flag){
        root = createNode(i, child);
    }
}

// What is this for?
void copySuccessor(struct BTreeNode* myNode, int pos){
    struct BTreeNode* dummy;
    dummy = myNode->link[pos];

    for(; dummy->link[0] != NULL;){
        dummy = dummy->link[0];
    }
    myNode->item[pos] = dummy->item[1];
}

void removeVal(struct BTreeNode* myNode, int pos){
    int i = pos + 1;
    while(i <= myNode->count){
        myNode->item[i - 1] = myNode->item[i];
        myNode->link[i - 1] = myNode->link[i];
    }
    myNode->count--;
}

void rightShift(struct BTreeNode *myNode, int pos){
    struct BTreeNode* x = myNode->link[pos];
    int j = x->count;

    while(j > 0){
        x->item[j + 1] = x->item[j];
        x->link[j + 1] = x->link[j];
        j--;
    }

    x->item[1] = myNode->item[j];
    x->link[1] = x->link[0];
    x->count++;

    x = myNode->link[pos - 1];
    myNode->item[pos] = x->item[x->count];
    myNode->link[pos] = x->link[x->count];
    x->count--;
    return;
}

void leftShift(struct BTreeNode *myNode, int pos){
    struct BTreeNode* x = myNode->link[pos - 1];
    
    x->count++;
    x->item[x->count] = myNode->item[pos];
    x->link[x->count] = myNode->link[pos]->link[0];

    x = myNode->link[pos];
    myNode->item[pos] = x->item[1];
    x->link[0] = x->link[1];
    x->count--;

    int j = 1;
    while(j <= x->count){
        x->item[j] = x->item[j + 1];
        x->link[j] = x->link[j + 1];
        j++;
    }
    return;
}

void mergeNodes(struct BTreeNode* myNode, int pos){
    int j = 1;
    struct BTreeNode* x1 = myNode->link[pos];
    struct BTreeNode* x2 = myNode->link[pos - 1];

    x2->count++;
    x2->item[x2->count] = myNode->item[pos];
    x2->link[x2->count] = myNode->link[0];

    while(j <= x1->count){
        x2->count++;
        x2->item[x2->count] = x1->item[j];
        j++;
    }

    j = pos;
    while (j < myNode->count)
    {
        myNode->item[j] = myNode->item[j + 1];
        myNode->link[j] = myNode->link[j + 1];
        j++;
    }
    myNode->count--;
    free(x1);
}

void adjustNode(struct BTreeNode *myNode, int pos){
    if(!pos){ // when there is empty nodes?
        if(myNode->link[1]->count > MIN){
            leftShift(myNode, 1);
        }
        else{
            mergeNodes(myNode, 1);
        }
    }
    else{
        if(myNode->count != pos){ // no the last one
            if(myNode->link[pos - 1]->count > MIN){ // Left child node have more
                rightShift(myNode, pos);
            }
            else{
                if(myNode->link[pos + 1]->count > MIN){
                    leftShift(myNode, pos + 1);
                }
                else{
                    mergeNodes(myNode, pos);
                }
            }
        }
        else{
            if(myNode->link[pos - 1]->count > MIN){
                rightShift(myNode, pos);
            }
            else{
                mergeNodes(myNode, pos);
            }
        }
    }
}

int delValFromNode(int item, struct BTreeNode* myNode){
    int pos = 0;
    int flag = 0;

    if(myNode){
        if(item < myNode->item[1]){
            pos = 0;
            flag = 0;
        }
        else{
            for(pos = myNode->count; (item < myNode->item[pos] && pos > 1); pos--){
                if(item == myNode->item[pos]){
                    flag = 1;
                }
                else{
                    flag = 0;
                }
            }
        }
        if(flag){
            if(myNode->link[pos - 1]){ // determine leaf node or not
                copySuccessor(myNode, pos);
                flag = delValFromNode(myNode->item[pos], myNode->link[pos]);
                if(flag == 0){
                    printf("Given data is not present in B-Tree\n");
                }
            }
            else{
                removeVal(myNode, pos);
            }
        }
        else{
            flag = delValFromNode(item, myNode->link[pos]);
        }

        if(myNode->link[pos]){
            if(myNode->link[pos]->count < MIN){
                adjustNode(myNode, pos);
            }
        }
    }
    return flag;
}

void delete(int item, struct BTreeNode* myNode){
    struct BTreeNode* tmp;
    if(!delValFromNode(item, myNode)){
        print("Not present\n");
        return;
    }
    else{
        if(myNode->count == 0){
            tmp = myNode;
            myNode = myNode->link[0];
            free(tmp);
        }
    }
    root = myNode;
    return;
}

void print_tree(struct BTreeNode* x, int l){
  printf("Level %d %d :", l, x->count);
  int i = 1;
  while(i <= x->count){
    printf("%d ", x->item[i]);
    i++;
  }
  printf("\n");
  l+=1;
  int k = 0;
  while(k <= MAX && x->link[k] != NULL){
    print_tree(x->link[k], l);
    k++;
  }
}

int main(){
    insertion(1);
    insertion(2);
    insertion(3);
    insertion(4);

    print_tree(root, 0);
    return 0;
}