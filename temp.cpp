#include <iostream>

class BaseClass {
  public:
    virtual void doSomething() = 0;
};

class SubClass : public BaseClass {
  public:
    virtual void doSomething() override = 0;
};

class GrandChild : public SubClass {
  public:
    virtual void doSomething(int tmp) override {
      std::cout << "Hello Friend." << std::endl;
    }
};

int main() {
  GrandChild test;
  int i = 3;
  test.doSomething(i);

  return 0;
}
