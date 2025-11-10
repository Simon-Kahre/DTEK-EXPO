/* main.c

   This file written 2024 by Artur Podobas, Pedro Antunes, Simon Kåhre and Leo Ehrenberg

   For copyright and licensing, see file COPYING */


/* Below functions are external and found in other files. */
extern void print(const char*);
extern void print_dec(unsigned int);
extern void display_string(char*);
extern void time2string(char*,int);
extern void tick(int*);
extern void delay(int);
extern int nextprime( int );
extern void enable_interrupt();
void set_displays(int,int);

int prime = 1234567;

int mytime = 0x5957;
char textstring[] = "text, more text, and even more text!";
int seconds = 0;
int timeoutcount = 0;
int hours = 0;
int swStatus = 0;


// Aquires status of all the switches on the board
int get_sw()
{
  int *volatile ptr;
  ptr = (int*) 0x04000010; // Sets address for the pointer
  int activeSwitches = *ptr; // Aquires value from address which ptr points to
  activeSwitches = activeSwitches & 1023; // Makes sure only the 10 LSBs are used
  
  return activeSwitches;
}

// Aquires status of button on the board
int get_btn()
{
  int *volatile ptr;
  ptr = (int*) 0x040000d0; // Sets address for the pointer
  int btnValue = *ptr; // Aquires value from address which ptr points to
  btnValue = btnValue & 1; // Makes sure only the 1 LSBs is used
  
  return btnValue;
}

// Sets status of LEDs on the board
void set_leds(int led_mask)
{
  led_mask = led_mask & 1023; // Makes sure only the 10 LSBs are used
  int *volatile ptr;
  ptr = (int*) 0x04000000; // Sets address for the pointer

  *ptr = led_mask; // Sets value in address which ptr points to
}

/* Below is the function that will be called when an interrupt is triggered. */
void handle_interrupt(unsigned cause) 
{
  if(cause== 16)
  {
    int *volatile status;
    status = (int*) 0x04000020;
    *status = 0;
    timeoutcount++;
    if(timeoutcount == 10)
    {
      timeoutcount = 0;

      // Sets each display to show it's corresponding number
      set_displays(0,(mytime & 15));
      set_displays(1,((mytime & 240) >> 4));
      set_displays(2,((mytime & 3840) >> 8));
      set_displays(3,((mytime & 61440) >> 12));
      set_displays(4,(hours & 15));
      set_displays(5,(hours & 240) >> 4);

      tick(&mytime); // Ticks myTime

      mytime = mytime & 0xffff;

      if(mytime == 0x0000) // Checks if 'hours' should increase
      {
        hours += 1; // Increases 'hours'
        if((hours & 15) == 10) // Checks if the last hexadecimal in hours is "a" (aka 10)
        {
          hours -= 10; // Resets first digit of 'hours'
          hours += 16; // Increases second digit of 'hours'
        }

        if(hours > 0x99)
        {
          hours = 0;
        }
      }
    }
  }
}

/* Add your code here for initializing interrupts. */
void labinit(void)
{
  int *volatile periodl;
  int *volatile periodh;

  periodl = (int*) 0x04000028;
  periodh = (int*) 0x0400002C;

  //3 million: 101101 1100011011000000
  *periodl = 50880; //binary 1100011011000000
  *periodh = 45;    //binary 101101

  int *volatile ctrlPtr;
  ctrlPtr = (int*) 0x04000024;

  *ctrlPtr = 0x7;

  enable_interrupt();
}

void set_displays(int display_number, int value)
{
  int *volatile ptr;
  ptr = (int*) (67108944 + display_number * 16);

  switch(value)
  {
    case 0:
      *ptr = 192;
      break;
    case 1:
      *ptr = 249;
      break;
    case 2:
      *ptr = 164;
      break;
    case 3:
      *ptr = 176;
      break;
    case 4:
      *ptr = 153;
      break;
    case 5:
      *ptr = 146;
      break;
    case 6:
      *ptr = 130;
      break;
    case 7:
      *ptr = 248;
      break;
    case 8:
      *ptr = 128;
      break;
    case 9:
      *ptr = 144;
      break;

    default:
      print("Value not found\n");
      break;
  }
}

/* Your code goes into main as well as any needed functions. */
int main() {
  // Call labinit()
  labinit();

  while (1) 
  {
    print("Prime: ");
    prime = nextprime( prime );
    print_dec( prime );
    print("\n");
  }  
}


