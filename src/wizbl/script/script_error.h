#ifndef WIZBLCOIN_SCRIPT_SCRIPT_ERROR_H
#define WIZBLCOIN_SCRIPT_SCRIPT_ERROR_H

typedef enum ScriptError_t
{
    SCRIPT_ERROR_OK = 0,
    SCRIPT_ERROR_UNKNOWN_ERROR,
    SCRIPT_ERROR_EVAL_FALSE,
    SCRIPT_ERROR_OPERATION_RETURN,

    SCRIPT_ERROR_SCRIPT_SIZE,
    SCRIPT_ERROR_PUSH_SIZE,
    SCRIPT_ERROR_OPERATION_COUNT,
    SCRIPT_ERROR_STACK_SIZE,
    SCRIPT_ERROR_SIG_COUNT,
    SCRIPT_ERROR_PUBKEY_COUNT,

    SCRIPT_ERROR_VERIFY,
    SCRIPT_ERROR_EQUALVERIFY,
    SCRIPT_ERROR_CHECKMULTISIGVERIFY,
    SCRIPT_ERROR_CHECKSIGVERIFY,
    SCRIPT_ERROR_NUMEQUALVERIFY,

    SCRIPT_ERROR_BAD_OPCODE,
    SCRIPT_ERROR_DISABLED_OPCODE,
    SCRIPT_ERROR_INVALID_STACK_OPERATION,
    SCRIPT_ERROR_INVALID_ALTSTACK_OPERATION,
    SCRIPT_ERROR_UNBALANCED_CONDITIONAL,

    SCRIPT_ERROR_NEGATIVE_LOCKTIME,
    SCRIPT_ERROR_UNSATISFIED_LOCKTIME,

    SCRIPT_ERROR_SIG_HASHTYPE,
    SCRIPT_ERROR_SIG_DER,
    SCRIPT_ERROR_MINIMALDATA,
    SCRIPT_ERROR_SIG_PUSHONLY,
    SCRIPT_ERROR_SIG_HIGH_S,
    SCRIPT_ERROR_SIG_NULLDUMMY,
    SCRIPT_ERROR_PUBKEYTYPE,
    SCRIPT_ERROR_CLEANSTACK,
    SCRIPT_ERROR_MINIMALIF,
    SCRIPT_ERROR_SIG_NULLFAIL,

    SCRIPT_ERROR_DISCOURAGE_UPGRADABLE_NOPS,
    SCRIPT_ERROR_DISCOURAGE_UPGRADABLE_WITNESS_PROGRAM,

    SCRIPT_ERROR_WITNESS_PROGRAM_WRONG_LENGTH,
    SCRIPT_ERROR_WITNESS_PROGRAM_WITNESS_EMPTY,
    SCRIPT_ERROR_WITNESS_PROGRAM_MISMATCH,
    SCRIPT_ERROR_WITNESS_MALLEATED,
    SCRIPT_ERROR_WITNESS_MALLEATED_P2SH,
    SCRIPT_ERROR_WITNESS_UNEXPECTED,
    SCRIPT_ERROR_WITNESS_PUBKEYTYPE,

    SCRIPT_ERROR_MUST_USE_FORKID,

    SCRIPT_ERROR_ERROR_COUNT
} ScriptError;

#define SCRIPT_ERROR_LAST SCRIPT_ERROR_ERROR_COUNT

const char* ScriptErrorString(const ScriptError error);

#endif
                                                                                                                                                                                                                             