/* This file contains the NEW structure definition */
/* The old one is read and the new one is written */


struct mheader_new
{
  uint8_t magic;
  uint32_t poster;
  bool deleted;
  uint32_t deleted_by;
  char deleted_by_name[MAXALIAS + 1];
  bool quotedx;
  bool mail;
  bool approval;
  uint16_t hlen;
  uint16_t len;
  uint32_t msgid;
  uint16_t forum;
  uint8_t mtype;
  time_t ptime;
  /* 128 bytes that can be used in the future to reduce 
  conversion updates */
  uint8_t future_use[128];
  union
  {
    struct
    {
      uint32_t recipient;
    } mail;
  } ext;
};
