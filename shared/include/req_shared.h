#ifndef REQ_SHARED_H
#define REQ_SHARED_H

// --- param requests ---
// GET_NUM: in:- out:(u08)total_params
#define REQ_PARAM_GET_NUM   0x00
// FIND_TAG: in:(u32)tag  out_extra:param_index
#define REQ_PARAM_FIND_TAG  0x01
// GET_DEF: in_extra:param_index out:param_def
#define REQ_PARAM_GET_DEF   0x04
// GET_VAL: in_extra:param_index out:param_data
#define REQ_PARAM_GET_VAL   0x05
// SET_VAL: in_extra:param_index in:param_data
#define REQ_PARAM_SET_VAL   0x06

// --- prefs requests ---
// PREFS_RESET: in:- out:-
#define REQ_PREFS_RESET     0x10
// PREFS_LOAD: in:- out:-
#define REQ_PREFS_LOAD      0x11
// PREFS_SAVE: in:- out:-
#define REQ_PREFS_SAVE      0x12

// --- MAC requests ---
// MAC_GET_DEF: out:mac
#define REQ_MAC_GET_DEF     0x20
// MAC_GET_CUR: out:mac
#define REQ_MAC_GET_CUR     0x21
// MAC_SET_CUR: in:mac
#define REQ_MAC_SET_CUR     0x22


// error codes
#define REQ_OK                    0
#define REQ_ERROR_SANA2           1
#define REQ_ERROR_IN_TOO_LARGE    2
#define REQ_ERROR_OUT_TOO_LARGE   3
#define REQ_ERROR_LOADING_DATA    4
#define REQ_ERROR_SAVING_DATA     5
#define REQ_ERROR_UNKNOWN_COMMAND 6
#define REQ_ERROR_WRONG_IN_SIZE   7
#define REQ_ERROR_UNKNOWN         0xff

#endif
