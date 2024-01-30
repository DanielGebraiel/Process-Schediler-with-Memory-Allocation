#include <stdio.h>
#include <math.h>

typedef struct treeNode
{
    int pid;
    int size;
    short isAllocated;
    int start;
    int end;
    struct treeNode *left;
    struct treeNode *right;
    struct treeNode *parent;

} treeNode;
treeNode *newTreeNode(int size, int start, int end)
{
    treeNode *temp = (treeNode *)malloc(sizeof(treeNode));
    temp->size = size;
    temp->start = start;
    temp->end = end;
    temp->isAllocated = 0;
    temp->left = NULL;
    temp->right = NULL;
    temp->parent = NULL;
    temp->pid = -1;
    return temp;
}
treeNode *splitNode(treeNode *root)
{
    root->left = newTreeNode(root->size / 2, root->start, root->start + root->size / 2 - 1);
    root->right = newTreeNode(root->size / 2, root->start + root->size / 2, root->end);
    root->left->parent = root;
    root->right->parent = root;
    return root->left;
}
treeNode *searchforNode(int size, treeNode *current)
{
    // search for best fit for given size if not found return null
    if (current->left == NULL && current->right == NULL && current->size == size && !current->isAllocated)
    {
        return current;
    }
    else if (current->left == NULL && current->right == NULL && current->size == size && current->isAllocated)
    {
        return NULL;
    }
    else if (current->left == NULL && current->right == NULL && current->size != size)
    {
        return NULL;
    }
    else
    {
        treeNode *foundLeft = searchforNode(size, current->left);
        if (foundLeft != NULL)
            return foundLeft;
        return searchforNode(size, current->right);
    }
}
void printTree(treeNode *root)
{
    if (root == NULL)
    {
        return;
    }
    printTree(root->left);
    printf("size: %d, start: %d, end: %d, pid: %d, isAllocated: %d\n", root->size, root->start, root->end, root->pid, root->isAllocated);
    printTree(root->right);
}
treeNode *allocateProcess(treeNode *root, int pid, int size)
{

    printf("size: %d\n", size);
    int temp = size;
    int sizeToSearchFor = 1;
    int power = 0;
    while (temp > pow(2, power))
    {
        power++;
    }

    sizeToSearchFor = pow(2, power);
    int foundSize = sizeToSearchFor;
    treeNode *foundNode = searchforNode(foundSize, root);
    while (foundNode == NULL && foundSize < 1024)
    {
        foundSize *= 2;
        foundNode = searchforNode(foundSize, root);
    }
    if (foundNode == NULL)
    {
        return NULL;
    }

    if (foundNode->size == 1024 && foundNode->left != NULL && foundNode->right != NULL)
    {
        return NULL;
    }
    while (foundSize / 2 > size)
    {
        foundNode = splitNode(foundNode);
        foundSize /= 2;
    }
    
    if (foundSize / 2 == size)
    {
        foundNode = splitNode(foundNode);
        foundSize /= 2;
    }

    foundNode->isAllocated = 1;
    foundNode->pid = pid;
    return foundNode;
}

treeNode *searchForProcess(int pid, treeNode *current)
{
    if (current->left == NULL && current->right == NULL && current->pid == pid)
    {
        return current;
    }
    else if (current->left == NULL && current->right == NULL && current->pid != pid)
    {
        return NULL;
    }
    else
    {
        treeNode *foundLeft = searchForProcess(pid, current->left);
        if (foundLeft != NULL)
            return foundLeft;
        return searchForProcess(pid, current->right);
    }
}
treeNode *merge(treeNode *root)
{
    while (root != NULL)
    {
        if (root->left->left == NULL && root->left->right == NULL && root->right->left == NULL && root->right->right == NULL && !root->left->isAllocated && !root->right->isAllocated)
        {
            free(root->left);
            free(root->right);
            root->left = NULL;
            root->right = NULL;
            root = root->parent;
        }
        else
        {
            return root;
        }
    }
}
void deallocateProcess(treeNode *current, int pid)
{
    treeNode *found = searchForProcess(pid, current);
    if (found == NULL)
    {
        return;
    }
    found->isAllocated = 0;
    found->pid = -1;
    merge(found->parent);
}
