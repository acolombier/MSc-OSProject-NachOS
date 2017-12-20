/*! Function that copy a string from user memory to kernel memory. */
/*!
 * \param from Pointer to the begining of the string in the user memory.
 * \param max_size Maximum size of the string to be copied, if '\0' has not been encountered before. If '\0' appears before, the copying finishes at it.
 * \return Pointer to the begining of the string in the kernel memory.
 */
char *copyStringFromMachine(int from, unsigned int max_size);

/*! Function that copies a string from kernel memory to user memory. */
/*!
 * \param string Pointer to the begining of the string in kernel memory.
 * \param to Pointer to where the string will be copied in user memory.
 * \param max_size Maximum size of the string to be copied, if '\0' as not been encountered before. If '\0' appears before, the copying finishes at it.
 */
void copyStringToMachine(char *string, int to, unsigned max_size);
