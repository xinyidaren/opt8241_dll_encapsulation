#include <iostream>
using namespace std;
 
class TestLib
{
    public:
        void display();
        void display(int a);
        void display(char* msg);
        int sum(int x);
        int add(int a,int b);
        int sum_array(int array[], int num);
        void modify_array(int array[], int num);
    private:
};
void TestLib::display()
{
    cout<<"First display"<<endl;
}
 
void TestLib::display(int a)
{
    cout<<"Second display:"<<a<<endl;
}

void TestLib::display(char* msg)
{
    cout<<"Third display:"<<msg<<endl;
}

int TestLib::sum(int x)
{
    int i, result=0;
    for(i=0; i<=x; i++)
    {
        result+=i;
    }
    return result;
};

int TestLib::add(int a,int b)
{
    return a+b;
}

int TestLib::sum_array(int array[], int num)
{
    int i =0, sum = 0;
    for(i=0; i<num; ++i)
        sum += array[i];
    return sum;
}

void TestLib::modify_array(int array[], int num)
{
    int i =0, sum = 0;
    for(i=0; i<num; ++i)
        array[i] *= 10;
}
extern "C"
{
    TestLib obj;
    void display()
    {
        obj.display();
    }
    void display_int()
    {
        obj.display(2);
    }
    void display_char()
    {
        char * c="display_char";
        obj.display(c);
    }
    int sum()
    {
        obj.sum(100);
    }
    int add(int a,int b)
    {
       return obj.add(a,b);
    }
    int sum_array(int array[], int num)
    {
       return obj.sum_array(array,num);
    }
    void modify_array(int array[], int num)
    {
        obj.modify_array(array,num);
    }
}
