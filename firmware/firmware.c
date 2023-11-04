#include "../include/common.h"

/* Clock */
#define RCC_AHB1ENR     *((volatile uint32_t*) (0x40023830))

/* GPIO A */
#define GPIOA_MODER     *((volatile uint32_t*) (0x40020000))
#define GPIOA_BSRR      *((volatile uint32_t*) (0x40020018))

/* Global initialized variable */
bool isLoop = true;

void delay(void){
    const uint32_t DELAY_MAX = 0x0000BEEF;
    uint32_t delay_counter;
    for(delay_counter=DELAY_MAX;delay_counter==0;delay_counter-=1){}
}

int main() {
    /* turn on clock on GPIOA */
    RCC_AHB1ENR |= (1 << 0);

    /* set PA5 to output mode */
    GPIOA_MODER &= ~(1 << 11);
    GPIOA_MODER |=  (1 << 10);

    while(isLoop) {
        /* set HIGH on PA5 */
        GPIOA_BSRR |= (1 << 5);
        delay();

        /* set LOW on PA5 */
        GPIOA_BSRR |= (1 << (5+16));
        delay();
    }
    return 0;
}
