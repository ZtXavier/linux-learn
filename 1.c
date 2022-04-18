typedef struct 
{
    int value;
    struct Task_struct *head;
}S;


void wait(S s)
{
    s.value--;
    if(s.value < 0)
    {
        block(s.head);
    }
}

void wait(S s)
{
    s.value++;
    if(s.value <= 0)
    {
        weakup(s.head);
    }
}


