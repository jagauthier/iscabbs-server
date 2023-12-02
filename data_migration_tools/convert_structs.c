#include "convert_utils.h"

int main(int argc, char **argv) {
    uint8_t ret;

    /* Copy everything to the conversion temp space */
    printf("Setting up conversion directories\n");
    system("rm -rf /home/bbs/src/convert_output/*");
    system("mkdir /home/bbs/src/convert_output/data");
    system("mkdir -p /home/bbs/src/convert_output/message/desc");
    /* system("rm -rf /home/bbs/data.orig /home/bbs/message.orig");
    system("mkdir /home/bbs/data.orig");
    system("mkdir /home/bbs/message.orig");
    system("cp -r /home/bbs/data_x64/* /home/bbs/data.orig");
    system("cp -r /home/bbs/message_x64/* /home/bbs/message.orig");*/
    
    /* This expects the msgdata here*/
    system("cp /home/bbs/data.orig/msgdata /home/bbs/src/convert_output/data/msgdata");

    find_magics();

    /* This uses the BBS function to open all the files. */
    /* Most used in development for data validation purposes */

    if (openfiles() < 0) {
        my_printf("openfiles() problem!\n");
        return 1;
    }

    // struct user *d=finduser(NULL, 248,0);
    printf("--- Converting Room Descs ---\n");
    ret = convert_room_descs();
    if (ret) {
        return ret;
    }
    
    /* do messages before msgdata */
    printf("--- Rebuilding Messages ---\n");
    ret = rebuild_messages();
    if (ret) {
        return ret;
    }

    // convert mail
    printf("--- Converting Mail ---\n");
    ret = convert_mail();
    if (ret) {
        return ret;
    }
}
