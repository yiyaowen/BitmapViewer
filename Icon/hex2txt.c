#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#pragma warning(disable : 6031)

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("Usage: hex2txt (binary file) (text file) [line width = 4]\n");
        return 0;
    }

    int linew = 4;
    if (argc == 4)
    {
        if (sscanf(argv[3], "%d", &linew) != 1 || linew <= 0)
        {
            printf("Invalid [line width] cmd arg.\n");
            return 0;
        }
    }

    FILE* fpbin = fopen(argv[1], "rb");
    if (fpbin == NULL)
    {
        printf("Failed to open file: %s\n", argv[1]);
        return 0;
    }

    fseek(fpbin, 0, SEEK_END);
    size_t bs = ftell(fpbin);
    fseek(fpbin, 0, SEEK_SET);

    uint8_t* buff = (uint8_t*)malloc(bs * sizeof(uint8_t));
    if (buff == NULL)
    {
        printf("Failed to load binary data: out of memory.\n");
        goto load_bin_failed;
    }
    if (fread(buff, sizeof(uint8_t), bs, fpbin) != bs)
    {
        printf("Failed to load binary data: file may be broken.\n");
        goto load_bin_failed;
    }

    FILE* fptxt = fopen(argv[2], "w");
    if (fptxt == NULL)
    {
        printf("Failed to open text file.\n");
        goto open_txt_failed;
    }

    uint8_t high = 0, low = 0;
    for (size_t i = 0; i < bs; ++i)
    {
        fputc('0', fptxt);
        fputc('x', fptxt);

        high = (buff[i] >> 4);
        low = (buff[i] & 0x0F);

        // high hex
        if (high >= 10)
            fputc(high - 10 + 'A', fptxt);
        else
            fputc(high + '0', fptxt);
        // low hex
        if (low >= 10)
            fputc(low - 10 + 'A', fptxt);
        else
            fputc(low + '0', fptxt);

        fputc(',', fptxt);
        // new line or space
        if ((i + 1) % linew == 0)
            fputc('\n', fptxt);
        else
            fputc(' ', fptxt);
    }

    fclose(fptxt);
open_txt_failed:
load_bin_failed:
    free(buff);
    fclose(fpbin);
    return 0;
}