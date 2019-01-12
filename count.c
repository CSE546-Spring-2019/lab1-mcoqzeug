#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Calculate the size of a file given its name
off_t file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) return st.st_size;
    perror("Failed");
    exit(1);
}

// computes the longest prefix suffix array for the KMP algorithm
void compute_lps(const char *search_str, int *lps, int m) {
    lps[0] = 0;

    for (int i = 1; i < m; ++i) {
        int j = lps[i-1];
        while (j > 0 && search_str[j] != search_str[i]) j = lps[j-1];
        if (search_str[j] == search_str[i]) lps[i] = j + 1;
        else lps[i] = 0;
    }
}

// counts the number of occurrences of search_str in txt,
// implemented using the KMP algorithm.
int count_match(char *search_str, char *txt, int m) {
    int n = (int)strlen(txt);
    int count = 0;

    int i = 0;  // idx for txt[]
    int j = 0;  // idx for search_str[]

    // longest prefix suffix
    // lps[i] = argmax_k{ search_str[0...k-1] == search_str[i-k...i] }
    int lps[m];
    compute_lps(search_str, lps, m);

    while (i < n && m <= n) {
        if (search_str[j] == txt[i]) {
            i++; j++;

            if (j == m) {  // found a match
                count++;

                // Let k = lps[j - 1], then by definition
                // search_str[0...k-1] == search_str[j-1-k...j-1].
                // We also know that txt[i-1-k...i-1] == search_str[j-1-k...j-1].
                // So txt[i-1-k...i-1] == search_str[0...k-1].
                // We only need to compare txt[i] and search_str[k].
                j = lps[j - 1];
            }

        } else {
            if (j == 0) i++;
            else j = lps[j - 1];
        }
    }

    return count;
}

// counts the
int count_match_total(FILE *fp, char *search_str, off_t size) {
    int chunk_size = 100;
    int match_count = 0;
    int m = (int)strlen(search_str);

    char chunk[chunk_size];

    while(fread(chunk, 1, (size_t)chunk_size, fp) == chunk_size) {
        match_count += count_match(search_str, chunk, m);
        fseek(fp, 1-m, SEEK_CUR);
    }

    // In the loop above, fp proceeds chunk_size + 1 - m bytes at
    // each iteration. So size % (chunk_size+1-m) is the ending
    // position of the last block.
    chunk[size % (chunk_size + 1 - m)] = '\0';

    // last chunk
    match_count += count_match(search_str, chunk, m);

    return match_count;
}

int main(int argc, char** argv) {
    // Check number of arguments
    if (argc != 4) {
        printf("Incorrect number of arguments!\n"
        "Usage:\n"
        "\tcount <input-filename> <search-string> <output-filename>\n");
        exit(1);
    }

    char *in_file = argv[1];
    char *search_str = argv[2];
    char *out_file = argv[3];

    FILE *fp_in = fopen(in_file, "r");
    FILE *fp_out = fopen(out_file, "w");

    // Check if we can open the input file
    if (fp_in == NULL) {
        perror("Failed to open the input file");
        exit(1);
    }

    // Check if we can open the output file
    if (fp_out == NULL) {
        perror("Failed to open the output file");
        exit(1);
    }

    // Check search string size
    if (strlen(search_str) > 20) {
        printf("Search string should be less than 20 bytes!");
        exit(1);
    }

    off_t size = file_size(in_file);

    int match_count = count_match_total(fp_in, search_str, size);

    // Output to console and file
    char size_str[256];
    char match_str[256];

    sprintf(size_str, "Size of file is %d\n", (int)size);
    sprintf(match_str, "Number of matches = %d\n", match_count);

    printf("%s", size_str);
    fputs(size_str, fp_out);

    printf("%s", match_str);
    fputs(match_str, fp_out);

    fclose(fp_in);
    fclose(fp_out);

    return 0;
}
