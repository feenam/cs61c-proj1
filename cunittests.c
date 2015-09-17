#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <CUnit/Basic.h>
#include "beargit.h"
#include "util.h"

/* printf/fprintf calls in this tester will NOT go to file. */

#undef printf
#undef fprintf

/* The suite initialization function.
 * You'll probably want to delete any leftover files in .beargit from previous
 * tests, along with the .beargit directory itself.
 *
 * You'll most likely be able to share this across suites.
 */
int init_suite(void)
{
    // preps to run tests by deleting the .beargit directory if it exists
    fs_force_rm_beargit_dir();
    unlink("TEST_STDOUT");
    unlink("TEST_STDERR");
    return 0;
}

/* You can also delete leftover files after a test suite runs, but there's
 * no need to duplicate code between this and init_suite 
 */
int clean_suite(void)
{
    return 0;
}

/* Simple test of fread().
 * Reads the data previously written by testFPRINTF()
 * and checks whether the expected characters are present.
 * Must be run after testFPRINTF().
 */
void simple_sample_test(void)
{
    // This is a very basic test. Your tests should likely do more than this.
    // We suggest checking the outputs of printfs/fprintfs to both stdout
    // and stderr. To make this convenient for you, the tester replaces
    // printf and fprintf with copies that write data to a file for you
    // to access. To access all output written to stdout, you can read 
    // from the "TEST_STDOUT" file. To access all output written to stderr,
    // you can read from the "TEST_STDERR" file.
    int retval;
    retval = beargit_init();
    CU_ASSERT(0==retval);
    retval = beargit_add("asdf.txt");
    CU_ASSERT(0==retval);
}

struct commit {
  char msg[MSG_SIZE];
  struct commit* next;
};


void free_commit_list(struct commit** commit_list) {
  if (*commit_list) {
    free_commit_list(&((*commit_list)->next));
    free(*commit_list);
  }

  *commit_list = NULL;
}

void run_commit(struct commit** commit_list, const char* msg) {
    int retval = beargit_commit(msg);
    CU_ASSERT(0==retval);

    struct commit* new_commit = (struct commit*)malloc(sizeof(struct commit));
    new_commit->next = *commit_list;
    strcpy(new_commit->msg, msg);
    *commit_list = new_commit;
}

void simple_log_test(void)
{
    struct commit* commit_list = NULL;
    int retval;
    retval = beargit_init();
    CU_ASSERT(0==retval);
    FILE* asdf = fopen("asdf.txt", "w");
    fclose(asdf);
    retval = beargit_add("asdf.txt");
    CU_ASSERT(0==retval);
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!1");
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!2");
    run_commit(&commit_list, "THIS IS BEAR TERRITORY!3");

    retval = beargit_log(10);
    CU_ASSERT(0==retval);

    struct commit* cur_commit = commit_list;

    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);

    while (cur_commit != NULL) {
      char refline[LINE_SIZE];

      // First line is commit -- don't check the ID.
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT(!strncmp(line,"commit", strlen("commit")));

      // Second line is msg
      sprintf(refline, "   %s\n", cur_commit->msg);
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT_STRING_EQUAL(line, refline);

      // Third line is empty
      CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
      CU_ASSERT(!strcmp(line,"\n"));

      cur_commit = cur_commit->next;
    }

    CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstdout));

    // It's the end of output
    CU_ASSERT(feof(fstdout));
    fclose(fstdout);

    free_commit_list(&commit_list);
}


/* Simple test of the Beargit Status Command.
 * Reads the data previously written by testFPRINTF()
 * and checks whether the expected characters are present.
 * Must be run after testFPRINTF().
 */
void empty_status_test(void)
{
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    int retval;
    beargit_init();
    retval = beargit_status();
    CU_ASSERT(0==retval);

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"Tracked files:", strlen("Tracked files:")));

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"", strlen("")));

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"", strlen("")));

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"There are 0 files total.", strlen("There are 0 files total.")));

    CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstdout));
    // It's the end of output
    CU_ASSERT(feof(fstdout));
    fclose(fstdout);

}

/* Simple test of the Beargit Status Command.
 * Reads 
 */
void oneFile_status_test(void)
{       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];
 
    // beargit_init();
    beargit_add("test.txt");
   
    beargit_status();

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);

    int i = 0;
    while (i++ < 4) fgets(line, LINE_SIZE, fstdout);
    
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"Tracked files:", strlen("Tracked files:")));

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"", strlen("")));

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"test.txt", strlen("test.txt")));

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"", strlen("")));

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"There are 1 files total.", strlen("There are 1 files total.")));

    CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstdout));
    // It's the end of output
    CU_ASSERT(feof(fstdout));
    fclose(fstdout);
}


/* Simple test of the Beargit Status Command.
 * Reads 
 */
void twoFile_status_test(void)
{       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];
 
    beargit_add("test.txt");
    beargit_add("test2.txt");
   
    beargit_status();

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);

    int i = 0;
    while (i++ < 9) fgets(line, LINE_SIZE, fstdout);
    
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"Tracked files:", strlen("Tracked files:")));

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"", strlen("")));

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"test.txt", strlen("test.txt")));

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"test2.txt", strlen("test2.txt")));

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"", strlen("")));

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"There are 2 files total.", strlen("There are 2 files total.")));

    CU_ASSERT_PTR_NULL(fgets(line, LINE_SIZE, fstdout));
    // It's the end of output
    CU_ASSERT(feof(fstdout));
    fclose(fstdout);
}

/* Simple test of the Beargit Status Command.
 * Reads 
 */
void oneFile_rm_test(void)
{       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    int retval;
    beargit_init();

    beargit_add("test.txt");
    retval = beargit_rm("test.txt");
    CU_ASSERT(0==retval);

    beargit_status();

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);

    int i = 0;
    while (i++ < 3) fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"There are 0 files total.", strlen("There are 0 files total.")));

    fclose(fstdout);
}


/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int cunittester()
{
   CU_pSuite pSuite = NULL;  /* Sample code provided with skeleton */
   CU_pSuite pSuite2 = NULL; /* Suite of testing for the STATUS command */
   CU_pSuite pSuite3 = NULL; /* Suite of testing for the RM command */

   // CU_pSuite pSuite2 = NULL;  /* Sample code provided with skeleton */


   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();


///////////////

   /* add SAMPLE Suite #1 to the registry */
   pSuite = CU_add_suite("Sample Suite_1", init_suite, clean_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the SAMPLE Suite #1 */
   if (NULL == CU_add_test(pSuite, "Simple Test #1", simple_sample_test))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

///////////////

   /* add STATUS Suite to the registry */
   pSuite2 = CU_add_suite("Status Suite", init_suite, clean_suite);
   if (NULL == pSuite2) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the STATUS Suite */
   if ((NULL == CU_add_test(pSuite2, "Status Test #1: empty test", empty_status_test)) ||
       (NULL == CU_add_test(pSuite2, "Status Test #2: one file test", oneFile_status_test)) ||
       (NULL == CU_add_test(pSuite2, "Status Test #3: two files test", twoFile_status_test)))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

///////////////

   /* add RM Suite to the registry */
   pSuite3 = CU_add_suite("RM Suite", init_suite, clean_suite);
   if (NULL == pSuite3) {
      CU_cleanup_registry();
      return CU_get_error();
   }
   /* Add tests to the RM Suite */
   if (NULL == CU_add_test(pSuite3, "RM Test #1: one file to remove test", oneFile_rm_test))
    /* Edge cases still to test: No files to remove, file isn't being tracked, */
   {
      CU_cleanup_registry();
      return CU_get_error();
   }




   // /* add sample Suite #2 to the registry */
   // pSuite2 = CU_add_suite("Sample Suite_2", init_suite, clean_suite);
   // if (NULL == pSuite2) {
   //    CU_cleanup_registry();
   //    return CU_get_error();
   // }

   // /* Add tests to the sample Suite #2 */
   // if (NULL == CU_add_test(pSuite2, "Log output test", simple_log_test))
   // {
   //    CU_cleanup_registry();
   //    return CU_get_error();
   // }



   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}


