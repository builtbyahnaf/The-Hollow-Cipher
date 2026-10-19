#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_LEVELS 2

static bool level_states[MAX_LEVELS] = {false};

void load_progress(){
    FILE *pfile = fopen("data/progress.thcdb", "r");

    if(pfile != NULL){
        if(is_signature_valid(pfile)){
            int level_index, state;

            while (fscanf(pfile, "%d %d", &level_index, &state)){
                if (level_index >= 0 && level_index <= MAX_LEVELS || (state == 0 || state == 1) ){
                    if(level_index == 0) {
                        //Harcoding so that the first level is always unlocked
                        level_states[0] = true;
                    } else {
                        level_states[level_index] = (state) ? true : false;
                    }
                    
                } else {
                    //This logic id for the cases where it might read unexpected values or broken characters implying a corrupted data
                    printf("The progress data is corrupted.");
                    fclose(pfile);
                    break;
                }
            }
            fclose(pfile);
        }
    } else if (ferror(pfile)){
        //Case: If compiler returns a file reading error
        fclose(pfile);
        printf("Unknown error while loading the file.");
    } else {
        //Case: The file could not be found and hence the file pointer returned null
        printf("Error reading file. The progress data is either missing or corrupted.");
    }
}
bool is_signature_valid(FILE *pfile){
    char signature[10];
    bool sec_stat = false;

    if(fscanf(pfile, "%9s", signature) != 1 || signature[0] == '\0'){
        fclose(pfile);
        printf("Error: Failed to check signature.");
    }

    if (strcmp(signature, "THCDB_V1") == 0){
        sec_stat = true;
    } else{
        fclose(pfile);
        printf("Invalid signature detected");
    }
    return sec_stat;
}

bool is_level_unllocked(int level_index){
    if (level_index < 0 || level_index > MAX_LEVELS){
        //Invalid index passing
        //This check exists entirely for debugging purposes
        printf("Error: Invalid level_index was passed to is_level_unlocked()");
    } else{
        return level_states[level_index];
    }
}

//This function should only be accessed via play_screen.c
void set_level_unlocked(int level_index){
    if (level_index < 0 || level_index > MAX_LEVELS){
        //Invalid index passing
        //This check exists entirely for debugging purposes
        printf("Error: Invalid level_index was passed to set_level_unlocked()");
    } else{
        //Updating the array first
        level_states[level_index] = true;

        //Now rewriting the progress.thcdb to update progress
        FILE *up_file = fopen("data/progress.thcdb", "w");
        fclose(up_file);
    }
}