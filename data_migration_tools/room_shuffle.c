#include "convert_utils.h"

struct pair {
    uint8_t old;
    uint8_t new;
};

void main(void) {
    uint16_t totalusers;
    uint16_t which, link;
    struct userdata *udata;
    struct user *source;
    size_t msg_size, user_size, size;
    struct msg *msg;
    uint8_t room_convs;
    unsigned char *msgstart;
    struct mheader *mh;
    struct pair room_pairs[] = {{3, 16}, {4, 17}, {5, 18}};
    char cp_cmd[256];
    char filename[256];
    char room_desc[256];
    FILE *fp;
    struct mheader mh_del;

    /* Copy everything to the conversion temp space */
    printf("Setting up conversion directories\n");
    system("rm -rf /home/bbs/src/convert_output/*");
    system("mkdir /home/bbs/src/convert_output/data");
    system("mkdir -p /home/bbs/src/convert_output/message/desc");

    system("cp /home/bbs/data/userdata convert_output/data");
    system("cp /home/bbs/data/msgdata convert_output/data");
    system("cp /home/bbs/message/msgmain convert_output/message");

    room_convs = sizeof(room_pairs) / sizeof(struct pair);

    msg_size = sizeof(struct msg);
    if (!(msg = (struct msg *)mmap_file("convert_output/data/msgdata",
                                        &msg_size))) {
        my_printf("msgdata\n");
        return;
    }

    user_size = 0;
    if (!(udata = (struct userdata *)mmap_file("convert_output/data/userdata",
                                               &user_size))) {
        my_printf("msgdata\n");
        return;
    }

    size = 0;
    if (!(msgstart = mmap_file("convert_output/message/msgmain", &size))) {
        my_printf("msgmain\n");
        return;
    }

    /* Copy the room structures */
    for (uint8_t i = 0; i < room_convs; i++) {
        sprintf(cp_cmd,
                "cp /home/bbs/message/desc/room%i "
                "convert_output/message/desc/room%i",
                room_pairs[i].old, room_pairs[i].new);
        system(cp_cmd);
        sprintf(filename, "convert_output/message/desc/room%i", room_pairs[i].new);

        /* change the room description info*/
        size = 0;
        mh = (struct mheader *)mmap_file(filename, &size);
        if (!mh) {
            my_printf("room desc.");
            return;
        }
        mh->forum = room_pairs[i].new;

        memcpy(&msg->room[room_pairs[i].new], &msg->room[room_pairs[i].old],
               sizeof(msg->room[0]));
        memset(&msg->room[room_pairs[i].old], 0, sizeof(msg->room[0]));

        /* copy Yell since it's got all the making of sysop only*/
        memcpy(&msg->room[DELMSG_RM_NBR], &msg->room[YELLS_RM_NBR],
               sizeof(msg->room[0]));
        strcpy(msg->room[DELMSG_RM_NBR].name, "Deleted Messages");
        msg->room[DELMSG_RM_NBR].gen = 0;
        msg->room[DELMSG_RM_NBR].posted = 0;
        msg->room[DELMSG_RM_NBR].highest = 0;

        for (uint8_t j = 0; j < MSGSPERRM; j++) {
            msg->room[DELMSG_RM_NBR].pos[j] = 0;
            msg->room[DELMSG_RM_NBR].num[j] = 0;
            msg->room[DELMSG_RM_NBR].chron[j] = 0;
        }
        /* change the messages */
        for (uint8_t p = 0; p < MSGSPERRM; p++) {
            if (msg->room[room_pairs[i].new].pos[p]) {
                mh = (struct mheader *)(msgstart + msg->room[room_pairs[i].new].pos[p]);
                if (mh->magic != M_MAGIC) {
                    printf("Uh oh, position %i is not magic.\n",
                           msg->room[room_pairs[i].new].pos[p]);
                    return;
                }
                if (mh->forum != room_pairs[i].old) {
                    printf("And the assigned room is not %i (%i): pos %i.\n",
                           room_pairs[i].old, mh->forum,
                           msg->room[room_pairs[i].new].pos[p]);
                    return;
                }
                mh->forum = room_pairs[i].new;
            }
        }

        /* repoint the user indexes */
        which = udata->which;
        totalusers = udata->totalusers[which];
        for (uint16_t u = 0; u < totalusers; u++) {
            link = udata->num[which][u];
            source = (struct user *)(udata + 1) + (link);

            // printf("%s, generation[%i->%i]: %i\n", source->name, room_pairs[i].old,
            //        room_pairs[i].new, source->generation[room_pairs[i].old]);
            source->generation[room_pairs[i].new] =
                source->generation[room_pairs[i].old];
            // printf("%s, lastseen[%i->%i]: %i\n", source->name, room_pairs[i].old,
            //        room_pairs[i].new, source->lastseen[room_pairs[i].old]);
            source->lastseen[room_pairs[i].new] = source->lastseen[room_pairs[i].old];
            // printf("%s, forget[%i->%i]: %i\n", source->name, room_pairs[i].old,
            //        room_pairs[i].new, source->forget[room_pairs[i].old]);
            source->forget[room_pairs[i].new] = source->forget[room_pairs[i].old];

            source->generation[room_pairs[i].old] = -1;
            source->lastseen[room_pairs[i].old] = 0;
            source->forget[room_pairs[i].old] = -1;
            if (source->f_aide) {
                source->lastseen[DELMSG_RM_NBR] = 0;
                source->forget[DELMSG_RM_NBR] = -1;
                source->generation[DELMSG_RM_NBR] = msg->room[DELMSG_RM_NBR].gen;
            }
        }
        printf("Make sure to remove /home/bbs/message/desc/room%i\n",
               room_pairs[i].old);
    }
    /* Create the roominfo for Deleted Messages */
    strcpy(room_desc, "This is where deleted messages can be reviewed.");
    memset(&mh_del, 0, sizeof(struct mheader));
    mh_del.magic = M_MAGIC;
    mh_del.hlen = sizeof(struct mheader);
    mh_del.len = strlen(room_desc);
    mh_del.forum = DELMSG_RM_NBR;
    mh_del.mtype = 'D';
    mh_del.ptime = time(0);
    sprintf(filename, "convert_output/message/desc/room%i", DELMSG_RM_NBR);
    fp = fopen(filename, "w");
    if (!fp) {
        printf("Can't create %s\n", filename);
        return;
    }
    fwrite(&mh_del, sizeof(struct mheader), 1, fp);
    fwrite(room_desc, strlen(room_desc), 1, fp);
    fclose(fp);
}