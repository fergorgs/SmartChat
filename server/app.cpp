#include "Hub.h"

void cancel(int sig) {
    std::cout << "What are you trying to do? Don't do that! >:C" << std::endl;
}

int main() {
    signal(SIGINT, cancel);

    Hub h;
    h.run(6667);
}