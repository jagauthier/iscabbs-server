#include "convert_utils.h"

uint8_t convert_bigbtmp(void) {
  FILE *fp;

  struct bigbtmp temp_bigbtmp;

  /* Open the file directly and then read through it generating a new
  bigbtmp with new data sizes.  This should work on x86 and x64 */
  fp = fopen("/home/bbs/data.orig/tmpdata", "r");
  if (!fp) {
    printf("Could not open %s\n", "/home/bbs/data.orig/tmpdata");
    return 1;
  }
  fseek(fp, 0, SEEK_END);
  printf("bigbtmp file size: %li\n", ftell(fp));
  fseek(fp, 0, SEEK_SET);

  /* Going to read in each field.  This took some data file hex viewing
  because extra bytes showed up around byte boundaries - mostly when 16 integers
  */

  memset(&temp_bigbtmp, 0, sizeof(struct bigbtmp));

  fread(&temp_bigbtmp.users, 1, S32, fp);
  temp_bigbtmp.users = 0;
  fread(&temp_bigbtmp.queued, 1, S32, fp);
  temp_bigbtmp.queued = 0;
  fread(&temp_bigbtmp.index, MAXUSERS, S16, fp);
  fread(&temp_bigbtmp.ghostcheck, 1, S32, fp);
  fread(&temp_bigbtmp.eternal, 1, S32, fp);
  fread(&temp_bigbtmp.qp, 1, S16, fp);
  fread(&temp_bigbtmp.oldqp, 1, S16, fp);
  fread(&temp_bigbtmp.maxqp, 1, S16, fp);
  fread(&temp_bigbtmp.socks, 1, S16, fp);
  fread(&temp_bigbtmp.forks, 1, S32, fp);
  fread(&temp_bigbtmp.reaps, 1, S32, fp);
  fread(&temp_bigbtmp.connectable, 1, S16, fp);
  fread(&temp_bigbtmp.startable, 1, S16, fp);
  fread(&temp_bigbtmp.aidewiz, 1, S32, fp);
  fread(&temp_bigbtmp.upgrade, 1, S32, fp);
  fread(&temp_bigbtmp.nonupgrade, 1, S32, fp);
  fread(&temp_bigbtmp.starts, 1, S16, fp);
  fread(&temp_bigbtmp.qflag, 1, S16, fp);
  fread(&temp_bigbtmp.hello, 1000, S8, fp);
  fread(&temp_bigbtmp.unused, 16, S8, fp);
  fread(&temp_bigbtmp.sig_alrm, 1, S8, fp);
  fread(&temp_bigbtmp.sig_cld, 1, S8, fp);
  fread(&temp_bigbtmp.sig_hup, 1, S8, fp);
  fread(&temp_bigbtmp.sig_term, 1, S8, fp);
  fread(&temp_bigbtmp.sig_quit, 1, S8, fp);
  fread(&temp_bigbtmp.sig_urg, 1, S8, fp);
  fread(&temp_bigbtmp.sig_usr1, 1, S8, fp);
  fread(&temp_bigbtmp.sig_usr2, 1, S8, fp);
  fread(&temp_bigbtmp.hellolen, 1, S16, fp);
  fread(&temp_bigbtmp.down, 1, S8, fp);

  // alignment? takes up 2 bytes in the file
  fseek(fp, 1, SEEK_CUR);
  fread(&temp_bigbtmp.limit, 1, S16, fp);
  /* Move to the PID as a sanity check */
  /* the two longs don't appear to be in the file here.. */
  fseek(fp, 10, SEEK_CUR);

  fread(&temp_bigbtmp.pid, 1, sizeof(pid_t), fp);

  /* Now copy the BTMP structure. Originally, this was big block copies
  with memcpy, but after about 30 iterations memcpy seemed not copy data
  so as a test I reverted to just copying the elements to see if it works better
  */
  fseek(fp, 3644, SEEK_SET);

  for (uint16_t i = 0; i < MAXUSERS; i++) {
    // eternal
    continue;
    fread(&temp_bigbtmp.btmp[i].eternal, 1, S32, fp);
    fread(&temp_bigbtmp.btmp[i].usernum, 1, S32, fp);
    fread(&temp_bigbtmp.btmp[i].time, 1, S32, fp);
    fread(&temp_bigbtmp.btmp[i].remaddr, 1, S32, fp);
    fread(&temp_bigbtmp.btmp[i].remport, 1, S16, fp);
    fread(&temp_bigbtmp.btmp[i].pid, 1, sizeof(pid_t), fp);
    // move 2 ahead
    fseek(fp, 2, SEEK_CUR);
    fread(&temp_bigbtmp.btmp[i].name, MAXALIAS + 1, S8, fp);
    fread(&temp_bigbtmp.btmp[i].remote, 40, S8, fp);
    fread(&temp_bigbtmp.btmp[i].doing, 40, S8, fp);
    fread(&temp_bigbtmp.btmp[i].remlogin, 16, S8, fp);
    fread(&temp_bigbtmp.btmp[i].xstat, 1, S8, fp);
    fread(&temp_bigbtmp.btmp[i].elf, 1, S8, fp);
    fread(&temp_bigbtmp.btmp[i]._was_anonymous, 1, S8, fp);
    fread(&temp_bigbtmp.btmp[i].guest, 1, S8, fp);
    fread(&temp_bigbtmp.btmp[i].connecting, 1, S8, fp);
    fread(&temp_bigbtmp.btmp[i].client, 1, S8, fp);
    fread(&temp_bigbtmp.btmp[i].nox, 1, S8, fp);
    fread(&temp_bigbtmp.btmp[i].unused, 1, S8, fp);
    fread(&temp_bigbtmp.btmp[i].sleeptimes, 1, S16, fp);
    // move 2 ahead
    fseek(fp, 2, SEEK_CUR);

    fread(&temp_bigbtmp.btmp[i].ulink, 1, S32, fp);
    if (x86) {
      /*
      printf("eternal[%i]: %li %li\n", i, bigbtmp->btmp[i].eternal,
             temp_bigbtmp.btmp[i].eternal);
      printf("usernum[%i]: %li %li\n", i, bigbtmp->btmp[i].usernum,
             temp_bigbtmp.btmp[i].usernum);
      printf("time[%i]: %li %li\n", i, bigbtmp->btmp[i].time,
             temp_bigbtmp.btmp[i].time);
      printf("remaddr[%i]: %li %li\n", i, bigbtmp->btmp[i].remaddr,
             temp_bigbtmp.btmp[i].remaddr);
      printf("remport[%i]: %li %li\n", i, bigbtmp->btmp[i].remport,
             temp_bigbtmp.btmp[i].remport);
      printf("pid[%i]: %li %li\n", i, bigbtmp->btmp[i].pid,
             temp_bigbtmp.btmp[i].pid);
      printf("loc: %i\n", ftell(fp));
      printf("name[%i]: '%s' '%s'\n", i, bigbtmp->btmp[i].name,
             temp_bigbtmp.btmp[i].name);
      printf("remote[%i]: '%s' '%s'\n", i, bigbtmp->btmp[i].remote,
             temp_bigbtmp.btmp[i].remote);
      printf("doing[%i]: '%s' '%s'\n", i, bigbtmp->btmp[i].doing,
             temp_bigbtmp.btmp[i].doing);
      printf("remlogin[%i]: '%s' '%s'\n", i, bigbtmp->btmp[i].remlogin,
             temp_bigbtmp.btmp[i].remlogin);
      // printf("%i '%s'\n", temp_bigbtmp.btmp[i].usernum,
      // temp_bigbtmp.btmp[i].name);
      printf("sleeptimes[%i]: %li %li\n", i, bigbtmp->btmp[i].sleeptimes,
             temp_bigbtmp.btmp[i].sleeptimes);
      printf("\n\n");
      */
    }
    /*
        if (temp_bigbtmp.btmp[i].usernum) {
          printf("[%i]: %i %s\n", i, temp_bigbtmp.btmp[i].usernum,
                 temp_bigbtmp.btmp[i].name);
        }*/

    /* there is no need to convert qtmp because it's just empty anyway */
  }
  fclose(fp);

  fp = fopen("convert_output/data/tmpdata", "w");
  fwrite(&temp_bigbtmp, sizeof(struct bigbtmp), 1, fp);
  printf("tmpdata: Expected: %u, Actual: %li\n\n", (uint32_t)sizeof(struct bigbtmp),
         ftell(fp));
  fclose(fp);

  return 0;
}
