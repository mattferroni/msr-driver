#include <stdio.h>

int main(void)
{
    unsigned eax;
    __asm__ __volatile__("cpuid" : "=a"(eax) : "a"(0x0a));
    printf("Perf Mon Version: %d\n", (0xFF & eax));
    return 0;
}