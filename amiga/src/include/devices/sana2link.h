/* sana2link.h

   an experimental extension to SANA-II that allows to deal with
   the link status of an ethernet controller.
*/

#ifndef SANA2_SANA2LINK_H
#define SANA2_SANA2LINK_H 1

/*
   NAME
        S2_LINK_STATUS -- Obtain the current or wait for an update of
           the link layer connection status.

   FUNCTION
        The link status gives information on the ability to of a network device
        to actually send and receive frames on the link layer. If transmission
        is possible then the link is considered to be up. Otherwise its called
        down and may return to up when the external conditions allow
        transmission again. Additionally, if the link is broken and may never
        return to up state on its own then the link status can represent an
        error.

        On a wired network the link is up if the ethernet cable is plugged in.
        On a wireless network the link if assumed to be up if a client is
        registered at the access point and can transfer user data. This state
        is reached after station discovery and authentication. In such a
        network errors might prevent the link of getting up and thus the link
        status is an error: the SSID might be unknown or authentication data is
        wrong. These errors can only be resolved by changing device
        parameters. The link will never reach up state without such changes.

        With the S2_LINK_STATUS command a device driver is able to report the
        current status of the link. You can either poll the current status and
        get the link status immedetiately or wait for a status change. In the
        latter case the command will be on hold until the change is detected.

        The link status is reported with the following structure:

            struct Sana2LinkStatus
            {
               ULONG            s2ls_Size;
               WORD             s2ls_QueryMode;
               BYTE             s2ls_PreviousStatus;
               BYTE             s2ls_CurrentStatus;
               struct EClockVal s2ls_TimeStamp;
            };

        The s2ls_Size contains the size of the structure and allows to extend
        the structure in the future. Its set as sizeof(struct Sana2LinkStatus).

        The s2ls_QueryMode allows to select whether to return the current link
        status immediately (S2LS_QUERYMODE_IMMEDIATE) or if the command waits
        for a link status change (S2LS_QUERYMODE_ONCHANGE). An "on change"
        query might wait for a long time. However, you can abort it any time
        with AbortIO() if its not needed anymore.

        The s2ls_CurrentStatus holds the current link status. The
        s2ls_PreviousStatus is used by the driver to back up the current link
        status when the command is processed and allows the driver to track
        changes. It will be updated for both immediate and on change
        operation.

        Both status values have to be initialized with S2LINKSTATUS_UNKNOWN
        before calling the command for the first time. If the command returns
        you can regrigger a new call and re-use the current link status
        structure for the next on change query: It will wait until a new link
        status is reported.

        The s2ls_TimeStamp is set by the driver to the current E clock value
        when the link status was set. If you have multiple link status request
        in transit then this time stamp allows to order the results and
        determine the latest one.

        The link status struct has to be allocated by the driver user and passed
        via the ios2_StatData field of the SANA-II IO request. While the
        command is in execution the struct must neither be changed nor be
        free()ed.

        The following values for link status are currently defined:

            S2LINKSTATUS_UNKNOWN
            S2LINKSTATUS_DOWN
            S2LINKSTATUS_UP
            S2LINKSTATUS_FAILED
            S2LINKSTATUS_ERROR_NO_NET
            S2LINKSTATUS_ERROR_BAD_AUTH

        The unknown status is used when the actual status cannot be delivered.
        If the device is OFFLINE then the status is always unknown. Also
        drivers that cannot deduce the link status may always return this
        value. If a driver needs some time to actually bring up its media
        access section then it may also report unknown in this phase.

        The status is down if the link is currently not available. If may return
        to up at any time, e.g. when the network cable is plugged in.

        If the status is up then the link is operational and can send and
        receive frames.

        If a persistent error is detected that prevents the device to reach up
        state then the driver may respond with an error link status. Failed
        status is reported if an unspecified reason caused the failure.
        "No net" and "Bad Auth" are used in wireless networks to denote problems
        finding the SSID or problems with authentication. This error status
        can be used to give detailed hints to the user on how to repair the
        setup.

        If the status is not up then a driver may use the error code
        S2WERR_LINK_DOWN as a result for aborted transfer commands including
        CMD_READ, CMD_WRITE, or S2_BROADCAST.

   IO REQUEST
        ios2_Command          - S2_LINK_STATUS
        ios2_StatData         - pointer to struct Sana2LinkStatus

   RESULTS
        ios2_Req.io_Error     - Zero if successful; non-zero otherwise
        ios2_WireError        - More specific error number

   NOTES
        While the naming of the S2_ONLINE/S2_OFFLINE commands at first glance
        seem to already fulfill the task of S2_LINK_STATUS this is not the
        case: A driver is ONLINE if it has access to and control of the
        actual network hardware. But it does not reflect the state of the
        medium access contoller and the associated link status. Here the
        link status delivered by this command fills this information gap.
*/

/* new command */
#define S2_LINK_STATUS 0xC008

struct Sana2LinkStatus
{
   ULONG            s2ls_Size;
   WORD             s2ls_QueryMode;
   BYTE             s2ls_PreviousStatus;
   BYTE             s2ls_CurrentStatus;
   struct EClockVal s2ls_TimeStamp;
};

/* s2ls_QueryMode */
#define S2LS_QUERYMODE_IMMEDIATE 0
#define S2LS_QUERYMODE_ONCHANGE  1

/* s2ls_LinkStatus */
#define S2LINKSTATUS_UNKNOWN -1
#define S2LINKSTATUS_DOWN 0
#define S2LINKSTATUS_UP 1
#define S2LINKSTATUS_FAILED 2
#define S2LINKSTATUS_ERROR_NO_NET 3
#define S2LINKSTATUS_ERROR_BAD_AUTH 4

/* new wire error */
#define S2WERR_LINK_DOWN 24 /* no carrier */

#endif
