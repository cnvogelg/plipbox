#ifndef REQ_SHARED_H
#define REQ_SHARED_H

// --- param requests ---
// GET_NUM: in:- out:(u08)total_params
#define REQ_PARAM_GET_NUM         0x00
// FIND_TAG: in:(u32)tag  out_extra:param_index
#define REQ_PARAM_FIND_TAG        0x01
// GET_DEF: in_extra:param_index out:param_def
#define REQ_PARAM_GET_DEF         0x04
// GET_VAL: in_extra:param_index out:param_data
#define REQ_PARAM_GET_VAL         0x05
// SET_VAL: in_extra:param_index in:param_data
#define REQ_PARAM_SET_VAL         0x06
// RESET: in:- out:-
#define REQ_PARAM_RESET           0x07
// LOAD: in:- out:-
#define REQ_PARAM_LOAD            0x08
// SAVE: in:- out:-
#define REQ_PARAM_SAVE            0x09
// GET_DEF_MAX: out:mac
#define REQ_PARAM_GET_DEF_MAC     0x0a
// GET_CUR_MAC: out:mac
#define REQ_PARAM_GET_CUR_MAC     0x0b
// MAC_SET_CUR: in:mac
#define REQ_PARAM_SET_CUR_MAC     0x0c


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
