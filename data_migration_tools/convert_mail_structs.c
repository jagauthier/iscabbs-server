#include "convert_utils.h"

void update_others_mail(uint32_t me, int32_t num, int32_t mail_pos,
                        struct userdata *udata) {
    uint32_t which = udata->which;
    uint32_t totalusers = udata->totalusers[which];
    uint32_t link;

    struct user *user;

    for (uint16_t i = 0; i < totalusers; i++) {
        link = udata->num[which][i];
        user = (struct user *)(udata + 1) + (link);
        if (user->usernum == me) {
            continue;
        }

        for (uint16_t m = 0; m < MAILMSGS; m++) {
            if (user->mr[m].num == num) {
                user->mr[m].pos = -mail_pos;
                // printf("    Updated user %i slot %i message: %i %#010x\n",
                //        user->usernum, m, user->mr[m].pos, user->mr[m].pos);
                return;
            }
        }
    }
}

uint8_t convert_mail(void) {
    uint32_t mm_origin, file_position, new_file_position;
    struct userdata *udata;
    struct user *source, *search;
    FILE *fp_orig;
    char filename[255];
    size_t size = 0;
    uint8_t *mm_new;
    struct msg *msg;

    uint8_t magic;
    struct mheader_new mh;
    uint32_t bitfield;
    char *text;
    uint32_t which;
    uint32_t totalusers;
    uint32_t link;

    int32_t msg_location;
    int f;

    if ((f = open("convert_output/data/userdata", O_RDWR)) < 0) {
        perror("openuser open");
        return NULL;
    }

    size = lseek(f, 0, SEEK_END);
    printf("size: %i\n", size);

    udata = (struct userdata *)mmap(
        0, size,
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, f, 0);
    if (!udata) {
        printf("Cannot get udata.\n");
        return 0;
    }
    close(f);

    which = udata->which;
    totalusers = udata->totalusers[which];

    // Open the old and new message files

    fp_orig = fopen("/home/bbs/message.orig/msgmain", "r");
    if (!fp_orig) {
        printf("Could not open %s\n", "/home/bbs/message.orig/msgmain");
        return 1;
    }
    size = MM_FILELEN;
    sprintf(filename, "convert_output/message/msgmain");
    mm_new = mmap_file(filename, &size);

    size = sizeof(struct msg);
    if (!(msg = (struct msg *)mmap_file("convert_output/data/msgdata", &size))) {
        my_printf("msgdata not there\n");
        return 0;
    }

    // The file origin is where data will start to be written.
    // which is the pointer to in the message structure
    file_position = (uintptr_t)msg->curpos;
    mm_origin = (uintptr_t)mm_new;

    // Move the start of the mmap file too
    mm_new += file_position;

    /* Find the valid users. */
    for (uint16_t i = 0; i < totalusers; i++) {
        link = udata->num[which][i];
        source = (struct user *)(udata + 1) + (link);

        /*if (!(source->usernum == 1139 || source->usernum == 248)) {
            continue;
        }*/

        // printf("%s %i\n", source->name, source->usernum);
        for (uint8_t m = 0; m < MAILMSGS; m++) {
            // Find the original message
            if (source->mr[m].num && source->mr[m].pos) {
                // if (source->mr[m].num != 522705) continue;
                /* check if this has already been copied */
                // printf(" Processing %i %#010x\n", source->mr[m].num,
                // source->mr[m].pos);
                msg_location = source->mr[m].pos;
                if (source->mr[m].pos < 0) {
                    msg_location = -source->mr[m].pos;
                }
                // This could be done using the mmap pointer, I'm sure. But I wasn't
                // smart enough to do it */
                magic = 0;
                FILE *mfp = fopen("convert_output/message/msgmain", "r");
                int fs;
                fs = fseek(mfp, msg_location, SEEK_SET);
                if (!fs) {
                    fread(&magic, 1, S8, mfp);
                }
                fclose(mfp);
                if (magic == M_MAGIC) {
                    // printf("  Detected that %i was updated/copied under another
                    // user.\n",
                    //        source->mr[m].num);
                    continue;
                }

                // printf("%i %#010x\n", m, source->mr[m].pos);
                //  It's negative when I am the *sender*
                //  These need to be updated after the new location is determined
                if (source->mr[m].pos == -1) {
                    // but check if it exists or not
                    fseek(fp_orig, 0xffffffff - source->mr[m].pos + 1, SEEK_SET);
                    fread(&magic, 1, S8, fp_orig);
                    if (magic != M_MAGIC) {
                        source->mr[m].num = source->mr[m].pos = 0;
                        continue;
                    }
                }
                msg_location = source->mr[m].pos;
                if (source->mr[m].pos < 0) {
                    msg_location = -source->mr[m].pos;
                }
                fseek(fp_orig, msg_location, SEEK_SET);
                // printf("Copying message at location %#010x\n",
                //        (uint32_t)ftell(fp_orig));
                fread(&magic, 1, S8, fp_orig);
                if (magic != M_MAGIC) {
                    // printf("Mail message %i for %s not valid.\n", m, source->name);
                    source->mr[m].num = source->mr[m].pos = 0;
                    // printf("  Invalid message.\n");
                    continue;
                }

                // move it back
                fseek(fp_orig, -1, SEEK_CUR);
                /* Success, so copy the data */
                memset(&mh, 0, sizeof(struct mheader_new));
                fread(&bitfield, 1, S32, fp_orig);
                mh.magic = getbits(bitfield, 0, 8);
                if (mh.magic != M_MAGIC) {
                    printf("It's not magic now. How?\n");
                    return 0;
                }

                mh.poster = getbits(bitfield, 8, 24);
                fread(&bitfield, 1, S32, fp_orig);
                mh.quotedx = getbits(bitfield, 5, 1);
                mh.mail = getbits(bitfield, 6, 1);
                mh.approval = getbits(bitfield, 7, 1);
                mh.hlen = getbits(bitfield, 8, 6);

                mh.len = getbits(bitfield, 14, 18);
                fread(&mh.msgid, 1, S32, fp_orig);
                fread(&mh.forum, 1, S16, fp_orig);
                fread(&mh.mtype, 1, S8, fp_orig);
                // move forward one
                fseek(fp_orig, 1, SEEK_CUR);

                fread(&mh.ptime, 1, sizeof(time_t), fp_orig);
                text = calloc(1, mh.len + 1);
                if (!text) {
                    printf("Couldn't allocate any memory\n");
                    return 1;
                }
                fread(&bitfield, 1, S32, fp_orig);
                mh.ext.mail.recipient = getbits(bitfield, 8, 24);
                fseek(fp_orig, msg_location + mh.hlen, SEEK_SET);
                // printf("Copying text at location: %#010x\n",
                // (uint32_t)ftell(fp_orig));
                fread(text, mh.len, S8, fp_orig);
                text[mh.len] = 0;
                mh.hlen = sizeof(struct mheader_new);
                memcpy(mm_new, &mh, mh.hlen);
                memcpy(mm_new + mh.hlen, text, mh.len);

                // The file position is calculated in the convert messages area
                // We'll reposition it after this too
                file_position = (uintptr_t)mm_new - mm_origin;
                // update the message to point to new location
                if (source->mr[m].pos < 0) {
                    source->mr[m].pos = -file_position;
                } else {
                    source->mr[m].pos = file_position;
                }
                // printf("  Setting %i to %#010x\n", m, file_position);
                /* Now, go through and set all users from the old location
                to the new location */
                // printf("  New file location: %#010x\n", file_position);
                update_others_mail(source->usernum, source->mr[m].num,
                                   source->mr[m].pos, udata);

                // Move the pointer forward, and add 1 to get a 0 after the string */
                mm_new += mh.hlen + strlen(text) + 1;

                // This is where it should go
                file_position = (uintptr_t)mm_new - mm_origin;

                if (file_position % 4 != 0) {
                    // printf("%#010x is not 32b aligned\n", new_file_position);
                }

                new_file_position = (file_position + 4 - file_position % 4);
                if (new_file_position % 4 == 0) {
                    // printf("%#010x is now 32b aligned\n", new_file_position);
                }
                // move forward the difference?
                mm_new += new_file_position - file_position;
                // printf("File position next: %#010x\n", file_position);
            }
        }
    }
    // Pointer to the next post.
    msg->curpos = (uintptr_t)mm_new - mm_origin;
    printf("Final location: %#010lx\n", msg->curpos);
    munmap(mm_new, MM_FILELEN);
    munmap(msg, sizeof(struct msg));
    fclose(fp_orig);
    return 0;
}
