#include <iostream>
using namespace std;
 
class TestLib
{
    public:
        void display();
        void display(int a);
    private:
        int i;
};
void TestLib::display() {
    cout<<"First display"<<endl;
    i = 100;
}
 
void TestLib::display(int a) {
    cout<<"Second display:"<<a<<endl;
    cout<<"i:"<<i<<endl;
}
extern "C" {
    TestLib obj;
    void display() {
        obj.display();
      }
    void display_int() {
        obj.display(2);
      }
}
