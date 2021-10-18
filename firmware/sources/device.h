#ifndef __DEVICE_H
#define __DEVICE_H

// Peripherals in use:
//
// Timer 2: Buzzer sound generation
// Timer 3: Software UART
// Timer 4: Timing functions

#define CFG_PORT GPIOB
#define CFG_PIN GPIO_PIN_3

#define GLED_PORT GPIOC
#define GLED_PIN GPIO_PIN_4
#define BLED_PORT GPIOC
#define BLED_PIN GPIO_PIN_3
#define RLED_PORT GPIOC
#define RLED_PIN GPIO_PIN_2
#define BUZ_PORT GPIOC
#define BUZ_PIN GPIO_PIN_1

#define GLEDI_PORT GPIOB
#define GLEDI_PIN GPIO_PIN_0
#define RLEDI_PORT GPIOB
#define RLEDI_PIN GPIO_PIN_1
#define BUZI_PORT GPIOB
#define BUZI_PIN GPIO_PIN_2

#define RDM_TXPORT GPIOC
#define RDM_TXPIN GPIO_PIN_6
#define RDM_RXPORT GPIOC
#define RDM_RXPIN GPIO_PIN_7
#define RDM_RXPORT_EXTI EXTI_PORT_GPIOC
#define RDM_RXPORT_IRQ ITC_IRQ_PORTC
#define RDM_BAUD 1667

// Note that DATA0 is inverted!
#define DATA0_PORT GPIOD
#define DATA0_PIN GPIO_PIN_5
#define DATA1_PORT GPIOD
#define DATA1_PIN GPIO_PIN_4

#endif /* __DEVICE_H */
