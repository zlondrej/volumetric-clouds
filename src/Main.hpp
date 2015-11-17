#pragma once

class Main {

  private:
    int argc;
    char **argv;

  public:
    Main(int argc, char **argv);
    void run();
};

int main(int argc, char **argv);
