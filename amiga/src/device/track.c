#include <proto/exec.h>

#include <exec/memory.h>
#include <exec/lists.h>
#include <exec/nodes.h>

#include <string.h>

#include "global.h"
#include "debug.h"

PUBLIC BOOL addtracktype(BASEPTR, ULONG type);
PUBLIC BOOL remtracktype(BASEPTR, ULONG type);
PUBLIC VOID dotracktype(BASEPTR, ULONG type, ULONG ps, ULONG pr, ULONG bs, ULONG br, ULONG pd);
PUBLIC BOOL gettrackrec(BASEPTR, ULONG type, struct Sana2PacketTypeStats *info);
PUBLIC VOID freetracktypes(BASEPTR);
PRIVATE struct TrackRec *findtracktype(BASEPTR, ULONG type);

PRIVATE INLINE struct TrackRec *findtracktype(BASEPTR, ULONG type)
{
  struct TrackRec *tr;

  for (tr = (struct TrackRec *)pb->pb_TrackList.lh_Head; tr->tr_Link.mln_Succ;
       tr = (struct TrackRec *)tr->tr_Link.mln_Succ)
  {
    if (tr->tr_PacketType == type)
      return (tr);
  }

  return (NULL);
}

PUBLIC BOOL addtracktype(BASEPTR, ULONG type)
{
  struct TrackRec *tr;
  BOOL rv = FALSE;

  ObtainSemaphore(&pb->pb_TrackListSem);
  if (!(tr = findtracktype(pb, type)))
  {
    if ((tr = AllocVec(sizeof(*tr), MEMF_CLEAR)) != NULL)
    {
      tr->tr_Count = 1;
      tr->tr_PacketType = type;
      AddTail((struct List *)&pb->pb_TrackList, (struct Node *)tr);
      rv = TRUE;
    }
  }
  else
  {
    ++tr->tr_Count;
    rv = TRUE;
  }
  ReleaseSemaphore(&pb->pb_TrackListSem);

  return rv;
}

PUBLIC BOOL remtracktype(BASEPTR, ULONG type)
{
  struct TrackRec *tr;
  BOOL rv = FALSE;

  ObtainSemaphore(&pb->pb_TrackListSem);
  if ((tr = findtracktype(pb, type)) != NULL)
  {
    if (!(--tr->tr_Count))
    {
      Remove((struct Node *)tr);
      FreeVec(tr);
    }
    rv = TRUE;
  }
  ReleaseSemaphore(&pb->pb_TrackListSem);

  return rv;
}

PUBLIC VOID dotracktype(BASEPTR, ULONG type, ULONG ps, ULONG pr, ULONG bs, ULONG br, ULONG pd)
{
  struct TrackRec *tr;

  ObtainSemaphore(&pb->pb_TrackListSem);
  if ((tr = findtracktype(pb, type)) != NULL)
  {
    tr->tr_Sana2PacketTypeStats.PacketsSent += ps;
    tr->tr_Sana2PacketTypeStats.PacketsReceived += pr;
    tr->tr_Sana2PacketTypeStats.BytesSent += bs;
    tr->tr_Sana2PacketTypeStats.BytesReceived += br;
    tr->tr_Sana2PacketTypeStats.PacketsDropped += pd;
  }
  ReleaseSemaphore(&pb->pb_TrackListSem);
}

PUBLIC BOOL gettrackrec(BASEPTR, ULONG type, struct Sana2PacketTypeStats *info)
{
  struct TrackRec *tr;
  BOOL rv = FALSE;

  ObtainSemaphoreShared(&pb->pb_TrackListSem);
  if ((tr = findtracktype(pb, type)) != NULL)
  {
    *info = tr->tr_Sana2PacketTypeStats;
    rv = TRUE;
  }
  ReleaseSemaphore(&pb->pb_TrackListSem);

  return rv;
}

PUBLIC VOID freetracktypes(BASEPTR)
{
  struct Node *tr;

  ObtainSemaphore(&pb->pb_TrackListSem);
  while (tr = RemHead((struct List *)&pb->pb_TrackList))
    FreeVec(tr);
  ReleaseSemaphore(&pb->pb_TrackListSem);
}
