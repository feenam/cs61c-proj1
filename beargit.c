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
  int j = 0;
  char msg_char = 0; 
  char go_char = 0;
  
  do {
    msg_char = msg[i];
    go_char = go_bears[j];
    if (go_char == '\0') {
      return 1;
    }
    if (msg_char == go_char) {
      j++;
    }
    else {
      j = 0;
      // Account for reset shaving first letter off
      if (msg[i] == go_bears[j]){
        i--;
      }
    }
    i++;
  } while (msg_char != '\0'); 
  
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
  fclose(fbranch_name);
  fclose(fnewCommit);
}


int beargit_commit(const char* msg) {
  /* check that the commit message is okay with is_commit_msg_okay function */
  if (!is_commit_msg_ok(msg)) {
    fprintf(stderr, "ERROR:  Message must contain \"%s\"\n", go_bears);
    return 1;
  }

  /* check that you are on the head of the branch */
  char commit_id[COMMIT_ID_SIZE];
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  char* initial_commit = "0000000000000000000000000000000000000000";

  char branch_name[BRANCHNAME_SIZE];
  char branch_file[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", branch_name, BRANCHNAME_SIZE);
  sprintf(branch_file, ".beargit/.branch_%s", branch_name);  

  // fprintf(stdout, "\ncommit_id =      %s\n", commit_id);                 /* DELETE ME WHEN WORKING*/
  // fprintf(stdout, "initial_commit = %s\n", initial_commit);                 /* DELETE ME WHEN WORKING*/
  
  // Check we're not dealing with the initial commit
  if (strcmp(commit_id, initial_commit)){
    // char branch_head_commit_id[COMMIT_ID_SIZE];
    // read_string_from_file(branch_file, branch_head_commit_id, COMMIT_ID_SIZE);
    
  // Check on head of the branch before allowing commit
    if (strlen(branch_name) < 3 ) {
      fprintf(stderr, "ERROR:  Need to be on HEAD of a branch to commit.");
      return 1;
    }
  /* cover for special case where checkout occurs before .branch_master is created */
  } else {
    FILE *fbranches = fopen(".beargit/.branches", "r");
    int counter = 0;
    char line[FILENAME_SIZE];
    while(fgets(line, sizeof(line), fbranches)) {
      counter++;
    }
    if (counter == 1){
      fs_cp(".beargit/.prev", ".beargit/.branch_master");    
    }
    fclose(fbranches);
  }

  /* get the next commit ID with the next_commit_id function*/
  next_commit_id(commit_id);                                      /* CHANGE THIS TO A POINTER SO IT CAN BE UPDATED WITHOUT A STRING BUFFER?*/

  /* read the new commit hash and save it as a local variable*/
  FILE *fpassedCommit = fopen(".beargit/.newCommit", "r");
  char new_hash[COMMIT_ID_SIZE];
  fgets(new_hash, sizeof(new_hash), fpassedCommit);
  fclose(fpassedCommit);                                         /* change to write to string if it worsks */
  fs_rm(".beargit/.newCommit");
  strtok(new_hash, "\n");
  // fprintf(stdout, "\n\nFUNCTION: beargit_commit:\n-----------------------\n%s read in next_commit_id. \n", new_hash);     /* DELETE ME WHEN WORKING*/
  
  /* make a new directory for the commit and save the commit message*/
  char new_commit_name[BRANCHNAME_SIZE+50];
  sprintf(new_commit_name, ".beargit/%s", new_hash);

  // fprintf(stdout, "\ncreating folder \n%s \n", new_commit_name);     /* DELETE ME WHEN WORKING*/

  /* make a new directory for the commit and save the commit message*/
  fs_mkdir(new_commit_name);
  char new_msg[BRANCHNAME_SIZE];
  sprintf(new_msg, "%s/.msg", new_commit_name);
  FILE* fmsg = fopen(new_msg, "w");                            /* change to write to string if it worsks */
  fprintf(fmsg, "%s\n", msg);
  fclose(fmsg);

  /* copy over .index file and all tracked files that are listed in it*/
  char new_index[BRANCHNAME_SIZE];
  sprintf(new_index, "%s/.index", new_commit_name);                            /* THIS NEEDS CHANGING TO AN IF 
                                                                            STATMENT FOR PREV COMMIT NAME */
  fs_cp(".beargit/.index", new_index);
  // fprintf(stdout, "\n%s saved.\n", new_index);                 /* DELETE ME WHEN WORKING*/

  FILE* findex = fopen(new_index, "r");
  char line[FILENAME_SIZE];
  int count = 0;
  char path[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    count++;
    strtok(line, "\n");
    memset(path, 0, sizeof(path));                            /* MEMORY LEAK HERE? BETTER WAY OF DOING IT IN THE STACK?*/
    sprintf(path, "%s/", new_commit_name);
    strcat(path, line); 
    fs_cp(line, path);
    // fprintf(stdout, "\n%s saved.", path);                      /* DELETE ME WHEN WORKING*/
  }
  fclose(findex);

  // fprintf(stdout, "\n%d tracked files copied across.\n", count);   /* DELETE ME WHEN WORKING*/


  /* copy over .prev file and (over)write the new commit ID into it*/
  char new_prev[BRANCHNAME_SIZE];
  sprintf(new_prev, "%s/.prev", new_commit_name);
  fs_cp(".beargit/.prev", new_prev);
  // fprintf(stdout, "\n%s saved.", new_prev);                      /* DELETE ME WHEN WORKING*/
  write_string_to_file(".beargit/.prev", new_hash);
  // fprintf(stdout, "\n%s (over)written to .beargit/.prev.", new_hash);    /* DELETE ME WHEN WORKING*/


  /* create .branch_<branchname> and copy over .prev file */
  fs_cp(".beargit/.prev", branch_file);
  // fprintf(stdout, "\n%s saved.", branch_file);                      /* DELETE ME WHEN WORKING*/

  return 0;
}

/* beargit log
 *
 * See "Step 4" in the project spec.
 *
 */

int beargit_log(int limit) {
  /* COMPLETE THE REST */
  int cond = 1;
  int count = 0;
  char commit_id[COMMIT_ID_SIZE];
  char msg[BRANCHNAME_SIZE];
  char temp1[BRANCHNAME_SIZE];  
  char temp2[BRANCHNAME_SIZE] = ".beargit/";
  read_string_from_file(".beargit/.prev", commit_id, COMMIT_ID_SIZE);
  strcat(temp2, commit_id);
  strcat(temp2, "/.msg");
  while (cond > 0 && count < limit) {
    if (strcmp(commit_id, "0000000000000000000000000000000000000000") == 0) {
      fprintf(stderr, "ERROR:  There are no commits.\n");
      return 1;
    }

    else {
      read_string_from_file(temp2, msg, BRANCHNAME_SIZE);
      strtok(msg, "\n");
      printf("commit ");
      fprintf(stdout, commit_id, COMMIT_ID_SIZE);
      fprintf(stdout, "\n   ");
      fprintf(stdout, msg, BRANCHNAME_SIZE);
      fprintf(stdout, "\n");
      fprintf(stdout, "\n");
      sprintf(temp1, ".beargit/%s/.prev", commit_id);
      read_string_from_file(temp1, commit_id, COMMIT_ID_SIZE);
      sprintf(temp2, ".beargit/%s/.msg", commit_id);
      
      if (strcmp(commit_id, "0000000000000000000000000000000000000000") == 0) {
        cond = -1;
      }
    }
    count++;
  }
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

      // fprintf(stdout, "\nget_branch_number function returns: %d", branch_index);                      /* DELETE ME WHEN WORKING*/
  
  return branch_index;
}

/* beargit branch
 *
 * See "Step 5" in the project spec.
 *
 */

int beargit_branch() {
  FILE* fbranches = fopen(".beargit/.branches", "r");
  char line[FILENAME_SIZE];
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);
  
  while(fgets(line, sizeof(line), fbranches)) {
    strtok(line, "\n");
    if (*line == *current_branch) {
      fprintf(stdout, "*  %s\n", line);
    } else fprintf(stdout, "   %s\n", line);
  }
  
  fclose(fbranches);
  return 0;
}

/* beargit checkout
 *
 * See "Step 6" in the project spec.
 *
 */

/* Helper function to deal with files that have been added but not yet commited */
int current_index_counter(){
  FILE* fcurrent_index = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  int count = 0;
  while(fgets(line, FILENAME_SIZE, fcurrent_index)) {
    strtok(line, "\n");
    count++;
  }
  // fprintf(stdout, "\ncurrent index has %d .\n", count);                     /* DELETE ME WHEN WORKING*/
  fclose(fcurrent_index);
  return count;
}

/* Helper function to deal with files that have been added but not yet commited */
int index_counter(const char* commit_id) {
  // IF THERE ARE FILES IN THE .INDEX THAT ARE NOT IN THE .INDEX FILE OF COMMIT_DIR, 
  // COPY THEM INTO THE NEW COMMIT_DIR   
  char newCommit[FILENAME_SIZE];
  read_string_from_file(".beargit/.prev", newCommit, FILENAME_SIZE);

  int count1 = 0;
  count1 = current_index_counter();
  
  char commit_dir_index[FILENAME_SIZE] = ".beargit/";
  strcat(commit_dir_index, newCommit);
  strcat(commit_dir_index, "/.index");
  FILE* fcommit_dir_index = fopen(commit_dir_index, "r");
  char line2[FILENAME_SIZE];
  int count2 = 0;
  while(fgets(line2, sizeof(line2), fcommit_dir_index)) {
    strtok(line2, "\n");
    count2++;
  }
  // fprintf(stdout, "commit index has %d .\n", count2);                       /* DELETE ME WHEN WORKING*/
  fclose(fcommit_dir_index);
  
  if (count1 == count2) {
  // fprintf(stdout, "\nno untracked adds to copy");                       /* DELETE ME WHEN WORKING*/
    return 0;
  }
  return count2;
}

int checkout_commit(const char* commit_id) {
  char* initial_commit = "0000000000000000000000000000000000000000";
  int extra_adds = 0;
  
  /* Update the commit ID stored in .prev*/
  FILE* fnewPrev = fopen(".beargit/.prev", "w");
  char prevID[COMMIT_ID_SIZE];
  sprintf(prevID, "%s", commit_id);
  prevID[strlen(commit_id)] = '\0';
    // fprintf(stdout, "writing new commit to .prev: %s", prevID);                       /* DELETE ME WHEN WORKING*/
  
  fprintf(fnewPrev,  "%s", prevID);
  fclose(fnewPrev);

  // if (*commit_id != *initial_commit){
  //   char newPrev[FILENAME_SIZE];
  //   sprintf(newPrev, ".beargit/%s/.prev", commit_id);
  //   fs_cp(".beargit/.prev", newPrev);  
  // } else {
  //   FILE* fprev = fopen(".beargit/.prev", "w");
  //   fclose(fprev);
  //   write_string_to_file(".beargit/.prev", "0000000000000000000000000000000000000000");
  // }

  /*  IF THERE ARE FILES IN THE .INDEX THAT ARE NOT IN THE .INDEX FILE OF COMMIT_DIR, 
      COPY THEM INTO THE NEW COMMIT_DIR   */
  if (*commit_id != *initial_commit) {
    extra_adds = index_counter(commit_id);
  } else if (current_index_counter()){
    // fprintf(stdout, "exiting checkout_commit at line 480");                       /* DELETE ME WHEN WORKING*/
      return 0;
  }  
  
  // // fprintf(stdout, "%d", extra_adds);                       /* DELETE ME WHEN WORKING*/
  // if (extra_adds > 0){
  //   // fprintf(stdout, " - passed index if:\n");                       /* DELETE ME WHEN WORKING*/

  //   char commit_dir[FILENAME_SIZE] = ".beargit/";
  //   strcat(commit_dir, commit_id);
  //   strcat(commit_dir, "/");

  //   char commit_dir_index[FILENAME_SIZE] = "";
  //   strcpy(commit_dir_index, commit_dir);
  //   strcat(commit_dir_index, ".index");
  //   FILE* fcommit_dir_index = fopen(commit_dir_index, "a");

  //   FILE* fcurrent_index = fopen(".beargit/.index", "r");
  //   char line[FILENAME_SIZE];
  //   int count = 0;
  //   char tempa [FILENAME_SIZE];
  //   while(fgets(line, sizeof(line), fcurrent_index)) {
  //     strtok(line, "\n");
  //     if (count >= extra_adds) {
  //       sprintf(tempa, "%s", commit_dir); 
  //       strcat(tempa, line);    
  //   // fprintf(stdout, "saving %s to ", line);                       /* DELETE ME WHEN WORKING*/
  //   // fprintf(stdout, "%s folder", tempa);                       /* DELETE ME WHEN WORKING*/
  //       fs_cp(line, tempa);
  //       fprintf(fcommit_dir_index,  "%s\n", line);
  //     }
  //     count++;
  //   }
  //   fclose(fcommit_dir_index);  
  //   fclose(fcurrent_index);  
  // }

  /* Clear out all files listed in the current index file so we can replace them*/
  FILE* findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];

  // fprintf(stdout, "\nRemoving tracked files from old index:\n");                      /* DELETE ME WHEN WORKING*/

  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    // fprintf(stdout, "  deleting: %s\n", line);                     /* DELETE ME WHEN WORKING*/
    fs_rm(line);
  }
  fclose(findex);
  
  /* Check if new commit is 00.0 commit */
  if (*commit_id == *initial_commit) {
    FILE* findex2 = fopen(".beargit/.index", "w");
    fclose(findex2);
    return 0;
  }

  char commit_dir[FILENAME_SIZE] = ".beargit/";
  strcat(commit_dir, commit_id);
  
  // fprintf(stdout, "\nReinstating commit id: %s", commit_id);                      /* DELETE ME WHEN WORKING*/
  /* Copy the index from the commit that is being checked out to the .beargit directory */
  char commit_index[FILENAME_SIZE];
  strcpy (commit_index, commit_dir);
  strcat(commit_index, "/.index");
  // fprintf(stdout, "\ncopying index file to working dir from: %s", commit_index);                      /* DELETE ME WHEN WORKING*/
  fs_cp(commit_index, ".beargit/.index");

  /* Copy all that commit's tracked files from the commit's directory into the current directory */
  FILE* fnewIndex = fopen(".beargit/.index", "r");
  // fprintf(stdout, "\nCopying across tracked files from new index:\n");                      /* DELETE ME WHEN WORKING*/
  char temp [FILENAME_SIZE];
  while(fgets(line, sizeof(line), fnewIndex)) {
    strtok(line, "\n");
    // fprintf(stdout, "  copying: %s\n", line);                     /* DELETE ME WHEN WORKING*/ 
    sprintf(temp, "%s/", commit_dir);                       /* PROBLEM HERE? MAYBE IT'S NOT OVERWRITING TEMP*/
    strcat(temp, line);
    fs_cp(temp, line);
  }
  fclose(fnewIndex);

  return 0;
}

int is_it_a_commit_id(const char* commit_id) {
  char commit_dir[BRANCHNAME_SIZE+50];
  sprintf(commit_dir, ".beargit/%s", commit_id);
  return fs_check_dir_exists(commit_dir);
}

int beargit_checkout(const char* arg, int new_branch) {
  // Get the current branch
  char current_branch[BRANCHNAME_SIZE];
  read_string_from_file(".beargit/.current_branch", current_branch, BRANCHNAME_SIZE);
      

  // If not detached, update the current branch by storing the current HEAD into that branch's file...
  if (strlen(current_branch)) {
    char current_branch_file[BRANCHNAME_SIZE+50];
    sprintf(current_branch_file, ".beargit/.branch_%s", current_branch);
    fs_cp(".beargit/.prev", current_branch_file);
  }

  // Check whether the argument is a commit ID. If yes, we just stay in detached mode
  // without actually having to change into any other branch.
  if (is_it_a_commit_id(arg)) {
    
      // fprintf(stdout, "\nrecognised as commit ID");                      /* DELETE ME WHEN WORKING */
    char commit_dir[FILENAME_SIZE] = ".beargit/";
    strcat(commit_dir, arg);
    if (!fs_check_dir_exists(commit_dir)) {
      fprintf(stderr, "ERROR:  Commit %s does not exist.\n", arg);        /* THIS CODE IS REDUNDANT */
      return 1;
    }

    // Set the current branch to none (i.e., detached).
    write_string_to_file(".beargit/.current_branch", "");

    return checkout_commit(arg);
  }

  // Just a better name, since we now know the argument is a branch name.
  const char* branch_name = arg;

      // fprintf(stdout, "\nbranch_exists = %d", branch_exists);                      /* DELETE ME WHEN WORKING*/
      // fprintf(stdout, "\nnew branch = %d", new_branch);                      /* DELETE ME WHEN WORKING*/
      // fprintf(stdout, "\ntest");                      /* DELETE ME WHEN WORKING*/
      // fprintf(stdout, "\nbranch_name = %s", branch_name);                      /* DELETE ME WHEN WORKING*/
      // fprintf(stdout, "\ntest2");                      /* DELETE ME WHEN WORKING*/
      // fprintf(stdout, "\narg = %s", arg);                      /* DELETE ME WHEN WORKING*/
      // fprintf(stdout, "\nbranch_name = %s", branch_name);                      /* DELETE ME WHEN WORKING*/
      // fprintf(stdout, "\ntest3");                      /* DELETE ME WHEN WORKING*/

  // Read branches file (giving us the HEAD commit id for that branch).
  int branch_exists = (get_branch_number(branch_name) >= 0);

  // Check for errors:
  /* if it's a new branch and the branch exists */
  if ((new_branch) && (branch_exists)) {
    fprintf(stderr, "ERROR:  A branch named %s already exists.\n", branch_name);
    return 1;
  } 
  /* if it's not a new branch and the branch does not exist */
  if ((!new_branch) && (!branch_exists)) {
    fprintf(stderr, "ERROR:  No branch or commit %s exists.\n", branch_name);
    return 1;
  }


  // File for the branch we are changing into.
  char branch_file[FILENAME_SIZE] = ".beargit/.branch_";
  strcat(branch_file, branch_name);

      // fprintf(stdout, "\n%s", branch_file);                      /* DELETE ME WHEN WORKING*/
      // fprintf(stdout, "\ntest4");                      /* DELETE ME WHEN WORKING*/

  // Update the branch file if new branch is created (now it can't go wrong anymore)
  if (new_branch) {
    FILE* fbranches = fopen(".beargit/.branches", "a");
    fprintf(fbranches, "%s\n", branch_name);
    fclose(fbranches);
    fs_cp(".beargit/.prev", branch_file);
  }

  write_string_to_file(".beargit/.current_branch", branch_name);

  // /* if branch's head is the 00.0 commit */
  // char master_dir[FILENAME_SIZE] = ".beargit/branch_master";
  // if (!fs_check_dir_exists(master_dir)) {

  // }

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
  char preCommit[COMMIT_ID_SIZE+50];
  sprintf(preCommit, ".beargit/%s/.index", commit_id);
  int count = 0;
  FILE* commitIndex = fopen(preCommit, "r");

  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), commitIndex)) {
    strtok(line, "\n");
    if (strcmp(line, filename) == 0) {
      count++;
    }
  }
  if (count == 0) {
    fprintf(stderr, "ERROR:  %s is not in the index of commit %s.\n", filename, commit_id);
    return 1;
  }

  fclose(commitIndex);
//------------------


  // Copy the file to the current working directory
  /* COMPLETE THIS PART */
  char temp[BRANCHNAME_SIZE];
  sprintf(preCommit, ".beargit/%s/%s", commit_id, filename);
  sprintf(temp, "./%s", filename);
  strtok(preCommit, "\n");
  strtok(temp, "\n");
  fs_cp(preCommit, temp);

  // Add the file if it wasn't already there
  /* COMPLETE THIS PART */ 
  FILE* findex = fopen(".beargit/.index", "r");
  FILE *fnewindex = fopen(".beargit/.newindex", "w");

  char fline[BRANCHNAME_SIZE];
  while(fgets(fline, sizeof(fline), findex)) {
    strtok(fline, "\n");
    if (strcmp(fline, filename) == 0) {
      fclose(findex);
      fclose(fnewindex);
      fs_rm(".beargit/.newindex");
      return 3;
    }
    fprintf(fnewindex, "%s\n", fline);
  }
  fprintf(fnewindex, "%s\n", filename);
  fclose(findex);
  fclose(fnewindex);

  fs_mv(".beargit/.newindex", ".beargit/.index");

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