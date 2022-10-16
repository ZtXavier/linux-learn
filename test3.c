#include<stdio.h>
#include<stdlib.h>

typedef struct node{
    char next;
    int a;
    int b;
}n;

typedef struct _test{
    char b;
    int a;
    char next;
}t;



int main()
{
    int a = 1;
    if(a == 2) goto ret_r;  
   printf("a\n");
   printf("b\n");
ret_r:
    printf("%d",0x1f);
    return 0;
}

 // 题目要求的节点的规则是不大于子节点的任意一个
class Solution {
public:
    int findSecondMinimumValue(TreeNode* root) {
        int ret = -1;
        int rootval = root->val;
        function<void(TreeNode*)> my_trace = [&](TreeNode *root) {
            if(root == NULL){
                return;
            }
            if(ret != -1 && root->val >= ret){
                return;
            }
            if(root->val > rootval){
                ret = root->val;
            }
            my_trace(root->left);
            my_trace(root->right);
        }
        my_trace(root);
        return ret;
    }
};