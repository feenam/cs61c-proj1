#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "beargit.h"
#include "util.h"

/* Implementation Notes:
 *
 * - Functions return 0 if successful, 1 if there is an error.
 * - All error conditions in the function description need to be implemented
 *   and written to stderr. We catch some additional errors for you in main.c.
 * - Output to stdout needs to be exactly as specified in the function description.
 * - Only edit this file (beargit.c)
 * - Here are some of the helper functions from util.h:
 *   * fs_mkdir(dirname): create directory <dirname>
 *   * fs_rm(filename): delete file <filename>
 *   * fs_mv(src,dst): move file <src> to <dst>, overwriting <dst> if it exists
 *   * fs_cp(src,dst): copy file <src> to <dst>, overwriting <dst> if it exists
 *   * write_string_to_file(filename,str): write <str> to filename (overwriting contents)
 *   * read_string_from_file(filename,str,size): read a string of at most <size> (incl.
 *     NULL character) from file <filename> and store it into <str>. Note that <str>
 *     needs to be large enough to hold that string.
 *  - You NEED to test your code. The autograder we provide does not contain the
 *    full set of tests that we will run on your code. See "Step 5" in the project spec.
 */

/* beargit init
 *
 * - Create .beargit directory
 * - Create empty .beargit/.index file
 * - Create .beargit/.prev file containing 0..0 commit id
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_init(void) {
  fs_mkdir(".beargit");

  FILE* findex = fopen(".beargit/.index", "w");
  fclose(findex);

  FILE* fbranches = fopen(".beargit/.branches", "w");
  fprintf(fbranches, "%s\n", "master");
  fclose(fbranches);

  write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");
  write_string_to_file(".beargit/.current_branch", "master");

  return 0;
}



/* beargit add <filename>
 *
 * - Append filename to list in .beargit/.index if it isn't in there yet
 *
 * Possible errors (to stderr):
 * >> ERROR:  File <filename> has already been added.
 *
 * Output (to stdout):
 * - None if successful
 */

int beargit_add(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      fprintf(stderr, "ERROR:  File %s has already been added.\n", filename);
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }

    fprintf(fnewindex, "%s\n", line);
  }

  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

  return 0;
}

/* beargit status
 *
 * See "Step 1" in the project spec.
 *
 */

int beargit_status() {
  FILE* findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  fprintf(stdout, "Tracked files:\n\n");

  int count = 0;
  while(fgets(line, sizeof(line), findex)) {
    count++;
    strtok(line, "\n");
    fprintf(stdout, "%s\n", line);
  }
  fprintf(stdout, "\nThere are %d files total.\n", count);
  
  fclose(findex);
  
  return 0;
}

/* beargit rm <filename>
 *
 * See "Step 2" in the project spec.
 *
 */

int beargit_rm(const char* filename) {
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  int len1 = 0;
  int len2 = 0;
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) != 0) {
      fprintf(fnewindex, "%s\n", line);
      len1++;
    }
    len2++;
  }
  if (len1 == len2) {
    fprintf(stderr, "ERROR:  File %s not tracked.\n", filename);
    fs_rm(".beargit/.newindex");
    return 1;
  }
  else {
    fclose(findex);
    fclose(fnewindex);
    fs_mv(".beargit/.newindex", ".beargit/.index");
    return 0;
  }
}

/* beargit commit -m <msg>
 *
 * See "Step 3" in the project spec.
 *
 */

const char* go_bears = "THIS IS BEAR TERRITORY!";

int is_commit_msg_ok(const char* msg) {
  int i = 0;
  char msg_char = 0; 
  char go_char = 0;
  do {
    msg_char = msg[i];
    go_char = go_bears[i];
    if (msg_char != go_char) return 0;
    i++;
  } while (msg_char != '\0' || go_char != '\0'); 
  if (msg_char == '\0' && go_char == '\0') return 1;
  return 0;
}

/* Use next_commit_id to fill in the rest of the commit ID.
 *
 * Hints:
 * You will need a destination string buffer to hold your next_commit_id, before you copy it back to commit_id
 * You will need to use a function we have provided for you.
 */

void next_commit_id(char* commit_id) {
     /* COMPLETE THE REST */
  FILE* fprevCommit = fopen(".beargit/.prev", "r");
  FILE* fbranch_name = fopen(".beargit/.current_branch", "r");
  FILE *fnewCommit = fopen(".beargit/.newCommit", "w");

  char line[COMMIT_ID_SIZE];
  char branch_name[COMMIT_ID_SIZE];
  char dst[SHA_HEX_BYTES + 1];

  fgets(line, sizeof(line), fprevCommit);
  strtok(line, "\n");

  fgets(branch_name, sizeof(branch_name), fbranch_name);
  strtok(branch_name, "\n");
  
  strcat(line, branch_name);
      // fprintf(stdout, "FUNCTION: next_commit_id:\n-----------------------\n%s sent to cryptohash.\n", line);     /* DELETE ME WHEN WORKING*/
  
  cryptohash(line, dst);
      // fprintf(stdout, "%s received from cryptohash. \n", dst);     /* DELETE ME WHEN WORKING*/

  fprintf(fnewCommit, "%s\n", dst);
  fclose(fprevCommit);
  fclose(fnewCommit);
}


int beargit_commit(const char* msg) {
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR:  Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  next_commit_id(commit_id);

  /* read the new commit hash and save it as a local variable*/
  FILE *fpassedCommit = fopen(".beargit/.newCommit", "r");
  char new_hash[COMMIT_ID_SIZE];
  fgets(new_hash, sizeof(new_hash), fpassedCommit);
  fclose(fpassedCommit);                                         /* change to write to string if it worsks */
  fs_rm(".beargit/.newCommit");
  strtok(new_hash, "\n");
  // fprintf(stdout, "\n\nFUNCTION: beargit_commit:\n-----------------------\n%s read in next_commit_id. \n", new_hash);     /* DELETE ME WHEN WORKING*/
  
  /* make a new directory for the commit and save the commit message*/
  char new_commit_name[BRANCHNAME_SIZE];
  strcpy(new_commit_name, ".beargit/");
  strcat(new_commit_name, new_hash);

  // fprintf(stdout, "\ncreating folder \n%s \n", new_commit_name);     /* DELETE ME WHEN WORKING*/

  /* make a new directory for the commit and save the commit message*/
  fs_mkdir(new_commit_name);
  char new_msg[BRANCHNAME_SIZE];
  strcpy(new_msg, new_commit_name);
  strcat(new_msg, "/.msg");
  FILE* fmsg = fopen(new_msg, "w");                            /* change to write to string if it worsks */
  fprintf(fmsg, "%s\n", msg);
  fclose(fmsg);

  /* copy over .index file and all tracked files that are listed in it*/
  char new_index[BRANCHNAME_SIZE];
  strcpy(new_index, new_commit_name);
  strcat(new_index, "/.index");                               /* THIS NEEDS CHANGING TO AN IF STATMENT FOR PREV COMMIT NAME */
  fs_cp(".beargit/.index", new_index);
  // fprintf(stdout, "\n%s saved.\n", new_index);                 /* DELETE ME WHEN WORKING*/

  FILE* findex = fopen(new_index, "r");
  char line[FILENAME_SIZE];
  int count = 0;
  char path[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    count++;
    strtok(line, "\n");
    memset(path, 0, sizeof(path));
    strcpy(path, new_commit_name);
    strcat(path, "/");
    strcat(path, line); 
    fs_cp(line, path);
    // fprintf(stdout, "\n%s saved.", path);                      /* DELETE ME WHEN WORKING*/
  }

  // fprintf(stdout, "\n%d tracked files copied across.\n", count);   /* DELETE ME WHEN WORKING*/


  /* copy over .prev file and (over)write the new commit ID into it*/
  char new_prev[BRANCHNAME_SIZE];
  strcpy(new_prev, new_commit_name);
  strcat(new_prev, "/.prev");
  fs_cp(".beargit/.prev", new_prev);
  // fprintf(stdout, "\n%s saved.", new_prev);                      /* DELETE ME WHEN WORKING*/
  write_string_to_file(".beargit/.prev", new_hash);
  // fprintf(stdout, "\n%s (over)written to .beargit/.prev.", new_hash);    /* DELETE ME WHEN WORKING*/



  // FILE* fbranches = fopen(".beargit/.branches", "w");
  // fclose(fbranches);

  // write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");
  // write_string_to_file(".beargit/.current_branch", "master");

  /* COMPLETE THE REST */

 


  // if (branch_head_commit_id == commit_id) {
    // WRITE A CHECK TO SEE IF THE BRANCH HEAD IS THE SAME AS CURRENT COMMIT ID
    // fprintf(stderr, "ERROR:  Need to be on HEAD of a branch to commit");
  // }

  return 0;
}

/* beargit log
 *
 * See "Step 4" in the project spec.
 *
 */

int beargit_log(int limit) {
  /* COMPLETE THE REST */
  return 0;
}


// This helper function returns the branch number for a specific branch, or
// returns -1 if the branch does not exist.
int get_branch_number(const char* branch_name) {
  FILE* fbranches = fopen(".beargit/.branches", "r");

  int branch_index = -1;
  int counter = 0;
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), fbranches)) {
    strtok(line, "\n");
    if (strcmp(line, branch_name) == 0) {
      branch_index = counter;
    }
    counter++;
  }

  fclose(fbranches);

  return branch_index;
}

/* beargit branch
 *
 * See "Step 5" in the project spec.
 *
 */

int beargit_branch() {
  /* COMPLETE THE REST */

  return 0;
}

/* beargit checkout
 *
 * See "Step 6" in the project spec.
 *
 */

int checkout_commit(const char* commit_id) {
  /* COMPLETE THE REST */
  return 0;
}

int is_it_a_commit_id(const char* commit_id) {
  /* COMPLETE THE REST */
  return 1;
}

int beargit_checkout(const char* arg, int new_branch) {
  // Get the current branch
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", "current_branch", BRANCHNAME_SIZE);

  // If not detached, update the current branch by storing the current HEAD into that branch's file...
  if (strlen(current_branch)) {
    char current_branch_file[BRANCHNAME_SIZE+50];
    sprintf(current_branch_file, ".beargit/.branch_%s", current_branch);
    fs_cp(".beargit/.prev", current_branch_file);
  }

  // Check whether the argument is a commit ID. If yes, we just stay in detached mode
  // without actually having to change into any other branch.
  if (is_it_a_commit_id(arg)) {
    char commit_dir[FILENAME_SIZE] = ".beargit/";
    strcat(commit_dir, arg);
    if (!fs_check_dir_exists(commit_dir)) {
      fprintf(stderr, "ERROR:  Commit %s does not exist.\n", arg);
      return 1;
    }

    // Set the current branch to none (i.e., detached).
    write_string_to_file(".beargit/.current_branch", "");

    return checkout_commit(arg);
  }

  // Just a better name, since we now know the argument is a branch name.
  const char* branch_name = arg;

  // Read branches file (giving us the HEAD commit id for that branch).
  int branch_exists = (get_branch_number(branch_name) >= 0);

  // Check for errors.
  if (!(!branch_exists || !new_branch)) {
    fprintf(stderr, "ERROR:  A branch named %s already exists.\n", branch_name);
    return 1;
  } else if (!branch_exists && new_branch) {
    fprintf(stderr, "ERROR: No branch %s exists\n", branch_name);
    return 1;
  }

  // File for the branch we are changing into.
  char* branch_file = ".beargit/.branch_";
  strcat(branch_file, branch_name);

  // Update the branch file if new branch is created (now it can't go wrong anymore)
  if (new_branch) {
    FILE* fbranches = fopen(".beargit/.branches", "a");
    fprintf(fbranches, "%s\n", branch_name);
    fclose(fbranches);
    fs_cp(".beargit/.prev", branch_file);
  }

  write_string_to_file(".beargit/.current_branch", branch_name);

  // Read the head commit ID of this branch.
  char branch_head_commit_id[COMMIT_ID_SIZE];
  read_string_from_file(branch_file, branch_head_commit_id, COMMIT_ID_SIZE);

  // Check out the actual commit.
  return checkout_commit(branch_head_commit_id);
}

/* beargit reset
 *
 * See "Step 7" in the project spec.
 *
 */

int beargit_reset(const char* commit_id, const char* filename) {
  if (!is_it_a_commit_id(commit_id)) {
      fprintf(stderr, "ERROR:  Commit %s does not exist.\n", commit_id);
      return 1;
  }

  // Check if the file is in the commit directory
  /* COMPLETE THIS PART */

  // Copy the file to the current working directory
  /* COMPLETE THIS PART */

  // Add the file if it wasn't already there
  /* COMPLETE THIS PART */

  return 0;
}

/* beargit merge
 *
 * See "Step 8" in the project spec.
 *
 */

int beargit_merge(const char* arg) {
  // Get the commit_id or throw an error
  char commit_id[COMMIT_ID_SIZE];
  if (!is_it_a_commit_id(arg)) {
      if (get_branch_number(arg) == -1) {
            fprintf(stderr, "ERROR:  No branch or commit %s exists.\n", arg);
            return 1;
      }
      char branch_file[FILENAME_SIZE];
      snprintf(branch_file, FILENAME_SIZE, ".beargit/.branch_%s", arg);
      read_string_from_file(branch_file, commit_id, COMMIT_ID_SIZE);
  } else {
      snprintf(commit_id, COMMIT_ID_SIZE, "%s", arg);
  }

  // Iterate through each line of the commit_id index and determine how you
  // should copy the index file over
   /* COMPLETE THE REST */

  return 0;
}