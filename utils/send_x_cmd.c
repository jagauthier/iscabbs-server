
#include "defs.h"
#include "ext.h"
int main(int argc, char **argv) {
    setenv("BBSNAME", "Dredd", 0);

    if (openfiles() < 0) {
        my_printf("openfiles() problem!\n");
        return (-1);
    }
    ARGV = argv;
    
  init_system();

  reserve_slot();

  do_login();

    xbroadcast();
    ouruser->f_admin = true;
    aide_menu();
    ouruser->f_admin = false;
    dologout();
    return 0;
}