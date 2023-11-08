#define MAXTOTALUSERS 5000


/*
 * Location info for user data, indexed by ->name and ->num.
 */
struct userlink
{
  int32_t free;		/* Index to next free page/link */
  uint32_t usernum;		/* User number */
  char name[MAXALIAS+1];/* User name */
};


/*
 * Main user data structure, user data pages follow immediately after.
 */

struct userdata
{
  int32_t gen;		/* Generation number (see finduser()) */
  int32_t totalusers[2];	/* Total users, used as boundary in ->name and ->num */
  int32_t free[2];		/* Index to next free page/link */
  int32_t retries;		/* Stats: times gen changed in finduser() */
  int32_t unused[1017];
  int32_t which;		/* Which of the mirrored values is currently in use */

  int32_t name[2][MAXTOTALUSERS];
  int32_t num[2][MAXTOTALUSERS];
  struct userlink link[MAXTOTALUSERS];
};


struct voteinfo
{
  struct vote
  {
    char msg[1024];                     /* Message to display */
    char answer[MAXTOTALUSERS];         /* Vote results */
    char inuse;                         /* in use */
  } vote[MAXVOTES];
};
