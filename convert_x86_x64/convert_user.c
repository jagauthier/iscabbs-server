#include "convert_utils.h"

uint8_t convert_user(void) {
  FILE *fp;
  size_t bytes;
  struct userdata temp_udata;
  uint32_t file_size;
  
  /* Open the file directly and then read through it generating a new
  bigbtmp with new data sizes.  This should work on x86 and x64 */

  fp = fopen("/home/bbs/data.orig/userdata", "r");
  if (!fp) {
    printf("Could not open %s\n", "/home/bbs/data.orig/userdata");
    return 1;
  }

  fseek(fp, 0, SEEK_END);
  file_size = ftell(fp);
  printf("user file size: %i\n", file_size);
  fseek(fp, 0, SEEK_SET);

  fread(&temp_udata.gen, 1, S32, fp);

  fread(&temp_udata.totalusers, 2, S32, fp);
  fread(&temp_udata.free, 2, S32, fp);
  fread(&temp_udata.retries, 1, S32, fp);
  fread(&temp_udata.unused, 1017, S32, fp);
  fread(&temp_udata.which, 1, S32, fp);
  fread(&temp_udata.name, MAXTOTALUSERS * 2, S32, fp);
  fread(&temp_udata.num, MAXTOTALUSERS * 2, S32, fp);

  for (uint16_t i = 0; i < MAXTOTALUSERS; i++) {
    fread(&temp_udata.link[i].free, 1, S32, fp);
    fread(&temp_udata.link[i].usernum, 1, S32, fp);

    fread(&temp_udata.link[i].name, MAXALIAS + 1, S8, fp);
  }

  struct user *temp_user;
  temp_user = calloc(sizeof(struct user), MAXTOTALUSERS);
  if (!temp_user) {
    printf("Could not allocate memory.\n");
    return 1;
  }
  /* Now we're on the users... Should be interesting */
  for (uint16_t j = 0; j < MAXTOTALUSERS; j++) {
    uint32_t bitfield1, bitfield2;
    // For debugging
    // user = &temp_user[j];
    bytes = fread(&temp_user[j].usernum, 1, S32, fp);
    if (!bytes) {
      printf("End of file at %i iterations.\n", j);
      break;
    };

    fread(&bitfield1, 1, S32, fp);
    fread(&bitfield2, 1, S32, fp);

    temp_user[j].an_www = getbits(bitfield1, 0, 1);
    temp_user[j].an_site = getbits(bitfield1, 1, 1);
    temp_user[j].an_mail = getbits(bitfield1, 2, 1);
    temp_user[j].an_phone = getbits(bitfield1, 3, 1);
    temp_user[j].an_location = getbits(bitfield1, 4, 1);
    temp_user[j].an_addr = getbits(bitfield1, 5, 1);
    temp_user[j].an_name = getbits(bitfield1, 6, 1);
    temp_user[j].an_all = getbits(bitfield1, 7, 1);

    temp_user[j].f_duplicate = getbits(bitfield1, 8, 1);
    temp_user[j].f_admin = getbits(bitfield1, 9, 1);
    temp_user[j].f_restricted = getbits(bitfield1, 10, 1);
    temp_user[j].f_prog = getbits(bitfield1, 11, 1);
    temp_user[j].f_badinfo = getbits(bitfield1, 12, 1);
    temp_user[j].f_newbie = getbits(bitfield1, 13, 1);
    temp_user[j].f_inactive = getbits(bitfield1, 14, 1);
    temp_user[j].f_deleted = getbits(bitfield1, 15, 1);

    temp_user[j].f_ownnew = getbits(bitfield1, 16, 1);
    temp_user[j].f_namechanged = getbits(bitfield1, 17, 1);
    temp_user[j].f_noanon = getbits(bitfield1, 18, 1);
    temp_user[j].f_noclient = getbits(bitfield1, 19, 1);
    temp_user[j].f_ansi = getbits(bitfield1, 20, 1);
    temp_user[j].f_trouble = getbits(bitfield1, 21, 1);
    temp_user[j].f_invisible = getbits(bitfield1, 22, 1);
    temp_user[j].f_autoelf = getbits(bitfield1, 23, 1);

    temp_user[j].f_novice = getbits(bitfield1, 24, 1);
    temp_user[j].f_elf = getbits(bitfield1, 25, 1);
    temp_user[j].f_nobeep = getbits(bitfield1, 26, 1);
    temp_user[j].f_xmsg = getbits(bitfield1, 27, 1);
    temp_user[j].f_ampm = getbits(bitfield1, 28, 1);
    temp_user[j].f_shortwho = getbits(bitfield1, 29, 1);
    temp_user[j].f_twit = getbits(bitfield1, 30, 1);
    temp_user[j].f_beeps = getbits(bitfield1, 31, 1);

    temp_user[j].f_lastold = getbits(bitfield2, 0, 1);
    temp_user[j].f_xoff = getbits(bitfield2, 1, 1);
    temp_user[j].f_clear = getbits(bitfield2, 2, 1);
    temp_user[j].f_aide = getbits(bitfield2, 3, 1);
    temp_user[j].f_revwho = getbits(bitfield2, 4, 1);
    fread(&temp_user[j].timescalled, 1, S32, fp);
    fread(&temp_user[j].posted, 1, S32, fp);
    fread(&temp_user[j].time, 1, S32, fp);
    fread(&temp_user[j].timeoff, 1, S32, fp);
    fread(&temp_user[j].timetot, 1, S32, fp);
    fread(&temp_user[j].firstcall, 1, S32, fp);
    fread(&temp_user[j].name, MAXALIAS + 1, S8, fp);
    if (temp_user[j].usernum) {
      // printf("Converting %s\n", temp_user[j].name);
    }
    fread(&temp_user[j].passwd, 14, S8, fp);
    fread(&temp_user[j].remote, 40, S8, fp);
    fread(&temp_user[j].real_name, MAXNAME + 1, S8, fp);
    fread(&temp_user[j].addr1, MAXNAME + 1, S8, fp);
    fread(&temp_user[j].addr2, MAXNAME + 1, S8, fp);
    fread(&temp_user[j].city, 21, S8, fp);
    fread(&temp_user[j].state, 21, S8, fp);
    fread(&temp_user[j].zip, 11, S8, fp);
    fread(&temp_user[j].phone, 21, S8, fp);
    fread(&temp_user[j].mail, 41, S8, fp);
    fread(&temp_user[j].desc1, 81, S8, fp);
    fread(&temp_user[j].desc2, 81, S8, fp);
    fread(&temp_user[j].desc3, 81, S8, fp);
    fread(&temp_user[j].desc4, 81, S8, fp);
    fread(&temp_user[j].desc5, 81, S8, fp);
    fread(&temp_user[j].generation, MAXROOMS, S8, fp);
    fread(&temp_user[j].forget, MAXROOMS, S8, fp);
    //* gotta move forward 3 characters */
    /* 00036fe0: ff00 0000 b5a4 0900 088d 0700 79c4 0700  */
    // end of forget^       ^ start of last seen
    fseek(fp, 3, SEEK_CUR);
    fread(&temp_user[j].lastseen, MAXROOMS, S32, fp);

    fread(&temp_user[j].quickx, 10, S32, fp);

    for (uint8_t i = 0; i < NXCONF; i++) {
      fread(&bitfield1, 1, S32, fp);
      temp_user[j].xconf[i].which = getbits(bitfield1, 7, 1);
      temp_user[j].xconf[i].usernum = getbits(bitfield1, 8, 24);
    }
    fread(&temp_user[j].www, 60, S8, fp);
    fread(&temp_user[j].xconftime, 1, S32, fp);

    for (uint8_t i = 0; i < MAILMSGS; i++) {
      fread(&temp_user[j].mr[i].num, 1, S32, fp);
      fread(&temp_user[j].mr[i].pos, 1, S32, fp);
    }

    fread(&temp_user[j].aideinfo, 81, S8, fp);
    fread(&temp_user[j].reminder, 81, S8, fp);
    fread(&temp_user[j].loginname, 16, S8, fp);
    fread(&temp_user[j].unused0, 1, S8, fp);
    // move forward because of the strings/byte boundaries
    fseek(fp, 1, SEEK_CUR);
    fread(&temp_user[j].totalx, 1, S32, fp);
    fread(&temp_user[j].A_real_name, MAXNAME + 1, S8, fp);
    fread(&temp_user[j].A_addr1, MAXNAME + 1, S8, fp);
    fread(&temp_user[j].A_city, 21, S8, fp);
    fread(&temp_user[j].A_state, 21, S8, fp);
    fread(&temp_user[j].A_zip, 11, S8, fp);
    fread(&temp_user[j].A_phone, 21, S8, fp);
    fread(&temp_user[j].A_mail, 41, S8, fp);
    fread(&temp_user[j].doing, 40, S8, fp);
    fread(&temp_user[j].vanityflag, 40, S8, fp);
    // move again
    fseek(fp, 1, SEEK_CUR);
    fread(&temp_user[j].btmpindex, 1, S16, fp);
    fread(&temp_user[j].xseenpos, 1, S32, fp);
    fread(&temp_user[j].xmaxpos, 1, S32, fp);
    fread(&temp_user[j].xmsgnum, 1, S16, fp);
    fseek(fp, 2, SEEK_CUR);
    fread(&temp_user[j].xminpos, 1, S32, fp);
    fread(&temp_user[j].yells, 1, S16, fp);
    fread(&temp_user[j].vals, 1, S16, fp);
  }
  fclose(fp);

  fp = fopen("convert_output/data/userdata", "w");
  bytes = fwrite(&temp_udata, 1, sizeof(struct userdata), fp);
  printf("Wrote user header bytes: %i\n", (uint32_t)bytes);
  bytes = fwrite(temp_user, 1, sizeof(struct user) * MAXTOTALUSERS, fp);
  if (bytes != sizeof(struct user) * MAXTOTALUSERS) {
    perror("fwrite");
  }
  printf("Wrote user bytes: %i\n", (uint32_t)bytes);
  printf("userdata: Expected: %i, Actual: %li\n\n",
         (uint32_t)(sizeof(struct userdata) +
                    (sizeof(struct user) * MAXTOTALUSERS)),
         ftell(fp));
  fclose(fp);

  return 0;
}
