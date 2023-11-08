#include "convert_utils.h"

uint8_t convert_messages(void) {
    FILE *fp_in;
    FILE *fp_out;

    uint32_t bitfield;

    struct mheader mh;
    // uint8_t *p;
    char filename[255];
    bool done = false;
    uint32_t file_loc;
    char *text;
    uint16_t padding = 0;
    uint32_t magic_location, last_magic_location;
    uint8_t magic;

    fp_in = fopen("/home/bbs/message.orig/msgmain", "r");
    if (!fp_in) {
        printf("Could not open %s\n", "/home/bbs/message.orig/msgmain");
        return 1;
    }

    fseek(fp_in, 0, SEEK_END);
    printf("msgmain file size: %li\n", ftell(fp_in));
    fseek(fp_in, 0, SEEK_SET);

    // p = mmap_file(MSGMAIN, &size);

    sprintf(filename, "convert_output/message/msgmain");
    fp_out = fopen(filename, "w");

    while (!done) {
        memset(&mh, 0, sizeof(struct mheader));

        // printf("Location: %ld\n", ftell(fp_in));
        fread(&bitfield, 1, S32, fp_in);
        mh.magic = getbits(bitfield, 0, 8);
        file_loc = ftell(fp_in);
        magic_location = file_loc - S32;

        if (mh.magic != M_MAGIC) {
            printf("Invalid magic number. %ld bytes in.\n", ftell(fp_in));
            printf("Last location: %#x\n", last_magic_location);
            fclose(fp_in);
            return 1;
        }
        last_magic_location = magic_location;
        mh.poster = getbits(bitfield, 8, 24);
        fread(&bitfield, 1, S32, fp_in);
        mh.quotedx = getbits(bitfield, 5, 1);
        mh.mail = getbits(bitfield, 6, 1);
        mh.approval = getbits(bitfield, 7, 1);
        mh.hlen = getbits(bitfield, 8, 6);
        if (mh.hlen > 20) {
            // printf("hlen: %i: %ld bytes in\n", mh.hlen, ftell(fp_in));
        }
        mh.len = getbits(bitfield, 14, 18);
        fread(&mh.msgid, 1, S32, fp_in);
        fread(&mh.forum, 1, S16, fp_in);
        fread(&mh.mtype, 1, S8, fp_in);
        // move forward one
        fseek(fp_in, 1, SEEK_CUR);

        fread(&mh.ptime, 1, S32, fp_in);
        // just some extra
        text = calloc(1, mh.len + 5000);
        if (!text) {
            printf("Couldn't allocate any memory\n");
            return 1;
        }
        fread(&bitfield, 1, S32, fp_in);
        mh.ext.mail.recipient = getbits(bitfield, 8, 24);
        fseek(fp_in, magic_location + mh.hlen, SEEK_SET);
        fread(text, mh.len, S8, fp_in);

        // now move forward to the next magic number.
        padding = 0;
        for (uint16_t i = 0; i < 5000; i++) {
            fread(&magic, 1, S8, fp_in);
            if (magic != M_MAGIC) {
                padding++;
                text[mh.len + i] = magic;
                if (ftell(fp_in) == MM_FILELEN) {
                    done = true;
                    break;
                }
            } else {
                fseek(fp_in, -1, SEEK_CUR);
                if (i > 10) {
                    printf("Large padding at location %i (%i)\n", (int32_t)ftell(fp_in), padding);
                }
                break;
            }
        }
        if (!x86) {
            // because ptime is now 8 bytes and not 4.
            mh.hlen += S32;
        }

        fwrite(&mh, mh.hlen, 1, fp_out);
        fwrite(text, mh.len + padding, 1, fp_out);
        if (text) free(text);
    }

    fclose(fp_in);
    fclose(fp_out);

    return 0;
}
