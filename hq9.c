/* hq9.c -- HQ9+ Interpreter in C v0.2.1
  
 Mark Stock
  http://hoop-la.ca/software/contact.html
 
 Compile with: make hq9
 Usage: ./hq9 ""
 Additional usage: ./hq9 "" ""
 Examples:
  ./hq9 qqqq
  ./hq9 hq++++ ; echo $PIPESTATUS
  ./hq9 H
  ./hq9 "" -9qH H one letter palindrome
  ./hq9 99 | say -v "Cellos"

 For information on HQ9+ see
  http://www.cliff.biffle.org/esoterica/hq9plus.html
  
 Version 0.1 implements the H, 9, Q, and + commands.
  - The return value of the program is the accumulator.
  - Commands are not case sensitive.
  - Unknown commands emit error messages.
  - Any problem with a file may emit an error message and stop the program.
  - The author has no clue as to whether the H command prints a newline
    after printing the entire text of the source code file, or whether
    the newline character is part of the source code file and the newline
    character is to be implemented as no operation.
 
 Version 0.2 added options, see additional usage for details.
	0.2.1 - renamed source from hq9+ to hq9 to play nice with URLs
 */

#include <errno.h>
#include <stdio.h>

typedef struct {
	int error_messages;
	int error_stop;
	int ignore_newlines;
	int parse_if_no_file;
	int be_exact;
	char * helloworld[2];
	char * filename;
} t_config;


/* http://99-bottles-of-beer.net/lyrics.html */
static void ninetyninebottlesofbeer()
{
	int i;

	printf("99 bottles of beer on the wall, 99 bottles of beer.\n", i, i);
	for (i = 98; i > 1; i--) {
		printf("Take one down and pass it around, %d bottles of beer on the wall.\n", i);
		printf("\n");
		printf("%d bottles of beer on the wall, %d bottles of beer.\n", i, i);
	}
	printf("Take one down and pass it around, 1 bottle of beer on the wall.\n", i);
	printf("\n");
	printf("1 bottle of beer on the wall, 1 bottle of beer.\n");
	printf("Take one down and pass it around, no more bottles of beer on the wall.\n");
	printf("\n");
	printf("No more bottles of beer on the wall, no more bottles of beer.\n");
	printf("Go to the store and buy some more, 99 bottles of beer on the wall.\n");
}


static int copyfile(FILE * in, FILE * out)
{
	int c;
	int last_char_is_newline;

	last_char_is_newline = 0;
	while ((c = fgetc(in)) != EOF) {
		fputc(c, out); /*** no error checking is being done! ***/
		last_char_is_newline = (c == '\n');
	}
	return last_char_is_newline;
}



static void log_error(t_config * p, int to_write)
{
	char * access[2] = { "read", "write" };

	if (p->error_messages)
		fprintf(stderr, "Unable to open file to %s, error %d: %s\n", access[to_write], errno, p->filename);
}

static int qnine(t_config * p)
{
	FILE * fp;
	int last_char_is_newline;

	fp = fopen(p->filename, "r");
	if (fp) {
		last_char_is_newline = copyfile(fp, stdout);
		fclose(fp);
		if (!last_char_is_newline)
			printf("\n");
	} else {
		log_error(p, 0);
	}
	return (!fp);
}


static int fhq9plus(FILE * fp, int * accumulator_ptr, t_config * p)
{
	int c;
	int error;
	int errors;

	errors = 0;
	while ((c = fgetc(fp)) != EOF) {
		if (c == 'h' || c == 'H') {
			printf("%s\n", p->helloworld[p->be_exact]);
		} else if (c == 'q' || c == 'Q') {
			if (qnine(p)) return errors + 1; /* aborts because of a problem with a file */
		} else if (c == '9') {
			ninetyninebottlesofbeer();
		} else if (c == '+') {
			(*accumulator_ptr)++;
		} else {
			if (!(c == '\n' && p->ignore_newlines)) {
				errors++;
				if (p->error_messages) {
					fprintf(stderr, "Unknown command: ");
					if (c < 33 || c >= 127)
						fprintf(stderr, "chr(%d)", c);
					else
						fprintf(stderr, "%c", c);
					fprintf(stderr, "\n");
				}
				if (p->error_stop)
					return errors;
			}
		}
	}
	return errors;
}


int hq9plus(int * accumulator_ptr, t_config * p)
{
	FILE * fp;
	char * tmp_filename;
	int errors;

	errors = 1;
	fp = fopen(p->filename, "r");
	if (!fp) {
		if (p->parse_if_no_file && errno == ENOENT) {
			tmp_filename = tmpnam(NULL);
			if (errno != ENOENT) {
				fprintf(stderr, "tmpnam error %d\n", errno);
			} else {
				fp = fopen(tmp_filename, "w");
				if (fp) {
					fprintf(fp, p->filename);
					fclose(fp);
					p->filename = tmp_filename;
					errors = hq9plus(accumulator_ptr, p);
					remove(tmp_filename);
				} else {
					p->filename = tmp_filename;
					log_error(p, 1);
				}
			}
		} else {
			log_error(p, 0);
		}
	} else {
		errors = fhq9plus(fp, accumulator_ptr, p);
	}
	return errors;
}

int main(int argc, char * argv[])
{
	int accumulator;
	FILE * fp;
	t_config config;
	int i;
	int quit;
	int additional_usage;
	int start;
	
	accumulator = 0;
	config.error_messages = 1;
	config.error_stop = 0;
	config.ignore_newlines = 1;
	config.be_exact = 0;
/* default hello world */
	config.helloworld[0] = "Hello, world!";
/* exact hello world see
 http://stackoverflow.com/questions/659752/programming-challenge-can-you-code-a-hello-world-program-as-a-palindrome/660614#660614
 */
	config.helloworld[1] = "Hello, World";
	start = 1;
	quit = 0;
	if (argc > start) {
		config.parse_if_no_file = 1;
		additional_usage = 0;
		if (!argv[1][0]) {
			start++;
			if (argc == 2) {
				fprintf(stderr, "usage: hq9 [h | q | 9 | + | file ...]\n");
				fprintf(stderr, "       hq9 \"\" [\"\"]\n");
				fprintf(stderr, "HQ9+ is an esoteric programming language.\n");
				fprintf(stderr, "  H  Prints \"%s\"\n", config.helloworld[0]);
				fprintf(stderr, "  Q  Prints the entire text of the source code file.\n");
				fprintf(stderr, "  9  Prints the complete canonical lyrics to \"99 Bottles of Beer on the Wall\"\n");
				fprintf(stderr, "  +  Increments the accumulator.\n");
				fprintf(stderr, "  For information on HQ9+ see\n");
				fprintf(stderr, "      http://www.cliff.biffle.org/esoterica/hq9plus.html\n");
				fprintf(stderr, "  Version 0.2 of this C implementation of HQ9+ is by Mark Stock.\n");
				fprintf(stderr, "      http://hoop-la.ca/software/contact.html\n");
				fprintf(stderr, "    - The return value of the program is the value of the accumulator\n");
				fprintf(stderr, "      which is a %d-bit signed integer.\n", sizeof(int) * 8);
				fprintf(stderr, "    - Commands are not case sensitive.\n");
				fprintf(stderr, "    - Unknown commands emit error messages.\n");
				fprintf(stderr, "    - Any problem with a file may emit an error message and stop the program.\n");
				quit = 1;
			} else if (argc >= 3 && argv[2][0] == '-') {
				start++;
				for (i = 1; argv[2][i]; i++) {
					switch (argv[2][i]) {
						case 'q': config.error_messages = 0; break;
						case '9': config.error_stop = 1; break;
						case 'H': config.be_exact = 1; break;
						case ' ': config.ignore_newlines = 0; break;
						default:  additional_usage = 1;
					}
				}
			} else {
				additional_usage = 1;
			}
			if (additional_usage) {
				fprintf(stderr, "usage: hq9 \"\" -9qH  [h | q | 9 | + | file ...]\n");
				fprintf(stderr, "options:\n");
				fprintf(stderr, "  9  stops when an unknown command is encountered\n");
				fprintf(stderr, "  q  turns off error messages for unknown commands\n");
				fprintf(stderr, "  H  the output for the H command will be exactly \"%s\"\n", config.helloworld[1]);
				fprintf(stderr, " \" \"  processes newline characters\n");
				quit = 1;
			}
		}
	}		
	if (!quit) {
		if (argc <= start) {
			config.parse_if_no_file = 0;
			config.filename = tmpnam(NULL);
			if (errno != ENOENT) {
				fprintf(stderr, "tmpnam error %d\n", errno);
			} else {
				fp = fopen(config.filename, "w");
				if (fp) {
					copyfile(stdin, fp);
					fclose(fp);
					hq9plus(&accumulator, &config);
					remove(config.filename);
				} else {
					log_error(&config, 1);
				}
			}
		} else {
			for (i = start; i < argc; i++) {
				config.filename = argv[i];
				if (hq9plus(&accumulator, &config) && config.error_stop)
					break;
			}
		}
	}

	return accumulator;
}
