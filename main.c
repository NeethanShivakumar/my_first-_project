/***************************************************************************//**
 * @file                    main.c
 * @author                  Neethan Shivakumar
 * @date                    07-01-2023
 * @matriculation number    5367019
 * @e-mail contact          neethanskumar@gmail.com
 *
 * @brief   Exercise 6 - Dynamic Circuit Configuration

 *
 * Description: in this exercise relay is used to implement the two functionalities of piezo element (Buzzer) that is to play the melody and detect knock.
 *  knocks are detected using interrupt mechanism. Melody 1 is played for single knock. melody 2 is played for 2 knocks.

 *
 * Pin connections:
 *   BUZZER (X1)   -  the single pole of the relay (middle pin of CON4)
 *   CON3:P3.6     -  left output of relay in CON4
 *   DAC_IN (X2)   -  Right output of relay in CON4
 *   CON3:P1.3     -  COMP_OUT (X7)
 *   CON3:P1.4     -  REL_STAT (X5)
 *   Set jumper JP5 to VFO

 *
 * Theory answers: all answered in the text file (Task-1b) attached
 *
 * Tasks completed: TASK 1 - A,B and Task 2.
 *
 * @note    The project was exported using CCS  Version: 12.1.0.00007
 ******************************************************************************/
#include <templateEMP.h>
#include <msp430.h> 
volatile int knock_flag = 0;

int data1[6] = { 440, 300, 300, 440, 300, 300 };
int data2[6] = { 880, 880, 880, 698, 1000, 880 };

void prepare_to_play_melody()
{
    P1OUT |= BIT4;       // connect the buzzer to your audio output pin (Set to PWM mode by setting P1.4 to 1)

    P1IE &= ~BIT3;      // Disable interrupt as interrupt should not be detected while playing melody

    P3REN &= ~BIT6;     //disable pull-up

    P3DIR |= BIT6; // P3 .6 output
    P3SEL |= BIT6; // P3 .6 TA0 .2 option
}

void prepare_to_detect_knock()
{
    P3SEL &= ~BIT6;
    P3DIR &= ~BIT6;                        // P3.6 is made as input
    P3REN |= BIT6;                         //Enable Pull resistor
    P3OUT &= ~BIT6;                        //Set to Pull-down for P3.6 (This id done to remove the accumulated charges )

    knock_flag = 0;
    P1OUT &= ~BIT4;         // Cleared to detect the Knock
    P1IFG &= ~ BIT3;        // Clear interrupt flag
    P1IE |= BIT3;           // Enable interrupt
    if (knock_flag > 0)
    {
        knock_flag = 0;
    }
}

void stop_melody()
{
    TA0CCTL2 = OUTMOD_3; // CCR2 set / reset
    TA0CCR0 = 0; // PWM Period : 1000 us
    TA0CCR2 = 0; // CCR2 PWM duty cycle (50 %)
    TA0CTL = TASSEL_2 + MC_1; // SMCLK ; MC_1 -> up mode ;
}

void playNote(int i)
{
    int x = (1000000 / i);
    int y = x / 2;
    TA0CCTL2 = OUTMOD_3; // CCR2 set / reset
    TA0CCR0 = x; // PWM Period : 1000 us
    TA0CCR2 = y; // CCR2 PWM duty cycle (50 %)
    TA0CTL = TASSEL_2 + MC_1; // SMCLK ; MC_1 -> up mode ;
}

void play_melody1()
{

    int counter, j;
    for (j = 0; j < 4; j++)
    {

        for (counter = 0; counter < 6; counter++)
        {

            playNote(data1[counter]);
            _delay_cycles(250000);

        }

    }
}

void play_melody2()
{

    int counter, j;
    for (j = 0; j < 4; j++)
    {

        for (counter = 0; counter < 6; counter++)
        {

            playNote(data2[counter]);
            _delay_cycles(250000);

        }

    }
}

void main(void)
{
    initMSP();
    P1SEL &= ~(BIT3 | BIT4);
    P1SEL2 &= ~(BIT3 | BIT4);

    P1DIR &= ~BIT3;
    P1REN |= BIT3;        // Enable pull - resistors
    P1OUT &= ~BIT3;        // Set to pull - down

    P1DIR |= BIT4;        // P1.4 is made as output pin to control the relay
    P1OUT &= ~BIT4;       // pin made zero to detect the  Knock (initially)

    P3DIR |= BIT6;        // P3 .6 output
    P3SEL |= BIT6;        // P3 .6 TA0 .2 option

    P1IES &= ~BIT3;       // Setting for Low to High Edge
    P1IFG &= ~(BIT3);     // Clearing the interrupt flag
    P1IE |= BIT3;         // Enabling the interrupt

    TA0CCTL2 = OUTMOD_3;       // CCR2 set / reset
    TA0CCR0 = 0;               // PWM Period : 1000 us
    TA0CCR2 = 0;               // CCR2 PWM duty cycle (50 %)
    TA0CTL = TASSEL_2 + MC_1;  // SMCLK ; MC_1 -> up mode ;


    // Main part of the program: We do the following stuff until the power is cut.
    while (1)
    {

        if (knock_flag)
        {
            __delay_cycles(500000);   //delay is added to detect if there is another knock after the first knock
            if (knock_flag == 1)
            {
                serialPrintln("number of knock=");
                serialPrintInt(knock_flag);

                prepare_to_play_melody();

                play_melody1();
                stop_melody();

                prepare_to_detect_knock();

            }
            if (knock_flag == 2)
            {
                serialPrintln("number of knock=");
                serialPrintInt(knock_flag);

                prepare_to_play_melody();

                play_melody2();
                stop_melody();

                prepare_to_detect_knock();

            }
            if (knock_flag > 0)
            {
                knock_flag = 0;
            }

        }

    }

}

# pragma vector = PORT1_VECTOR
__interrupt void Port_1(void)
{
    if ((P1IFG & BIT3) == BIT3)
    {

        knock_flag = knock_flag + 1;


            serialPrintln("knock detected");

            __delay_cycles(100000);


    }

    P1IFG &= ~ BIT3;
}
