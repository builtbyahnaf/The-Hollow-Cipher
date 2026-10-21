
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "vault_pass.h"

int generate_pass(void)
{
    long seed = 10000000L + rand() % 90000000L;

    long step1 = seed >> 3;
    long step2 = step1 << 2;
    long step3 = step2 ^ 0xABCD;
    long step4 = step3 & 0x7FFFFFFF;

    int result = 10000 + (int)(step4 % 90000);

    return result;
}

bool check_vault_unlocked(FILE *file)
{
    int status;

    if (file == NULL)
    {
        return false;
    }

    rewind(file);

    if (fscanf(file, "%d", &status) != 1)
    {
        return false;
    }

    return status == 1;
}

void set_vault_unlocked(FILE *file)
{
    if (file == NULL)
    {
        return;
    }

    rewind(file);

    if (fprintf(file, "1\n") < 0)
    {
        return;
    }

    fflush(file);
}
