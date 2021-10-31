#include <iostream>
#include<stack>

int main()
{
    std::stack<int> s;
    for(int i = 0;i < 5;i++)
    {
        s.push(i);
    }
    for(int i = 0;i <= 5;i++)
    {
        std::cout << s.top() << std::endl;
        s.pop();
    }
    return 0;
}