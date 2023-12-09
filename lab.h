#ifndef LAB_H
#define LAB_H
#include <stdlib.h>
#include <stdbool.h>

#define MAX_VERSION_STRING 10
#define lab_VERSION_MAJOR 1
#define lab_VERSION_MINOR 0


#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * @brief Returns a string containing the version of the library. This string
   * has been allocated using malloc and must be freed by the caller. The
   * version of the library is generated using the cmake version.
   *
   * @return char* The version string
   */
  char *getVersion(void);

  /**
   * @brief Function that leaks some data!
   *
   * @param a some number
   * @return a pointer to some memory
   */
  int *leakyFunction(int a);

  /**
   * @brief This function causes a segfault
   *
   */
  int segfault(void);

  /**
   * @brief This function causes an array out of bounds error
   *
   */
  void outOfBounds(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
