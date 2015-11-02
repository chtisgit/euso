/*!
	\file calc.c
	\author Christian Fiedler <e1363562@student.tuwien.ac.at>
	\date 12.10.2015

	\brief postfix calculator program

	This program calculates the results of terms given
	in reverersed polish notation (RPN).
*/

#include <ctype.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define STACKSIZE	1024
#define LINEMAX		1024

char *progname;
int flag_absolute = 0, flag_integer = 0;

void error(const char *s)
{
	(void)printf("%s error: %s\n", progname, s);
	exit(1);
}

/*! \brief RPN stack structure */
static struct{
	double buf[STACKSIZE];
	int i;
} stack;

/*! \brief pushes double value onto stack */
void stack_push(double n)
{
	/* checking for "Stack Overflow" */
	if(stack.i == STACKSIZE)
		error("too many values! (stack overflow)");
	stack.buf[stack.i++] = n;
}

/*! \brief pops double value from stack */
double stack_pop(void)
{
	/* checking for "Stack Underflow" */
	if(stack.i == 0)
		error("too many operators! (stack underflow)");
	return stack.buf[--stack.i];
}

/*! \brief resets the stack */
void stack_reset(void)
{
	stack.i = 0;
}

/*! 
	\brief skips whitespace in a string
	\param ptr
		address of a (non-owning) pointer (in)to string/char array

*/
void skip_white(char **ptr)
{
	char *s = *ptr;
	for(; *s != '\0'; s++){
		if(!isspace(*s))
			break;
	}
	*ptr = s;
}

/*!
	\brief executes an arithmetic operation on the RPN stack
	\param op
		arithmetic operator to be used (one of '+', '-', '*', '/', 's' (= sin), 'c' (= cos) )
*/
void operator(char op)
{
	double x, y;

	switch(op){
	case '+':
		y = stack_pop();
		x = stack_pop();
		stack_push(x+y);
		break;
	case '-':
		y = stack_pop();
		x = stack_pop();
		stack_push(x-y);
		break;
	case '*':
		y = stack_pop();
		x = stack_pop();
		stack_push(x*y);
		break;
	case '/':
		y = stack_pop();
		x = stack_pop();
		stack_push(x/y);
		break;
	case 's':
		x = stack_pop();
		stack_push(sin(x));
		break;
	case 'c':
		x = stack_pop();
		stack_push(cos(x));
		break;
	default:
		error("syntax error! (unknown operator)");
	}
}

/*!
	\brief calculates result of the next line
	\param inputf
		input file
	\return result of the calculation on the next line in the file
	Reads a line from inputf, calculates the result, pops and returns it from the stack
*/
double parse_line(FILE *inputf)
{
	static char line[LINEMAX];
	
	char *p = line, *oldp;
	double d;

	stack_reset();
	
	fgets(line, LINEMAX, inputf);
	
	while(p-line < LINEMAX-1){
		skip_white(&p);
		
		if(*p == '\0')
			break;
		
		oldp = p;
		d = strtod(p, &p);
		if(oldp == p){
			/* no conversion happened */
			operator(*p++);
		}else{
			stack_push(d);
		}
	}

	return stack_pop();
}


/*!
	\brief calculates all RPN terms in given file
	\param inputf
		input file
	\details
		This procedure uses the global variables flag_absolute and flag_integer 
		to determine if the result should be echoed as absolute value
		or as integer, respectively.
	
	The results of all calculations are printed to standard output

*/
void parse(FILE *inputf)
{
	double result = parse_line(inputf);
	
	if(feof(inputf))
		return;

	if(flag_absolute)
		result = fabs(result);
	
	if(flag_integer)
		(void)printf("%d\n", (int) result);
	else
		(void)printf("%f\n", result);
}

/*! \brief This function prints the synopsis of the program */
void usage(void)
{
	(void)fprintf(stderr,"Usage: %s [-i] [-a] [file1 [file2 ...]] \n",progname);
}

/*!
	\brief reads parameters from command line and uses parse() on the given files (or stdin if none given)	
*/
int main(int argc, char **argv)
{
	int opt, i;
	FILE *f;
	
	progname = argv[0];

	while ((opt = getopt(argc, argv, "ia")) != -1) {
		switch (opt) {
		case 'i':
			flag_integer = 1;
			break;
		case 'a':
			flag_absolute = 1;
			break;
		default:
			usage();
			return 1;
		}
	}

	if(optind == argc){
		/* no files in command args */
		while(!feof(stdin))
			parse(stdin);	
	}else{
		for(i = optind; i < argc; i++){
			f = fopen(argv[i], "r");
			if(f == NULL){
				usage();
				return 1;
			}

			while(!feof(f))
				parse(f);

			fclose(f);
		}
		
	}

	return 0;
}

