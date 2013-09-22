/*F*/ /* includes */
#ifndef CLIB_EXEC_PROTOS_H
#include <clib/exec_protos.h>
#include <pragmas/exec_sysbase_pragmas.h>
#endif

#ifndef EXEC_MEMORY_H
#include <exec/memory.h>
#endif
#ifndef EXEC_LISTS_H
#include <exec/lists.h>
#endif
#ifndef EXEC_NODES_H
#include <exec/nodes.h>
#endif

#ifndef _STRING_H
#include <string.h>
#endif

#ifndef __GLOBAL_H
#include "global.h"
#endif

#ifndef __DEBUG_H
#include "debug.h"
#endif

/*E*/
/*F*/ /* exports */
PUBLIC BOOL addtracktype(BASEPTR, ULONG type);
PUBLIC BOOL remtracktype(BASEPTR, ULONG type);
PUBLIC VOID dotracktype(BASEPTR, ULONG type, ULONG ps, ULONG pr, ULONG bs, ULONG br, ULONG pd);
PUBLIC BOOL gettrackrec(BASEPTR, ULONG type, APTR info);
PUBLIC VOID freetracktypes(BASEPTR);
/*E*/
/*F*/ /* private */
PRIVATE struct TrackRec *findtracktype(BASEPTR, ULONG type);
/*E*/

/*F*/ PRIVATE INLINE struct TrackRec *findtracktype(BASEPTR, ULONG type)
{
   struct TrackRec * tr;

   for (tr = (struct TrackRec *) pb->pb_TrackList.lh_Head; tr->tr_Link.mln_Succ;
                                                      tr = (struct TrackRec *) tr->tr_Link.mln_Succ)
   {
      if( tr->tr_PacketType == type )
         return( tr );
   }

   return( NULL );
}
/*E*/
/*F*/ PUBLIC BOOL addtracktype(BASEPTR, ULONG type)
{
   struct TrackRec *tr;
   BOOL rv = FALSE;

   ObtainSemaphore(&pb->pb_TrackListSem);
   if (!(tr = findtracktype(pb, type)))
   {
      if (tr = AllocVec(sizeof(*tr), MEMF_CLEAR))
      {
         tr->tr_Count = 1;
         tr->tr_PacketType = type;
         AddTail((struct List*)&pb->pb_TrackList, (struct Node *)tr);
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
/*E*/
/*F*/ PUBLIC BOOL remtracktype(BASEPTR, ULONG type)
{
   struct TrackRec *tr;
   BOOL rv = FALSE;

   ObtainSemaphore( &pb->pb_TrackListSem );
   if (tr = findtracktype(pb, type))
   {
      if (!(--tr->tr_Count))
      {
         Remove((struct Node *)tr);
         FreeVec(tr);
      }
      rv = TRUE;
   }
   ReleaseSemaphore( &pb->pb_TrackListSem );

   return rv;
}
/*E*/
/*F*/ PUBLIC VOID dotracktype(BASEPTR, ULONG type, ULONG ps, ULONG pr, ULONG bs, ULONG br, ULONG pd)
{
   struct TrackRec * tr;

   ObtainSemaphore(&pb->pb_TrackListSem);
   if (tr = findtracktype(pb, type))
   {
      tr->tr_Sana2PacketTypeStats.PacketsSent += ps;
      tr->tr_Sana2PacketTypeStats.PacketsReceived += pr;
      tr->tr_Sana2PacketTypeStats.BytesSent += bs;
      tr->tr_Sana2PacketTypeStats.BytesReceived += br;
      tr->tr_Sana2PacketTypeStats.PacketsDropped += pd;
   }
   ReleaseSemaphore(&pb->pb_TrackListSem);
}
/*E*/
/*F*/ PUBLIC BOOL gettrackrec(BASEPTR, ULONG type, struct Sana2PacketTypeStats *info)
{
   struct TrackRec * tr;
   BOOL rv = FALSE;

   ObtainSemaphoreShared( &pb->pb_TrackListSem );
   if (tr = findtracktype(pb, type))
   {
      *info = tr->tr_Sana2PacketTypeStats;
      rv = TRUE;
   }
   ReleaseSemaphore( &pb->pb_TrackListSem );

   return rv;
}
/*E*/
/*F*/ PUBLIC VOID freetracktypes(BASEPTR)
{
   struct Node *tr;

   ObtainSemaphore(&pb->pb_TrackListSem);
   while(tr = RemHead((struct List*)&pb->pb_TrackList))
      FreeVec(tr);
   ReleaseSemaphore(&pb->pb_TrackListSem);
}
/*E*/

