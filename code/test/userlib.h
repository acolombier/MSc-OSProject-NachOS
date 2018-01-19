#ifndef USER_LIB
#define USER_LIB

#define SIGINT 2
#define SIGKILL 9
#define SIGTERM 15
#define SIGCONT 18
#define SIGSTOP 18
#define SIGTSTP 20

#define NULL 0

#define PageSize 128
#define divRoundDown(n,s)  ((n) / (s))
#define divRoundUp(n,s)    (((n) / (s)) + ((((n) % (s)) > 0) ? 1 : 0))

#include "syscall.h"

/*! \brief Compare two strings.
 *  \param s1 the first string
 * 	\param s2 the second string
 * 	\return Returns 0 if strings are the same, else the number of characters that differ.
 */
int strcmp(char* s1, char* s2);

/*! \brief Calculate the length of the string s, excluding the terminating null byte ('\0').
 *  \param str the string
 * 	\return Returns the number of bytes in the string s.
 */
int strlen(const char* str);

/*! \brief Locate character in string.
 *  \param str the string
 * 	\param character the character to be located
 * 	\return Returns a pointer to the last occurrence of the character in the string.
 */
char* strrchr (char* str, int character);

const char* getlogin();
const char* getcwd();
const char* gethostname();

/*! \brief Convert a string to an integer.
 *  \param s the string
 * 	\return Converts the the string pointed to by s to int.
 */
int atoi(char* s);

/*! \brief Convert an integer to string.
 *  \param value value to be converted to string
 * 	\param str array in memory where to store the resulting non-terminating thread
 * 	\param base numerical base used to represent the value as a string
 * 	\return Stores the result in the array given by the str parameter
 */
char* itoa(int value, char* str, int base);

/*! \brief Copy memory area. Copy n bytes from memory area src to memory area dest. Current POSIX standards.
 *  \param dest the memory area destination
 *  \param src the memory area source
 * 	\param n the number of bytes to be copied
 * 	\return Returns a pointer to the destination string dest.
 */
void *memcpy(void *dest, const void *src, size_t n);

/*! \brief Copy the string pointed to by src, including the terminating null byte ('\0'), to the buffer pointed to by dest.
 *  \param dest the memory area destination
 *  \param src the memory area source
 * 	\return Returns a pointer to the destination string dest.
 */
void strcpy(char *dest, const char *src);

/*! \brief Fill memory with a constant byte. Fill the first n bytes of the memory area pointed to by s with the constant byte c. Current POSIX standards.
 *  \param s memory area to set
 * 	\param c the byte to be used as a constant 
 * 	\param n the number of bytes to fill in the memory area
 * 	\return Returns a pointer to the memory area s.
 */
void *memset(void *s, int c, size_t n);

/*! \brief Extract tokens from strings.
 *  \param str the string to be parsed
 * 	\param delimiters a set of bytes that delimit the tokens in the parsed string
 * 	\return Returns a pointer to the next token, or NULL if there are no more tokens.
 */
char *strtok (char *str, char *delimiters );

/*! \brief Format date and time in a predefined nachos format and store it into a sharacter array.
 *  \param str the character array to store the formated date and time
 * 	\param size the maximum size of the character array str
 * 	\param time the time to be broken down
 * 	\return Returns 0 at success, otherwise -1 or 1.
 */
int simple_strftime(char *str, size_t size, int time);

/*! \brief Add padding to current string so that it reaches the wanted lenght. Padding is added at the end of the string.
 *  \param str the string to pad
 * 	\param i the final size of the padded string
 * 	\param padder the character to be used as padding
 * 	\return Returns the pointer to padded string.
 */
char* strpad(char* str, size_t i, char padder);

/*! \brief Reverse the string.
 *  \param str the string to be reversed
 * 	\return Returns the pointer to the reversed string.
 */
char* reverse(char *str);

#endif
