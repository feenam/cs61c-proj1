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

void simple_log_test(void) {
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
      // fprintf(stdout, "\nline:   %s", line);                     /* DELETE ME WHEN WORKING*/
      // fprintf(stdout, "refline: %s", refline);                     /* DELETE ME WHEN WORKING*/
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
void empty_status_test(void) {
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
void oneFile_status_test(void) {       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];
 
    // beargit_init();
    FILE* test = fopen("test.txt", "w");
    fclose(test);
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
void twoFile_status_test(void) {       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];
    
    FILE* test = fopen("test.txt", "w");
    fclose(test);
    FILE* test2 = fopen("test2.txt", "w");
    fclose(test2);
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

/* Simple test of the Beargit Rm Command.
 * Reads 
 */
void noFile_rm_test(void) {       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    int retval;
    beargit_init();


    retval = beargit_rm("false.txt");
    CU_ASSERT(1==retval);
    
    FILE* test = fopen("test.txt", "w");
    fclose(test);
    beargit_add("test.txt");
    retval = beargit_rm("false.txt");
    CU_ASSERT(1==retval);    

    beargit_add("false.txt");
    retval = beargit_rm("false.txt");
    CU_ASSERT(0==retval);
    retval = beargit_rm("false.txt");
    CU_ASSERT(1==retval);

    beargit_status();

    FILE* fstderr = fopen("TEST_STDERR", "r");
    CU_ASSERT_PTR_NOT_NULL(fstderr);

    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
    CU_ASSERT(!strncmp(line,"ERROR:  File false.txt not tracked.", strlen("ERROR:  File false.txt not tracked.")));
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
    CU_ASSERT(!strncmp(line,"ERROR:  File false.txt not tracked.", strlen("ERROR:  File false.txt not tracked.")));

    fclose(fstderr);

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);

    fgets(line, LINE_SIZE, fstdout);
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"test.txt", strlen("test.txt")));
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"There are 1 files total.", strlen("There are 1 files total.")));
    
    fclose(fstdout);
    beargit_rm("test.txt");
}


/* Simple test of the Beargit Rm Command.
 * Reads 
 */
void oneFile_rm_test(void) {   
    unlink("TEST_STDOUT");
    unlink("TEST_STDERR");
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    int retval;
    
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

    int exists = access("TEST_STDERR", F_OK);
    CU_ASSERT(-1==exists);

}


/* Simple test of the Beargit Rm Command.
 * Reads 
 */
void twoFile_rm_test(void) {       
    unlink("TEST_STDOUT");
    unlink("TEST_STDERR");
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    int retval;

    beargit_add("test.txt");
    retval = beargit_rm("test.txt");
    CU_ASSERT(0==retval);
    beargit_add("test.txt");
    retval = beargit_rm("test.txt");
    CU_ASSERT(0==retval);

    FILE* test2 = fopen("test2.txt", "r");
    fclose(test2);
    beargit_add("test2.txt");
    beargit_add("test.txt");
    retval = beargit_rm("test2.txt");
    CU_ASSERT(0==retval);

    beargit_status();

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);

    
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
    fclose(fstdout);

    int exists = access("TEST_STDERR", F_OK);
    CU_ASSERT(-1==exists);

}


/* Basic test of the Beargit COMMIT Command.
 * Reads 
 */
void one_empty_commit_test(void) {       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    int retval;
    beargit_init();
    retval = beargit_commit("THIS IS BEAR TERRITORY!");    
    CU_ASSERT(0==retval);
    
    int exists = access("TEST_STDOUT", F_OK);
    CU_ASSERT(-1==exists);

    beargit_log(10);

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout);
    
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout));
    CU_ASSERT(!strncmp(line,"commit", strlen("commit")));
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"   THIS IS BEAR TERRITORY!", strlen("   THIS IS BEAR TERRITORY!")));
    fclose(fstdout);

    unlink("TEST_STDOUT");
    beargit_commit("THIS IS BEAR TERRITORY!");   
    beargit_log(10);

    FILE* fstdout2 = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout2);
    
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstdout2));
    CU_ASSERT(!strncmp(line,"commit", strlen("commit")));
    fgets(line, LINE_SIZE, fstdout2);
    CU_ASSERT(!strncmp(line,"   THIS IS BEAR TERRITORY!", strlen("   THIS IS BEAR TERRITORY!")));
    
    fgets(line, LINE_SIZE, fstdout2);
    CU_ASSERT(!strncmp(line,"", strlen("")));
    
    fgets(line, LINE_SIZE, fstdout2);
    CU_ASSERT(!strncmp(line,"commit", strlen("commit")));
    fgets(line, LINE_SIZE, fstdout2);
    CU_ASSERT(!strncmp(line,"   THIS IS BEAR TERRITORY!", strlen("   THIS IS BEAR TERRITORY!")));

    exists = access("TEST_STDERR", F_OK);
    CU_ASSERT(-1==exists);
    
    fclose(fstdout2);
    init_suite();
}


/* Basic test of the Beargit COMMIT Command.
 * Checks whether an added file is saved in the correct commit folders 
 */
void oneFile_commit_test(void) {
    unlink("TEST_STDOUT");
    unlink("TEST_STDERR");       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    int retval;
    beargit_init();

    // Empty first commit
    retval = beargit_commit("THIS IS BEAR TERRITORY!1");    
    CU_ASSERT(0==retval);
      
    char commit_id[COMMIT_ID_SIZE];
    read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);

    // Create a file in between the two commits
    FILE* test = fopen("test.txt", "w");
    fclose(test);
    beargit_add("test.txt");

    // Second commit including file
    retval = beargit_commit("THIS IS BEAR TERRITORY!2");    
    CU_ASSERT(0==retval);
    
    // Check that the file doesn't exist in the previous commit
    char commit_dir_file[BRANCHNAME_SIZE];
    sprintf(commit_dir_file, ".beargit/%s/test.txt", commit_id);
    int exists; 
    exists = access(commit_dir_file, F_OK);
    CU_ASSERT(-1==exists);

    // Check that the file does exist in this commit
    read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
    sprintf(commit_dir_file, ".beargit/%s/test.txt", commit_id);
    exists = access(commit_dir_file, F_OK);
    CU_ASSERT(0==exists);

    beargit_log(10);

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout); 
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"commit", strlen("commit")));
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"   THIS IS BEAR TERRITORY!2", strlen("   THIS IS BEAR TERRITORY!2")));
    fgets(line, LINE_SIZE, fstdout);
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"commit", strlen("commit")));
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"   THIS IS BEAR TERRITORY!1", strlen("   THIS IS BEAR TERRITORY!1")));
    
    fclose(fstdout);
    init_suite();
}


/* Basic test of the Beargit COMMIT Command.
 * Reads 
 */
void messages_commit_test(void) {
    unlink("TEST_STDOUT");
    unlink("TEST_STDERR");       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    int retval;
    beargit_init();
    // Add a file 
    FILE* test = fopen("test.txt", "w");
    fclose(test);
    beargit_add("test.txt");
    retval = beargit_commit("THIS IS BEAR TERRITORY!");    
    CU_ASSERT(0==retval);
      
    // Check that incorrect commit messages throw error and do not create commit folders
    char commit_id[COMMIT_ID_SIZE];
    read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
    retval = beargit_commit("THIS IS BEAR TERRITORY");    
    CU_ASSERT(1==retval);
    
    char commit_dir[BRANCHNAME_SIZE];
    sprintf(commit_dir, ".beargit/%s", commit_id);
    int exists; 
    exists = fs_check_dir_exists(commit_dir);
    CU_ASSERT(1==exists);

    beargit_commit("    THIS IS BEAR TERRITORY!    ");
    beargit_commit("THIS THIS THIS THIS IS BEAR TERRITORY!");
    beargit_commit("THIS IS BEAR TERRITORY!!!");
    beargit_commit("*&$THIS IS BEAR TERRITORY!$&*");
    beargit_commit("THIS IS THIS IS BEAR TERRITORY!TERRITORY BEAR IS THIS");
    beargit_log(5);

    FILE* fstdout = fopen("TEST_STDOUT", "r");
    CU_ASSERT_PTR_NOT_NULL(fstdout); 
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"commit", strlen("commit")));
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"   THIS IS THIS IS BEAR TERRITORY!TERRITORY BEAR IS THIS", strlen("   THIS IS THIS IS BEAR TERRITORY!TERRITORY BEAR IS THIS")));
    fgets(line, LINE_SIZE, fstdout);
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"commit", strlen("commit")));
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"   *&$THIS IS BEAR TERRITORY!$&*", strlen("   *&$THIS IS BEAR TERRITORY!$&*")));
    fgets(line, LINE_SIZE, fstdout);
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"commit", strlen("commit")));
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"   THIS IS BEAR TERRITORY!!!", strlen("   THIS IS BEAR TERRITORY!!!")));
    fgets(line, LINE_SIZE, fstdout);
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"commit", strlen("commit")));
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"   THIS THIS THIS THIS IS BEAR TERRITORY!", strlen("   THIS THIS THIS THIS IS BEAR TERRITORY!")));
    fgets(line, LINE_SIZE, fstdout);
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"commit", strlen("commit")));
    fgets(line, LINE_SIZE, fstdout);
    CU_ASSERT(!strncmp(line,"       THIS IS BEAR TERRITORY!    ", strlen("       THIS IS BEAR TERRITORY!    ")));
    
    fclose(fstdout);

    FILE* fstderr = fopen("TEST_STDERR", "r");
    CU_ASSERT_PTR_NOT_NULL(fstderr);
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
    CU_ASSERT(!strncmp(line,"ERROR:  Message must contain \"THIS IS BEAR TERRITORY!\"", strlen("ERROR:  Message must contain \"THIS IS BEAR TERRITORY!\"")));
    
    fclose(fstderr);

}


/* Basic test of the Beargit COMMIT Command.
 * Reads 
 */
void headbranch_commit_test(void) {
    unlink("TEST_STDOUT");
    unlink("TEST_STDERR");       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    int retval;
    beargit_branch();
    beargit_checkout("77dcbd31e7237ce30d041dd3857381401c7996c0", 0);
    beargit_branch();
    retval = beargit_commit("THIS IS BEAR TERRITORY!");    
    CU_ASSERT(1==retval);
    FILE* fstderr = fopen("TEST_STDERR", "r");
    CU_ASSERT_PTR_NOT_NULL(fstderr);
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
    CU_ASSERT(!strncmp(line,"ERROR:  Need to be on HEAD of a branch to commit.", strlen("ERROR:  Need to be on HEAD of a branch to commit.")));
    fclose(fstderr);

    retval = beargit_commit("THIS IS BEAR!");    
    CU_ASSERT(1==retval);

    retval = beargit_commit("THIS IS BEAR TERRITORY!");    
    CU_ASSERT(1==retval);

    // Check that you cannot create a commit even if you checkout the commit id that's the head of the branch
    beargit_checkout("9a3cd068a5be9fbd2069ee50c166b9d1cff13049", 0);
    retval = beargit_commit("THIS IS THIS IS BEAR TERRITORY!TERRITORY BEAR IS THIS");    
    CU_ASSERT(1==retval);

    // Check that switching back to the head of the branch re-enables commit
    beargit_checkout("master", 0);
    beargit_branch();
    retval = beargit_commit("THIS IS THIS IS BEAR TERRITORY!TERRITORY BEAR IS THIS");    
    CU_ASSERT(0==retval);
    
}

/* Basic test of the Beargit CHECKOUT Command.
 * Checks whether an added file has differing saved changes on different branches 
 */
void one_file_checkout_test(void) {
    unlink("TEST_STDOUT");
    unlink("TEST_STDERR");       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    int retval;
    beargit_init();

    // Write "Simon" to text file
    FILE* test = fopen("test.txt", "w");
    fclose(test);
    write_string_to_file("test.txt", "Simon");
    beargit_add("test.txt");

    // first commit before branching
    retval = beargit_commit("THIS IS BEAR TERRITORY!1");    
    CU_ASSERT(0==retval);
      
    // Check "Simon" in in text file
    FILE* ftext = fopen("test.txt", "r");
    CU_ASSERT_PTR_NOT_NULL(ftext); 
    fgets(line, LINE_SIZE, ftext);
    CU_ASSERT(!strncmp(line,"Simon", strlen("Simon")));
    fclose(ftext);

    // Checkout new branch
    beargit_checkout("2nd", 1);

    // Change text file to "Daniel" and commit the change
    write_string_to_file("test.txt", "Daniel");
    retval = beargit_commit("THIS IS BEAR TERRITORY!1");    
    CU_ASSERT(0==retval);

    // Check "Daniel" in in text file
    ftext = fopen("test.txt", "r");
    CU_ASSERT_PTR_NOT_NULL(ftext); 
    fgets(line, LINE_SIZE, ftext);
    CU_ASSERT(!strncmp(line,"Daniel", strlen("Daniel")));
    fclose(ftext);
    
    // Checkout back to master branch
    beargit_checkout("master", 0);

    // Check "Simon" in in text file
    ftext = fopen("test.txt", "r");
    CU_ASSERT_PTR_NOT_NULL(ftext); 
    fgets(line, LINE_SIZE, ftext);
    CU_ASSERT(!strncmp(line,"Simon", strlen("Simon")));
    fclose(ftext);

    // Checkout back to new branch
    beargit_checkout("2nd", 0);

    // Check "Daniel" in in text file
    ftext = fopen("test.txt", "r");
    CU_ASSERT_PTR_NOT_NULL(ftext); 
    fgets(line, LINE_SIZE, ftext);
    CU_ASSERT(!strncmp(line,"Daniel", strlen("Daniel")));
    fclose(ftext);

    retval = beargit_commit("THIS IS BEAR TERRITORY!1");    
    CU_ASSERT(0==retval);    

    // Check "Daniel" in in text file
    ftext = fopen("test.txt", "r");
    CU_ASSERT_PTR_NOT_NULL(ftext); 
    fgets(line, LINE_SIZE, ftext);
    CU_ASSERT(!strncmp(line,"Daniel", strlen("Daniel")));
    fclose(ftext);

    write_string_to_file("test.txt", "Daniel1");
    retval = beargit_commit("THIS IS BEAR TERRITORY!1");    
    CU_ASSERT(0==retval);    
    
    // Check the previous commit
    char prev[LINE_SIZE];
    read_string_from_file(".beargit/.prev", prev, LINE_SIZE);
    beargit_checkout(prev, 0);

    // Check "Daniel" in in text file
    ftext = fopen("test.txt", "r");
    CU_ASSERT_PTR_NOT_NULL(ftext); 
    fgets(line, LINE_SIZE, ftext);
    CU_ASSERT(!strncmp(line,"Daniel", strlen("Daniel")));
    fclose(ftext);
  
    // Checkout back to new branch head
    beargit_checkout("2nd", 0);
    ftext = fopen("test.txt", "r");
    CU_ASSERT_PTR_NOT_NULL(ftext); 
    fgets(line, LINE_SIZE, ftext);
    CU_ASSERT(!strncmp(line,"Daniel1", strlen("Daniel1")));
    fclose(ftext);

    init_suite();
}


/* Test of the Beargit CHECKOUT Command.
 * Checks whether an added files change on different branches 
 */
void three_files_checkout_test(void) {
    unlink("TEST_STDOUT");
    unlink("TEST_STDERR");       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    int retval;
    int exists;
    beargit_init();

    // Add a file 
    FILE* fone = fopen("1.txt", "w");
    fclose(fone);
    beargit_add("1.txt");

    // first commit 
    retval = beargit_commit("THIS IS BEAR TERRITORY!1");    
    CU_ASSERT(0==retval);
    exists = access("1.txt", F_OK);
    CU_ASSERT(0==exists);
    exists = access("2.txt", F_OK);
    CU_ASSERT(-1==exists);
    exists = access("3.txt", F_OK);
    CU_ASSERT(-1==exists);

    // Checkout new branch
    beargit_checkout("2nd", 1);

    // Add another file and commit the change
    FILE* ftwo = fopen("2.txt", "w");
    fclose(ftwo);
    beargit_add("2.txt");
    retval = beargit_commit("THIS IS BEAR TERRITORY!2");    
    CU_ASSERT(0==retval);
    exists = access("1.txt", F_OK);
    CU_ASSERT(0==exists);
    exists = access("2.txt", F_OK);
    CU_ASSERT(0==exists);
    exists = access("3.txt", F_OK);
    CU_ASSERT(-1==exists);

    // Checkout new branch
    beargit_checkout("3rd", 1);

    // Add another file and commit the change
    FILE* fthree = fopen("3.txt", "w");
    fclose(fthree);
    beargit_add("3.txt");
    retval = beargit_commit("THIS IS BEAR TERRITORY!2");    
    CU_ASSERT(0==retval);
    exists = access("1.txt", F_OK);
    CU_ASSERT(0==exists);
    exists = access("2.txt", F_OK);
    CU_ASSERT(0==exists);
    exists = access("3.txt", F_OK);
    CU_ASSERT(0==exists);
    
    // Checkout back to 2nd branch
    beargit_checkout("2nd", 0);

    // Check that only 2 files exist
    exists = access("1.txt", F_OK);
    CU_ASSERT(0==exists);
    exists = access("2.txt", F_OK);
    CU_ASSERT(0==exists);
    exists = access("3.txt", F_OK);
    CU_ASSERT(-1==exists);   
    
    // Check the previous commit
    char prev[LINE_SIZE];
    read_string_from_file(".beargit/.prev", prev, LINE_SIZE);
    beargit_checkout(prev, 0);

     // Check that only 2 files exist
    exists = access("1.txt", F_OK);
    CU_ASSERT(0==exists);
    exists = access("2.txt", F_OK);
    CU_ASSERT(0==exists);
    exists = access("3.txt", F_OK);
    CU_ASSERT(-1==exists);   
  
    // Check the master commit
    beargit_checkout("master", 0);

     // Check that only 2 files exist
    exists = access("1.txt", F_OK);
    CU_ASSERT(0==exists);
    exists = access("2.txt", F_OK);
    CU_ASSERT(-1==exists);
    exists = access("3.txt", F_OK);
    CU_ASSERT(-1==exists);   

    init_suite();
}



/* Test of the Beargit CHECKOUT Command.
 * Checks whether the appropriate error messages are triggered 
 */
void errors_checkout_test(void) {
    unlink("TEST_STDOUT");
    unlink("TEST_STDERR");       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    int retval;
    int exists;
    beargit_init();

    // Checkout new branch
    beargit_checkout("2nd", 1);

    // Add a file 
    FILE* fone = fopen("1.txt", "w");
    fclose(fone);
    beargit_add("1.txt");

    // Add another file and commit the change
    FILE* ftwo = fopen("2.txt", "w");
    fclose(ftwo);
    beargit_add("2.txt");
    retval = beargit_commit("TH1S 1S BE4R T3RR1T0RY!");    
    CU_ASSERT(1==retval);
    retval = beargit_commit("THIS IS BEAR TERRITORY!2");    
    CU_ASSERT(0==retval);

    FILE* fstderr = fopen("TEST_STDERR", "r");
    CU_ASSERT_PTR_NOT_NULL(fstderr);
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr));
    CU_ASSERT(!strncmp(line,"ERROR:  Message must contain \"THIS IS BEAR TERRITORY!\"", strlen("ERROR:  Message must contain \"THIS IS BEAR TERRITORY!\"")));
    fclose(fstderr);

    // Checkout the previous commit
    char prev[LINE_SIZE];
    read_string_from_file(".beargit/.prev", prev, LINE_SIZE);
    beargit_checkout(prev, 0);

    // Try to commit from a detached branch 
    unlink("TEST_STDERR"); 
    retval = beargit_commit("TH1S 1S BE4R T3RR1T0RY!");    
    CU_ASSERT(1==retval);
    retval = beargit_commit("THIS IS BEAR TERRITORY!2");    
    CU_ASSERT(1==retval);

    FILE* fstderr2 = fopen("TEST_STDERR", "r");
    CU_ASSERT_PTR_NOT_NULL(fstderr2);
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr2));
    CU_ASSERT(!strncmp(line,"ERROR:  Message must contain \"THIS IS BEAR TERRITORY!\"", strlen("ERROR:  Message must contain \"THIS IS BEAR TERRITORY!\"")));
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr2));
    CU_ASSERT(!strncmp(line,"ERROR:  Need to be on HEAD of a branch to commit.", strlen("ERROR:  Need to be on HEAD of a branch to commit.")));
    fclose(fstderr2);

  
    // Checkout the master commit
    unlink("TEST_STDERR"); 
    beargit_checkout("master", 0);
    retval = beargit_commit("TH1S 1S BE4R T3RR1T0RY!");    
    CU_ASSERT(1==retval);
    retval = beargit_commit("THIS IS BEAR TERRITORY!2");    
    CU_ASSERT(0==retval);
 
    FILE* fstderr3 = fopen("TEST_STDERR", "r");
    CU_ASSERT_PTR_NOT_NULL(fstderr3);
    CU_ASSERT_PTR_NOT_NULL(fgets(line, LINE_SIZE, fstderr3));
    CU_ASSERT(!strncmp(line,"ERROR:  Message must contain \"THIS IS BEAR TERRITORY!\"", strlen("ERROR:  Message must contain \"THIS IS BEAR TERRITORY!\"")));
    fclose(fstderr3);
    // init_suite();
}


/* Simple test of the Beargit Status Command.
 * Reads 
 */
void setup_function(void) {       
    const int LINE_SIZE = 512;
    char line[LINE_SIZE];

    beargit_init();
    FILE* test = fopen("test.txt", "w");
    fclose(test);
    beargit_add("test.txt");
    beargit_commit("THIS IS BEAR TERRITORY!");
    beargit_checkout("2nd", 1);
    FILE* test2 = fopen("test2.txt", "w");
    fclose(test2);
    beargit_add("test2.txt");
    beargit_commit("THIS IS BEAR TERRITORY!");

    // FILE* test3 = fopen("test3.txt", "w");
    // beargit_add("test3.txt");
    // beargit_commit("THIS IS BEAR TERRITORY!");
    // beargit_checkout("master", 0);
 }


/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int cunittester()
{
   CU_pSuite pSuite = NULL;  /* Sample#1 code provided with skeleton - simple test*/
   CU_pSuite pSuite1 = NULL; /* Sample#2 code provided with skeleton - Log */
   CU_pSuite pSuite2 = NULL; /* Suite of testing for the STATUS command */
   CU_pSuite pSuite3 = NULL; /* Suite of testing for the RM command */
   CU_pSuite pSuite4 = NULL; /* Suite of testing for the COMMIT command */
   CU_pSuite pSuite5 = NULL; /* Suite of testing for the CHECKOUT command */
   
   // CU_pSuite pSetup = NULL; /* Autoloads beargit in a state ready for manual testing */


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

   /* add SAMPLE Suite #2 to the registry */
   pSuite1 = CU_add_suite("Sample Suite_2", init_suite, clean_suite);
   if (NULL == pSuite1) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Add tests to the sample Suite #2 */
   if (NULL == CU_add_test(pSuite1, "Log output test", simple_log_test))
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
   if ((NULL == CU_add_test(pSuite3, "RM Test #1: no files to remove test", noFile_rm_test)) ||
       (NULL == CU_add_test(pSuite3, "RM Test #2: one file to remove test", oneFile_rm_test)) ||
       (NULL == CU_add_test(pSuite3, "RM Test #3: two files to remove test", twoFile_rm_test)))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

///////////////


   /* add COMMIT Suite to the registry */
   pSuite4 = CU_add_suite("COMMIT Suite", init_suite, clean_suite);
   if (NULL == pSuite4) {
      CU_cleanup_registry();
      return CU_get_error();
   }
   /* Add tests to the COMMIT Suite */
   if ((NULL == CU_add_test(pSuite4, "COMMIT Test #1: Basic one empty commit test", one_empty_commit_test)) ||
       (NULL == CU_add_test(pSuite4, "COMMIT Test #2: One file commited test", oneFile_commit_test)) ||
       (NULL == CU_add_test(pSuite4, "COMMIT Test #3: Different commit messages test", messages_commit_test)) ||
       (NULL == CU_add_test(pSuite4, "COMMIT Test #4: On head of the branch test", headbranch_commit_test)))
    
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

///////////////

   /* add CHECKOUT Suite to the registry */
   pSuite5 = CU_add_suite("CHECKOUT Suite", init_suite, clean_suite);
   if (NULL == pSuite5) {
      CU_cleanup_registry();
      return CU_get_error();
   }
   /* Add tests to the COMMIT Suite */
   if ((NULL == CU_add_test(pSuite5, "CHECKOUT Test #1: Basic one file checkout test", one_file_checkout_test)) || 
       (NULL == CU_add_test(pSuite5, "CHECKOUT Test #2: Many files checkout test", three_files_checkout_test)) ||
       (NULL == CU_add_test(pSuite5, "COMMIT Test #4: Checkout errors test", errors_checkout_test)))
   
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

///////////////




   // Checkout:
      // TEST TO SEE IF CONTENTS OF TEST.TXT and TEST2.TXT are actually different when saved
      // swap between branches with no commit
      // test detached warning

   // Log:

   // Branch:

   // Reset:

   // Merge:


   // /* add SETUP to the registry */
   // pSetup = CU_add_suite("Setup Function", init_suite, clean_suite);
   // if (NULL == pSetup) {
   //    CU_cleanup_registry();
   //    return CU_get_error();
   // }
   // /* Add tests to the RM Suite */
   // if (NULL == CU_add_test(pSetup, "Running commands to leave beargit in testing state", setup_function))
   //  /* Edge cases still to test: No files to remove, file isn't being tracked, */
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


