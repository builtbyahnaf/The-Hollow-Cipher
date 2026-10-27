#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "vault_pass.h"

static bool seeded = false;

int generate_pass(void) {
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    int digits[10] = {0,1,2,3,4,5,6,7,8,9};
    for (int i = 9; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = digits[i];
        digits[i] = digits[j];
        digits[j] = temp;
    }

    int result = 0;
    for (int i = 0; i < 5; i++) {
        result = result * 10 + digits[i];
    }

    return result;
}

bool check_vault_unlocked(FILE *file) {
    int status;

    if (file == NULL) {
        return false;
    }

    //moves the file position indicator (file pointer) back to the beginning of a file
    rewind(file);

    if (fscanf(file, "%d", &status) != 1) {
        return false;
    }

    return status == 1;
}

void set_vault_unlocked(FILE *file) {
    if (file == NULL) {
        return;
    }

    rewind(file);

    if (fprintf(file, "1\n") < 0) {
        return;
    }

    fflush(file);
}
