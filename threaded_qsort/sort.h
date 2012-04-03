
/* the following routine allows the user of the sort program to */
/* set the maximum number of threads that can be created by the */
/* sort program.  The calling thread counts as one thread.      */

void setSortThreads(int maxThreads);

/* the following procedure sorts an array of strings.  The first   */
/* parameter is a pointer to the first element of a array of count */
/* string pointers (char*)                                         */

void sortThreaded(char** array, unsigned int count);

