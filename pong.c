/*
 * AUTHOR: Jarred Morris with J. Fairbrother
 *
 * Introduction to those viewing through GitHub
 *
 * This software drives a simple clone of the game Pong to run on an embedded
 * system using an ARM board. Note that this is a simpler version than the
 * finished version, and does not include features such as displaying user scores,
 * as with other items on my GitHub, this is for plagiarism protection as this
 * assignment may be set to pupils at the university once again.
 * It hosts the game for two players; each controlling a bat with an analogue
 * paddle-like joystick and it draws its graphics to an oscilloscope display.
 * This requires a hefty amount of DAC and ADC work. It also relied on an
 * understanding of the physical limitations of moving an electron gun manually
 * to create graphics, as precise movement was needed to prevent a faded
 * appearance or skewed imagery.
 * Given that this is is a task in embedded systems engineering it is unlikely
 * to compile or run properly on ordinary systems.
 * Please do get in contact if you would like a description of the hardware it
 * was created for. This work naturally required plenty of "talking" to the
 * hardware through registers and was a fascinating exercise in computer
 * organisation and architecture.
 *
 * No Math library was provided on the system so concepts like random generation
 * had to be implemented by myself.
 */
typedef volatile unsigned int ioreg;

/* Enable peripherals */
#define PIO_PER (ioreg *) 0xfffff400 //PIO Enable Register
#define PIO_OER (ioreg *) 0xfffff410 //PIO Output Enable Register
#define PIO_CODR (ioreg *) 0xfffff434 //Clear Output Data Register
#define PIO_SODR (ioreg *) 0xfffff430 //Set Output Data Register
#define PIO_PCER (ioreg *) 0xfffffc10 //Peripheral Clock Enable Register

/* Enable more peripherals */
#define PIO_PDR (ioreg *) 0xfffff404 //PIO Disable Register
#define PIO_ASR (ioreg *) 0xfffff470 //PIO A Select Register
#define PIOA (ioreg *) 0x00000004 //Parallel I/O Controller A
#define ADC (ioreg *) 0x00000010 //Analog­ to ­Digital Converter
#define SPI (ioreg *) 0x00000020 //Serial Peripheral Interface

/* Analog-­to-­digital conversion (and driving the display!) */
#define SPI_CR (ioreg *) 0xfffe0000 //SPI Control Register
#define SPI_MR (ioreg *) 0xfffe0004 //SPI Mode Register
#define SPI_SR (ioreg *) 0xfffe0010 //SPI status register
#define SPI_TDR (ioreg *) 0xfffe000c //SPI Transmit Data Register
#define SPI_CSR0 (ioreg *) 0xfffe0030 //SPI Chip Select Register 0

/* Interpreting analog inputs */
#define ADC_CR (ioreg *) 0xfffd8000 //ADC control register
#define ADC_MR (ioreg *) 0xfffd8004 //ADC mode register
#define ADC_CHER (ioreg *) 0xfffd8010 //ADC channel enable register
#define ADC_SR (ioreg *) 0xfffd801c //ADC status register
#define ADC_CDR4 (ioreg *) 0xfffd8040 // ADC channel 4 data register (Player1 paddle)
#define ADC_CDR5 (ioreg *) 0xfffd8044 //ADC channel 5 data register (Player2paddle)

/* Defining a coordinate system */
#define PORT_A 0xc000 //Port to write x-coordinate
#define PORT_B 0x4000 //Port to write y-coordinate

#define PIOA_IDENTIFIER 0x00000034 // BITS 2, 4 & 5

/*
 * Main method
 * Contains an infinite while loop which naively speaking allows the simulation of
 * event­-based programming.
 */
int main(void)
{
	LowLevelInit(); //Configure master clock
	preamble(); //Configure the registers

	int ball[2] = {350, 500}; /*Position vector for the ball of the form (ball[0], ball[1])*/
	int velocity[2] = {3, 0}; /*Velocity vector for the ball of form (velocity[0], velocity[1])*/
	int pad1; //y­center of Player1's bat
	int pad2; //y­center of Player2's bat

	int p1Score = 0;
	int p2Score = 0;

	//Infinite while loop begins...
	while (1)
	{
		/* Draw ball at the point dictated by its position vector. This is done 100 times
		 * to make the point appear more intense and noticeable on the display. */
		int k;
		for (k = 0; k < 100; ++k)
		{
			drawPoint(ball[0], ball[1]);
		}

		//Add velocity of ball to move it in the direction of that vector
		ball[0] += velocity[0];
		ball[1] += velocity[1];

		if (ball[1] > 776 || ball[1] <= 0)
		{
			/* ^^ If here, then the ball has met roof or floor barriers and needs to be
			 * deflected by same angle of collision but reflected in normal to the barrier.*/
			velocity[1] = -1 * velocity[1]; //Invert y­velocity
			ball[1] = ball[1] + velocity[1]; //Move ball along in x­direction
		}
		if (ball[0] <= 16)
		{
			/* ^^ The ball has come into line with or surpassed the Player1's bat, so let's
			* check if a successful hit was made...*/
			if (ball[1] < pad1 + 70 && ball[1] > pad1 - 70)
			{
				/* ^^ Ball is within the range of the bat; hence hit was successful.
				 * Closer ball hit bat center, the less steep the angle of deflection: */
				velocity[1] = (ball[1] - pad1) / 20;

				/* Make sure ball is sent back in opposite x­direction by multiplying by -1: */
				velocity[0] = -­1 * velocity[0];
			}
			else
			{
				/* The ball wasn't hit. So now we need to reset the position of the ball */

				/* Ball x-position reset to roughly middle of screen, y-position is reset
				 * to what is a pseudorandom value formed as a function of the bat centers
				 * mod 768 */
				ball[0] = 504;
 				ball[1] = psuedorandom(pad1, pad2, 768);

				/* Velocity vector is reset to initial value (3,0) */
				velocity[0] = 3;
				velocity[1] = 0;

				/* Long delay to hold error ball in place as a way of giving
				 * players to prepare themselves for the next go */
				delay(1000000);

				p2Score += 1; //increment player 2 score because p1 missed
			}
		}
		else if (ball[0] >= 1008)
		{
			/* The ball has come into line with or surpassed the Player2's bat, so
			 * check if a successful hit was made...*/
			if (ball[1] < pad2 + 70 && ball[1] > pad2 - 70)
			{
				/* Ball is within the range of the bat; hence hit was successful. See equivalent player1 code for explanations */
				velocity[0] = -­1 * velocity[0];
				velocity[1] = (ball[1] - pad2)/(20);
			}
			else
			{
				/* The ball wasn't hit. So now need to reset the position of the ball. See equivalent player1 code for explanations */
				ball[0] = 504;
				ball[1] = psuedorandom(pad1, pad2, 768);
				velocity[0] = 3;
				velocity[1] = 0;
				delay(1000000);
				p1Score += 1;
			}
		}

		*ADC_CR = 0x2;

		// start ang-to-dig conversion with ADC control register
		while (*ADC_SR & 0x10 == 0); // wait for ch4 to complete conversion

		pad1 = *ADC_CDR4; //Center of Player1's bat set to value of ch4 data register
		pad2 = *ADC_CDR5; //Center of Player1's bat set to value of ch4 data register

		//Normalise pad1
		pad1 = (pad1 ­ 153) * 2;
		if (pad1 - 50 < 0)
		{
			pad1 = -1 * pad1;
		}

		//Normalise pad2
		pad2 = (pad2 - 153) * 2;
		if (pad2 - 50 < 0)
		{
			pad2 = -1 * pad2;
		}

		/* Draw player2's bat inline with x=1008, centered at position vector (1008,pad2)*/
		drawBat(1008, pad2);

		int j;
		for (j = 0; j < 10; ++j)
		{
			/* Draw the arena (10 times so that its intense enough on the display) */
			drawRoofFloor();
		}

		/* Draw player2's bat inline with x=16, centered at position vector (16,pad2) */
		drawBat(16, pad1);
	}
}
/*
 * A function that takes the value of each paddle and then uses this to produce a pseudo­
 * random value modded by the value of x (so that a value can be produced between 0 and x­1)
 * Note that this the x and y position of a paddle is decided by human, we can assume this
 * to be reasonably random, and certainly better than anything computable on the primitive
 * hardware
 */
int psuedorandom(int pad1, int pad2, int x)
{
	return (pad1 + pad2) % x;
}

/*
 * A function that draws a bat inline with x, with the top at y
 */
int drawBat(int x, int y)
{
	y -= 50;
	int j = 0;
	for (j = 0; j < 100; j+= 4)
	{
		if (j+y < 1) {
			drawPoint(x, -1 * (j+y));
		}
		else
		{
			drawPoint(x, j+y);
		}
	}
	/* Temporarily hold electron beam at (x,0) to decrease trailing */
	drawPoint(x, 0);
	delay(1000);
}

/*
 * A method for drawing a point at (x,y) in 2-space. Uses the transmit data register and then
 * ports A and B are used for each respective axis. It moves the electron gun to the position
 * at (x,y) on the display, thus drawing a point there
 */
int drawPoint(int x, int y)
{
	/* Set transmit data register to value of PORT_A bitwise OR with the value of x bit shifted to the left by two bits */
	*SPI_TDR = PORT_A | (x << 2);
	delayWithSPIConversion(1000);

	/* Set transmit data register to value of PORT_B bitwise OR with the value of y bit shifted to the left by two bits */
	*SPI_TDR = PORT_B | (y << 2);
	delayWithSPIConversion(1000);

	return 0;
}

/*
 * A function to draw the roof and the floor to form a boundary for the in-play area.
 */
int drawRoofFloor()
{
	/* Draw the floor */
	int i = 0;
	for (i = 0; i < 1023; i += 16)
	{
		drawPoint(i, 0);
	}

	/* Draw the roof */
	for (i = 1023; i > 0; i -= 16)
	{
		drawPoint(i, 776);
		i -= 16;
	}
}

/*
 * A function to perform a delay proportional in length to the size of parameter x
 */
int delay(int x)
{
	int j = 0;
	for (j = 0; j < x; ++j)
	{
		1;
		// above line to prevent this loop being optimised away by the compiler
	}
}

/*
 * A function to perform a delay with SPI conversion. Ensures that program can be made to
 * wait for SPI conversion so that the serialisation process is not interrupted.
 */
int delayWithSPIConversion(int x)
{
	while (*SPI_SR & 0x2 == 0)
	{
		delay(x);
	}
}

/*
 * A preamble function to be called at the start of the program to initialise all of
 * the registers appropriately.
 */
int preamble()
{
	*PIO_PCER = PIOA_IDENTIFIER;
	*PIO_PDR = 0x7800;
	*PIO_ASR = 0x7800;
	*SPI_CR = 0x80;
	*SPI_CR = 0x1;
	*SPI_MR = 0x11;
	*SPI_CSR0 = 0x183;
	*SPI_TDR = 0xd002;

	delayWithSPIConversion(100);

	*ADC_CR = 0x1; // reset the ADC
	*ADC_CHER = 0x30; // enable analog channels 4 and 5
	*ADC_MR = 0x030b0400; //set sample+hold time = 3, startup = b, prescale = 4
}
