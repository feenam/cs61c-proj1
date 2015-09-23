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
    fclose(findex);
    fclose(fnewindex);
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
  cryptohash(line, dst);
  
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

  // Check we're not dealing with the initial commit
  if (strcmp(commit_id, initial_commit)){
    
  // Check on head of the branch before allowing commit
    if (strlen(branch_name) < 3 ) {
      fprintf(stderr, "ERROR:  Need to be on HEAD of a branch to commit.\n");
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
  next_commit_id(commit_id);                               

  /* read the new commit hash and save it as a local variable*/
  FILE *fpassedCommit = fopen(".beargit/.newCommit", "r");
  char new_hash[COMMIT_ID_SIZE];
  fgets(new_hash, sizeof(new_hash), fpassedCommit);
  fclose(fpassedCommit);                                        
  fs_rm(".beargit/.newCommit");
  strtok(new_hash, "\n");
  
  /* make a new directory for the commit and save the commit message*/
  char new_commit_name[BRANCHNAME_SIZE+50];
  sprintf(new_commit_name, ".beargit/%s", new_hash);

  /* make a new directory for the commit and save the commit message*/
  fs_mkdir(new_commit_name);
  char new_msg[BRANCHNAME_SIZE];
  sprintf(new_msg, "%s/.msg", new_commit_name);
  FILE* fmsg = fopen(new_msg, "w");                            
  fprintf(fmsg, "%s\n", msg);
  fclose(fmsg);

  /* copy over .index file and all tracked files that are listed in it*/
  char new_index[BRANCHNAME_SIZE];
  sprintf(new_index, "%s/.index", new_commit_name);                            
  fs_cp(".beargit/.index", new_index);
 
  FILE* findex = fopen(".beargit/.index", "r");
  char line[FILENAME_SIZE];
  // int count = 0;
  char path[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
   
    strtok(line, "\n");
    sprintf(path, "%s/%s", new_commit_name, line);
    fs_cp(line, path);
  }
  fclose(findex);

  /* copy over .prev file and (over)write the new commit ID into it*/
  char new_prev[BRANCHNAME_SIZE];
  sprintf(new_prev, "%s/.prev", new_commit_name);
  fs_cp(".beargit/.prev", new_prev);
  write_string_to_file(".beargit/.prev", new_hash);

  /* create .branch_<branchname> and copy over .prev file */
  fs_cp(".beargit/.prev", branch_file);

  return 0;
}

/* beargit log
 *
 * See "Step 4" in the project spec.
 *
 */

int beargit_log(int limit) {
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
  fclose(fcommit_dir_index);
  
  if (count1 == count2) {
    return 0;
  }
  return count2;
}


int checkout_commit(const char* commit_id) {
  char initial_commit[COMMIT_ID_SIZE] = "0000000000000000000000000000000000000000";
  char current_index[FILENAME_SIZE] = ".beargit/.index";  
  char commit_dir[FILENAME_SIZE];           
  sprintf(commit_dir, ".beargit/%s/", commit_id); 
  
  /* Clear out all files listed in the current index file so we can replace them*/
  FILE* findex = fopen(current_index, "r");
  
  char line[FILENAME_SIZE];
  while(fgets(line, sizeof(line), findex)) {
    strtok(line, "\n");
    fs_rm(line);
  }
  fs_rm(current_index);
  fclose(findex);
 
  /* Check if new commit is not the 00.0 commit */
  if (strcmp(commit_id, initial_commit)) {
    char commitIndex [FILENAME_SIZE];
    sprintf(commitIndex, "%s.index", commit_dir);  
 
    /* Copy all that commit's tracked files from the commit's directory into the current directory */
    FILE* fnewIndex = fopen(commitIndex, "r");
    char line2 [FILENAME_SIZE];
    while(fgets(line2, sizeof(line), fnewIndex)) {
      strtok(line2, "\n");
      char commitFile[FILENAME_SIZE];
      sprintf(commitFile, "%s%s", commit_dir, line2);                    
      fs_cp(commitFile, line2);
    }
    /* Copy the index from the commit that is being checked out to the .beargit directory */
    fs_cp(commitIndex, current_index);
    fclose(fnewIndex);  
  }
  else {
    FILE* findex2 = fopen(current_index, "w");
    fclose(findex2);
  }
 
  /* Write the new ID into .beargit/.prev. */
  write_string_to_file(".beargit/.prev", commit_id);
 
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
  char temp[BRANCHNAME_SIZE];
  sprintf(preCommit, ".beargit/%s/%s", commit_id, filename);
  sprintf(temp, "./%s", filename);
  strtok(preCommit, "\n");
  strtok(temp, "\n");
  fs_cp(preCommit, temp);

  // Add the file if it wasn't already there
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
  char comIndex[COMMIT_ID_SIZE+50];
  snprintf(comIndex, COMMIT_ID_SIZE+50, ".beargit/%s/.index", commit_id);
  FILE* fcomIndex = fopen(comIndex, "r");

  char fline[COMMIT_ID_SIZE+50];  
  char line[BRANCHNAME_SIZE];
  char line1[BRANCHNAME_SIZE];
  char temp1[COMMIT_ID_SIZE+50];
  char temp2[COMMIT_ID_SIZE+50];
  int trig = 0;
 
  while(fgets(fline, sizeof(fline), fcomIndex)) {
    strtok(fline, "\n");
    FILE* workIndex = fopen(".beargit/.index", "r");
    while(fgets(line, sizeof(line), workIndex)) {
      strtok(line, "\n");
      if (strcmp(line, fline) == 0) {
        sprintf(temp1, ".beargit/%s/%s", commit_id, fline);
        sprintf(temp2, "./%s.%s", fline, commit_id);
        fs_cp(temp1, temp2);
        fprintf(stdout, "%s conflicted copy created\n", fline);
        trig++;
      }
    }
    fclose(workIndex);
    if(trig == 0) {
      sprintf(temp1, ".beargit/%s/%s", commit_id, fline);
      sprintf(temp2, "./%s", fline);
      fs_cp(temp1, temp2);
      fprintf(stdout, "%s added\n", fline);
      FILE* index = fopen(".beargit/.index", "r");
      FILE* newindex = fopen(".beargit/.newindex", "w");
      while(fgets(line1, sizeof(line1), index)) {
        strtok(line1, "\n");
        if (strcmp(line1, fline) == 0) {
          fclose(index);
          fclose(newindex);
          fs_rm(".beargit/.newindex");
          return 0;
        }
        fprintf(newindex, "%s\n", line1);
      }
      fprintf(newindex, "%s\n", fline);
      fclose(index);
      fclose(newindex);
      fs_mv(".beargit/.newindex", ".beargit/.index"); 
    }
    trig = 0; 
    
  }
  fclose(fcomIndex);
}