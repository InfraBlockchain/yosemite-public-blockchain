/**
 *  @file
 *  @copyright defined in yosemite/LICENSE
 */
#pragma once
#include <eosio/chain/exceptions.hpp>

namespace yosemite { namespace chain {

    using namespace eosio::chain;

//FC_DECLARE_DERIVED_EXCEPTION( transaction_exception,             chain_exception,
//                              3040000, "Transaction exception" )

    FC_DECLARE_DERIVED_EXCEPTION( invalid_trx_vote_target_account, transaction_exception,
                                  3041001, "Invalid Transaction-Vote Target Account Data In Tx-Ext" )

    FC_DECLARE_DERIVED_EXCEPTION( invalid_trx_fee_payer_account, transaction_exception,
                                  3041002, "Invalid Transaction Fee Payer Account Data In Tx-Ext" )

  FC_DECLARE_DERIVED_EXCEPTION( yosemite_standard_token_exception, chain_exception,
                                3711000, "YOSEMITE standard token exception" )

    FC_DECLARE_DERIVED_EXCEPTION( token_meta_validate_exception, yosemite_standard_token_exception,
                                  3711001, "Invalid token meta info" )

    FC_DECLARE_DERIVED_EXCEPTION( token_action_validate_exception, yosemite_standard_token_exception,
                                  3711002, "Invalid token action data" )

    FC_DECLARE_DERIVED_EXCEPTION( token_not_yet_created_exception, yosemite_standard_token_exception,
                                  3711003, "No token metadata, token is not yet created" )

    FC_DECLARE_DERIVED_EXCEPTION( no_token_target_account_exception, yosemite_standard_token_exception,
                                  3711004, "token target account does not exist" )

    FC_DECLARE_DERIVED_EXCEPTION( token_symbol_mismatch_exception, yosemite_standard_token_exception,
                                  3711005, "Token symbol mismatch" )

    FC_DECLARE_DERIVED_EXCEPTION( invalid_token_balance_update_access_exception, yosemite_standard_token_exception,
                                  3711006, "Invalid token balance update access" )

    FC_DECLARE_DERIVED_EXCEPTION( token_balance_overflow_exception, yosemite_standard_token_exception,
                                  3711007, "Token balance overflow" )

    FC_DECLARE_DERIVED_EXCEPTION( token_balance_underflow_exception, yosemite_standard_token_exception,
                                  3711008, "Token balance underflow" )

    FC_DECLARE_DERIVED_EXCEPTION( insufficient_token_balance_exception, yosemite_standard_token_exception,
                                  3711009, "Insufficient token balance" )

  FC_DECLARE_DERIVED_EXCEPTION( yosemite_transaction_fee_exception, chain_exception,
                                3712000, "YOSEMITE transaction fee exception" )

//    FC_DECLARE_DERIVED_EXCEPTION( invalid_tx_fee_setup_exception, yosemite_transaction_fee_exception,
//                                  3712001, "Invalid transaction fee setup" )


}} // namespace yosemite::chain
