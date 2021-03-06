/**
 *  @file
 *  @copyright defined in yosemite/LICENSE
 */
#pragma once

#include <yosemitelib/yx_asset.hpp>
#include <yosemitelib/system_accounts.hpp>
#include <yosemitelib/identity.hpp>
#include <eosiolib/eosio.hpp>
#include <string>

namespace yosemite { namespace non_native_token {

    using namespace eosio;
    using std::string;

    /* Option flags for can_set_options */
    #define TOKEN_CAN_SET_OPTIONS_NONE                      0b0000000000000000
    #define TOKEN_CAN_SET_OPTIONS_FREEZE_TOKEN_TRANSFER     0b0000000000000001
    #define TOKEN_CAN_SET_OPTIONS_FREEZE_ACCOUNT            0b0000000000000010
    #define TOKEN_CAN_SET_OPTIONS_SET_KYC_RULE              0b0000000000000100
    #define TOKEN_CAN_SET_OPTIONS_SET_ACCOUNT_TYPE_RULE     0b0000000000001000
    #define TOKEN_CAN_SET_OPTIONS_MAX                       0b0000000000001111

    /* Option flags for account management */
    #define TOKEN_ACCOUNT_OPTIONS_NONE                      0b0000000000000000
    #define TOKEN_ACCOUNT_OPTIONS_FREEZE_ACCOUNT            0b0000000000000001

    /* Option flags for token management */
    #define TOKEN_OPTIONS_NONE                              0b0000000000000000
    #define TOKEN_OPTIONS_FREEZE_TOKEN_TRANSFER             0b0000000000000001
    #define TOKEN_OPTIONS_MAX                               0b0000000000000001

    /* Rules for token */
    enum token_rule_type {
        TOKEN_KYC_RULE_TYPE_TRANSFER_SEND    = 0,
        TOKEN_KYC_RULE_TYPE_TRANSFER_RECEIVE = 1,

        TOKEN_KYC_RULE_TYPE_MAX // MUST NOT EXCEED MORE THAN 255
    };
    using token_rule_t = uint8_t;

    /* scope = token symbol */
    struct token_stats {
        yx_asset supply;
        uint16_t can_set_options = TOKEN_CAN_SET_OPTIONS_NONE; // can set only at token creation time
        uint16_t options = TOKEN_OPTIONS_NONE;
        std::vector<token_rule_t> kyc_rules;
        std::vector<identity::identity_kyc_t> kyc_rule_flags; // from yosemitelib/identity.hpp

        uint64_t primary_key() const { return supply.issuer; }
    };

    typedef eosio::multi_index<N(tstats), token_stats> stats;

    /* scope = owner */
    struct balance_holder {
        uint64_t id = 0;
        yx_asset token;
        uint16_t options = TOKEN_ACCOUNT_OPTIONS_NONE;

        uint64_t primary_key() const { return id; }
        uint128_t by_yx_symbol() const { return token.get_yx_symbol().to_uint128(); }
    };

    typedef eosio::multi_index<N(taccounts), balance_holder,
            indexed_by<N(yxsymbol), const_mem_fun<balance_holder, uint128_t, &balance_holder::by_yx_symbol> >
    > accounts;

    class token : public contract {
    public:
        explicit token(account_name self) : contract(self) {
        }

        virtual ~token() {}

        void create(const yx_symbol &ysymbol, uint16_t can_set_options);
        virtual void transfer(account_name from, account_name to, const yx_asset &token, const string &memo) = 0;
        void setkycrule(const yx_symbol &ysymbol, token_rule_t type, identity::identity_kyc_t kyc);
        void setoptions(const yx_symbol &ysymbol, uint16_t options, bool reset);
        void freezeacc(const yx_symbol &ysymbol, const vector<account_name> &accs, bool freeze);

    protected:
        virtual void inner_check_create_parameters(const yx_symbol &ysymbol, uint16_t can_set_options) = 0;

        void check_issue_parameters(const account_name &to, const yx_asset &token, const string &memo);
        void check_transfer_parameters(account_name from, account_name to, const yx_asset &token, const string &memo);
        void check_transfer_rules(account_name from, account_name to, const yx_asset &token);

        void charge_fee(const account_name &payer, uint64_t operation);

        void add_token_balance(const account_name &owner, const yx_asset &token);
        void sub_token_balance(const account_name &owner, const yx_asset &token);
        bool check_identity_auth_for_transfer(account_name account, const token_rule_type &kycrule_type,
                                              const token_stats &tstats);
    };

    bool does_token_exist(const uint64_t contract, const yx_symbol &ysymbol) {
        eosio_assert(static_cast<uint32_t>(!ysymbol.is_native()), "native token is not allowed for this API");

        stats stats_table(contract, ysymbol.value);
        const auto &tstats = stats_table.find(ysymbol.issuer);
        return tstats != stats_table.end();
    }
}}
