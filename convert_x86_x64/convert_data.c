#include "convert_utils.h"

bool x86;

int main(int argc, char **argv) {
    uint8_t ret;

    x86 = false;
#ifdef X86
    x86 = true;
#endif

    find_magics();

    /* This uses the BBS function to open all the files. */
    /* Most used in development for data validation purposes */

    if (x86) {
        if (openfiles() < 0) {
            my_printf("openfiles() problem!\n");
            return 1;
        }
    }

    printf("--- Converting Users ---\n");
    ret = convert_user();
    if (ret) {
        return ret;
    }

    //struct user *d=finduser(NULL, 248,0);
    printf("--- Converting Room Descs ---\n");
    ret = convert_room_descs();
    if (ret) {
        return ret;
    }

    printf("--- Converting Message Data ---\n");
    ret = convert_msgdata();
    if (ret) {
        return ret;
    }

    /* do messages before msgdata */
    printf("--- Rebuilding Messages ---\n");
    ret = rebuild_messages();
    if (ret) {
        return ret;
    }

    printf("--- Converting Users ---\n");
    ret = convert_user();
    if (ret) {
        return ret;
    }

    printf("--- Converting bigbtmp ---\n");
    ret = convert_bigbtmp();
    if (ret) {
        return ret;
    }
    // convert mail
    printf("--- Converting Mail ---\n");
    ret = convert_mail();
    if (ret) {
        return ret;
    }

    // convert xmsgs - depends on messages being converted and users.
    printf("--- Converting Xmsgs ---\n");
    ret = rebuild_xmsgs();

    return ret;
    struct userdata *udata;
    udata = get_users();
    struct user *Dredd = my_finduser("Dredd", 0, 0, udata);

    printf("Dredd's mail:\n");
    for (uint8_t d = 0; d < MAILMSGS; d++) {
        if (Dredd->mr[d].num && Dredd->mr[d].num) {
            printf("\tIndex: %i. num/location: %i/%#010x\n", d, Dredd->mr[d].num,
                   Dredd->mr[d].pos);
        }
    }
}
